#include "stdafx.h"
#include "EditorSettingsWindow.h"
#include "../SandMan.h"
#include "../../MiscHelpers/Common/Settings.h"
#include "../../MiscHelpers/Common/CodeEdit.h"
#include "../Helpers/IniHighlighter.h"

// Editor setting metadata structure for DRY code (renamed to avoid conflict with IniHighlighter.h)
struct EditorSettingInfo {
	const char* configKey;          // Config key name (e.g., "Options/ValidateIniKeys")
	const char* displayName;        // Display name in table
	bool isTriState;                // True if tri-state checkbox, false if binary
	int defaultValue;               // Default value: 0=Unchecked, 1=PartiallyChecked, 2=Checked
	const char* descUnchecked;      // Description for unchecked state
	const char* descPartial;        // Description for partial state ("-" if not applicable)
	const char* descChecked;        // Description for checked state
};

// Define all settings in one place - this is the single source of truth
#define SETTING_COUNT 6

// Get setting metadata - single source of truth for all settings
// This is a file-scope function with internal linkage
static EditorSettingInfo GetSettingInfo(int index)
{
	static const EditorSettingInfo settings[SETTING_COUNT] = {
		// ValidateIniKeys
		{"Options/ValidateIniKeys", QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Validate INI Keys"), false, 2, 
		 QT_TRANSLATE_NOOP("CEditorSettingsWindow", "No validation"), "-", QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Full validation (default)")},
		// EnableIniTooltips
		{"Options/EnableIniTooltips", QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Enable INI Tooltips"), true, 1,
		 QT_TRANSLATE_NOOP("CEditorSettingsWindow", "No tooltips"), QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Basic info (default)"), QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Full details")},
		// EnablePopupTooltips
		{"Options/EnablePopupTooltips", QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Enable Popup Tooltips"), true, 1,
		 QT_TRANSLATE_NOOP("CEditorSettingsWindow", "No tooltips"), QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Basic info (default)"), QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Full details")},
		// AutoCompletionConsent
		{"Options/AutoCompletionConsent", QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Auto Completion Consent"), false, 0,
		 QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Not consented (default)"), "-", QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Consented")},
		// EnableAutoCompletion
		{"Options/EnableAutoCompletion", QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Enable Auto Completion"), true, 0,
		 QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Disabled (default)"), QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Manual"), QT_TRANSLATE_NOOP("CEditorSettingsWindow", "While Typing")},
		// EnableFuzzyMatching
		{"Options/EnableFuzzyMatching", QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Enable Fuzzy Matching"), false, 0,
		 QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Disabled (default)"), "-", QT_TRANSLATE_NOOP("CEditorSettingsWindow", "Enabled")}
	};
	
	return (index >= 0 && index < SETTING_COUNT) ? settings[index] : settings[0];
}

CEditorSettingsWindow::CEditorSettingsWindow(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	setWindowTitle(tr("Editor Settings"));
	
	// Initialize all reset flags to false using a loop
	m_anyResetOccurred = false;
	m_wasValidateIniKeysReset = false;
	m_wasEnableIniTooltipsReset = false;
	m_wasEnableAutoCompletionReset = false;
	m_previousConsentState = false;
	
	for (int i = 0; i < SETTING_COUNT; ++i) {
		bool* resetFlag = GetResetFlagByIndex(i);
		if (resetFlag) *resetFlag = false;
	}
	
	InitializeTable();
	LoadSettings();
	StoreOriginalValues();
	UpdateTable();

	// Set Reset All button text and tooltip from code so translations are sourced here
	ui.btnResetAll->setText(tr("Reset All to Defaults"));
	ui.btnResetAll->setToolTip(tr("Reset all settings to their default values and remove custom config entries."));
	
	// Initialize previous consent state after loading settings
	m_previousConsentState = chkAutoCompletionConsent->isChecked();
	
	// Initial highlighting update after everything is set up
	OnSettingChanged();
	
	// Update dependencies (disable dependent settings if prerequisites not met)
	UpdateDependencies();

	// Note: QDialogButtonBox automatically connects accepted() -> accept() and rejected() -> reject()
	// No need to manually connect these signals
	// connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	// connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
	connect(ui.btnResetAll, SIGNAL(clicked()), this, SLOT(OnResetAll()));
}

