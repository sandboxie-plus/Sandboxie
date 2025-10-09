#pragma once

#include "../mischelpers_global.h"

#include <QSyntaxHighlighter>
#include <QCompleter>
#include <QStringListModel>

class MISCHELPERS_EXPORT CCodeEdit : public QWidget
{
	Q_OBJECT

public:
	CCodeEdit(QSyntaxHighlighter* pHighlighter = NULL, QWidget* pParent = NULL);

	static constexpr int AUTO_COMPLETE_MIN_LENGTH = 2;  // Minimum chars to trigger autocompletion

	// Autocompletion mode enumeration
	enum class AutoCompletionMode {
		Disabled = 0,        // Qt::Unchecked - no autocompletion
		ManualOnly = 1,      // Qt::PartiallyChecked - only Ctrl+Space and Ctrl+R
		FullAuto = 2         // Qt::Checked - full autocompletion including typing/deleting
	};

	void SetCompleter(QCompleter* completer);
	QCompleter* GetCompleter() const { return m_pCompleter; }
	void UpdateCompletionModel(const QStringList& candidates);

	void SetCode(const QString& code) { m_pSourceCode->setPlainText(code); }
	QString GetCode() const { return m_pSourceCode->toPlainText(); }

	void SetFont(const QFont& Font);
	const QFont& GetFont() const;

	void SetCaseCorrectionCallback(std::function<QString(const QString&)> callback);
	void SetCompletionFilterCallback(std::function<bool(const QString&)> callback);
	void SetCaseCorrectionFilterCallback(std::function<bool(const QString&)> callback);
	void SetPopupTooltipCallback(std::function<QString(const QString&)> tooltipCallback);

	// Static autocompletion mode control (similar to tooltip mode)
	static void SetAutoCompletionMode(int checkState);
	static AutoCompletionMode GetAutoCompletionMode();
	
	// Fuzzy matching control (OptionsWindow / SettingsWindow will call these)
	static void SetFuzzyMatchingEnabled(bool bEnabled);
	static bool GetFuzzyMatchingEnabled();
	// Dynamic control for fuzzy prefix length (defaults)
	static void SetMaxFuzzyPrefixLength(int length);
	static int GetMaxFuzzyPrefixLength();
	static void SetMinFuzzyPrefixLength(int length);
	static int GetMinFuzzyPrefixLength();
	static int s_maxFuzzyPrefixLength;
	static int s_minFuzzyPrefixLength;
	static void ClearFuzzyCache();

	// Fuzzy cache logging control
	static void SetFuzzyCacheLoggingEnabled(bool bEnabled);
	static bool GetFuzzyCacheLoggingEnabled();
	static bool s_fuzzyCacheLoggingEnabled;

	// Token cache logging control
	static void SetTokenCacheLoggingEnabled(bool bEnabled);
	static bool GetTokenCacheLoggingEnabled();
	static bool s_tokenCacheLoggingEnabled;

	void ScheduleWithDelay(int delayMs, std::function<void()> task, const QString& taskName);

	static int s_popupTooltipsMode;
	static void SetPopupTooltipsEnabled(int checkState);
	static int GetPopupTooltipsEnabled();
	void ShowPopupTooltipForCurrentItem();

signals:
	void textChanged();

public slots:
	void OnFind();
	void OnFindNext();
	void OnGoTo();
	void ReplaceLastCorrection();
	void OnTextChanged();

	/*void				OnComment();
	void				OnUnComment();*/
	
protected:
	bool eventFilter(QObject* obj, QEvent* event) override;

	// Helper structures for organizing cursor and word boundary data
	struct CursorContext {
		QTextCursor cursor;
		QTextBlock block;
		QString text;
		int position;
	};

	struct WordBoundaries {
		int start;
		int end;
	};

	// Refactored helper methods for common operations
	bool IsWordCharacter(QChar c) const;
	CursorContext GetCursorContext() const;
	WordBoundaries FindWordBoundaries(const QString& text, int position) const;
	QString ExtractWordAtCursor(const CursorContext& context) const;
	bool IsInKeyPosition(const CursorContext& context) const;
	bool IsWordAtLineStart(const CursorContext& context) const;
	QString GetCompletionWord() const;
	void TriggerCompletion(const QString& prefix, int minimumLength = 3);
	void HandleCaseCorrection(const QString& word, bool wasPopupVisible);
	void HandleCompletion(const QString& prefix);
	void HandleCaseCorrectionForDelimiter(QChar delimiter, bool wasPopupVisible);
	bool CanTriggerCompletion() const;
	void ValidateCaseCorrection();

