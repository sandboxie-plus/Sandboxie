#include "stdafx.h"
#include "CodeEdit.h"


#define TAB_SPACES "   "

// Anonymous namespace helpers for fuzzy matching
namespace {
	constexpr int POS_UNKNOWN = std::numeric_limits<int>::max();

	// Compute Damerau-Levenshtein distance (optimal string alignment allowing transpositions)
	static int DamerauLevenshtein(const QString& aIn, const QString& bIn)
	{
		QString a = aIn.toLower();
		QString b = bIn.toLower();

		int m = a.length();
		int n = b.length();
		if (m == 0) return n;
		if (n == 0) return m;

		const int INF = m + n;
		std::vector<std::vector<int>> H(m + 2, std::vector<int>(n + 2, 0));
		H[0][0] = INF;
		for (int i = 0; i <= m; ++i) {
			H[i + 1][1] = i;
			H[i + 1][0] = INF;
		}
		for (int j = 0; j <= n; ++j) {
			H[1][j + 1] = j;
			H[0][j + 1] = INF;
		}

		std::unordered_map<quint32, int> DA; // map from QChar.unicode() to last row seen
		DA.reserve(m + n);
		for (int i = 0; i < m; ++i) DA[static_cast<quint32>(a[i].unicode())] = 0;
		for (int j = 0; j < n; ++j) DA[static_cast<quint32>(b[j].unicode())] = 0;

		for (int i = 1; i <= m; ++i) {
			int db = 0;
			for (int j = 1; j <= n; ++j) {
				int i1 = DA[static_cast<quint32>(b[j - 1].unicode())];
				int j1 = db;
				int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
				if (cost == 0) db = j;
				int substitution = H[i][j] + cost;
				int insertion = H[i + 1][j] + 1;
				int deletion = H[i][j + 1] + 1;
				int transposition = H[i1][j1] + (i - i1 - 1) + 1 + (j - j1 - 1);
				H[i + 1][j + 1] = std::min({ substitution, insertion, deletion, transposition });
			}
			DA[static_cast<quint32>(a[i - 1].unicode())] = i;
		}

		return H[m + 1][n + 1];
	}

	// Check if 'small' is a subsequence of 'large' (case-insensitive)
	static bool IsSubsequenceCI(const QString& smallIn, const QString& largeIn)
	{
		QString small = smallIn.toLower();
		QString large = largeIn.toLower();
		int si = 0, li = 0;
		while (si < small.length() && li < large.length()) {
			if (small[si] == large[li]) {
				++si;
			}
			++li;
		}
		return si == small.length();
	}

	// Compare prefix to substrings of candidate and return minimal Damerau-Levenshtein distance.
	// This improves matching when candidate is longer than the short prefix.
	static int MinDistanceToCandidateSubstrings(const QString& pref, const QString& cand, int maxLenDelta = 2)
	{
		QString p = pref.toLower();
		QString c = cand.toLower();
		int pLen = p.length();
		int cLen = c.length();
		if (pLen == 0) return 0;
		int minD = POS_UNKNOWN;

		// Try substrings of candidate with lengths around prefix length to handle missing/extra letters.
		int startLen = std::max(1, pLen - maxLenDelta);
		int endLen = std::min(cLen, pLen + maxLenDelta);

		for (int len = startLen; len <= endLen; ++len) {
			for (int i = 0; i + len <= cLen; ++i) {
				QString sub = c.mid(i, len);
				int d = DamerauLevenshtein(p, sub);
				if (d < minD) minD = d;
				if (minD == 0) return 0; // best possible
			}
		}

		// fallback compare to whole candidate if nothing better found
		int whole = DamerauLevenshtein(p, c);
		minD = std::min(minD, whole);

		if (minD == POS_UNKNOWN) return pLen;
		return minD;
	}

	// Find earliest subsequence start index of `pref` inside `cand`.
	// Returns -1 if no subsequence exists.
	static int FindSubsequenceStart(const QString& pref, const QString& cand)
	{
		if (pref.isEmpty() || cand.isEmpty()) return -1;
		QString p = pref.toLower();
		QString c = cand.toLower();

		for (int start = 0; start < c.length(); ++start) {
			if (c[start] != p[0]) continue;
			int pi = 0;
			int ci = start;
			while (pi < p.length() && ci < c.length()) {
				if (p[pi] == c[ci]) {
					++pi;
				}
				++ci;
			}
			if (pi == p.length())
				return start; // subsequence starting at 'start' works
		}
		return -1;
	}

	// Small thread-safe cache used by BuildFuzzyMatches (LRU by simple list)
	static const int FUZZY_CACHE_MAX_ENTRIES = 64;
	static QHash<QString, QStringList> s_fuzzyCache;
	static QList<QString> s_cacheOrder;
	static QMutex s_fuzzyCacheMutex;

	// Try to get value and move entry to MRU; returns true on hit and fills out.
	static bool FuzzyCacheGet(const QString& key, QStringList& out)
	{
		QMutexLocker lk(&s_fuzzyCacheMutex);
		auto it = s_fuzzyCache.find(key);
		if (it == s_fuzzyCache.end())
			return false;

		// Move key to MRU (optimized: we know key exists from the hash)
		int idx = s_cacheOrder.indexOf(key);
		if (idx >= 0) {
			s_cacheOrder.removeAt(idx);
			s_cacheOrder.append(key);
		}

		out = it.value();
		return true;
	}

	// Insert or update value and evict LRU if over capacity
	static void FuzzyCacheInsert(const QString& key, const QStringList& value)
	{
		QMutexLocker lk(&s_fuzzyCacheMutex);

		auto it = s_fuzzyCache.find(key);
		if (it != s_fuzzyCache.end()) {
			// Update existing in-place and move to MRU
			it.value() = value;
			int idx = s_cacheOrder.indexOf(key);
			if (idx >= 0) {
				s_cacheOrder.removeAt(idx);
				s_cacheOrder.append(key);
			}
			//qDebug() << "[FuzzyCache] UPDATE key=" << key << "cacheSize=" << s_fuzzyCache.size();
		}
		else {
			// Insert new entry
			s_fuzzyCache.insert(key, value);
			s_cacheOrder.append(key);

			// Evict if needed
			if (s_cacheOrder.size() > FUZZY_CACHE_MAX_ENTRIES) {
				QString old = s_cacheOrder.takeFirst();
				s_fuzzyCache.remove(old);
				//qDebug() << "[FuzzyCache] EVICT key=" << old << "newCacheSize=" << s_fuzzyCache.size();
			}
			//qDebug() << "[FuzzyCache] INSERT key=" << key << "cacheSize=" << s_fuzzyCache.size() << "storedEntries=" << value.size();
		}
	}

