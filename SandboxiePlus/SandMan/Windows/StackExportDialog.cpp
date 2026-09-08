#include "StackExportDialog.h"

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>

CStackExportDialog::CStackExportDialog(const SStackCapture& capture, QWidget* parent)
	: QDialog(parent), m_Capture(capture)
{
	setWindowTitle(tr("Export Stack Trace"));
	resize(760, 480);

	// Freeze the redacted view for the entire dialog lifetime so a live
	// diagnostic refresh cannot swap it out from under the copy/save button.
	m_Redacted = CRedactedStackExport::transform(m_Capture);

	QVBoxLayout* pLayout = new QVBoxLayout(this);

	QLabel* pHeader = new QLabel(tr(
		"The <b>share preview</b> is what Copy and Save will produce. "
		"Selected fields are omitted from that view; this does not change "
		"the visibility of the modules inside the process."));
	pHeader->setWordWrap(true);
	pLayout->addWidget(pHeader);

	QSplitter* pSplit = new QSplitter(Qt::Vertical);
	QWidget* pLocal = new QWidget();
	QVBoxLayout* pLocalLayout = new QVBoxLayout(pLocal);
	pLocalLayout->setContentsMargins(0, 0, 0, 0);
	pLocalLayout->addWidget(new QLabel(tr("Local details (stay in this dialog)")));
	m_pLocal = new QPlainTextEdit();
	m_pLocal->setReadOnly(true);
	m_pLocal->setPlainText(renderLocal(m_Capture));
	pLocalLayout->addWidget(m_pLocal);
	pSplit->addWidget(pLocal);

	QWidget* pShare = new QWidget();
	QVBoxLayout* pShareLayout = new QVBoxLayout(pShare);
	pShareLayout->setContentsMargins(0, 0, 0, 0);
	QHBoxLayout* pHeader2 = new QHBoxLayout();
	pHeader2->addWidget(new QLabel(tr("Share preview (this is what leaves the dialog)")));
	pHeader2->addStretch();
	pHeader2->addWidget(new QLabel(tr("Format:")));
	m_pFormat = new QComboBox();
	m_pFormat->addItem(tr("Text"), QStringLiteral("text"));
	m_pFormat->addItem(tr("JSON"), QStringLiteral("json"));
	pHeader2->addWidget(m_pFormat);
	pShareLayout->addLayout(pHeader2);
	m_pShare = new QPlainTextEdit();
	m_pShare->setReadOnly(true);
	pShareLayout->addWidget(m_pShare);
	pSplit->addWidget(pShare);
	pLayout->addWidget(pSplit, 1);

	m_pWarning = new QLabel();
	m_pWarning->setWordWrap(true);
	pLayout->addWidget(m_pWarning);

	QHBoxLayout* pButtons = new QHBoxLayout();
	m_pCopy = new QPushButton(tr("Copy Share View"));
	m_pSave = new QPushButton(tr("Save Share View..."));
	pButtons->addWidget(m_pCopy);
	pButtons->addWidget(m_pSave);
	pButtons->addStretch();
	QDialogButtonBox* pBox = new QDialogButtonBox(QDialogButtonBox::Close);
	pButtons->addWidget(pBox);
	pLayout->addLayout(pButtons);

	connect(m_pFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(onFormatChanged()));
	connect(m_pCopy, SIGNAL(clicked(bool)), this, SLOT(onCopy()));
	connect(m_pSave, SIGNAL(clicked(bool)), this, SLOT(onSave()));
	connect(pBox, SIGNAL(rejected()), this, SLOT(reject()));

	onFormatChanged();
}

QString CStackExportDialog::renderLocal(const SStackCapture& capture) const
{
	QString out;
	if (capture.state == SStackCapture::ePartial)
		out += tr("(capture reported as partial)\n");
	for (int i = 0; i < capture.frames.size(); ++i) {
		const SStackFrame& frame = capture.frames[i];
		out += QString::number(i);
		out += QLatin1Char('\t');
		out += frame.symbol.isEmpty() ? tr("<unresolved>") : frame.symbol;
		out += QStringLiteral("  0x");
		out += QString::number(frame.address, 16);
		out += QLatin1Char('\n');
	}
	return out;
}

QByteArray CStackExportDialog::currentShareBytes(bool* asJson) const
{
	bool json = m_pFormat->currentData().toString() == QLatin1String("json");
	if (asJson) *asJson = json;
	if (json)
		return QJsonDocument(m_Redacted.toJson()).toJson(QJsonDocument::Indented);
	return m_Redacted.toText().toUtf8();
}

QString CStackExportDialog::localText() const { return m_pLocal->toPlainText(); }
QString CStackExportDialog::shareText() const { return m_Redacted.toText(); }
QString CStackExportDialog::shareJson() const { return QString::fromUtf8(QJsonDocument(m_Redacted.toJson()).toJson(QJsonDocument::Indented)); }

void CStackExportDialog::onFormatChanged()
{
	QByteArray bytes = currentShareBytes();
	m_pShare->setPlainText(QString::fromUtf8(bytes));
	m_pWarning->setText(tr(
		"Selected fields were omitted from the share view: paths, absolute "
		"addresses and offsets, module and symbol names, capture owner and "
		"free-form error messages. Module markers are opaque within this "
		"report only. Preserving frame order does not prevent correlation "
		"between reports."));
}

bool CStackExportDialog::copyShareToClipboard()
{
	m_LastError.clear();
	QByteArray bytes = currentShareBytes();
	QClipboard* pClipboard = QApplication::clipboard();
	if (!pClipboard) {
		m_LastError = QStringLiteral("no clipboard available");
		return false;
	}
	pClipboard->setText(QString::fromUtf8(bytes));
	return true;
}

bool CStackExportDialog::saveShareTo(const QString& path)
{
	m_LastError.clear();
	bool json = false;
	QByteArray bytes = currentShareBytes(&json);
	if (!CRedactedStackExport::writeAtomic(path, bytes, &m_LastError))
		return false;
	return true;
}

void CStackExportDialog::onCopy()
{
	if (!copyShareToClipboard())
		QMessageBox::warning(this, "Sandboxie-Plus",
			tr("Copy failed: %1").arg(m_LastError));
}

void CStackExportDialog::onSave()
{
	bool json = m_pFormat->currentData().toString() == QLatin1String("json");
	QString filter = json ? tr("Redacted stack (*.json)") : tr("Redacted stack (*.txt)");
	QString path = QFileDialog::getSaveFileName(this, tr("Save redacted stack"), QString(), filter);
	if (path.isEmpty())
		return;
	if (!saveShareTo(path))
		QMessageBox::warning(this, "Sandboxie-Plus",
			tr("Save failed: %1").arg(m_LastError));
}