CEditorSettingsWindow::~CEditorSettingsWindow()
{
}

// Helper function to block/unblock all checkbox signals at once
void CEditorSettingsWindow::BlockAllSignals(bool block)
{
	for (int i = 0; i < SETTING_COUNT; ++i) {
		QCheckBox* cb = GetCheckBoxByIndex(i);
		if (cb) cb->blockSignals(block);
	}
}

// Helper function to get checkbox pointer by index
QCheckBox* CEditorSettingsWindow::GetCheckBoxByIndex(int index)
{
	switch (index) {
		case 0: return chkValidateIniKeys;
		case 1: return chkEnableIniTooltips;
		case 2: return chkEnablePopupTooltips;
		case 3: return chkAutoCompletionConsent;
		case 4: return chkEnableAutoCompletion;
		case 5: return chkEnableFuzzyMatching;
		default: return nullptr;
	}
}

// Helper function to get reset flag pointer by index
bool* CEditorSettingsWindow::GetResetFlagByIndex(int index)
{
	switch (index) {
		case 0: return &m_resetValidateIniKeys;
		case 1: return &m_resetEnableIniTooltips;
		case 2: return &m_resetEnablePopupTooltips;
		case 3: return &m_resetAutoCompletionConsent;
		case 4: return &m_resetEnableAutoCompletion;
		case 5: return &m_resetEnableFuzzyMatching;
		default: return nullptr;
	}
}

// Helper function to reset an individual setting
void CEditorSettingsWindow::ResetIndividualSetting(int index)
{
	bool* resetFlag = GetResetFlagByIndex(index);
	if (resetFlag) {
		*resetFlag = true;
	}
	
	m_anyResetOccurred = true;
	
	QCheckBox* cb = GetCheckBoxByIndex(index);
	if (!cb) return;
	
	// Get default value from setting metadata
	EditorSettingInfo info = GetSettingInfo(index);
	
	// Set checkbox to default value
	cb->blockSignals(true);
	if (info.isTriState) {
		cb->setCheckState(static_cast<Qt::CheckState>(info.defaultValue));
	} else {
		cb->setChecked(info.defaultValue != 0);
	}
	cb->blockSignals(false);
	
	OnSettingChanged();
	UpdateDependencies(); // Update row enabled/disabled states after reset
}