	// Build a fuzzy-sorted candidate list from supplied candidates given prefix
	static QStringList BuildFuzzyMatches(const QStringList& candidates, const QString& prefix)
	{
		QString pref = prefix.toLower();

		// Compute a cheap fingerprint for the candidates set (order-sensitive).
		auto CandidatesFingerprint = [](const QStringList& list) -> quint64 {
			const quint64 FNV_OFFSET = 14695981039346656037ULL;
			const quint64 FNV_PRIME = 1099511628211ULL;
			quint64 h = FNV_OFFSET;

			for (const QString& s : list) {
				quint64 v = static_cast<quint64>(qHash(s));
				h ^= v;
				h *= FNV_PRIME;
			}

			// Mix length in
			h ^= static_cast<quint64>(list.size());
			h *= FNV_PRIME;

			return h;
			};

		quint64 fingerprint = CandidatesFingerprint(candidates);
		const QString cacheKey = QString::number(fingerprint) + QLatin1Char('|') + pref;
		QStringList cached;
		if (FuzzyCacheGet(cacheKey, cached)) {
		//	// Debug: show cache hit and small sample of cached value
		//	const int sampleLimit = 20;
		//	QString sample = cached.size() > sampleLimit ? (cached.mid(0, sampleLimit).join(", ") + "...") : cached.join(", ");
		//	{
		//		QMutexLocker lk(&s_fuzzyCacheMutex); // protect order read for consistent output
		//		qDebug() << "[FuzzyCache] HIT key=" << cacheKey << "cacheSize=" << s_fuzzyCache.size() << "orderSize=" << s_cacheOrder.size();
		//		qDebug() << "[FuzzyCache] keys(order) =" << s_cacheOrder;
		//	}
		//	qDebug() << "[FuzzyCache] valueSample =" << sample;
		return cached;
		}

		// Added hasSubstr flag so exact substring matches can be boosted
		// matchedLen is added to prefer candidates that match a larger portion of the prefix
		// tokenScore boosts token-boundary / token-equality matches (helps prefer 'Enabled' over 'NormalFilePathDisabled')
		// coverage prefers compact matches where matchedLen covers a larger fraction of the candidate.
		struct Entry { QString text; int dist; int pos; int len; bool hasSubstr; int matchedLen; int tokenScore; int coverage; };
		std::vector<Entry> matches;
		matches.reserve(candidates.size());

		// adaptive threshold: allow more errors for longer inputs.
		// Map prefix length minPrefix..maxPrefix -> maxDist 1..maxCap (linear).
		int pLen = pref.length();
		const int minPrefix = CCodeEdit::AUTO_COMPLETE_MIN_LENGTH;
		const int maxPrefix = CCodeEdit::GetMaxFuzzyPrefixLength();
		const int maxCap = 3; // maximum allowed distance for longest prefixes

		// helper: longest common substring length (O(m*n) DP but small inputs)
		auto LongestCommonSubstring = [](const QString& a, const QString& b) -> int {
			int m = a.length();
			int n = b.length();
			if (m == 0 || n == 0) return 0;
			std::vector<int> prev(n + 1, 0), cur(n + 1, 0);
			int best = 0;
			for (int i = 1; i <= m; ++i) {
				for (int j = 1; j <= n; ++j) {
					if (a[i - 1] == b[j - 1]) {
						cur[j] = prev[j - 1] + 1;
						if (cur[j] > best) best = cur[j];
					}
					else {
						cur[j] = 0;
					}
				}
				std::fill(prev.begin(), prev.end(), 0);
				prev.swap(cur);
			}
			return best;
			};

		// helper: token match score (tokens by upper-case transitions, '_' and '.')
		auto TokenMatchScore = [](const QString& cand, const QString& prefLower) -> int {
			// split into tokens
			std::vector<QString> tokens;
			int n = cand.length();
			int start = 0;
			for (int i = 0; i < n; ++i) {
				QChar c = cand[i];
				bool boundary = false;
				if (c == '_' || c == '.') boundary = true;
				else if (i > 0 && cand[i].isUpper() && cand[i - 1].isLower()) boundary = true;
				if (boundary) {
					if (i - start > 0) tokens.push_back(cand.mid(start, i - start));
					start = i;
				}
			}
			if (start < n) tokens.push_back(cand.mid(start));

			int best = 0;
			for (const QString& t : tokens) {
				QString tl = t.toLower();
				if (tl == prefLower) return 100;            // exact token match -> big boost
				if (tl.startsWith(prefLower)) best = std::max(best, 60); // token starts with prefix
				if (tl.endsWith(prefLower)) best = std::max(best, 50); // token ends with suffix
				if (tl.contains(prefLower)) best = std::max(best, 40);   // token contains prefix
			}

			// acronym match: take initials (e.g. NormalFilePathDisabled -> NFPD) and compare prefix
			QString initials;
			for (const QString& t : tokens) {
				if (!t.isEmpty())
					initials.append(t[0].toUpper());
			}
			if (!initials.isEmpty()) {
				if (initials.toLower().startsWith(prefLower)) best = std::max(best, 30);
				if (initials.toLower().endsWith(prefLower)) best = std::max(best, 20);
				if (initials.toLower().contains(prefLower)) best = std::max(best, 10);
			}

			return best;
			};

		auto computeCoverage = [](int matchedLen, int candLen) -> int {
			if (candLen <= 0) return 0;
			// scale by 1000 to keep integer precision
			return (matchedLen * 1000) / candLen;
			};

		// sort comparator
		// 1) distance (all 0 here),
		// 2) prefer hasSubstr (true),
		// 3) prefer tokenScore,
		// 4) prefer coverage (compactness of match),
		// 5) prefer larger matchedLen,
		// 6) earliest position,
		// 7) shorter candidate,
		// 8) alphabetical
		auto EntryComparator = [](const Entry& a, const Entry& b) -> bool {
			if (a.dist != b.dist) return a.dist < b.dist;
			if (a.hasSubstr != b.hasSubstr) return a.hasSubstr > b.hasSubstr;
			if (a.tokenScore != b.tokenScore) return a.tokenScore > b.tokenScore;
			if (a.coverage != b.coverage) return a.coverage > b.coverage;
			if (a.matchedLen != b.matchedLen) return a.matchedLen > b.matchedLen;
			if (a.pos != b.pos) return a.pos < b.pos;
			if (a.len != b.len) return a.len < b.len;
			return a.text.toLower() < b.text.toLower();
			};

		// If prefix is non-empty but shorter than configured (minPrefix), do NOT run fuzzy
		// distance matching — only accept strict contains matches. This avoids
		// single-char noise (e.g. "=") producing many fuzzy matches.
		if (!pref.isEmpty() && pLen < minPrefix) {
			for (const QString& cand : candidates) {
				// Use contains to match non-fuzzy behavior used elsewhere (Ctrl+Space, short prefixes)
				if (cand.toLower().contains(pref)) {
					int candLen = cand.length();
					int matchedLen = pLen;
					int tokenScore = TokenMatchScore(cand, pref);
					int coverage = computeCoverage(matchedLen, candLen);
					matches.push_back(Entry{ cand, 0, 0, candLen, true, matchedLen, tokenScore, coverage });
				}
			}

			// sort & return (use shared comparator)
			std::sort(matches.begin(), matches.end(), EntryComparator);
			QStringList out;
			out.reserve(matches.size());
			for (const auto& e : matches) out.append(e.text);

			// store in cache and return via helper
			FuzzyCacheInsert(cacheKey, out);
			return out;
		}

		int maxDist;
		if (pLen <= minPrefix) {
			maxDist = 1;
		}
		else if (pLen >= maxPrefix) {
			maxDist = maxCap;
		}
		else {
			// Linear interpolation between 1 and maxCap
			int span = std::max(1, maxPrefix - minPrefix);
			double scale = double(pLen - minPrefix) / double(span); // 0..1
			maxDist = 1 + int(std::round(scale * (maxCap - 1)));
		}

		for (const QString& cand : candidates) {
			QString candLower = cand.toLower();
			int candLen = candLower.length();

			// Fast accept: startsWith (case-insensitive)
			if (!pref.isEmpty() && candLower.startsWith(pref)) {
				int matchedLen = pLen;
				int tokenScore = TokenMatchScore(cand, pref);
				int coverage = computeCoverage(matchedLen, candLen);
				matches.push_back(Entry{ cand, 0, 0, candLen, true, matchedLen, tokenScore, coverage });
				continue;
			}

			// Exact substring match (anywhere) should be preferred
			if (!pref.isEmpty()) {
				int subPos = candLower.indexOf(pref);
				if (subPos >= 0) {
					// exact substring -> distance 0, mark hasSubstr true
					int matchedLen = pLen;
					int tokenScore = TokenMatchScore(cand, pref);
					int coverage = computeCoverage(matchedLen, candLen);
					matches.push_back(Entry{ cand, 0, subPos, candLen, true, matchedLen, tokenScore, coverage });
					continue;
				}
			}

			// If prefix is empty, include all (manual completion case)
			if (pref.isEmpty()) {
				matches.push_back(Entry{ cand, 0, 0, candLen, false, 0, 0, 0 });
				continue;
			}

			// If prefix is a subsequence of candidate, accept with low distance
			if (IsSubsequenceCI(pref, candLower)) {
				// Smaller pseudo-distance based on how sparse the match is
				int score = 1 + std::max(0, int(candLower.length() - pref.length()) / 4);
				score = std::min(score, maxDist);
				int pos = FindSubsequenceStart(pref, candLower);
				if (pos < 0) pos = POS_UNKNOWN / 2;
				int lcs = LongestCommonSubstring(pref, candLower);
				int matchedLen = std::max(1, pLen - score); // approximate matched prefix portion
				int tokenScore = TokenMatchScore(cand, pref);
				matchedLen = std::max(matchedLen, lcs); // boost matchedLen with actual LCS info
				int coverage = computeCoverage(matchedLen, candLen);
				matches.push_back(Entry{ cand, score, pos, candLen, false, matchedLen, tokenScore, coverage });
				continue;
			}

			// Compute minimal Damerau-Levenshtein distance against candidate substrings
			int d = MinDistanceToCandidateSubstrings(pref, candLower, 2);
			if (d <= maxDist) {
				// compute an approximate favorable position: try to find exact substring match of a short slice
				int pos = -1;
				int probeLen = std::min(3, pLen);
				if (probeLen > 0) {
					QString probe = pref.left(probeLen);
					pos = candLower.indexOf(probe);
				}
				if (pos < 0) pos = FindSubsequenceStart(pref, candLower); // fallback to subsequence start (may still be -1)
				if (pos < 0) pos = POS_UNKNOWN / 2;
				// matchedLen = how much of prefix is effectively matched (approx)
				int lcs = LongestCommonSubstring(pref, candLower);
				int matchedLen = std::max(0, pLen - d);
				matchedLen = std::max(matchedLen, lcs);
				int tokenScore = TokenMatchScore(cand, pref);
				int coverage = computeCoverage(matchedLen, candLen);
				matches.push_back(Entry{ cand, d, pos, candLen, false, matchedLen, tokenScore, coverage });
				continue;
			}

			// For very short prefixes, allow single-char edits
			if (pref.length() <= 2 && d <= 1) {
				int pos = FindSubsequenceStart(pref, candLower);
				if (pos < 0) pos = POS_UNKNOWN / 2;
				int lcs = LongestCommonSubstring(pref, candLower);
				int matchedLen = std::max(0, pLen - d);
				matchedLen = std::max(matchedLen, lcs);
				int tokenScore = TokenMatchScore(cand, pref);
				int coverage = computeCoverage(matchedLen, candLen);
				matches.push_back(Entry{ cand, d, pos, candLen, false, matchedLen, tokenScore, coverage });
				continue;
			}
		}

		std::sort(matches.begin(), matches.end(), EntryComparator);

		QStringList out;
		out.reserve(matches.size());
		for (const auto& e : matches) out.append(e.text);

		// store in cache and return via helper
		FuzzyCacheInsert(cacheKey, out);
		return out;
	}

} // namespace

CCodeEdit::AutoCompletionMode CCodeEdit::s_autoCompletionMode = AutoCompletionMode::FullAuto;
QMutex CCodeEdit::s_autoCompletionModeMutex;

bool CCodeEdit::s_fuzzyMatchingEnabled = false;
int CCodeEdit::s_minFuzzyPrefixLength = 4;
int CCodeEdit::s_maxFuzzyPrefixLength = 32;

