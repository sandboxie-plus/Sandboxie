#include "stdafx.h"
#include "RecoveryWindow.h"
#include "SandMan.h"
#include "../MiscHelpers/Common/Settings.h"
#include "../MiscHelpers/Common/TreeItemModel.h"
#include "../MiscHelpers/Common/Common.h"
#include "SettingsWindow.h"


#if defined(Q_OS_WIN)
#include <wtypes.h>
#include <QAbstractNativeEventFilter>
#include <dbt.h>
#endif


namespace {

typedef int (*RECOVERY_WILDCARD_CHAR_EQUAL_FN)(wchar_t patternChar, wchar_t textChar);

bool RecoveryWildcardIsPathSeparator(wchar_t ch)
{
	return ch == L'\\' || ch == L'/';
}

bool RecoveryWildcardMatchWholeCore(
	const wchar_t* pattern, const wchar_t* text,
	size_t textLen, size_t matchStart,
	unsigned char* rows, size_t rowsSize,
	RECOVERY_WILDCARD_CHAR_EQUAL_FN charEqual)
{
	size_t index;
	size_t rowSize;
	size_t requiredSize;
	unsigned char* previous;
	unsigned char* current;
	unsigned char* swap;
	wchar_t token;

	if (!pattern || !text || !rows || !charEqual)
		return false;

	if (textLen > ((((size_t)-1) / 2) - 1))
		return false;

	rowSize = textLen + 1;
	requiredSize = rowSize * 2;
	if (rowsSize < requiredSize)
		return false;

	if (matchStart > textLen)
		return false;

	previous = rows;
	current = rows + rowSize;

	memset(rows, 0, requiredSize);
	previous[matchStart] = 1;
	for (index = matchStart + 1; index <= textLen; ++index) {
		if (RecoveryWildcardIsPathSeparator(text[index - 1]))
			previous[index] = 1;
	}

	while (*pattern) {
		memset(current, 0, rowSize);
		token = *pattern++;

		if (token == L'*' && *pattern == L'*') {
			++pattern;
			for (index = 1; index <= textLen; ++index) {
				if (!RecoveryWildcardIsPathSeparator(text[index - 1])) {
					if (previous[index - 1])
						current[index] = 1;
					else
						current[index] = current[index - 1];
				}
			}
			for (index = 1; index < textLen; ++index) {
				if (!RecoveryWildcardIsPathSeparator(text[index]))
					current[index] = 0;
			}

		} else if (token == L'*') {
			current[0] = previous[0];
			for (index = 1; index <= textLen; ++index)
				current[index] = previous[index] || current[index - 1];

		} else {
			for (index = 1; index <= textLen; ++index) {
				if ((token == L'?' &&
						!RecoveryWildcardIsPathSeparator(text[index - 1])) ||
						charEqual(token, text[index - 1]))
					current[index] = previous[index - 1];
			}
		}

		swap = previous;
		previous = current;
		current = swap;
	}

	return previous[textLen] != 0;
}

// Device prefix constants -- keep in sync with file_recovery.c globals:
//   File_Redirector        L"\\Device\\LanmanRedirector\\"
//   File_MupRedir          L"\\Device\\Mup\\;LanmanRedirector\\"
//   File_DfsClientRedir    L"\\Device\\Mup\\DfsClient\\"
//   File_HgfsRedir         L"\\Device\\Mup\\;hgfs\\"
//   File_Mup               L"\\Device\\Mup\\"
static const QString kRedirector     = QStringLiteral("\\Device\\LanmanRedirector\\");
static const QString kMupSemiRedir   = QStringLiteral("\\Device\\Mup\\;LanmanRedirector\\");
static const QString kDfsClientRedir = QStringLiteral("\\Device\\Mup\\DfsClient\\");
static const QString kHgfsRedir      = QStringLiteral("\\Device\\Mup\\;hgfs\\");
static const QString kMup            = QStringLiteral("\\Device\\Mup\\");

bool IsPathSeparator(QChar ch)
{
	return ch == '\\' || ch == '/';
}

bool IsFullPathPattern(const QString& pattern)
{
	return (!pattern.isEmpty() && IsPathSeparator(pattern.at(0))) ||
		(pattern.length() > 1 && pattern.at(1) == ':');
}

bool IsNtNamespacePath(const QString& path)
{
	return path.startsWith('\\') && !path.startsWith("\\\\");
}

int OrdinalWCharEqual(WCHAR leftChar, WCHAR rightChar)
{
	return CompareStringOrdinal(&leftChar, 1, &rightChar, 1, TRUE) == CSTR_EQUAL;
}

QString NormalizePath(QString path)
{
	path.replace('/', '\\');
	return path;
}

QString NormalizeLookupPath(QString path)
{
	path = NormalizePath(path);
	return path.toCaseFolded();
}

QString PathFileName(QString path)
{
	path = NormalizePath(path);
	while (path.endsWith('\\') || path.endsWith('/'))
		path.chop(1);

	const int separator = path.lastIndexOf('\\');
	return separator >= 0 ? path.mid(separator + 1) : path;
}

QString PathParent(QString path)
{
	path = NormalizePath(path);
	while (path.length() > 1 && path.endsWith('\\'))
		path.chop(1);

	const int separator = path.lastIndexOf('\\');
	if (separator < 0)
		return QString();
	if (separator == 0)
		return path.left(1);
	if (separator == 2 && path.length() > 1 && path.at(1) == ':')
		return path.left(3);
	return path.left(separator);
}

int SkipPathComponents(const QString& path, int start, int count)
{
	int pos = start;
	while (count > 0) {
		while (pos < path.length() && IsPathSeparator(path.at(pos)))
			++pos;
		if (pos >= path.length())
			return path.length();
		while (pos < path.length() && !IsPathSeparator(path.at(pos)))
			++pos;
		--count;
	}

	if (pos < path.length() && IsPathSeparator(path.at(pos)))
		++pos;
	return pos;
}

int RelativeMatchStart(const QString& path)
{
	if (path.length() > 1 && path.at(1) == ':')
		return path.length() > 2 && IsPathSeparator(path.at(2)) ? 3 : 2;

	if (path.length() > 1 && IsPathSeparator(path.at(0)) &&
		IsPathSeparator(path.at(1)))
		return SkipPathComponents(path, 2, 2);

	if (path.startsWith("\\??\\UNC\\", Qt::CaseInsensitive))
		return SkipPathComponents(path, 8, 2);

	int start = -1;
	if (path.startsWith(kRedirector, Qt::CaseInsensitive))
		start = kRedirector.length();
	else if (path.startsWith(kMupSemiRedir, Qt::CaseInsensitive))
		start = kMupSemiRedir.length();
	else if (path.startsWith(kDfsClientRedir, Qt::CaseInsensitive))
		start = kDfsClientRedir.length();
	else if (path.startsWith(kHgfsRedir, Qt::CaseInsensitive))
		return SkipPathComponents(path, kHgfsRedir.length(), 1);
	else if (path.startsWith(kMup, Qt::CaseInsensitive))
		start = kMup.length();
	else if (path.startsWith('\\'))
		return SkipPathComponents(path, 1, 2);
	else
		return 0;

	while (start < path.length() && path.at(start) == ';')
		start = SkipPathComponents(path, start, 1);
	return SkipPathComponents(path, start, 2);
}

bool FileMapContainsDiskPath(const QMap<QVariant, QVariantMap>& fileMap,
	const QString& diskPath)
{
	const QString lookupPath = NormalizeLookupPath(diskPath);
	for (auto it = fileMap.constBegin(); it != fileMap.constEnd(); ++it) {
		if (!it.key().canConvert<QString>())
			continue;
		if (NormalizeLookupPath(it.key().toString()) == lookupPath)
			return true;
	}
	return false;
}

QString NormalizeIgnorePattern(QString pattern)
{
	pattern = NormalizePath(pattern);
	if (pattern.endsWith('\\'))
		pattern.chop(1);
	return pattern;
}

QString GetDosPatternAlias(const QString& pattern, QString* ntPattern)
{
	if (!IsFullPathPattern(pattern))
		return QString();
	if ((pattern.length() > 1 && pattern.at(1) == ':') ||
		pattern.startsWith("\\\\"))
		return pattern;

	bool converted = false;
	QString dosPattern = NormalizePath(theAPI->Nt2DosPath(pattern, &converted));
	if (IsNtNamespacePath(dosPattern) &&
		dosPattern.compare(pattern, Qt::CaseInsensitive) != 0)
		*ntPattern = dosPattern;

	const QString dosDevicePrefix = "\\??\\";
	const QString uncDevicePrefix = "\\??\\UNC\\";
	if (dosPattern.startsWith(uncDevicePrefix, Qt::CaseInsensitive))
		dosPattern = "\\\\" + dosPattern.mid(uncDevicePrefix.length());
	else if (dosPattern.startsWith(dosDevicePrefix, Qt::CaseInsensitive))
		dosPattern.remove(0, dosDevicePrefix.length());

	if (dosPattern.startsWith(kRedirector, Qt::CaseInsensitive)) {
		QString uncPath = dosPattern.mid(kRedirector.length());
		while (uncPath.startsWith(';')) {
			const int separator = uncPath.indexOf('\\');
			if (separator == -1)
				return QString();
			uncPath.remove(0, separator + 1);
		}
		dosPattern = "\\\\" + uncPath;
	}

	if (converted || (dosPattern.length() > 1 && dosPattern.at(1) == ':') ||
		dosPattern.startsWith("\\\\"))
		return dosPattern;
	return QString();
}

QString ResolveNtCandidatePath(const QString& path)
{
	const QString normalizedPath = NormalizePath(path);
	if (IsNtNamespacePath(normalizedPath))
		return normalizedPath;

	int rootLength = 0;
	if (normalizedPath.length() > 1 && normalizedPath.at(1) == ':')
		rootLength = normalizedPath.length() > 2 &&
			IsPathSeparator(normalizedPath.at(2)) ? 3 : 2;
	else if (normalizedPath.startsWith("\\\\"))
		rootLength = SkipPathComponents(normalizedPath, 2, 2);
	else
		return QString();

	QString ntRoot = NormalizePath(
		theAPI->ResolveAbsolutePath(normalizedPath.left(rootLength)));
	if (!IsNtNamespacePath(ntRoot)) {
		const QString ntPath = NormalizePath(
			theAPI->ResolveAbsolutePath(normalizedPath));
		return IsNtNamespacePath(ntPath) ? ntPath : QString();
	}

	const QString suffix = normalizedPath.mid(rootLength);
	if (suffix.isEmpty())
		return ntRoot;
	if (!ntRoot.endsWith('\\'))
		ntRoot.append('\\');
	return ntRoot + suffix;
}

bool WildcardMatchWhole(const QString& pattern, const QString& text, bool relative,
	QVector<quint8>& rows)
{
	const size_t textLen = static_cast<size_t>(text.length());
	const size_t rowsRequired = (textLen + 1) * 2;
	rows.resize(static_cast<int>(rowsRequired));

	const size_t matchStart = relative
		? static_cast<size_t>(RelativeMatchStart(text))
		: static_cast<size_t>(0);

	const WCHAR* patternText = reinterpret_cast<const WCHAR*>(pattern.utf16());
	const WCHAR* candidateText = reinterpret_cast<const WCHAR*>(text.utf16());

	return RecoveryWildcardMatchWholeCore(
		patternText,
		candidateText,
		textLen,
		matchStart,
		reinterpret_cast<unsigned char*>(rows.data()),
		rowsRequired,
		OrdinalWCharEqual) != 0;
}

bool LiteralIgnoreMatch(const QString& pattern, const QString& text)
{
	const wchar_t* patternText = reinterpret_cast<const wchar_t*>(pattern.utf16());
	const wchar_t* candidateText = reinterpret_cast<const wchar_t*>(text.utf16());
	const size_t patternLength = static_cast<size_t>(pattern.length());

	if (_wcsnicmp(patternText, candidateText, patternLength) == 0) {
		if (text.length() == pattern.length() ||
			(text.length() > pattern.length() && text.at(pattern.length()) == '\\'))
			return true;
	}

	if (text.length() >= pattern.length()) {
		candidateText += text.length() - pattern.length();
		if (_wcsicmp(patternText, candidateText) == 0)
			return true;
	}

	return false;
}

bool IgnorePatternMatch(const QString& pattern, bool hasWildcard,
	const QString& text, bool relative, QVector<quint8>& rows)
{
	if (pattern.isEmpty() || text.isEmpty())
		return false;
	return hasWildcard
		? WildcardMatchWhole(pattern, text, relative, rows)
		: LiteralIgnoreMatch(pattern, text);
}

bool RecoverFolderPatternMatch(const QString& pattern, const QString& text,
	QVector<quint8>& rows)
{
	if (pattern.isEmpty() || text.isEmpty())
		return false;

	const bool relative = !IsFullPathPattern(pattern);
	if (WildcardMatchWhole(pattern, text, relative, rows))
		return true;

	const int matchStart = relative ? RelativeMatchStart(text) : 0;
	for (int end = text.length() - 1; end > matchStart; --end) {
		if (IsPathSeparator(text.at(end)) &&
			WildcardMatchWhole(pattern, text.left(end), relative, rows))
			return true;
	}

	return false;
}

}