void CEditorSettingsWindow::InitializeTable()
{
	// Set up the table
	ui.settingsTable->setRowCount(SETTING_COUNT);
	ui.settingsTable->setColumnCount(6);
	
	QStringList headers;
	headers << tr("Setting") << tr("State") << tr("Unchecked") << tr("Partial") << tr("Checked") << tr("Reset");
	ui.settingsTable->setHorizontalHeaderLabels(headers);
	
	// Create checkboxes based on setting metadata
	QCheckBox* checkboxes[SETTING_COUNT];
	checkboxes[0] = chkValidateIniKeys = new QCheckBox(this);
	checkboxes[1] = chkEnableIniTooltips = new QCheckBox(this);
	checkboxes[2] = chkEnablePopupTooltips = new QCheckBox(this);
	checkboxes[3] = chkAutoCompletionConsent = new QCheckBox(this);
	checkboxes[4] = chkEnableAutoCompletion = new QCheckBox(this);
	checkboxes[5] = chkEnableFuzzyMatching = new QCheckBox(this);
	
	// Configure checkboxes and populate table using metadata
	for (int i = 0; i < SETTING_COUNT; ++i) {
		EditorSettingInfo info = GetSettingInfo(i);
		
		// Configure checkbox tri-state mode
		checkboxes[i]->setTristate(info.isTriState);
		
		// Column 0: Setting name
		QTableWidgetItem* nameItem = new QTableWidgetItem(tr(info.displayName));
		nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
		ui.settingsTable->setItem(i, 0, nameItem);
		
		// Column 1: State (checkbox)
		QWidget* cbContainer = new QWidget(this);
		QHBoxLayout* cbLayout = new QHBoxLayout(cbContainer);
		cbLayout->setContentsMargins(0, 0, 0, 0);
		cbLayout->addWidget(checkboxes[i], 0, Qt::AlignCenter);
		ui.settingsTable->setCellWidget(i, 1, cbContainer);

		// Column 5: Reset button
		QPushButton* resetBtn = new QPushButton(tr("Reset"), this);
		resetBtn->setToolTip(tr("Reset this setting to default value"));
		resetBtn->setMaximumWidth(60);
		ui.settingsTable->setCellWidget(i, 5, resetBtn);

		// Connect reset button
		connect(resetBtn, &QPushButton::clicked, this, [this, i]() {
			ResetIndividualSetting(i);
			});
		
		// Connect checkbox signals
		connect(checkboxes[i], SIGNAL(stateChanged(int)), this, SLOT(ClearResetFlags()));
		connect(checkboxes[i], SIGNAL(stateChanged(int)), this, SLOT(OnSettingChanged()));
		connect(checkboxes[i], SIGNAL(stateChanged(int)), this, SLOT(UpdateDependencies()));
	}
	
	// Connect table events to handle cell clicks and maintain highlighting
	connect(ui.settingsTable, SIGNAL(itemSelectionChanged()), this, SLOT(OnTableItemClicked()));
	connect(ui.settingsTable, SIGNAL(cellClicked(int, int)), this, SLOT(OnTableCellClicked(int, int)));
	
	// Set up state descriptions
	UpdateTable();
	
	// Resize columns to content first - this adapts to language, DPI, and font size automatically
	ui.settingsTable->resizeColumnsToContents();
	
	// Add extra space to accommodate bold text highlighting in state description columns
	// This percentage-based approach works better across different DPIs and languages
	for (int col = 0; col < ui.settingsTable->columnCount(); ++col) {
		int currentWidth = ui.settingsTable->columnWidth(col);
		int extraSpace = 0;
		
		if (col >= 2 && col <= 4) {
			// State description columns need extra space for bold text
			extraSpace = currentWidth * 0.15; // 15% extra space for bold rendering
		} else if (col == 0) {
			// Setting name column - add some padding
			extraSpace = 20;
		} else if (col == 1 || col == 5) {
			// Checkbox and button columns - minimal extra space
			extraSpace = 10;
		}
		
		ui.settingsTable->setColumnWidth(col, currentWidth + extraSpace);
	}
	
	// Enable last section stretch to utilize available space
	ui.settingsTable->horizontalHeader()->setStretchLastSection(true);
	
	// Auto-resize dialog to fit table content
	int totalWidth = 0;
	for (int col = 0; col < ui.settingsTable->columnCount(); ++col) {
		totalWidth += ui.settingsTable->columnWidth(col);
	}
	
	// Add margins and scrollbar space
	int dialogWidth = totalWidth + 40; // Extra space for margins and potential scrollbar
	int dialogHeight = ui.settingsTable->rowCount() * 30 + 120; // Height for rows plus header and buttons
	
	// Set minimum and preferred size for the dialog
	setMinimumSize(dialogWidth, dialogHeight);
	resize(dialogWidth, dialogHeight);
}

void CEditorSettingsWindow::UpdateTable()
{
	// Populate state descriptions using metadata
	for (int row = 0; row < SETTING_COUNT; ++row) {
		EditorSettingInfo info = GetSettingInfo(row);
		
		// Column 2: Unchecked/Disabled state
		ui.settingsTable->setItem(row, 2, new QTableWidgetItem(tr(info.descUnchecked)));
		
		// Column 3: Partial/Basic state
		ui.settingsTable->setItem(row, 3, new QTableWidgetItem(tr(info.descPartial)));
		
		// Column 4: Checked/Full state
		ui.settingsTable->setItem(row, 4, new QTableWidgetItem(tr(info.descChecked)));
	}
	
	// Make all description items non-editable
	for (int row = 0; row < ui.settingsTable->rowCount(); ++row) {
		for (int col = 2; col < ui.settingsTable->columnCount(); ++col) {
			QTableWidgetItem* item = ui.settingsTable->item(row, col);
			if (item) {
				item->setFlags(item->flags() & ~Qt::ItemIsEditable);
			}
		}
	}
	
	// Highlight current states
	OnSettingChanged();
}