CCodeEdit::CCodeEdit(QSyntaxHighlighter* pHighlighter, QWidget* pParent)
	: QWidget(pParent), m_pCompleter(nullptr), m_lastWordStart(-1), m_lastWordEnd(-1),
	m_caseCorrectionInProgress(false), m_completionInsertInProgress(false),
	m_suppressNextAutoCompletion(false)
{
	m_pMainLayout = new QGridLayout(this);
	m_pMainLayout->setContentsMargins(0,0,0,0);
	setLayout(m_pMainLayout);

	m_pSourceCode = new QTextEdit();
	QFont Font = m_pSourceCode->font();
	Font.setFamily("Courier New");
	Font.setPointSize(10);
	m_pSourceCode->setFont(Font);
	m_pSourceCode->setLineWrapMode(QTextEdit::NoWrap);
	if(pHighlighter)
		pHighlighter->setDocument(m_pSourceCode->document());
	//m_pSourceCode->setTabStopWidth (QFontMetrics(Font).width(TAB_SPACES));
	m_pMainLayout->addWidget(m_pSourceCode, 0, 0);

	connect(m_pSourceCode, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
	
	// Connect to textChanged for real-time completion updates
	connect(m_pSourceCode, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));

	// Install event filter to handle autocompletion
	m_pSourceCode->installEventFilter(this);

	// Close popup when cursor moves right of '='
	connect(m_pSourceCode, &QTextEdit::cursorPositionChanged, this, &CCodeEdit::OnCursorPositionChanged);

	// hot keys
	m_pFind = new QAction(tr("Find"),this);
	m_pFind->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_F));
	connect(m_pFind, SIGNAL(triggered()), this, SLOT(OnFind()));
	m_pSourceCode->addAction(m_pFind);

	m_pFindNext = new QAction(tr("FindNext"),this);
	QList<QKeySequence> Finds;
	Finds << QKeySequence(Qt::Key_F3);
	Finds << QKeySequence(Qt::SHIFT | Qt::Key_F3) << QKeySequence(Qt::CTRL | Qt::Key_F3) << QKeySequence(Qt::ALT | Qt::Key_F3);
	Finds << QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F3) << QKeySequence(Qt::SHIFT | Qt::ALT | Qt::Key_F3) << QKeySequence(Qt::SHIFT | Qt::CTRL | Qt::Key_F3);
	Finds << QKeySequence(Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::Key_F3);
	m_pFindNext->setShortcuts(Finds);
	connect(m_pFindNext, SIGNAL(triggered()), this, SLOT(OnFindNext()));
	m_pSourceCode->addAction(m_pFindNext);

	m_pGoTo = new QAction(tr("GoTo"),this);
	m_pGoTo->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
	connect(m_pGoTo, SIGNAL(triggered()), this, SLOT(OnGoTo()));
	m_pSourceCode->addAction(m_pGoTo);

	// Case correction replacement
	m_pReplaceCorrection = new QAction("Replace with correction",this);
	m_pReplaceCorrection->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
	connect(m_pReplaceCorrection, SIGNAL(triggered()), this, SLOT(ReplaceLastCorrection()));
	m_pSourceCode->addAction(m_pReplaceCorrection);

	m_completionDebounceTimer = new QTimer(this);
	m_completionDebounceTimer->setSingleShot(true);
	connect(m_completionDebounceTimer, &QTimer::timeout, this, [this]() {
		if (!m_pCompleter)
			return;

		// If fuzzy matching is enabled use our fuzzy model for this prefix,
		// otherwise rely on QCompleter's built-in filtering behavior.
		if (GetFuzzyMatchingEnabled()) {
			QStringList fuzzy = ApplyFuzzyModelForPrefix(m_pendingPrefix);

			// Check if only one completion and it matches the prefix exactly
			if (fuzzy.size() == 1 && fuzzy[0].compare(m_pendingPrefix, Qt::CaseSensitive) == 0) {
				HidePopupSafely();
				return;
			}

			// Show popup if there are results
			if (!fuzzy.isEmpty()) {
				QRect rect = m_pSourceCode->cursorRect();
				rect.setWidth(m_pCompleter->popup()->sizeHintForColumn(0)
					+ m_pCompleter->popup()->verticalScrollBar()->sizeHint().width());
				m_pCompleter->complete(rect);
			} else {
				HidePopupSafely();
			}
		} else {
			// default behavior
			m_pCompleter->setCompletionPrefix(m_pendingPrefix);

			// Check if only one completion and it matches the prefix
			if (m_pCompleter->completionCount() == 1) {
				QString onlyCompletion = m_pCompleter->currentCompletion();
				if (onlyCompletion.compare(m_pendingPrefix, Qt::CaseSensitive) == 0) {
					HidePopupSafely();
					return;
				}
			}

			// Only show popup if there are completions available
			if (m_pCompleter->completionCount() > 0) {
				QRect rect = m_pSourceCode->cursorRect();
				rect.setWidth(m_pCompleter->popup()->sizeHintForColumn(0)
					+ m_pCompleter->popup()->verticalScrollBar()->sizeHint().width());
				m_pCompleter->complete(rect);
			}
			else {
				HidePopupSafely();
			}
		}
		});

	/*m_pComment = new QAction(tr("Comment"),this);
	m_pComment->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_R));
	connect(m_pComment, SIGNAL(triggered()), this, SLOT(OnComment()));
	m_pSourceCode->addAction(m_pComment);

	m_pUnComment = new QAction(tr("UnComment"),this);
	m_pUnComment->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
	connect(m_pUnComment, SIGNAL(triggered()), this, SLOT(OnUnComment()));
	m_pSourceCode->addAction(m_pUnComment);*/
}

void CCodeEdit::SetCompleter(QCompleter* completer)
{
	if (m_pCompleter) {
		m_pCompleter->disconnect(this);
		// Remove event filter from old popup
		if (m_pCompleter->popup()) {
			m_pCompleter->popup()->removeEventFilter(this);
		}
	}
	
	m_pCompleter = completer;
	
	if (!m_pCompleter)
		return;
	
	m_pCompleter->setWidget(m_pSourceCode);
	m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
	m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	connect(m_pCompleter, QOverload<const QString&>::of(&QCompleter::activated),
			this, &CCodeEdit::OnInsertCompletion);
	
	// Install event filter on the popup widget itself
	// This ensures we capture key events BEFORE the popup processes them
	if (m_pCompleter->popup()) {
		m_pCompleter->popup()->installEventFilter(this);
	}
	// remember base model if present
	m_baseModel = qobject_cast<QStringListModel*>(m_pCompleter->model());
	// clear any temp model reference
	if (m_tempFuzzyModel) {
		delete m_tempFuzzyModel;
		m_tempFuzzyModel = nullptr;
	}
}

bool CCodeEdit::ShouldHideKeyFromCompletion(const QString& keyName) const
{
	if (m_completionFilterCallback) {
		return m_completionFilterCallback(keyName);
	}
	return false;
}

void CCodeEdit::SetCaseCorrectionFilterCallback(std::function<bool(const QString&)> callback)
{
	m_caseCorrectionFilterCallback = std::move(callback);
}

void CCodeEdit::UpdateCompletionModel(const QStringList& candidates)
{
	if (!m_pCompleter)
		return;

	QStringList filteredCandidates;
	QStringList caseCorrectionCandidates; // Keys available for case correction (all non-underscore keys)
	bool correctionStillValid = false;
	int maxWidth = 0; // Track maximum width during filtering
	QFontMetrics metrics(m_pCompleter->popup()->font());

	// Single loop to filter candidates, check correction validity, and calculate width
	foreach(const QString & candidate, candidates) {
		bool shouldHideFromPopup = ShouldHideKeyFromCompletion(candidate);
		bool startsWithUnderscore = candidate.startsWith('_');

		if (!startsWithUnderscore && !shouldHideFromPopup) {
			// Normal candidates that appear in popup
			filteredCandidates.append(candidate);
			int width = metrics.horizontalAdvance(candidate);
			if (width > maxWidth)
				maxWidth = width;
		}

		// ALL non-underscore candidates are available for case correction
		if (!startsWithUnderscore) {
			caseCorrectionCandidates.append(candidate);
		}

		// Check if correction is still valid (regardless of visibility)
		if (!m_lastCorrectWord.isEmpty() && candidate.compare(m_lastCorrectWord, Qt::CaseInsensitive) == 0) {
			correctionStillValid = true;
		}
	}

	// Clear correction tracking if no longer valid
	if (!correctionStillValid) {
		ClearCaseCorrectionTracking();
	}

	// Store both lists for different purposes
	m_visibleCandidates = filteredCandidates;
	m_caseCorrectionCandidates = caseCorrectionCandidates;

	QStringListModel* model = qobject_cast<QStringListModel*>(m_pCompleter->model());
	if (model) {
		if (model->stringList() == filteredCandidates)
			return; // No change, skip update
		model->setStringList(filteredCandidates);
	}
	else {
		model = new QStringListModel(filteredCandidates, m_pCompleter);
		m_pCompleter->setModel(model);
	}

	// Adjust popup width based on visible candidates only
	if (m_pCompleter && m_pCompleter->popup()) {
		QWidget* popup = m_pCompleter->popup();
		int padding = popup->style()->pixelMetric(QStyle::PM_FocusFrameHMargin, nullptr, popup) * 2;
		int scrollbar = popup->style()->pixelMetric(QStyle::PM_ScrollBarExtent, nullptr, popup);
		qreal dpr = popup->devicePixelRatioF();
		maxWidth = int(maxWidth * dpr + padding + scrollbar + 16); // 16 for extra margin
		popup->setMinimumWidth(maxWidth);
	}
}