bool CRecoveryWindow::IsFileIgnored(const CSandBoxPtr& pBox, const QString& diskPath, const QString& boxedPath)
{
	if (pBox.isNull())
		return false;
	if (!pBox->GetBool("UseAutoRecoverIgnoreForQuick", true, true, true))
		return false;

	const QString ntPath = ResolveNtCandidatePath(diskPath);
	const QList<SIgnorePattern> ignorePatterns = LoadIgnorePatterns(pBox);
	if (ignorePatterns.isEmpty())
		return false;

	QVector<quint8> scratch;
	return MatchIgnorePatterns(
		ignorePatterns, diskPath, ntPath, boxedPath, scratch);
}


CRecoveryWindow::CRecoveryWindow(const CSandBoxPtr& pBox, bool bImmediate, QWidget *parent)
	: QDialog(parent)
{
	m_bImmediate = bImmediate;
		
	Qt::WindowFlags flags = windowFlags();
	flags |= Qt::CustomizeWindowHint;
	//flags &= ~Qt::WindowContextHelpButtonHint;
	//flags &= ~Qt::WindowSystemMenuHint;
	flags |= Qt::WindowMinMaxButtonsHint;
	flags |= Qt::WindowMinimizeButtonHint;
	//flags &= ~Qt::WindowCloseButtonHint;
	setWindowFlags(flags);

	//setWindowState(Qt::WindowActive);
	SetForegroundWindow((HWND)QWidget::winId());

	bool bAlwaysOnTop = theGUI->IsAlwaysOnTop() || (bImmediate && theConf->GetBool("Options/RecoveryOnTop", true));
	this->setWindowFlag(Qt::WindowStaysOnTopHint, bAlwaysOnTop);

	if (!bAlwaysOnTop) {
		HWND hWnd = (HWND)this->winId();
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		QTimer::singleShot(100, this, [hWnd]() {
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		});
	}

	ui.setupUi(this);
	this->setWindowTitle(tr("%1 - File Recovery").arg(pBox->GetName()));

	FixTriStateBoxPallete(this);

	ui.treeFiles->setAlternatingRowColors(theConf->GetBool("Options/AltRowColors", false));

	m_pBox = pBox;

	m_pCounter = NULL;

	m_LastTargetIndex = 0;
	m_bTargetsChanged = false;
	m_bReloadPending = false;
	m_DeleteSnapshots = false;
	m_UnfilteredFileCount = 0;
	m_IgnoredFileCount = 0;

	m_UseIgnoreForQuick = true;
	m_IgnorePatternsBuilt = false;

	ui.chkShowAll->setTristate(m_bImmediate);
	ui.chkShowIgnored->setChecked(false);
	UpdateShowIgnoredState();

	QStyle* pStyle = QStyleFactory::create("windows");
	ui.treeFiles->setStyle(pStyle);
	ui.treeFiles->setItemDelegate(new CTreeItemDelegate());
	ui.treeFiles->setExpandsOnDoubleClick(false);

	ui.btnDeleteAll->setVisible(false);

	m_pFileModel = new CSimpleTreeModel(this);
	m_pFileModel->SetUseIcons(true);
	m_pFileModel->AddColumn(tr("File Name"), "FileName");
	m_pFileModel->AddColumn(tr("File Size"), "FileSize");
	m_pFileModel->AddColumn(tr("Full Path"), "DiskPath");

	m_pSortProxy = new CSortFilterProxyModel(this);
	m_pSortProxy->setSortRole(Qt::EditRole);
	m_pSortProxy->setSourceModel(m_pFileModel);
	m_pSortProxy->setDynamicSortFilter(true);

	//ui.treeFiles->setItemDelegate(theGUI->GetItemDelegate());

	ui.treeFiles->setModel(m_pSortProxy);

	ui.treeFiles->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.treeFiles->setSortingEnabled(true);
	//ui.treeFiles->setUniformRowHeights(true);

	CFinder* pFinder = new CFinder(m_pSortProxy, this);
	ui.gridLayout->addWidget(pFinder, 3, 0, 1, 5);
	pFinder->SetTree(ui.treeFiles);
	ui.finder->deleteLater(); // remove place holder

	//connect(ui.treeFiles, SIGNAL(clicked(const QModelIndex&)), this, SLOT(UpdateSnapshot(const QModelIndex&)));
	//connect(ui.treeFiles->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)), this, SLOT(UpdateSnapshot(const QModelIndex&)));
	connect(ui.treeFiles, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnRecover()));

	connect(ui.btnAddFolder, SIGNAL(clicked(bool)), this, SLOT(OnAddFolder()));
	connect(ui.chkShowAll, SIGNAL(stateChanged(int)), this, SLOT(OnShowAllChanged(int)));
	connect(ui.chkShowIgnored, SIGNAL(clicked(bool)), this, SLOT(FindFiles()));
	connect(ui.btnRefresh, SIGNAL(clicked(bool)), this, SLOT(FindFiles()));
	connect(ui.btnRecover, SIGNAL(clicked(bool)), this, SLOT(OnRecover()));
	connect(ui.btnDelete, SIGNAL(clicked(bool)), this, SLOT(OnDelete()));
	connect(ui.cmbRecover, SIGNAL(currentIndexChanged(int)), this, SLOT(OnTargetChanged()));
	connect(ui.btnDeleteAll, SIGNAL(clicked(bool)), this, SLOT(OnDeleteAll()));
	connect(ui.btnClose, SIGNAL(clicked(bool)), this, SLOT(close()));

	QMenu* pRecMenu = new QMenu(ui.btnRecover);
	m_pRemember = pRecMenu->addAction(tr("Remember target selection"));
	m_pRemember->setCheckable(true);
	ui.btnRecover->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnRecover->setMenu(pRecMenu);

	QMenu* pDelMenu = new QMenu(ui.btnDeleteAll);
	pDelMenu->addAction(tr("Delete everything, including all snapshots"), this, SLOT(OnDeleteEverything()));
	ui.btnDeleteAll->setPopupMode(QToolButton::MenuButtonPopup);
	ui.btnDeleteAll->setMenu(pDelMenu);

	restoreGeometry(theConf->GetBlob("RecoveryWindow/Window_Geometry"));

	QByteArray Columns = theConf->GetBlob("RecoveryWindow/TreeView_Columns");
	if (!Columns.isEmpty())
		ui.treeFiles->header()->restoreState(Columns);

	for (int i = 0; i < m_pFileModel->columnCount(); i++)
		m_pFileModel->SetColumnEnabled(i, true);


	foreach(const QString& NtFolder, m_pBox->GetTextList("RecoverFolder", true, true, true))
	{
		// Wildcard entries are not directory scan roots.
		if (NtFolder.contains('*') || NtFolder.contains('?')) {
			m_WildcardRecoverFolders.append(NormalizeIgnorePattern(NtFolder));
			continue;
		}

		QString RecFolder = theAPI->ResolveAbsolutePath(NtFolder);

		bool bOk;
		QString Folder = theAPI->Nt2DosPath(RecFolder, &bOk);
		if(!bOk && RecFolder.left(11).compare("\\Device\\Mup", Qt::CaseInsensitive) == 0)
			Folder = "\\" + RecFolder.mid(11);

		if (!Folder.isEmpty() && (bOk ||
			RecFolder.left(11).compare("\\Device\\Mup", Qt::CaseInsensitive) == 0)) {
			m_RecoveryFolders.append(Folder);
			m_RecoveryNtFolders.insert(Folder,
				IsNtNamespacePath(RecFolder) ? NormalizePath(RecFolder) :
				ResolveNtCandidatePath(Folder));
		}
	}

	ui.cmbRecover->addItem(tr("Original location"), 0);
	ui.cmbRecover->addItem(tr("Browse for location"), 1);
	ui.cmbRecover->addItem(tr("Clear folder list"), -1);

	QStringList RecoverTargets = theAPI->GetUserSettings()->GetTextList("SbieCtrl_RecoverTarget", true);
	ui.cmbRecover->insertItems(ui.cmbRecover->count()-1, RecoverTargets);

	m_LastTargetIndex = theConf->GetInt("RecoveryWindow/LastTarget", -1);
	m_pRemember->setChecked(m_LastTargetIndex != -1);
	if (m_LastTargetIndex == -1)
		m_LastTargetIndex = 0;
	ui.cmbRecover->setCurrentIndex(m_LastTargetIndex);
}