	// Event handling helper methods
	bool HandleEqualsKeyInPopup(QKeyEvent* keyEvent);
	bool HandleTextEditKeyPress(QKeyEvent* keyEvent);
	bool HandleKeyPressWithPopupVisible(QKeyEvent* keyEvent);
	bool HandleEnterKeyInPopup(QKeyEvent* keyEvent);
	bool HandleBackspaceKeyInPopup();
	bool HandleDefaultKeyInPopup(QKeyEvent* keyEvent);
	bool HandleManualCompletionTrigger(QKeyEvent* keyEvent);
	bool HandleSingleCharacterInput(QKeyEvent* keyEvent);
	bool HandleBackspaceForCompletion(QKeyEvent* keyEvent);
	bool HandleDeleteForCompletion(QKeyEvent* keyEvent);

	// Original methods (now using helper methods internally)
	QString GetWordUnderCursor() const;
	QString GetWordBeforeCursor() const;
	QString GetKeyFromCurrentLine() const;
	void ShowCaseCorrection(const QString& wrongWord, const QString& correctWord);
	void OnInsertCompletion(const QString& completion);

private:
	QGridLayout* m_pMainLayout;
	QTextEdit* m_pSourceCode;
	QCompleter* m_pCompleter;

	// Track the base model (visible candidates) and temporary fuzzy model when fuzzy is active
	QStringListModel* m_baseModel = nullptr;
	QStringListModel* m_tempFuzzyModel = nullptr;

	// Find/Replace actions
	QAction*			m_pFind;
	QAction*			m_pFindNext;
	QAction*			m_pGoTo;
	QAction*			m_pReplaceCorrection;
	

	/*QAction*			m_pComment;
	QAction*			m_pUnComment;*/

	QString				m_CurFind;

	QTimer* m_completionDebounceTimer = nullptr;
	QString m_pendingPrefix;

	// Case correction tracking
	std::function<QString(const QString&)> m_caseCorrectionCallback;
	QString m_lastWrongWord;
	QString m_lastCorrectWord;
	int m_lastWordStart;
	int m_lastWordEnd;
	
	// Add flag to prevent duplicate case correction and unwanted popup triggers
	bool m_caseCorrectionInProgress;
	bool m_completionInsertInProgress;

	std::function<bool(const QString&)> m_completionFilterCallback;
	std::function<bool(const QString&)> m_caseCorrectionFilterCallback;
	bool ShouldHideKeyFromCompletion(const QString& keyName) const;

	std::function<QString(const QString&)> m_tooltipCallback;
	
	// Static autocompletion mode (similar to tooltip mode)
	static AutoCompletionMode s_autoCompletionMode;
	static QMutex s_autoCompletionModeMutex;

	// Fuzzy matching toggle
	static bool s_fuzzyMatchingEnabled;
	
	// Helper methods for common operations
	void ResetFlagAfterDelay(bool& flag, int delayMs = 100, const QString& flagName = "unnamed");

	void HidePopupSafely();
	void ClearCaseCorrectionTracking();
	void UpdateCaseCorrectionTracking(const QString& wrongWord, const QString& correctWord, int wordStart, int wordEnd);
	bool IsKeyAvailableInCompletionModel(const QString& key) const;
	void ConsolidateEqualsSignsAfterCursor(QTextCursor& cursor, bool moveCursorPastEquals = true);

	// Fuzzy-specific helpers (private)
	QStringList ApplyFuzzyModelForPrefix(const QString& prefix);
	void RestoreBaseCompletionModel();
	bool IsKeyAvailableConsideringFuzzy(const QString& key, const QString& wordForFuzzy) const;
	void ResetPopupScrollState();
	void ShowCompletionPopup(bool resetScroll = true);

	bool IsExistingKeyValueLine(const QString& lineText, int cursorPosition, int& equalsPosition) const;
	QString GetTextReplacement(const QString& originalWord, const QString& replacement, 
							  const QString& lineText, int wordPosition, bool addEquals) const;
	
	// Mode checking helpers
	bool ShouldTriggerAutoCompletion() const;
	bool ShouldHandleCaseCorrection() const;

	QSet<QString> m_scheduledTasks;

	QStringList m_visibleCandidates;       // Keys visible in popup
	QStringList m_caseCorrectionCandidates; // Keys hidden from popup but available for case correction

	bool m_suppressNextAutoCompletion = false;

	int m_lastKeyPressed = 0;

	private slots:
		void OnCursorPositionChanged();
		void OnPopupSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
};