// Helper method: Check if autocompletion should be triggered based on mode
bool CCodeEdit::ShouldTriggerAutoCompletion() const
{
	return GetAutoCompletionMode() == AutoCompletionMode::FullAuto;
}

// Helper method: Check if case correction should be handled based on mode
bool CCodeEdit::ShouldHandleCaseCorrection() const
{
	return GetAutoCompletionMode() != AutoCompletionMode::Disabled;
}

// Helper method: Check if character is valid for word boundaries
bool CCodeEdit::IsWordCharacter(QChar c) const
{
	return c.isLetterOrNumber() || c == '_' || c == '.';
}

// Helper method: Get current cursor context (position, block, text, etc.)
CCodeEdit::CursorContext CCodeEdit::GetCursorContext() const
{
	CursorContext context;
	context.cursor = m_pSourceCode->textCursor();
	context.block = context.cursor.block();
	context.text = context.block.text();
	context.position = context.cursor.position() - context.block.position();
	return context;
}

// Helper method: Find word boundaries from a given position
CCodeEdit::WordBoundaries CCodeEdit::FindWordBoundaries(const QString& text, int position) const
{
	WordBoundaries boundaries;
	boundaries.start = position;
	boundaries.end = position;
	
	// Move to start of word
	while (boundaries.start > 0 && IsWordCharacter(text[boundaries.start - 1])) {
		boundaries.start--;
	}
	
	// Move to end of word
	while (boundaries.end < text.length() && IsWordCharacter(text[boundaries.end])) {
		boundaries.end++;
	}
	
	return boundaries;
}

// Helper method: Extract word at cursor using Qt's word detection with custom extension
QString CCodeEdit::ExtractWordAtCursor(const CursorContext& context) const
{
	QTextCursor cursor = context.cursor;
	int originalPos = cursor.position();
	
	// Move to start of word
	cursor.movePosition(QTextCursor::StartOfWord);
	int startPos = cursor.position();
	
	// Move to end of word  
	cursor.movePosition(QTextCursor::EndOfWord);
	int endPos = cursor.position();
	
	// If no word boundary found, try manual detection for INI keys
	if (startPos == endPos) {
		WordBoundaries boundaries = FindWordBoundaries(context.text, context.position);
		return context.text.mid(boundaries.start, boundaries.end - boundaries.start);
	}
	
	// Get the word from start to end position
	cursor.setPosition(startPos);
	cursor.setPosition(endPos, QTextCursor::KeepAnchor);
	QString word = cursor.selectedText();
	
	// Extend the word to include dots and underscores that Qt might not include
	int relativeStart = startPos - context.block.position();
	int relativeEnd = endPos - context.block.position();
	
	// Extend backward
	while (relativeStart > 0 && (context.text[relativeStart - 1] == '_' || context.text[relativeStart - 1] == '.')) {
		relativeStart--;
	}
	
	// Extend forward
	while (relativeEnd < context.text.length() && (context.text[relativeEnd] == '_' || context.text[relativeEnd] == '.')) {
		relativeEnd++;
	}
	
	return context.text.mid(relativeStart, relativeEnd - relativeStart);
}

QString CCodeEdit::GetWordUnderCursor() const
{
	CursorContext context = GetCursorContext();
	return ExtractWordAtCursor(context);
}

QString CCodeEdit::GetWordBeforeCursor() const
{
	CursorContext context = GetCursorContext();
	
	// Find word boundaries going backwards from cursor position
	WordBoundaries boundaries = FindWordBoundaries(context.text, context.position);
	
	// Adjust to get word before cursor
	boundaries.end = context.position;
	
	// If we're already at a word boundary, return empty
	if (boundaries.start == boundaries.end)
		return QString();
	
	return context.text.mid(boundaries.start, boundaries.end - boundaries.start);
}

// Helper method: Check if we're in a key position (before equals sign)
bool CCodeEdit::IsInKeyPosition(const CursorContext& context) const
{
	int equalsPos = context.text.indexOf('=');
	if (equalsPos == -1) {
		return true; // No equals sign, assume we're editing a key
	}

	// Only allow completion if cursor is strictly before the equals sign
	return context.position <= equalsPos;
}

// Helper method: Check if word starts from beginning of line (ignoring whitespace)
bool CCodeEdit::IsWordAtLineStart(const CursorContext& context) const
{
	// Find the start of the current word
	WordBoundaries boundaries = FindWordBoundaries(context.text, context.position);

	// Check if there are only whitespace characters before the word start
	QString beforeWord = context.text.left(boundaries.start);
	return beforeWord.trimmed().isEmpty();
}

// Helper method: Get completion word considering key/value context
QString CCodeEdit::GetCompletionWord() const
{
	CursorContext context = GetCursorContext();

	// Only allow completion if the first character is completable character
	if (!context.text.isEmpty()) {
		QChar firstChar = context.text.at(0);
		if (!IsWordCharacter(firstChar)) {
			return QString(); // Non-completable
		}
	}

	// Check if we're editing an existing key in a key=value line
	QString existingKey = GetKeyFromCurrentLine();
	if (!existingKey.isEmpty()) {
		return existingKey;
	}

	// Standard detection for new keys - only if we're in key position
	if (IsInKeyPosition(context)) {
		QString word = ExtractWordAtCursor(context);

		// Only allow completion if the word starts from the beginning of the line (ignoring whitespace)
		if (!word.isEmpty() && IsWordAtLineStart(context)) {
			return word;
		}

		// For manual completion (Ctrl+Space), allow even if not at line start
		// This will be handled separately in HandleManualCompletionTrigger
	}

	return QString(); // We're in value position or word doesn't start from line beginning
}

void CCodeEdit::ShowCaseCorrection(const QString& wrongWord, const QString& correctWord)
{
	if (wrongWord.isEmpty() || correctWord.isEmpty() || wrongWord == correctWord)
		return;

	CursorContext context = GetCursorContext();

	// Prevent tooltip if in value part (after first '=')
	int equalsPos = context.text.indexOf('=');
	if (equalsPos != -1 && context.position > equalsPos)
		return;

	// Only allow if line starts with a completable character
	if (!context.text.isEmpty()) {
		QChar firstChar = context.text.at(0);
		if (!IsWordCharacter(firstChar))
			return;
	}

	int searchPos = context.position - 1;
	while (searchPos >= 0 && (context.text[searchPos].isSpace() || context.text[searchPos] == '=')) {
		searchPos--;
	}

	if (searchPos < 0)
		return;

	int end = searchPos + 1;
	int start = searchPos;
	while (start > 0 && IsWordCharacter(context.text[start - 1])) {
		start--;
	}

	QString foundWord = context.text.mid(start, end - start);
	if (foundWord.compare(wrongWord, Qt::CaseInsensitive) != 0)
		return;

	// Update case correction tracking
	UpdateCaseCorrectionTracking(wrongWord, correctWord, context.block.position() + start, context.block.position() + end);

	QToolTip::hideText();
	ScheduleWithDelay(50, [this, correctWord, context]() {
		QPoint globalPos = m_pSourceCode->mapToGlobal(m_pSourceCode->cursorRect(context.cursor).bottomLeft());
		QString tooltipText = tr("Did you mean: %1? (Press Ctrl+R to replace)").arg(correctWord);
		tooltipText = "<span style='white-space:nowrap;'>" + tooltipText + "</span>";
		QToolTip::showText(globalPos, tooltipText, m_pSourceCode, QRect(), 5000);
		}, "ShowCaseCorrectionTooltip");
}

void CCodeEdit::SetCaseCorrectionCallback(std::function<QString(const QString&)> callback)
{
	m_caseCorrectionCallback = std::move(callback);
}

void CCodeEdit::SetCompletionFilterCallback(std::function<bool(const QString&)> callback)
{
    m_completionFilterCallback = std::move(callback);
}

// Helper method: Handle completion trigger logic with improved reliability
void CCodeEdit::TriggerCompletion(const QString& prefix, int minimumLength)
{
	static bool completionTriggered = false;

	if (completionTriggered || !m_pCompleter || prefix.length() < minimumLength)
		return;

	if (m_pCompleter->popup() && m_pCompleter->popup()->isVisible() &&
		m_pCompleter->completionPrefix() == prefix)
		return;

	completionTriggered = true; // Mark completion as triggered
	m_pendingPrefix = prefix;
	m_completionDebounceTimer->stop();
	m_completionDebounceTimer->start(30); // 30ms debounce

	// Reset the flag after a short delay
	ResetFlagAfterDelay(completionTriggered, 50);
}