CRecoveryWindow::~CRecoveryWindow()
{
	theConf->SetBlob("RecoveryWindow/Window_Geometry",saveGeometry());

	theConf->SetBlob("RecoveryWindow/TreeView_Columns", ui.treeFiles->header()->saveState());
}

int	CRecoveryWindow::exec()
{
	//QDialog::setWindowModality(Qt::WindowModal);
	ui.btnDeleteAll->setVisible(true);
	CSandMan::SafeShow(this);
	return QDialog::exec();
}

bool CRecoveryWindow::IsDeleteDialog() const
{
	return ui.btnDeleteAll->isVisible();
}

void CRecoveryWindow::closeEvent(QCloseEvent *e)
{
	emit Closed();
	this->deleteLater();
}

void CRecoveryWindow::OnAddFolder()
{
	QString Folder = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
	if (Folder.isEmpty())
		return;

	if (m_RecoveryFolders.contains(Folder))
		return;

	m_RecoveryFolders.append(Folder);
	m_RecoveryNtFolders.insert(Folder, ResolveNtCandidatePath(Folder));
	m_pBox->AppendText("RecoverFolder", Folder);

	FindFiles(Folder);

	m_pFileModel->Sync(m_FileMap);
	ui.treeFiles->expandAll();
}

void CRecoveryWindow::OnTargetChanged()
{
	int op = ui.cmbRecover->currentData().toInt();
	if (op == 1)
	{
		QString Folder = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
		if (Folder.isEmpty()) {
			ui.cmbRecover->setCurrentIndex(m_LastTargetIndex);
			return;
		}
		m_LastTargetIndex = ui.cmbRecover->count() - 1;
		ui.cmbRecover->insertItem(m_LastTargetIndex, Folder);
		ui.cmbRecover->setCurrentIndex(m_LastTargetIndex);
		m_bTargetsChanged = true;
	}
	else if (op == -1)
	{
		while (ui.cmbRecover->count() > 3)
			ui.cmbRecover->removeItem(2);
		ui.cmbRecover->setCurrentIndex(0);
		m_bTargetsChanged = true;
	}
	else {
		m_LastTargetIndex = ui.cmbRecover->currentIndex();
	}
}