void CEditorSettingsWindow::OnSettingChanged()
{
	// Clear all highlighting first by resetting background colors
	for (int row = 0; row < ui.settingsTable->rowCount(); ++row) {
		for (int col = 2; col < 5; ++col) { // Only columns 2, 3, 4 (state descriptions)
			QTableWidgetItem* item = ui.settingsTable->item(row, col);
			if (item) {
				// Reset to default background
				item->setData(Qt::BackgroundRole, QVariant());
				// Reset font to normal
				QFont font = item->font();
				font.setBold(false);
				item->setFont(font);
			}
		}
	}
	
	// Decide whether the app is using a dark palette so we can pick contrasting highlight colors
	bool darkMode = (QApplication::palette().color(QPalette::Window).lightness() < 128);

	// Highlight current states with color-coded backgrounds
	for (int i = 0; i < SETTING_COUNT && i < ui.settingsTable->rowCount(); ++i) {
		QCheckBox* cb = GetCheckBoxByIndex(i);
		if (!cb) continue; // Safety check
		
		int state = cb->checkState();
		int col = 2; // Default to unchecked column (index 2)
		QColor highlightColor;
		QColor fgColor = QApplication::palette().color(QPalette::Text);
		
		if (state == Qt::Checked) {
			col = 4; // Checked column (index 4)
			if (darkMode) {
				highlightColor = QColor(0, 150, 0, 160); // semi-transparent green for dark mode
				fgColor = QColor(255, 255, 255);
			} else {
				highlightColor = QColor(144, 238, 144); // Light green for enabled/checked
				fgColor = QColor(0, 0, 0);
			}
		} else if (state == Qt::PartiallyChecked) {
			col = 3; // Partial column (index 3)
			if (darkMode) {
				highlightColor = QColor(200, 200, 0, 160); // darker yellow for dark mode
				fgColor = QColor(0, 0, 0);
			} else {
				highlightColor = QColor(255, 255, 153); // Light yellow for partial/basic
				fgColor = QColor(0, 0, 0);
			}
		} else {
			col = 2; // Unchecked column (index 2)
			if (darkMode) {
				highlightColor = QColor(200, 50, 50, 160); // reddish tint for dark mode
				fgColor = QColor(255, 255, 255);
			} else {
				highlightColor = QColor(255, 182, 193); // Light red/pink for disabled/unchecked
				fgColor = QColor(0, 0, 0);
			}
		}
		
		QTableWidgetItem* item = ui.settingsTable->item(i, col);
		if (item && item->text() != "-") {
			// Apply color-coded highlighting
			item->setData(Qt::BackgroundRole, QBrush(highlightColor));
			// Also make the text bold for better visibility
			QFont font = item->font();
			font.setBold(true);
			item->setFont(font);
			// Set foreground to ensure contrast in dark mode
			item->setForeground(QBrush(fgColor));
		}
	}
	
	// Force table to update its display
	ui.settingsTable->viewport()->update();
}

void CEditorSettingsWindow::LoadSettings()
{
	// Block all signals to prevent triggering during load
	BlockAllSignals(true);
	
	// Load all settings using metadata
	for (int i = 0; i < SETTING_COUNT; ++i) {
		EditorSettingInfo info = GetSettingInfo(i);
		QCheckBox* cb = GetCheckBoxByIndex(i);
		if (!cb) continue;
		
		// Special case for EnablePopupTooltips - default to EnableIniTooltips value if not set
		int defaultValue = info.defaultValue;
		if (i == 2) { // EnablePopupTooltips
			defaultValue = theConf->GetInt(GetSettingInfo(1).configKey, 1); // Get EnableIniTooltips value
		}
		
		if (info.isTriState) {
			int value = theConf->GetInt(info.configKey, defaultValue);
			cb->setCheckState(static_cast<Qt::CheckState>(value));
		} else {
			bool value = theConf->GetBool(info.configKey, defaultValue != 0);
			cb->setChecked(value);
		}
	}
	
	// Unblock all signals
	BlockAllSignals(false);
}