// Helper method: Handle case correction for delimiter input
void CCodeEdit::HandleCaseCorrection(const QString& word, bool wasPopupVisible)
{
	if (m_caseCorrectionInProgress || wasPopupVisible || !m_caseCorrectionCallback) {
		return;
	}

	// Check if case correction is enabled
	if (!ShouldHandleCaseCorrection()) {
		return;
	}

	QString correctedWord = m_caseCorrectionCallback(word);

	// If the callback didn't find a correction and fuzzy matching is enabled,
	// try a fuzzy lookup among candidates (case-correction candidates + visible ones).
	if (correctedWord.isEmpty() && GetFuzzyMatchingEnabled()) {
		// Build unified candidate list (preserve case from original lists)
		QStringList all = m_caseCorrectionCandidates;
		for (const QString& v : m_visibleCandidates) {
			bool found = false;
			for (const QString& a : all) {
				if (a.compare(v, Qt::CaseInsensitive) == 0) { found = true; break; }
			}
			if (!found) all.append(v);
		}

		if (!all.isEmpty()) {
			// BuildFuzzyMatches returns sorted candidates by fuzzy distance.
			QStringList fuzzy = BuildFuzzyMatches(all, word);

			if (!fuzzy.isEmpty()) {
				// Validate top fuzzy candidate with a small distance threshold before proposing it.
				const QString candidate = fuzzy.first();
				int dist = MinDistanceToCandidateSubstrings(word, candidate, 2);
				const int MAX_ACCEPTABLE_FUZZY_DIST = 2; // tunable: allow up to 2 edits for case-correction suggestion
				if (dist <= MAX_ACCEPTABLE_FUZZY_DIST) {
					correctedWord = candidate;
				}
			}
		}
	}

	if (!correctedWord.isEmpty() && IsKeyAvailableConsideringFuzzy(correctedWord, word)) {
		// Check if the corrected word should be hidden from case correction using callback
		if (m_caseCorrectionFilterCallback && m_caseCorrectionFilterCallback(correctedWord)) {
			return; // Don't show case correction for hidden keys
		}

		// Show case correction tooltip
		ShowCaseCorrection(word, correctedWord);
	}
}

void CCodeEdit::HandleCompletion(const QString& prefix)
{
	if (!m_pCompleter || prefix.length() < AUTO_COMPLETE_MIN_LENGTH)
		return;

	if (m_pCompleter->popup() && m_pCompleter->popup()->isVisible() &&
		m_pCompleter->completionPrefix() == prefix)
		return;

	m_pendingPrefix = prefix;
	m_completionDebounceTimer->stop();
	m_completionDebounceTimer->start(30); // 30ms debounce
}

// Helper method: Handle case correction for delimiter input
void CCodeEdit::HandleCaseCorrectionForDelimiter(QChar delimiter, bool wasPopupVisible)
{
	QString wordForCaseCorrection = GetWordBeforeCursor();
	if (!wordForCaseCorrection.isEmpty()) {
		HandleCaseCorrection(wordForCaseCorrection, wasPopupVisible);
	}
}

bool CCodeEdit::eventFilter(QObject* obj, QEvent* event)
{
	// Handle events from both the text edit and the popup
	if (obj != m_pSourceCode && (!m_pCompleter || obj != m_pCompleter->popup()))
		return QWidget::eventFilter(obj, event);
	
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

		// Record last key only for the source text edit (prevents popup keys from
		// affecting OnTextChanged suppression logic).
		if (obj == m_pSourceCode)
			m_lastKeyPressed = keyEvent->key();

		// Intercept Shift+Enter on the text edit to avoid Qt inserting U+2028
		if (obj == m_pSourceCode &&
			(keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) &&
			(keyEvent->modifiers() & Qt::ShiftModifier)) {

			// Hide popup if visible (so it doesn't stay open across blocks)
			HidePopupSafely();

			// Clear any pending case-correction tracking (positions would shift after block insertion)
			ClearCaseCorrectionTracking();

			// Insert a real paragraph/block (behaves like a hard Enter) and make it a single undo step
			QTextCursor cursor = m_pSourceCode->textCursor();
			cursor.beginEditBlock();
			cursor.insertBlock();
			cursor.endEditBlock();
			m_pSourceCode->setTextCursor(cursor);

			// Suppress any immediate auto-completion that could be triggered by the textChanged() fired by insertBlock()
			m_suppressNextAutoCompletion = true;
			ResetFlagAfterDelay(m_suppressNextAutoCompletion, 50);

			// Consume event so QTextEdit won't insert U+2028
			return true;
		}

		// Handle Backspace key
		if (keyEvent->key() == Qt::Key_Backspace) {
			if (m_pCompleter && m_pCompleter->popup() && m_pCompleter->popup()->isVisible()) {
				// Handle Backspace when popup is visible
				return HandleBackspaceKeyInPopup();
			}
			else {
				// Handle Backspace when popup is not visible
				return HandleBackspaceForCompletion(keyEvent);
			}
		}
		
		// Handle = key from POPUP widget directly
		if (m_pCompleter && obj == m_pCompleter->popup() && keyEvent->key() == Qt::Key_Equal) {
			return HandleEqualsKeyInPopup(keyEvent);
		}

		// Handle Enter/Return in the popup
		if (m_pCompleter && obj == m_pCompleter->popup()) {
			return HandleEnterKeyInPopup(keyEvent);
		}
		
		// Continue with existing text edit event handling...
		if (obj == m_pSourceCode) {
			return HandleTextEditKeyPress(keyEvent);
		}
	}
	
	return QWidget::eventFilter(obj, event);
}

// Helper method: Handle equals key press in popup
bool CCodeEdit::HandleEqualsKeyInPopup(QKeyEvent* keyEvent)
{
	// Get the word BEFORE any UI changes occur for potential case correction
	QString wordForCaseCorrection = GetWordBeforeCursor();

	// Hide the popup
	HidePopupSafely();

	// Check if we need to insert an = character
	CursorContext context = GetCursorContext();

	// Check if there's already an = sign in the line
	int equalsPos = context.text.indexOf('=');
	bool needsEquals = (equalsPos == -1);

	// Also check if there's an = after the cursor position
	QString restOfLine = context.text.mid(context.position);
	if (restOfLine.contains('=')) {
		needsEquals = false;
	}

	if (needsEquals) {
		// Insert the = character into the text edit
		QKeyEvent insertEvent(QEvent::KeyPress, Qt::Key_Equal, Qt::NoModifier, "=");
		QApplication::sendEvent(m_pSourceCode, &insertEvent);
	}

	// Trigger case correction for delimiter after popup is hidden
	HandleCaseCorrectionForDelimiter('=', /*wasPopupVisible=*/false);

	// Consume the event - don't let popup process it
	return true;
}

// Helper method: Handle Enter/Return in popup
bool CCodeEdit::HandleEnterKeyInPopup(QKeyEvent* keyEvent)
{
	if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Tab) {
		// Only accept completion if there are no modifiers (no Ctrl, Alt, etc.)
		// Allow the keypad modifier alone (some keyboards/reporters set KeypadModifier for NumPad Enter).
		Qt::KeyboardModifiers mods = keyEvent->modifiers();
		if ((mods & ~Qt::KeypadModifier) == Qt::NoModifier) {
			QModelIndex currentIndex = m_pCompleter->popup()->currentIndex();
			// If nothing is selected, select the first item
			if (!currentIndex.isValid() && m_pCompleter->completionCount() > 0) {
				currentIndex = m_pCompleter->popup()->model()->index(0, 0);
			}
			if (currentIndex.isValid()) {
				QString completion = m_pCompleter->completionModel()->data(currentIndex, Qt::DisplayRole).toString();
				if (!completion.isEmpty()) {
					OnInsertCompletion(completion);
					return true; // Consume the event
				}
			}
			HidePopupSafely();
			return true; // Consume the event
		}
	}
	return false;
}

// Helper method: Handle key press in text edit
bool CCodeEdit::HandleTextEditKeyPress(QKeyEvent* keyEvent)
{
	if (m_pCompleter && m_pCompleter->popup()->isVisible()) {
		return HandleKeyPressWithPopupVisible(keyEvent);
	}

	// Delete key handling (suppress popup for leading non-completers)
	if (keyEvent->key() == Qt::Key_Delete) {
		return HandleDeleteForCompletion(keyEvent);
	}
	
	// Check for autocompletion trigger (always allow manual trigger if completer exists)
	if (HandleManualCompletionTrigger(keyEvent)) {
		return true;
	}
	
	// Handle text input for auto-completion and case correction
	if (keyEvent->text().length() == 1) {
		QChar ch = keyEvent->text().at(0);
		if (!ch.isNull() && ch.isPrint()) {
			return HandleSingleCharacterInput(keyEvent);
		}
	}
	
	// Handle backspace to update completion
	HandleBackspaceForCompletion(keyEvent);
	
	return false;
}

// Helper method: Handle key press when popup is visible
bool CCodeEdit::HandleKeyPressWithPopupVisible(QKeyEvent* keyEvent)
{
	switch (keyEvent->key()) {
	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_Tab:
		return HandleEnterKeyInPopup(keyEvent);
	case Qt::Key_Backspace:
		return HandleBackspaceKeyInPopup();
	case Qt::Key_Equal:
		return HandleEqualsKeyInPopup(keyEvent);
	default:
		return HandleDefaultKeyInPopup(keyEvent);
	}
}

// Handles the Backspace key when the popup is visible
bool CCodeEdit::HandleBackspaceKeyInPopup()
{
	if (!m_pCompleter)
		return false;

	ScheduleWithDelay(10, [this]() {
		QString wordForCompletion = GetCompletionWord();
		if (!wordForCompletion.isEmpty() && wordForCompletion.length() >= AUTO_COMPLETE_MIN_LENGTH) {
			TriggerCompletion(wordForCompletion, AUTO_COMPLETE_MIN_LENGTH);
		}
		else {
			HidePopupSafely();
		}
		}, "HandleBackspaceKeyInPopup");
	return false;
}

bool CCodeEdit::HandleDefaultKeyInPopup(QKeyEvent* keyEvent)
{
	if (!keyEvent->text().isEmpty()) {
		QChar ch = keyEvent->text().at(0);
		if (!ch.isNull() && IsWordCharacter(ch))
			return false; // Let the keystroke process normally
	}
	HidePopupSafely();
	return false;
}

