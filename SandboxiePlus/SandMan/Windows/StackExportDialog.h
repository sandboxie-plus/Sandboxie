#pragma once

#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>

#include "../Helpers/RedactedStackExport.h"

// Displays two clearly separated presentations of one stack capture:
//   - "Local details" mirrors the captured symbols and is only shown here
//   - "Share preview" is the redacted transform, and matches the bytes that
//     Copy or Save will produce.
// Both views come from the same capture; a live refresh of the diagnostic
// never swaps content out from under a Copy or Save the user just approved.
class CStackExportDialog : public QDialog
{
	Q_OBJECT
public:
	CStackExportDialog(const SStackCapture& capture, QWidget* parent = nullptr);

	// Non-interactive shims used by the smoke tests.
	QString localText() const;
	QString shareText() const;
	QString shareJson() const;
	QString lastError() const { return m_LastError; }
	bool copyShareToClipboard();
	bool saveShareTo(const QString& path);

private slots:
	void onFormatChanged();
	void onCopy();
	void onSave();

private:
	QString renderLocal(const SStackCapture& capture) const;
	QByteArray currentShareBytes(bool* asJson = nullptr) const;

	SStackCapture m_Capture;
	SRedactedStack m_Redacted;
	QPlainTextEdit* m_pLocal;
	QPlainTextEdit* m_pShare;
	QComboBox* m_pFormat;
	QLabel* m_pWarning;
	QPushButton* m_pCopy;
	QPushButton* m_pSave;
	QString m_LastError;
};