// Helper to get original value for a setting (as int for both bool and tri-state)
int CEditorSettingsWindow::GetOriginalValue(int index) const
{
	switch (index) {
		case 0: return m_originalValidateIniKeys ? 1 : 0;
		case 1: return m_originalEnableIniTooltips;
		case 2: return m_originalEnablePopupTooltips;
		case 3: return m_originalAutoCompletionConsent ? 1 : 0;
		case 4: return m_originalEnableAutoCompletion;
		case 5: return m_originalEnableFuzzyMatching ? 1 : 0;
		default: return 0;
	}
}

// Helper to get current value from checkbox (as int for both bool and tri-state)
int CEditorSettingsWindow::GetCurrentValue(int index) const
{
	QCheckBox* cb = const_cast<CEditorSettingsWindow*>(this)->GetCheckBoxByIndex(index);
	if (!cb) return 0;
	
	EditorSettingInfo info = GetSettingInfo(index);
	if (info.isTriState) {
		return static_cast<int>(cb->checkState());
	} else {
		return cb->isChecked() ? 1 : 0;
	}
}

void CEditorSettingsWindow::SaveSettings()
{
	// Save all settings using metadata
	bool anyResetRemaining = false;
	
	// First, preserve which settings were reset (for OptionsWindow to check after dialog closes)
	m_wasValidateIniKeysReset = m_resetValidateIniKeys;
	m_wasEnableIniTooltipsReset = m_resetEnableIniTooltips;
	m_wasEnableAutoCompletionReset = m_resetEnableAutoCompletion;
	
	for (int i = 0; i < SETTING_COUNT; ++i) {
		EditorSettingInfo info = GetSettingInfo(i);
		bool* resetFlag = GetResetFlagByIndex(i);
		QCheckBox* cb = GetCheckBoxByIndex(i);
		
		if (!resetFlag || !cb) continue;
		
		if (*resetFlag) {
			// Setting was reset - delete from config to use default
			theConf->DelValue(info.configKey);
			*resetFlag = false;
		} else if (GetCurrentValue(i) != GetOriginalValue(i)) {
			// Value changed - save it
			if (info.isTriState) {
				theConf->SetValue(info.configKey, static_cast<int>(cb->checkState()));
			} else {
				theConf->SetValue(info.configKey, cb->isChecked());
			}
		}
		
		// Check if any reset flags remain
		if (*resetFlag) anyResetRemaining = true;
	}
	
	// Clear the global flag if all individual flags are cleared
	if (!anyResetRemaining) {
		m_anyResetOccurred = false;
	}
}

void CEditorSettingsWindow::StoreOriginalValues()
{
	m_originalValidateIniKeys = chkValidateIniKeys->isChecked();
	m_originalEnableIniTooltips = static_cast<int>(chkEnableIniTooltips->checkState());
	m_originalEnablePopupTooltips = static_cast<int>(chkEnablePopupTooltips->checkState());
	m_originalAutoCompletionConsent = chkAutoCompletionConsent->isChecked();
	m_originalEnableAutoCompletion = static_cast<int>(chkEnableAutoCompletion->checkState());
	m_originalEnableFuzzyMatching = chkEnableFuzzyMatching->isChecked();
}

void CEditorSettingsWindow::RestoreOriginalValues()
{
	for (int i = 0; i < SETTING_COUNT; ++i) {
		QCheckBox* cb = GetCheckBoxByIndex(i);
		if (!cb) continue;
		
		EditorSettingInfo info = GetSettingInfo(i);
		int originalValue = GetOriginalValue(i);
		
		if (info.isTriState) {
			cb->setCheckState(static_cast<Qt::CheckState>(originalValue));
		} else {
			cb->setChecked(originalValue != 0);
		}
	}
}

void CEditorSettingsWindow::accept()
{
	SaveSettings();
	QDialog::accept();
}

void CEditorSettingsWindow::reject()
{
	RestoreOriginalValues();
	QDialog::reject();
}

// Getter methods
int CEditorSettingsWindow::GetEnableIniTooltips() const
{
	return static_cast<int>(chkEnableIniTooltips->checkState());
}