void CRecoveryWindow::OnRecover()
{ 
	if (m_bTargetsChanged) {
		QStringList RecoverTargets;
		for (int i = 2; i < ui.cmbRecover->count() - 1; i++)
			RecoverTargets.append(ui.cmbRecover->itemText(i));
		theAPI->GetUserSettings()->UpdateTextList("SbieCtrl_RecoverTarget", RecoverTargets, true);
	}

	theConf->SetValue("RecoveryWindow/LastTarget", m_pRemember->isChecked() ? m_LastTargetIndex : -1);

	QString RecoveryFolder;
	if (ui.cmbRecover->currentIndex() > 0)
		RecoveryFolder = ui.cmbRecover->currentText();

	RecoverFiles(false, RecoveryFolder); 
}

void CRecoveryWindow::OnDelete()
{
	QMap<QString, SRecItem> FileMap = GetFiles();

	if (FileMap.isEmpty()) {
		QMessageBox::information(this, "Sandboxie-Plus", tr("No Files selected!"), QMessageBox::Ok);
		return;
	}

	if (QMessageBox::question(this, "Sandboxie-Plus", tr("Do you really want to delete %1 selected files?").arg(FileMap.count()), QMessageBox::Yes, QMessageBox::No | QMessageBox::Default | QMessageBox::Escape, QMessageBox::NoButton) != QMessageBox::Yes)
		return;

	foreach(const QString & FilePath, FileMap.keys())
		QFile::remove(FilePath);

	FindFiles();
}

void CRecoveryWindow::OnDeleteAll()
{
	this->setResult(1);
	this->close();
}

void CRecoveryWindow::OnDeleteEverything()
{
	m_DeleteSnapshots = true;
	OnDeleteAll();
}