// Helper method: Handle manual completion trigger (Ctrl+Space)
bool CCodeEdit::HandleManualCompletionTrigger(QKeyEvent* keyEvent)
{
	// Always allow manual completion trigger if completer exists and mode is not disabled
	if (m_pCompleter && GetAutoCompletionMode() != AutoCompletionMode::Disabled &&
		(keyEvent->key() == Qt::Key_Space && (keyEvent->modifiers() & Qt::ControlModifier))) {

		CursorContext context = GetCursorContext();
		// Only allow manual completion if cursor is in key position (not after '=')
		if (!IsInKeyPosition(context))
			return false;

		// For manual trigger, get word even if it doesn't start from line beginning
		QString wordUnderCursor = ExtractWordAtCursor(context);

		// Even if word is empty, show all completions for manual trigger
		if (GetFuzzyMatchingEnabled()) {
			// Build fuzzy matches (fuzzy-ranked)
			QStringList fuzzy = BuildFuzzyMatches(m_visibleCandidates, wordUnderCursor);

			// Preserve fuzzy ranking when the user has typed a prefix.
			// Only present an alphabetical list for the "show all" case (empty prefix).
			if (wordUnderCursor.isEmpty()) {
				std::sort(fuzzy.begin(), fuzzy.end(), [](const QString& a, const QString& b) {
					return a.compare(b, Qt::CaseInsensitive) < 0;
					});
			}

			if (!fuzzy.isEmpty()) {
				// Create a temporary model parented to this so we can control its lifetime
				if (m_tempFuzzyModel) {
					delete m_tempFuzzyModel;
					m_tempFuzzyModel = nullptr;
				}
				m_tempFuzzyModel = new QStringListModel(fuzzy, this);
				m_pCompleter->setModel(m_tempFuzzyModel);

				QRect rect = m_pSourceCode->cursorRect();
				rect.setWidth(m_pCompleter->popup()->sizeHintForColumn(0)
					+ m_pCompleter->popup()->verticalScrollBar()->sizeHint().width());
				m_pCompleter->complete(rect);
				return true;
			}
		}
		else {
			m_pCompleter->setCompletionPrefix(wordUnderCursor);
			if (m_pCompleter->completionCount() > 0) {
				QRect rect = m_pSourceCode->cursorRect();
				rect.setWidth(m_pCompleter->popup()->sizeHintForColumn(0)
					+ m_pCompleter->popup()->verticalScrollBar()->sizeHint().width());
				m_pCompleter->complete(rect);
				return true;
			}
		}
	}
	return false;
}

// Helper method: Handle single character input
bool CCodeEdit::HandleSingleCharacterInput(QKeyEvent* keyEvent)
{
	static bool inputProcessed = false;

	if (inputProcessed)
		return false; // Skip processing if input has already been handled

	inputProcessed = true; // Mark input as processed

	QChar inputChar = keyEvent->text().isEmpty() ? QChar() : keyEvent->text().at(0);

	// Capture popup state BEFORE any popup manipulation happens
	bool popupWasVisible = (m_pCompleter && m_pCompleter->popup()->isVisible());

	// Only treat delimiters as "plain" delimiters if there are no modifiers
	// (allow NumPad/Keypad modifier as an exception).
	Qt::KeyboardModifiers mods = keyEvent->modifiers();
	Qt::KeyboardModifiers modsNoKeypad = mods & ~Qt::KeypadModifier;

	int key = keyEvent->key();
	bool isSpaceKey = (key == Qt::Key_Space);
	bool isEqualsKey = (key == Qt::Key_Equal);
	bool isEnterKey = (key == Qt::Key_Return || key == Qt::Key_Enter);
	bool isTabKey = (key == Qt::Key_Tab);

	// treat Enter/Tab via key instead of relying on text() == '\n' or '\t'
	bool isPlainDelimiter = ((modsNoKeypad == Qt::NoModifier) ||
		(isEqualsKey && modsNoKeypad == Qt::ShiftModifier)) &&
		(isSpaceKey || isEqualsKey || isEnterKey || isTabKey);

	// Handle case correction BEFORE any other processing for delimiters
	if (isPlainDelimiter) {
		QString wordForCaseCorrection = GetWordBeforeCursor();
		if (!wordForCaseCorrection.isEmpty()) {
			HandleCaseCorrection(wordForCaseCorrection, popupWasVisible);
		}
	}

	// Close popup when = is pressed (AFTER capturing word for case correction)
	// Only close for plain '=' (no Ctrl/Alt/etc, but allow Shift for typing equals).
	if (isEqualsKey && (modsNoKeypad == Qt::NoModifier || modsNoKeypad == Qt::ShiftModifier)) {
		HidePopupSafely();
	}

	// Trigger auto-completion for letter/number input (only in FullAuto mode)
	if (m_pCompleter && !inputChar.isNull() && IsWordCharacter(inputChar) && ShouldTriggerAutoCompletion()) {
		// Let the keystroke process first, then trigger completion
		ScheduleWithDelay(10, [this]() {
			QString wordForCompletion = GetCompletionWord();
			if (!wordForCompletion.isEmpty()) {
				HandleCompletion(wordForCompletion);
			}
			}, "HandleSingleCharacterInput");
	}

	// Reset the flag after a short delay
	ResetFlagAfterDelay(inputProcessed, 50);
	return false; // Let the character be processed normally
}

// Helper method: Handle backspace for completion updates
bool CCodeEdit::HandleBackspaceForCompletion(QKeyEvent* keyEvent)
{
	if (m_pCompleter && keyEvent->key() == Qt::Key_Backspace) {
		QTextCursor cursor = m_pSourceCode->textCursor();
		QTextBlock block = cursor.block();
		QString lineText = block.text();
		int relPos = cursor.position() - block.position();

		// If cursor is just after first character and that first char is # or ; we are deleting it
		if (relPos == 1 && !lineText.isEmpty() && !IsWordCharacter(lineText.at(0))) {
			// Suppress any immediate autocompletion once the character is removed
			m_suppressNextAutoCompletion = true;
			// Also ensure any visible popup is hidden
			HidePopupSafely();
			return false;
		}

		// Suppress completion if backspace at start of line
		if (relPos == 0) {
			m_suppressNextAutoCompletion = true;
			HidePopupSafely();
			return false;
		}

		// Check if backspace would affect the tracked case correction
		if (!m_lastWrongWord.isEmpty() && m_lastWordStart >= 0 && m_lastWordEnd >= 0) {
			int currentPos = cursor.position();
			if (currentPos > m_lastWordStart && currentPos <= m_lastWordEnd) {
				ClearCaseCorrectionTracking();
			}
		}

		if (ShouldTriggerAutoCompletion()) {
			ScheduleWithDelay(10, [this]() {
				if (!m_pCompleter) return;
				if (m_suppressNextAutoCompletion) return; // safety check

				QString wordForCompletion = GetCompletionWord();
				if (!wordForCompletion.isEmpty() && wordForCompletion.length() >= AUTO_COMPLETE_MIN_LENGTH) {
					TriggerCompletion(wordForCompletion, AUTO_COMPLETE_MIN_LENGTH);
				}
				else {
					HidePopupSafely();
				}
				}, "TriggerCompletionWithDelay");
		}
		return false;
	}
	return false;
}

bool CCodeEdit::HandleDeleteForCompletion(QKeyEvent* keyEvent)
{
	if (m_pCompleter && keyEvent->key() == Qt::Key_Delete) {
		QTextCursor cursor = m_pSourceCode->textCursor();
		QTextBlock block = cursor.block();
		QString lineText = block.text();
		int relPos = cursor.position() - block.position();

		// If current line is empty and cursor is at column 0,
		// the user pressed Delete to remove the empty line (merge next line up).
		// Suppress any immediate autocompletion and hide popup — do not trigger popup.
		if (relPos == 0 && lineText.isEmpty()) {
			m_suppressNextAutoCompletion = true;
			HidePopupSafely();
			return false; // allow deletion to proceed but prevent popup
		}

		// If cursor is at column 0 and first character is non-completer Delete will remove it
		if (relPos == 0 && !lineText.isEmpty()) {
			QChar c = lineText.at(0);
			if (!(IsWordCharacter(c) || c == '=')) {
				m_suppressNextAutoCompletion = true;
				HidePopupSafely();
				return false; // allow deletion to proceed
			}
		}

		// Schedule recompute similarly to backspace (reuse task name for dedup)
		if (ShouldTriggerAutoCompletion()) {
			ScheduleWithDelay(10, [this]() {
				if (!m_pCompleter) return;
				if (m_suppressNextAutoCompletion) return;

				QString wordForCompletion = GetCompletionWord();
				if (!wordForCompletion.isEmpty() && wordForCompletion.length() >= AUTO_COMPLETE_MIN_LENGTH) {
					TriggerCompletion(wordForCompletion, AUTO_COMPLETE_MIN_LENGTH);
				}
				else {
					HidePopupSafely();
				}
				}, "TriggerCompletionWithDelay");
		}
	}
	return false;
}