bool CEditorSettingsWindow::GetAutoCompletionConsent() const
{
	return chkAutoCompletionConsent->isChecked();
}

int CEditorSettingsWindow::GetEnableAutoCompletion() const
{
	return static_cast<int>(chkEnableAutoCompletion->checkState());
}

bool CEditorSettingsWindow::GetEnableFuzzyMatching() const
{
	return chkEnableFuzzyMatching->isChecked();
}

bool CEditorSettingsWindow::GetValidateIniKeys() const
{
	return chkValidateIniKeys->isChecked();
}

int CEditorSettingsWindow::GetEnablePopupTooltips() const
{
	return static_cast<int>(chkEnablePopupTooltips->checkState());
}

// Setter methods
void CEditorSettingsWindow::SetEnableIniTooltips(int state)
{
	chkEnableIniTooltips->setCheckState(static_cast<Qt::CheckState>(state));
}

void CEditorSettingsWindow::SetAutoCompletionConsent(bool enabled)
{
	chkAutoCompletionConsent->setChecked(enabled);
}

void CEditorSettingsWindow::SetEnableAutoCompletion(int state)
{
	chkEnableAutoCompletion->setCheckState(static_cast<Qt::CheckState>(state));
}

void CEditorSettingsWindow::SetEnableFuzzyMatching(bool enabled)
{
	chkEnableFuzzyMatching->setChecked(enabled);
}

void CEditorSettingsWindow::SetValidateIniKeys(bool enabled)
{
	chkValidateIniKeys->setChecked(enabled);
}

void CEditorSettingsWindow::SetEnablePopupTooltips(int state)
{
	chkEnablePopupTooltips->setCheckState(static_cast<Qt::CheckState>(state));
}

void CEditorSettingsWindow::OnTableItemClicked()
{
	// Reapply highlighting when table selection changes
	OnSettingChanged();
}

void CEditorSettingsWindow::OnTableCellClicked(int row, int column)
{
	// Handle clicking on state description cells to change checkbox state
	if (row < 0 || row >= SETTING_COUNT || column < 2 || column > 4) {
		return; // Only handle state description columns
	}
	
	QCheckBox* cb = GetCheckBoxByIndex(row);
	if (!cb) return;
	
	// Don't allow interaction if the checkbox is disabled
	if (!cb->isEnabled()) {
		return;
	}
	
	QTableWidgetItem* item = ui.settingsTable->item(row, column);
	if (!item || item->text() == "-") return; // Don't handle invalid states
	
	// Determine the new state based on clicked column
	Qt::CheckState newState = Qt::Unchecked;
	if (column == 2) {
		newState = Qt::Unchecked;
	} else if (column == 3) {
		newState = Qt::PartiallyChecked;
	} else if (column == 4) {
		newState = Qt::Checked;
	}
	
	EditorSettingInfo info = GetSettingInfo(row);
	
	// For binary checkboxes, only allow Unchecked or Checked
	if (!info.isTriState && newState == Qt::PartiallyChecked) {
		return;
	}
	
	// Set the new state
	if (info.isTriState) {
		cb->setCheckState(newState);
	} else {
		cb->setChecked(newState == Qt::Checked);
	}
}

void CEditorSettingsWindow::OnResetAll()
{
	// Reset all settings to their defaults using metadata
	// Block signals first since we're resetting all at once
	BlockAllSignals(true);
	
	// Reset each setting (this also sets the reset flags)
	for (int i = 0; i < SETTING_COUNT; ++i) {
		bool* resetFlag = GetResetFlagByIndex(i);
		if (resetFlag) *resetFlag = true;
		
		QCheckBox* cb = GetCheckBoxByIndex(i);
		if (!cb) continue;
		
		EditorSettingInfo info = GetSettingInfo(i);
		if (info.isTriState) {
			cb->setCheckState(static_cast<Qt::CheckState>(info.defaultValue));
		} else {
			cb->setChecked(info.defaultValue != 0);
		}
	}
	
	m_anyResetOccurred = true;
	
	// Unblock all signals
	BlockAllSignals(false);
	
	// Update the visual highlighting
	OnSettingChanged();
	
	// Update dependencies after resetting all settings
	UpdateDependencies();
}

