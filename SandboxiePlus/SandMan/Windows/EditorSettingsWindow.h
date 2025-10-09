#pragma once

#include <QtWidgets/QDialog>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QCheckBox>
#include "ui_EditorSettingsWindow.h"

class CEditorSettingsWindow : public QDialog
{
	Q_OBJECT

public:
	CEditorSettingsWindow(QWidget *parent = Q_NULLPTR);
	~CEditorSettingsWindow();

	// Get current settings from the dialog (as int for tri-state)
	int GetEnableIniTooltips() const;
	bool GetAutoCompletionConsent() const;
	int GetEnableAutoCompletion() const;
	bool GetEnableFuzzyMatching() const;
	bool GetValidateIniKeys() const;
	int GetEnablePopupTooltips() const;

	// Set current settings in the dialog
	void SetEnableIniTooltips(int state);
	void SetAutoCompletionConsent(bool enabled);
	void SetEnableAutoCompletion(int state);
	void SetEnableFuzzyMatching(bool enabled);
	void SetValidateIniKeys(bool enabled);
	void SetEnablePopupTooltips(int state);

	// Check if any reset occurred during this session
	bool HasResetOccurred() const { return m_anyResetOccurred; }
	
	// Check if specific settings were reset (needed for OptionsWindow)
	// These use persistent flags that aren't cleared by SaveSettings
	bool WasValidateIniKeysReset() const { return m_wasValidateIniKeysReset; }
	bool WasEnableIniTooltipsReset() const { return m_wasEnableIniTooltipsReset; }
	bool WasEnableAutoCompletionReset() const { return m_wasEnableAutoCompletionReset; }

public slots:
	void accept() override;
	void reject() override;

private slots:
	void OnSettingChanged();
	void OnTableItemClicked();
	void OnTableCellClicked(int row, int column);
	void OnResetAll();
	void ClearResetFlags();
	void UpdateDependencies();

private:
	Ui::EditorSettingsWindow ui;
	
	// Checkbox controls (created dynamically and embedded in table)
	QCheckBox* chkValidateIniKeys;
	QCheckBox* chkEnableIniTooltips;
	QCheckBox* chkEnablePopupTooltips;
	QCheckBox* chkAutoCompletionConsent;
	QCheckBox* chkEnableAutoCompletion;
	QCheckBox* chkEnableFuzzyMatching;
	
	// Store original values for restore on cancel
	int m_originalEnableIniTooltips;
	bool m_originalAutoCompletionConsent;
	int m_originalEnableAutoCompletion;
	bool m_originalEnableFuzzyMatching;
	bool m_originalValidateIniKeys;
	int m_originalEnablePopupTooltips;
	
	// Reset flags to track which settings have been reset
	bool m_resetValidateIniKeys;
	bool m_resetEnableIniTooltips;
	bool m_resetEnablePopupTooltips;
	bool m_resetAutoCompletionConsent;
	bool m_resetEnableAutoCompletion;
	bool m_resetEnableFuzzyMatching;
	
	// Persistent flags to track which settings were reset (not cleared by SaveSettings)
	// These are used by OptionsWindow after dialog closes
	bool m_wasValidateIniKeysReset;
	bool m_wasEnableIniTooltipsReset;
	bool m_wasEnableAutoCompletionReset;
	
	// Flag to track if any reset occurred (for OptionsWindow)
	bool m_anyResetOccurred;
	
	// Track previous consent state to detect transitions
	bool m_previousConsentState;
	
	void LoadSettings();
	void SaveSettings();
	void StoreOriginalValues();
	void RestoreOriginalValues();
	void InitializeTable();
	void UpdateTable();
	
	// Helper functions to reduce code duplication
	void BlockAllSignals(bool block);
	void ResetIndividualSetting(int settingIndex);
	QCheckBox* GetCheckBoxByIndex(int index);
	bool* GetResetFlagByIndex(int index);
	int GetOriginalValue(int index) const;
	int GetCurrentValue(int index) const;
	void SetRowEnabled(int row, bool enabled);
};