void CCodeEdit::OnInsertCompletion(const QString& completion)
{
	if (!m_pCompleter)
		return;

	// Only allow completion if cursor is in key position (left of '=')
	CursorContext context = GetCursorContext();
	if (!IsInKeyPosition(context)) {
		HidePopupSafely();
		return;
	}

	// Clear case correction tracking since we're inserting a completion
	ClearCaseCorrectionTracking();

	// Set flag to prevent completion re-triggering during insertion
	m_completionInsertInProgress = true;

	// Hide the popup immediately to prevent it from staying open
	HidePopupSafely();

	QString prefix = m_pCompleter->completionPrefix();

	// Check if we're editing an existing key=value line
	QString existingKey = GetKeyFromCurrentLine();

	if (!existingKey.isEmpty()) {
		// We're editing an existing key, replace just the key part
		int equalsPos = context.text.indexOf('=');

		if (equalsPos != -1) {
			// Select from the start of the line to the equals sign
			QTextCursor cursor = context.cursor;
			cursor.movePosition(QTextCursor::StartOfLine);
			cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, equalsPos);

			// Replace with the completion (no extra = needed since it exists)
			cursor.insertText(completion);
			m_pSourceCode->setTextCursor(cursor);

			ConsolidateEqualsSignsAfterCursor(cursor, false);

			// Reset flag after a brief delay to allow text change events to settle
			ResetFlagAfterDelay(m_completionInsertInProgress, 50);
			return;
		}
	}

	// For all other cases, replace from start of line to end of current word
	WordBoundaries boundaries = FindWordBoundaries(context.text, context.position);

	// Check if there's already an = after the current word position
	QString restOfLine = context.text.mid(boundaries.end);
	bool hasEqualsAfter = restOfLine.contains('=');

	QTextCursor cursor = context.cursor;

	// Select from start of line to end of current word
	cursor.movePosition(QTextCursor::StartOfLine);
	cursor.setPosition(context.block.position() + boundaries.end, QTextCursor::KeepAnchor);

	if (hasEqualsAfter) {
		// There's already an = sign after the word, so just replace with completion
		cursor.insertText(completion);
	}
	else {
		// No equals sign after, so add one
		cursor.insertText(completion + "=");
	}

	m_pSourceCode->setTextCursor(cursor);

	// Reset flag after a brief delay to allow text change events to settle
	ResetFlagAfterDelay(m_completionInsertInProgress, 50);
}

void CCodeEdit::SetFont(const QFont& Font)
{
	m_pSourceCode->setFont(Font);
}

const QFont& CCodeEdit::GetFont() const
{
	return m_pSourceCode->font();
}

#define ADD_HISTORY(list,entry)	\
	list.removeAll(entry);		\
	list.prepend(entry);		\
	while(list.size() > 10)		\
		list.removeLast();

void CCodeEdit::OnFind()
{
	static QStringList Finds;
	bool bOK = false;
	m_CurFind = QInputDialog::getItem (this, tr("Find"),tr("F3: Find Next\n+ Shift: Backward\n+ Ctrl: Case Sensitively\n+ Alt: Whole Words\n\nFind String:") + QString(160,' '), Finds, 0, true, &bOK);
	if (!bOK)
		return;
	ADD_HISTORY(Finds, m_CurFind);
	OnFindNext();
}

void CCodeEdit::OnFindNext()
{
	if(m_CurFind.isEmpty())
		return;

	QTextDocument::FindFlags Flags = QTextDocument::FindFlags();
	Qt::KeyboardModifiers Mods = QApplication::keyboardModifiers();
	if(Mods & Qt::ShiftModifier)
		Flags |= QTextDocument::FindBackward;
	if(Mods & Qt::ControlModifier)
		Flags |= QTextDocument::FindCaseSensitively;
	if(Mods & Qt::AltModifier)
		Flags |= QTextDocument::FindWholeWords;

	m_pSourceCode->find(m_CurFind, Flags);
}

void CCodeEdit::OnGoTo()
{
	int iLine = QInputDialog::getText(this, tr("Go to Line:"),tr(""), QLineEdit::Normal, "").toInt();
	if(!iLine)
		return;
	
	QTextCursor Cursor = m_pSourceCode->textCursor();   
	Cursor.movePosition(QTextCursor::Start);
	while(iLine-- > 1)
		Cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor);
	//Cursor.select(QTextCursor::LineUnderCursor);
	m_pSourceCode->setTextCursor(Cursor);
}

void CCodeEdit::ReplaceLastCorrection()
{
	// Ensure autocompletion is enabled
	if (GetAutoCompletionMode() == AutoCompletionMode::Disabled)
		return;

	// Validate that the correction is still available
	if (m_lastWrongWord.isEmpty() || m_lastCorrectWord.isEmpty() ||
		m_lastWordStart < 0 || m_lastWordEnd < 0)
		return;

	// Use fuzzy-aware availability check so Ctrl+R works when fuzzy is enabled
	if (!IsKeyAvailableConsideringFuzzy(m_lastCorrectWord, m_lastWrongWord)) {
		// Clear tracking if the correction is no longer valid
		ClearCaseCorrectionTracking();
		return;
	}

	// Get the line text at the correction position
	QTextBlock correctionBlock = m_pSourceCode->document()->findBlock(m_lastWordStart);
	QString lineText = correctionBlock.text();
	int equalsPos = lineText.indexOf('=');

	// Do not trigger case correction if the correction is in the value part (after first '=')
	int correctionRelativePos = m_lastWordStart - correctionBlock.position();
	if (equalsPos != -1 && correctionRelativePos > equalsPos) {
		// In value part, do not trigger correction
		ClearCaseCorrectionTracking();
		return;
	}

	// Only allow correction if the first character is completable character
	if (!lineText.isEmpty()) {
		QChar firstChar = lineText.at(0);
		if (!IsWordCharacter(firstChar)) {
			ClearCaseCorrectionTracking();
			return;
		}
	}

	// Hide completion popup if it's visible before making the replacement
	HidePopupSafely();

	// Set flag to prevent completion re-triggering during case correction
	m_caseCorrectionInProgress = true;

	// Perform the replacement
	QTextCursor cursor = m_pSourceCode->textCursor();
	cursor.setPosition(m_lastWordStart);
	cursor.setPosition(m_lastWordEnd, QTextCursor::KeepAnchor);

	// Verify the selected text matches what we expect (case insensitive)
	QString selectedText = cursor.selectedText();
	if (selectedText.compare(m_lastWrongWord, Qt::CaseInsensitive) == 0) {

		// Check if this is an existing key=value line and we're correcting the key part
		int equalsPos2;
		if (IsExistingKeyValueLine(lineText, correctionRelativePos, equalsPos2)) {
			// If the correction is before the equals sign, we're correcting a key in an existing key=value line
			if (correctionRelativePos < equalsPos2) {
				// This is an existing key=value line - just replace the key, don't add =
				cursor.insertText(m_lastCorrectWord);
			}
			else {
				// This is a value correction or something else - just replace
				cursor.insertText(m_lastCorrectWord);
			}
		}
		else {
			// No equals sign in the line - this might be a new key, check if we should add =
			int correctionLinePos = m_lastWordStart - correctionBlock.position();
			QString replacementText = GetTextReplacement(m_lastWrongWord, m_lastCorrectWord,
				lineText, correctionLinePos, true);
			cursor.insertText(replacementText);
		}

		ConsolidateEqualsSignsAfterCursor(cursor, true);

		// Clear tracking after replacement
		ClearCaseCorrectionTracking();

		// Ensure popup stays hidden after correction is applied
		HidePopupSafely();
	}

	// Reset flag after a brief delay to allow text change events to settle
	ResetFlagAfterDelay(m_caseCorrectionInProgress, 100);
}

void CCodeEdit::OnTextChanged()
{
	if (m_suppressNextAutoCompletion) {
		// Consume one suppression cycle (prevents popup right after deleting # or ;)
		m_suppressNextAutoCompletion = false;
		return;
	}

	// Suppress completion if last key was Enter/Return
	if (m_lastKeyPressed == Qt::Key_Enter || m_lastKeyPressed == Qt::Key_Return) {
		m_lastKeyPressed = 0; // reset
		return;
	}

	if (!CanTriggerCompletion())
		return;

	// Validate case correction
	ValidateCaseCorrection();

	// Handle completion
	QString wordForCompletion = GetCompletionWord();
	if (wordForCompletion.isEmpty()) {
		HidePopupSafely();
		return;
	}

	if (ShouldTriggerAutoCompletion()) {
		HandleCompletion(wordForCompletion);
	}
	else {
		HidePopupSafely();
	}
}

// Determines if autocompletion can be triggered based on the current state
bool CCodeEdit::CanTriggerCompletion() const
{
	QTextCursor cursor = m_pSourceCode->textCursor();
	// Prevent popup if cursor is at position 0
	if (cursor.position() == 0)
		return false;

	if (m_suppressNextAutoCompletion)
		return false;

	return m_pCompleter && m_pSourceCode->hasFocus() &&
		!m_completionInsertInProgress && !m_caseCorrectionInProgress;
}

// Validates if the last case correction is still valid and clears it if not
void CCodeEdit::ValidateCaseCorrection()
{
	if (m_lastWrongWord.isEmpty() || m_lastWordStart < 0 || m_lastWordEnd < 0)
		return;

	QTextCursor cursor = m_pSourceCode->textCursor();
	QTextBlock block = m_pSourceCode->document()->findBlock(m_lastWordStart);

	// Combine all checks into a single condition
	if (!block.isValid() || cursor.position() == 0 ||
		cursor.position() < m_lastWordStart - 10 || cursor.position() > m_lastWordEnd + 10 ||
		!IsKeyAvailableConsideringFuzzy(m_lastCorrectWord, m_lastWrongWord) ||
		block.text().mid(m_lastWordStart - block.position(), m_lastWordEnd - m_lastWordStart)
		.compare(m_lastWrongWord, Qt::CaseInsensitive) != 0) {
		ClearCaseCorrectionTracking();
	}
}