void CRecoveryWindow::OnShowAllChanged(int state)
{
	Q_UNUSED(state);
	UpdateShowIgnoredState();
	FindFiles();
}

void CRecoveryWindow::AddFile(const QString& FilePath, const QString& BoxPath)
{
	if (m_bImmediate)
		ui.chkShowAll->setTristate(true);

	// Resolve the real (host-visible) path from BoxPath so it matches what
	// FindFiles produces via GetRealPath. The raw FilePath may arrive in NT
	// form (e.g. \Device\LanmanRedirector\...) while FindFiles yields UNC
	// form (\\server\share\...), causing m_NewFiles.contains() to fail and
	// the immediate-recovery window to close immediately.
	QString RealFilePath;
	if (!BoxPath.isEmpty())
		RealFilePath = theAPI->GetRealPath(m_pBox.data(), BoxPath);
	if (RealFilePath.isEmpty())
		RealFilePath = FilePath;

	const QString lookupPath = NormalizeLookupPath(RealFilePath);
	m_NewFiles.insert(lookupPath);
	m_NewFilePaths.insert(lookupPath, RealFilePath);
	if (!BoxPath.isEmpty())
		m_NewFileBoxPaths.insert(lookupPath, BoxPath);

	if (m_FileMap.isEmpty()) {
		const bool hadSignalsBlocked = ui.chkShowAll->blockSignals(true);
		ui.chkShowAll->setCheckState(Qt::PartiallyChecked);
		ui.chkShowAll->blockSignals(hadSignalsBlocked);
		UpdateShowIgnoredState();

		QMenu* pCloseMenu = new QMenu(ui.btnClose);
		pCloseMenu->addAction(tr("Close until all programs stop in this box"), this, SLOT(OnCloseUntil()));
		pCloseMenu->addAction(tr("Close and Disable Immediate Recovery for this box"), this, SLOT(OnAutoDisable()));
		ui.btnClose->setPopupMode(QToolButton::MenuButtonPopup);
		ui.btnClose->setMenu(pCloseMenu);
	}

	if (m_FileMap.isEmpty())
		FindFiles();
	else if (!m_bReloadPending)
	{
		m_bReloadPending = true;
		QTimer::singleShot(500, this, SLOT(FindFiles()));
	}
}

static bool AddFileToModel(
	QMap<QVariant, QVariantMap>& fileMap, QFileIconProvider& iconProvider,
	const QString& diskPath, const QString& boxPath)
{
	QFileInfo boxInfo(boxPath);
	if (!boxInfo.exists())
		return false;

	const QString parentPath = PathParent(diskPath);
	const QString parentBoxPath = PathParent(boxPath);
	if (!parentPath.isEmpty() && !fileMap.contains(parentPath)) {
		QVariantMap RecFolder;
		RecFolder["ID"] = parentPath;
		RecFolder["ParentID"] = QString();
		RecFolder["FileName"] = parentPath;
		RecFolder["FileSize"] = FormatSize(0);
		RecFolder["DiskPath"] = parentPath;
		RecFolder["BoxPath"] = parentBoxPath;
		RecFolder["Icon"] = iconProvider.icon(QFileIconProvider::Folder);
		RecFolder["IsDir"] = true;
		fileMap.insert(parentPath, RecFolder);
	}

	QVariantMap RecFile;
	RecFile["ID"] = diskPath;
	RecFile["ParentID"] = parentPath;
	RecFile["FileName"] = PathFileName(diskPath);
	RecFile["FileSize"] = FormatSize(boxInfo.size());
	RecFile["DiskPath"] = diskPath;
	RecFile["BoxPath"] = boxPath;
	RecFile["Icon"] = iconProvider.icon(boxInfo);
	RecFile["IsDir"] = false;
	fileMap.insert(diskPath, RecFile);
	return true;
}

