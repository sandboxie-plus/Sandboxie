#include "stdafx.h"
#include "CodeEdit.h"


#define TAB_SPACES "   "

CCodeEdit::AutoCompletionMode CCodeEdit::s_autoCompletionMode = AutoCompletionMode::FullAuto;
QMutex CCodeEdit::s_autoCompletionModeMutex;

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

	if (!correctedWord.isEmpty() && IsKeyAvailableInCompletionModel(correctedWord)) {
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
		if (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::KeypadModifier) {
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
		return HandleSingleCharacterInput(keyEvent);
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
	if (keyEvent->text().length() == 1 && IsWordCharacter(keyEvent->text().at(0))) {
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
		m_pCompleter->setCompletionPrefix(wordUnderCursor);

		if (m_pCompleter->completionCount() > 0) {
			QRect rect = m_pSourceCode->cursorRect();
			rect.setWidth(m_pCompleter->popup()->sizeHintForColumn(0)
				+ m_pCompleter->popup()->verticalScrollBar()->sizeHint().width());
			m_pCompleter->complete(rect);
			return true;
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

	QChar inputChar = keyEvent->text().at(0);

	// Capture popup state BEFORE any popup manipulation happens
	bool popupWasVisible = (m_pCompleter && m_pCompleter->popup()->isVisible());

	// Handle case correction BEFORE any other processing for delimiters
	if (inputChar == ' ' || inputChar == '=' || inputChar == '\n' || inputChar == '\t') {
		QString wordForCaseCorrection = GetWordBeforeCursor();
		if (!wordForCaseCorrection.isEmpty()) {
			HandleCaseCorrection(wordForCaseCorrection, popupWasVisible);
		}
	}

	// Close popup when = is pressed (AFTER capturing word for case correction)
	if (inputChar == '=') {
		HidePopupSafely();
	}

	// Trigger auto-completion for letter/number input (only in FullAuto mode)
	if (m_pCompleter && IsWordCharacter(inputChar) && ShouldTriggerAutoCompletion()) {
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

	if (!IsKeyAvailableInCompletionModel(m_lastCorrectWord)) {
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

		int pos = cursor.position();
		QTextBlock block = cursor.block();
		QString text = block.text();
		int relPos = pos - block.position();
		// If there are two '=' after the current position, remove one
		if (relPos < text.length() && text[relPos] == '=' && text[relPos + 1] == '=') {
			cursor.setPosition(block.position() + relPos);
			cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
			cursor.removeSelectedText();
			m_pSourceCode->setTextCursor(cursor);
		}
		// Move cursor past '=' if it exists right after the corrected key
		if (relPos < text.length() && text[relPos] == '=') {
			cursor.movePosition(QTextCursor::Right);
			m_pSourceCode->setTextCursor(cursor);
		}

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
		!IsKeyAvailableInCompletionModel(m_lastCorrectWord) ||
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