QString CCodeEdit::GetKeyFromCurrentLine() const
{
	CursorContext context = GetCursorContext();
	
	// Check if there's an equals sign on this line
	int equalsPos = context.text.indexOf('=');
	if (equalsPos == -1) {
		return QString(); // No equals sign, not a key=value line
	}
	
	// If cursor is after the equals sign, we're editing the value
	if (context.position > equalsPos) {
		return QString(); // We're in the value part
	}
	
	// Extract the key part (everything before the equals sign, but don't trim yet)
	QString keyPartRaw = context.text.left(equalsPos);
	
	// Check if cursor is within the raw key part
	if (context.position > keyPartRaw.length()) {
		return QString(); // Cursor is beyond the key part (shouldn't happen, but safety check)
	}
	
	// Find word boundaries within the raw key part at the cursor position
	WordBoundaries boundaries = FindWordBoundaries(keyPartRaw, context.position);
	
	if (boundaries.start == boundaries.end) {
		return QString(); // No word found
	}
	
	return keyPartRaw.mid(boundaries.start, boundaries.end - boundaries.start);
}

// Helper method: Reset flag after a specified delay to reduce code duplication
void CCodeEdit::ResetFlagAfterDelay(bool& flag, int delayMs)
{
	ScheduleWithDelay(delayMs, [&flag]() {
		flag = false;
		}, "ResetFlagAfterDelay");
}

// Helper method: Safely hide popup with null checks
void CCodeEdit::HidePopupSafely()
{
	if (m_pCompleter && m_pCompleter->popup() && m_pCompleter->popup()->isVisible()) {
		m_pCompleter->popup()->hide();
	}
	// If we used a temporary fuzzy model, restore the base model
	RestoreBaseCompletionModel();
}

// Helper method: Check if we're dealing with an existing key=value line
bool CCodeEdit::IsExistingKeyValueLine(const QString& lineText, int cursorPosition, int& equalsPosition) const
{
	equalsPosition = lineText.indexOf('=');
	return (equalsPosition != -1 && cursorPosition <= equalsPosition);
}

// Helper method: Generate appropriate text replacement based on context
QString CCodeEdit::GetTextReplacement(const QString& originalWord, const QString& replacement, 
									 const QString& lineText, int wordPosition, bool addEquals) const
{
	// Check if there's already an equals sign after the word position
	QString restOfLine = lineText.mid(wordPosition + replacement.length());
	bool hasEqualsAfter = restOfLine.contains('=');
	
	if (addEquals && !hasEqualsAfter) {
		return replacement + "=";
	}
	return replacement;
}

// Helper method: Clear case correction tracking information
void CCodeEdit::ClearCaseCorrectionTracking()
{
	UpdateCaseCorrectionTracking(QString(), QString(), -1, -1);

	// Only hide the tooltip if it is currently visible
	if (QToolTip::isVisible()) {
		QToolTip::hideText();
	}
}

void CCodeEdit::UpdateCaseCorrectionTracking(const QString& wrongWord, const QString& correctWord, int wordStart, int wordEnd)
{
	m_lastWrongWord = wrongWord;
	m_lastCorrectWord = correctWord;
	m_lastWordStart = wordStart;
	m_lastWordEnd = wordEnd;
}

// Helper method: Check if a key is available in the current completion candidates
bool CCodeEdit::IsKeyAvailableInCompletionModel(const QString& key) const
{
	if (!m_pCompleter)
		return false;

	// Check both visible candidates and case correction candidates
	foreach(const QString & candidate, m_visibleCandidates) {
		if (candidate.compare(key, Qt::CaseInsensitive) == 0) {
			return true;
		}
	}

	foreach(const QString & candidate, m_caseCorrectionCandidates) {
		if (candidate.compare(key, Qt::CaseInsensitive) == 0) {
			return true;
		}
	}

	return false;
}

// Helper method: Consolidate multiple consecutive equals signs into one
// and optionally move the cursor past the equals sign
void CCodeEdit::ConsolidateEqualsSignsAfterCursor(QTextCursor& cursor, bool moveCursorPastEquals)
{
	int pos = cursor.position();
	QTextBlock block = cursor.block();
	QString text = block.text();
	int relPos = pos - block.position();

	// Check for any equals signs after the current position
	int equalCount = 0;
	int i = relPos;
	while (i < text.length() && text[i] == '=') {
		equalCount++;
		i++;
	}

	// If we have more than one equals sign, remove all and add back just one
	if (equalCount > 1) {
		cursor.setPosition(block.position() + relPos);
		cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, equalCount);
		cursor.insertText("=");
		m_pSourceCode->setTextCursor(cursor);

		// Update text and position after modification
		text = block.text();
		relPos = cursor.position() - block.position();
	}

	// Move cursor past '=' if requested and if there is an equals sign after the cursor
	if (moveCursorPastEquals && relPos < text.length() && text[relPos] == '=') {
		cursor.movePosition(QTextCursor::Right);
		m_pSourceCode->setTextCursor(cursor);
	}
}

void CCodeEdit::SetAutoCompletionMode(int checkState)
{
	QMutexLocker locker(&s_autoCompletionModeMutex);

	switch (checkState) {
	case Qt::Unchecked:
		s_autoCompletionMode = AutoCompletionMode::Disabled;
		break;
	case Qt::PartiallyChecked:
		s_autoCompletionMode = AutoCompletionMode::ManualOnly;
		break;
	case Qt::Checked:
	default:
		s_autoCompletionMode = AutoCompletionMode::FullAuto;
		break;
	}
}

CCodeEdit::AutoCompletionMode CCodeEdit::GetAutoCompletionMode()
{
	QMutexLocker locker(&s_autoCompletionModeMutex);
	return s_autoCompletionMode;
}

void CCodeEdit::ScheduleWithDelay(int delayMs, std::function<void()> task, const QString& taskName)
{
	// Check if the task is already scheduled
	if (m_scheduledTasks.contains(taskName)) {
		return; // Skip scheduling duplicate tasks
	}

	// Add the task to the set of scheduled tasks
	m_scheduledTasks.insert(taskName);

	// Schedule the task with a delay
	//qDebug() << "Task scheduled:" << taskName << "with delay:" << delayMs << "ms";
	QTimer::singleShot(delayMs, this, [this, task, taskName]() {
		// Execute the task
		task();

		// Remove the task from the set after execution
		m_scheduledTasks.remove(taskName);
		});
}

void CCodeEdit::OnCursorPositionChanged()
{
	CursorContext context = GetCursorContext();
	if (!IsInKeyPosition(context)) {
		HidePopupSafely();
	}
}

// Helper: apply a fuzzy-model for the given prefix and return the fuzzy result list.
// This creates a temporary QStringListModel (owned by 'this') and assigns it to the completer.
QStringList CCodeEdit::ApplyFuzzyModelForPrefix(const QString& prefix)
{
	if (!m_pCompleter)
		return QStringList();

	// Build fuzzy matches from visible candidates
	QStringList fuzzy = BuildFuzzyMatches(m_visibleCandidates, prefix);

	// Create temporary model parented to this so we can control its lifetime
	if (m_tempFuzzyModel) {
		delete m_tempFuzzyModel;
		m_tempFuzzyModel = nullptr;
	}
	m_tempFuzzyModel = new QStringListModel(fuzzy, this);
	m_pCompleter->setModel(m_tempFuzzyModel);
	return fuzzy;
}

// Restore completer's model back to the base (visible) model and free any temporary model
void CCodeEdit::RestoreBaseCompletionModel()
{
	if (!m_pCompleter)
		return;

	// If a temp fuzzy model is active, remove it and restore base model
	if (m_tempFuzzyModel) {
		m_pCompleter->setModel(m_baseModel);
		delete m_tempFuzzyModel;
		m_tempFuzzyModel = nullptr;
	}
}

bool CCodeEdit::IsKeyAvailableConsideringFuzzy(const QString& key, const QString& wordForFuzzy) const
{
	// First keep original fast path
	if (IsKeyAvailableInCompletionModel(key))
		return true;

	// If fuzzy matching not enabled, respect original behavior
	if (!GetFuzzyMatchingEnabled())
		return false;

	// Build candidate set: include both visible and case-correction candidates (unique)
	QStringList all = m_visibleCandidates;
	for (const QString& c : m_caseCorrectionCandidates) {
		if (!all.contains(c, Qt::CaseInsensitive))
			all.append(c);
	}

	if (all.isEmpty())
		return false;

	QStringList fuzzy = BuildFuzzyMatches(all, wordForFuzzy);
	for (const QString& f : fuzzy) {
		if (f.compare(key, Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

void CCodeEdit::SetMaxFuzzyPrefixLength(int length)
{
	if (length < 4) length = 4; // minimum sensible bound
	s_maxFuzzyPrefixLength = length;
	//qDebug() << "[CodeEdit] s_maxFuzzyPrefixLength set to" << s_maxFuzzyPrefixLength;
}

int CCodeEdit::GetMaxFuzzyPrefixLength()
{
	return s_maxFuzzyPrefixLength;
}

void CCodeEdit::SetMinFuzzyPrefixLength(int length)
{
	if (length < 1) length = 1; // sensible lower bound
	s_minFuzzyPrefixLength = length;
	//qDebug() << "[CodeEdit] s_minFuzzyPrefixLength set to" << s_minFuzzyPrefixLength;
}

int CCodeEdit::GetMinFuzzyPrefixLength()
{
	return s_minFuzzyPrefixLength;
}

void CCodeEdit::SetFuzzyMatchingEnabled(bool bEnabled)
{
	s_fuzzyMatchingEnabled = bEnabled;
}

bool CCodeEdit::GetFuzzyMatchingEnabled()
{
	return s_fuzzyMatchingEnabled;
}

void CCodeEdit::ClearFuzzyCache()
{
	QMutexLocker lk(&s_fuzzyCacheMutex);
	s_fuzzyCache.clear();
	s_cacheOrder.clear();
	//qDebug() << "[FuzzyCache] CLEARED";
}