int CRecoveryWindow::FindFiles()
{
	m_bReloadPending = false;
	m_IgnoredFileCount = 0;
	ReloadIgnoreSettings();
	if (!m_NewFiles.isEmpty()) {
		ui.lblInfo->setText(tr("There are %1 new files available to recover.").arg(m_NewFiles.count()));
	}
	else if (m_pCounter == NULL) {
		m_pCounter = new CRecoveryCounter(m_pBox->GetFileRoot(), this);
		connect(m_pCounter, SIGNAL(Count(quint32, quint32, quint64)), this, SLOT(OnCount(quint32, quint32, quint64)));
	}

	m_FileMap.clear();
	int Count = 0;
	int UnfilteredCount = 0;

	if (ui.chkShowAll->checkState() == Qt::Checked)
	{
		//for(char drive = 'A'; drive <= 'Z'; drive++)
		QDir Dir(m_pBox->GetFileRoot() + "\\drive\\");
		foreach(const QFileInfo & Info, Dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
			Count += FindBoxFiles("\\drive\\" + Info.fileName(), &UnfilteredCount);

		if (m_pBox->GetBool("SeparateUserFolders", true, true)) {
			Count += FindBoxFiles("\\user\\current", &UnfilteredCount);
			Count += FindBoxFiles("\\user\\all", &UnfilteredCount);
			Count += FindBoxFiles("\\user\\public", &UnfilteredCount);
		}

		//Count += FindBoxFiles("\\share");
		QDir DirSvr(m_pBox->GetFileRoot() + "\\share\\");
		foreach(const QFileInfo & InfoSrv, DirSvr.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
			QDir DirPub(m_pBox->GetFileRoot() + "\\share\\" + InfoSrv.fileName());
			foreach(const QFileInfo & InfoPub, DirPub.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
				Count += FindBoxFiles("\\share\\" + InfoSrv.fileName() + "\\" +
					InfoPub.fileName(), &UnfilteredCount);
		}
	}
	else
	{
		foreach(const QString & Folder, m_RecoveryFolders)
			Count += FindFiles(Folder, &UnfilteredCount);
	}
	if (!m_WildcardRecoverFolders.isEmpty() &&
		ui.chkShowAll->checkState() != Qt::Checked) {
		if (m_bImmediate &&
			ui.chkShowAll->checkState() == Qt::PartiallyChecked) {
			foreach (const QString& path, m_NewFiles) {
				const QString diskPath = m_NewFilePaths.value(path);
				if (FileMapContainsDiskPath(m_FileMap, diskPath))
					continue;
				const QString boxPath = m_NewFileBoxPaths.value(path);
				if (!boxPath.isEmpty() &&
					AddFileToModel(m_FileMap, m_IconProvider, diskPath, boxPath)) {
					++Count;
					++UnfilteredCount;
				}
			}
		}
		else
			Count += FindWildcardRecoverFiles(&UnfilteredCount);
	}
	m_UnfilteredFileCount = UnfilteredCount;

	if (m_bImmediate && m_FileMap.isEmpty())
		this->close();

	m_pFileModel->Sync(m_FileMap);
	ui.treeFiles->expandAll();
	UpdateShowIgnoredState();
	
	if(m_bImmediate)
		SelectFiles();

	return Count;
}

void CRecoveryWindow::SelectFiles()
{
	//QModelIndex Index = m_pFileModel->index(0, 0);

	QModelIndex Index;
	for (int i = 0; i < m_pFileModel->rowCount(); ++i)
	{
		QModelIndex ModelIndex = m_pFileModel->index(i, 0);
		QVariant ID = m_pFileModel->GetItemID(ModelIndex);
		//QVariant ID = m_pFileModel->GetItemID(Index);

		QVariantMap File = m_FileMap.value(ID);
		if (File.isEmpty())
			continue;

		if (File["IsDir"].toBool() == false)
		{
			Index = ModelIndex;
			goto SelectFile;
		}
		else
		{
			QList<QModelIndex> Folders;
			Folders.append(ModelIndex);
			do
			{
				QModelIndex CurIndex = Folders.takeFirst();
				for (int i = 0; i < m_pFileModel->rowCount(CurIndex); i++)
				{
					QModelIndex ChildIndex = m_pFileModel->index(i, 0, CurIndex);

					QVariant ChildID = m_pFileModel->GetItemID(ChildIndex);
					QVariantMap File = m_FileMap.value(ChildID);
					if (File.isEmpty())
						continue;

					if (File["IsDir"].toBool() == false) 
					{
						Index = ChildIndex;
						goto SelectFile;
					}
					else
						Folders.append(ChildIndex);
				}
			} while (!Folders.isEmpty());
		}
	}

SelectFile:
	if (Index.isValid()) {
		QModelIndex ModelIndex = m_pSortProxy->mapFromSource(Index);
		ui.treeFiles->selectionModel()->setCurrentIndex(ModelIndex, QItemSelectionModel::SelectCurrent);
		ui.treeFiles->setCurrentIndex(ModelIndex);
		ui.treeFiles->setFocus();
	}
}

void CRecoveryWindow::ReloadIgnoreSettings()
{
	m_UseIgnoreForQuick = m_pBox->GetBool(
		"UseAutoRecoverIgnoreForQuick", true, true, true);
	m_IgnorePatternsBuilt = false;
	m_AutoRecoverIgnorePatterns.clear();
	m_IgnoreMatchScratch.clear();

	if (ShouldFilterIgnoredFiles())
		BuildIgnorePatterns();
}

void CRecoveryWindow::UpdateShowIgnoredState()
{
	const bool enableShowIgnored =
		(ui.chkShowAll->checkState() == Qt::Unchecked) &&
		(m_IgnoredFileCount > 0 || ui.chkShowIgnored->isChecked());
	ui.chkShowIgnored->setEnabled(enableShowIgnored);

	if (enableShowIgnored && m_IgnoredFileCount > 0)
		ui.chkShowIgnored->setToolTip(
			tr("%1 file(s) hidden by AutoRecoverIgnore").arg(m_IgnoredFileCount));
	else
		ui.chkShowIgnored->setToolTip(QString());
}

bool CRecoveryWindow::ShouldFilterIgnoredFiles() const
{
	if (!m_UseIgnoreForQuick)
		return false;
	if (ui.chkShowAll->checkState() != Qt::Unchecked)
		return false;
	if (ui.chkShowIgnored->isChecked())
		return false;
	return true;
}

bool CRecoveryWindow::IsInWildcardRecoverFolders(const QString& diskPath,
	const QString& ntPath, const QString& boxedPath)
{
	QVector<quint8> scratch;

	foreach (const QString& pattern, m_WildcardRecoverFolders) {
		if (RecoverFolderPatternMatch(pattern, diskPath, scratch) ||
			RecoverFolderPatternMatch(pattern, ntPath, scratch))
			return true;

		QString ntPattern;
		const QString dosPattern = GetDosPatternAlias(pattern, &ntPattern);
		if ((!ntPattern.isEmpty() &&
				RecoverFolderPatternMatch(ntPattern, ntPath, scratch)) ||
			(!dosPattern.isEmpty() &&
				RecoverFolderPatternMatch(dosPattern, diskPath, scratch)))
			return true;

		const QString boxedSource = dosPattern.isEmpty() ? pattern : dosPattern;
		const QString boxedPattern = theAPI->GetBoxedPath(m_pBox.data(), boxedSource);
		if (RecoverFolderPatternMatch(boxedPattern, boxedPath, scratch))
			return true;
	}

	return false;
}

void CRecoveryWindow::BuildIgnorePatterns()
{
	if (m_IgnorePatternsBuilt)
		return;

	m_AutoRecoverIgnorePatterns = LoadIgnorePatterns(m_pBox);
	m_IgnorePatternsBuilt = true;
}

bool CRecoveryWindow::IsExcludedByIgnorePatterns(const QString& diskPath,
	const QString& ntPath, const QString& boxedPath)
{
	return MatchIgnorePatterns(
		m_AutoRecoverIgnorePatterns, diskPath, ntPath,
		boxedPath, m_IgnoreMatchScratch);
}

QList<CRecoveryWindow::SIgnorePattern> CRecoveryWindow::LoadIgnorePatterns(
	const CSandBoxPtr& pBox)
{
	QList<SIgnorePattern> ignorePatterns;
	if (pBox.isNull())
		return ignorePatterns;

	const QStringList patterns = pBox->GetTextList(
		"AutoRecoverIgnore", true, true, true);
	for (const QString& configuredPattern : patterns) {
		SIgnorePattern pattern;
		pattern.Pattern = NormalizeIgnorePattern(configuredPattern);
		if (pattern.Pattern.isEmpty())
			continue;

		pattern.HasWildcard =
			pattern.Pattern.contains('*') || pattern.Pattern.contains('?');
		pattern.FullPath = IsFullPathPattern(pattern.Pattern);
		if (pattern.FullPath) {
			pattern.DosPattern =
				GetDosPatternAlias(pattern.Pattern, &pattern.NtPattern);
			const QString& boxedSource = pattern.DosPattern.isEmpty()
				? pattern.Pattern : pattern.DosPattern;
			pattern.BoxedPattern = theAPI->GetBoxedPath(pBox.data(), boxedSource);
		}

		ignorePatterns.append(pattern);
	}

	return ignorePatterns;
}

bool CRecoveryWindow::MatchIgnorePatterns(
	const QList<SIgnorePattern>& patterns, const QString& diskPath,
	const QString& ntPath, const QString& boxedPath, QVector<quint8>& scratch)
{
	for (const SIgnorePattern& pattern : patterns) {
		const bool relative = !pattern.FullPath;
		if (IgnorePatternMatch(pattern.Pattern, pattern.HasWildcard,
				diskPath, relative, scratch) ||
			IgnorePatternMatch(pattern.Pattern, pattern.HasWildcard,
				ntPath, relative, scratch) ||
			(!pattern.NtPattern.isEmpty() && IgnorePatternMatch(
				pattern.NtPattern, pattern.HasWildcard,
				ntPath, relative, scratch)))
			return true;

		if (pattern.FullPath) {
			if (IgnorePatternMatch(pattern.DosPattern, pattern.HasWildcard,
					diskPath, false, scratch) ||
				IgnorePatternMatch(pattern.BoxedPattern, pattern.HasWildcard,
					boxedPath, false, scratch))
				return true;
		}
	}
	return false;
}

int CRecoveryWindow::FindFiles(const QString& Folder, int* pUnfilteredCount)
{
	//int Count = 0;
	//foreach(const QString & Path, theAPI->GetBoxedPath(m_pBox, Folder))
	//	Count += FindFiles(Folder, Path, Folder);
	//return Count;
	return FindFiles(theAPI->GetBoxedPath(m_pBox.data(), Folder), Folder,
		m_RecoveryNtFolders.value(Folder), Folder, QString(),
		pUnfilteredCount).first;
}

int CRecoveryWindow::FindBoxFiles(const QString& Folder, int* pUnfilteredCount,
	bool bWildcardOnly)
{
	QString RealFolder = theAPI->GetRealPath(m_pBox.data(), m_pBox->GetFileRoot() + Folder);
	if (RealFolder.isEmpty())
		return 0;
	return FindFiles(m_pBox->GetFileRoot() + Folder, RealFolder,
		ResolveNtCandidatePath(RealFolder), RealFolder, QString(),
		pUnfilteredCount, bWildcardOnly).first;
}

int CRecoveryWindow::FindWildcardRecoverFiles(int* pUnfilteredCount)
{
	int Count = 0;

	QDir Dir(m_pBox->GetFileRoot() + "\\drive\\");
	foreach(const QFileInfo & Info, Dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
		Count += FindBoxFiles("\\drive\\" + Info.fileName(), pUnfilteredCount, true);

	if (m_pBox->GetBool("SeparateUserFolders", true, true)) {
		Count += FindBoxFiles("\\user\\current", pUnfilteredCount, true);
		Count += FindBoxFiles("\\user\\all", pUnfilteredCount, true);
		Count += FindBoxFiles("\\user\\public", pUnfilteredCount, true);
	}

	QDir DirSvr(m_pBox->GetFileRoot() + "\\share\\");
	foreach(const QFileInfo & InfoSrv, DirSvr.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
		QDir DirPub(m_pBox->GetFileRoot() + "\\share\\" + InfoSrv.fileName());
		foreach(const QFileInfo & InfoPub, DirPub.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
			Count += FindBoxFiles("\\share\\" + InfoSrv.fileName() + "\\" +
				InfoPub.fileName(), pUnfilteredCount, true);
	}

	return Count;
}

QPair<int, quint64>	CRecoveryWindow::FindFiles(const QString& BoxedFolder,
	const QString& RealFolder, const QString& NtFolder, const QString& Name,
	const QString& ParentID, int* pUnfilteredCount, bool bWildcardOnly)
{
	int Count = 0;
	quint64 Size = 0;

	QDir Dir(BoxedFolder);
	foreach(const QFileInfo& Info, Dir.entryInfoList(QDir::AllEntries))
	{
		QString Name = Info.fileName();
		if (Name == "." || Name == "..")
			continue;
		QString Path = Info.filePath().replace("/", "\\");

		if (Info.isFile())
		{
			QString RealPath = RealFolder + Path.mid(BoxedFolder.length());
			QString NtPath;
			if (!NtFolder.isEmpty())
				NtPath = NtFolder + Path.mid(BoxedFolder.length());

			if (bWildcardOnly && (m_FileMap.contains(RealPath) ||
					!IsInWildcardRecoverFolders(RealPath, NtPath, Path)))
				continue;

			if (pUnfilteredCount)
				++*pUnfilteredCount;

			if (ShouldFilterIgnoredFiles()) {
				BuildIgnorePatterns();
				if (IsExcludedByIgnorePatterns(RealPath, NtPath, Path)) {
					++m_IgnoredFileCount;
					continue;
				}
			}

			if (m_bImmediate && ui.chkShowAll->checkState() == Qt::PartiallyChecked &&
				!m_NewFiles.contains(NormalizeLookupPath(RealPath)))
				continue;

			Count++;
			Size += Info.size();

			QVariantMap RecFile;
			RecFile["ID"] = RealPath;
			RecFile["ParentID"] = RealFolder;
			RecFile["FileName"] = Name;
			RecFile["FileSize"] = FormatSize(Info.size());
			RecFile["DiskPath"] = RealPath;
			RecFile["BoxPath"] = Path;
			RecFile["Icon"] = m_IconProvider.icon(Info);
			RecFile["IsDir"] = false;
			m_FileMap.insert(RealPath, RecFile);
		}
		else
		{
			const QString NtPath = NtFolder.isEmpty()
				? QString() : NtFolder + "\\" + Name;
			auto CountSize = FindFiles(Path, RealFolder + "\\" + Name,
				NtPath, Name, RealFolder, pUnfilteredCount, bWildcardOnly);
			Count += CountSize.first;
			Size += CountSize.second;
		}
	}

	if (Count > 0) 
	{
		QVariantMap RecFolder;
		RecFolder["ID"] = RealFolder;
		RecFolder["ParentID"] = ParentID;
		RecFolder["FileName"] = Name;
		RecFolder["FileSize"] = FormatSize(Size);
		RecFolder["DiskPath"] = RealFolder;
		RecFolder["BoxPath"] = BoxedFolder;
		RecFolder["Icon"] = m_IconProvider.icon(QFileIconProvider::Folder);
		RecFolder["IsDir"] = true;
		m_FileMap.insert(RealFolder, RecFolder);
	}

	return qMakePair(Count, Size);
}

QMap<QString, CRecoveryWindow::SRecItem> CRecoveryWindow::GetFiles()
{
	//bool HasShare = false;
	QMap<QString, SRecItem> FileMap;
	foreach(const QModelIndex& Index, ui.treeFiles->selectionModel()->selectedIndexes())
	{
		QModelIndex ModelIndex = m_pSortProxy->mapToSource(Index);
		QVariant ID = m_pFileModel->GetItemID(ModelIndex);
		//QVariant ID = m_pFileModel->GetItemID(Index);

		QVariantMap File = m_FileMap.value(ID);
		if (File.isEmpty())
			continue;

		if (File["IsDir"].toBool() == false)
		{
			//if (File["DiskPath"].toString().indexOf("\\device\\mup", 0, Qt::CaseInsensitive) == 0)
			//	HasShare = true;
			QString CurPath = File["DiskPath"].toString();;
			FileMap[File["BoxPath"].toString()].FullPath = CurPath;
			FileMap[File["BoxPath"].toString()].RelPath = CurPath.mid(CurPath.lastIndexOf("\\"));
		}
		else
		{
			QString DirPath = File["DiskPath"].toString();
			//if(ModelIndex.parent().isValid())
			//	DirPath = Split2(DirPath, "\\", true).first;

			QList<QModelIndex> Folders;
			Folders.append(ModelIndex);
			do
			{
				QModelIndex CurIndex = Folders.takeFirst();
				for (int i = 0; i < m_pFileModel->rowCount(CurIndex); i++)
				{
					QModelIndex ChildIndex = m_pFileModel->index(i, 0, CurIndex);

					QVariant ChildID = m_pFileModel->GetItemID(ChildIndex);
					QVariantMap File = m_FileMap.value(ChildID);
					if (File.isEmpty())
						continue;

					if (File["IsDir"].toBool() == false) 
					{
						//if (File["DiskPath"].toString().indexOf("\\device\\mup") == 0)
						//	HasShare = true;
						QString CurPath = File["DiskPath"].toString();
						FileMap[File["BoxPath"].toString()].FullPath = CurPath;

						QString RelPath = CurPath.mid(Split2(DirPath, "\\", true).first.length());
						if (RelPath.length() > FileMap[File["BoxPath"].toString()].RelPath.length())
							FileMap[File["BoxPath"].toString()].RelPath = RelPath;
					}
					else
						Folders.append(ChildIndex);
				}
			} while (!Folders.isEmpty());
		}
	}

	return FileMap;
}

void CRecoveryWindow::RecoverFiles(bool bBrowse, QString RecoveryFolder)
{
	QMap<QString, SRecItem> FileMap = GetFiles();

	/*if (HasShare && !bBrowse) {
		QMessageBox::warning(this, "Sandboxie-Plus", tr("One or more selected files are located on a network share, and must be recovered to a local drive, please select a folder to recover all selected files to."));
		bBrowse = true;
	}*/

	if (bBrowse && RecoveryFolder.isEmpty()) {
		RecoveryFolder = QFileDialog::getExistingDirectory(this, tr("Select Directory")).replace("/", "\\");
		if (RecoveryFolder.isEmpty())
			return;
		
		QStringList RecoverTargets = theAPI->GetUserSettings()->GetTextList("SbieCtrl_RecoverTarget", true);
		if(!RecoverTargets.contains(RecoveryFolder))
			theAPI->GetUserSettings()->UpdateTextList("SbieCtrl_RecoverTarget", RecoverTargets, true);
	}


	QList<QPair<QString, QString>> FileList;
	for(QMap<QString, SRecItem>::const_iterator I = FileMap.begin(); I != FileMap.end(); ++I)
	{
		QString BoxedFilePath = I.key();
		QString RecoveryPath = I.value().FullPath;
		if (!RecoveryFolder.isEmpty())
		{
			//QString FileName = RecoveryPath.mid(RecoveryPath.lastIndexOf("\\") + 1);
			//RecoveryPath = RecoveryFolder + "\\" + FileName;
			RecoveryPath = RecoveryFolder + I.value().RelPath;
		}

		FileList.append(qMakePair(BoxedFilePath, RecoveryPath));
	}


	SB_PROGRESS Status = theGUI->RecoverFiles(m_pBox->GetName(), FileList, this);
	if (Status.GetStatus() == OP_ASYNC)
	{
		connect(Status.GetValue().data(), SIGNAL(Finished()), this, SLOT(FindFiles()));
		theGUI->AddAsyncOp(Status.GetValue(), false, tr("Recovering File(s)..."), this);
	}
}

void CRecoveryWindow::OnCount(quint32 fileCount, quint32 folderCount, quint64 totalSize)
{
	ui.lblInfo->setText(tr("There are %1 files and %2 folders in the sandbox, occupying %3 of disk space.").arg(fileCount).arg(folderCount).arg(FormatSize(totalSize)));
	m_pCounter->deleteLater();
	m_pCounter = NULL;
}

void CRecoveryWindow::OnCloseUntil()
{
	if (m_pBox->GetActiveProcessCount() > 0)
		m_pBox.objectCast<CSandBoxPlus>()->SetSuspendRecovery();
	close();
}

void CRecoveryWindow::OnAutoDisable()
{
	m_pBox.objectCast<CSandBoxPlus>()->SetSuspendRecovery();
	m_pBox->SetBoolSafe("AutoRecover", false);
	close();
}

void CRecoveryCounter::run()
{
	quint32 fileCount = 0;
	quint32 folderCount = 0;
	quint64 totalSize = 0;

	QStringList Folders;
	Folders.append(m_BoxPath);
	do {
		if (!m_run) break;

		QDir Dir(Folders.takeFirst());
		foreach(const QFileInfo & Info, Dir.entryInfoList(QDir::AllEntries))
		{
			if (!m_run) break;

			QString Name = Info.fileName();
			if (Name == "." || Name == "..")
				continue;
			QString Path = Info.filePath().replace("/", "\\");
			if (Info.isFile())
			{
				fileCount++;
				totalSize += Info.size();
			}
			else
			{
				Folders.append(Path);

				folderCount++;
			}
		}

	} while (!Folders.isEmpty());

	emit Count(fileCount, folderCount, totalSize);
}