void CEditorSettingsWindow::ClearResetFlags()
{
	// When user manually changes settings, clear reset flags so they can be saved
	QObject* senderObj = sender();
	
	// Find which checkbox triggered this and clear its reset flag
	for (int i = 0; i < SETTING_COUNT; ++i) {
		if (senderObj == GetCheckBoxByIndex(i)) {
			bool* resetFlag = GetResetFlagByIndex(i);
			if (resetFlag) *resetFlag = false;
			break;
		}
	}
	
	// If all individual reset flags are cleared, clear the global flag too
	bool anyResetRemaining = false;
	for (int i = 0; i < SETTING_COUNT; ++i) {
		bool* resetFlag = GetResetFlagByIndex(i);
		if (resetFlag && *resetFlag) {
			anyResetRemaining = true;
			break;
		}
	}
	if (!anyResetRemaining) {
		m_anyResetOccurred = false;
	}
}

// Helper function to enable/disable an entire table row
void CEditorSettingsWindow::SetRowEnabled(int row, bool enabled)
{
	if (row < 0 || row >= ui.settingsTable->rowCount()) return;
	
	// Get the checkbox for this row
	QCheckBox* cb = GetCheckBoxByIndex(row);
	if (cb) {
		cb->setEnabled(enabled);
	}
	
	// Enable/disable all items in the row
	for (int col = 0; col < ui.settingsTable->columnCount(); ++col) {
		QTableWidgetItem* item = ui.settingsTable->item(row, col);
		if (item) {
			Qt::ItemFlags flags = item->flags();
			if (enabled) {
				flags |= Qt::ItemIsEnabled;
			} else {
				flags &= ~Qt::ItemIsEnabled;
			}
			item->setFlags(flags);
			
			// Use the application's palette to choose readable foreground colors for enabled/disabled states
			QPalette pal = QApplication::palette();
			QColor fgEnabled = pal.color(QPalette::Text);
			QColor fgDisabled = pal.color(QPalette::Disabled, QPalette::Text);
			if (!enabled) {
				item->setForeground(QBrush(fgDisabled));
			} else {
				item->setForeground(QBrush(fgEnabled));
			}
		}
	}
	
	// Keep reset button always enabled so users can reset even when setting is disabled
	QPushButton* resetBtn = qobject_cast<QPushButton*>(ui.settingsTable->cellWidget(row, 5));
	if (resetBtn) {
		resetBtn->setEnabled(true); // Always enabled
	}
}

// Update dependency chain: Consent → AutoCompletion → FuzzyMatching
void CEditorSettingsWindow::UpdateDependencies()
{
	// Index 3: AutoCompletionConsent
	// Index 4: EnableAutoCompletion  
	// Index 5: EnableFuzzyMatching
	
	bool consentEnabled = chkAutoCompletionConsent->isChecked();
	bool consentJustEnabled = consentEnabled && !m_previousConsentState;
	
	// Update the previous consent state for next time
	m_previousConsentState = consentEnabled;
	
	// If consent is disabled, disable AutoCompletion and set it to unchecked
	if (!consentEnabled) {
		chkEnableAutoCompletion->setCheckState(Qt::Unchecked);
		SetRowEnabled(4, false); // Disable AutoCompletion row
	} else {
		// Only auto-enable AutoCompletion when consent was just enabled (transition from false to true)
		// AND AutoCompletion is currently disabled
		if (consentJustEnabled && chkEnableAutoCompletion->checkState() == Qt::Unchecked) {
			chkEnableAutoCompletion->setCheckState(Qt::Checked);
		}
		SetRowEnabled(4, true); // Enable AutoCompletion row
	}
	
	// Recalculate autoCompletionEnabled AFTER potentially changing it above
	bool autoCompletionEnabled = (chkEnableAutoCompletion->checkState() != Qt::Unchecked);
	
	// If AutoCompletion is unchecked (regardless of consent), disable FuzzyMatching
	if (!consentEnabled || !autoCompletionEnabled) {
		SetRowEnabled(5, false); // Disable FuzzyMatching row
	} else {
		SetRowEnabled(5, true); // Enable FuzzyMatching row
	}
}
