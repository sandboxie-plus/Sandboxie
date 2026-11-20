#include "stdafx.h"
#include "BorderFontDialog.h"
#include "OptionsWindow.h"
#include "SandMan.h"
#include <QFontComboBox>
#include <QFontDatabase>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QSignalBlocker>
#include <QFontInfo>
#include <QRawFont>
#include <QGuiApplication>
#include <windows.h>

static void TraceBorderFontDialog(const QString& message)
{
	DWORD lastError = GetLastError();
	if (IsDebuggerPresent()) {
		QString line = QString("[SbieBorderFont tid=%1] dialog %2\r\n").arg(GetCurrentThreadId()).arg(message);
		OutputDebugStringW(reinterpret_cast<LPCWSTR>(line.utf16()));
	}
	SetLastError(lastError);
}

static void TraceBorderFontNames(const QByteArray& table)
{
	TraceBorderFontDialog(QString("qt-name tableBytes=%1").arg(table.size()));
	if (table.size() < 6 || table.size() > 1024 * 1024) return;
	auto word = [&table](int offset) -> int {
		return (static_cast<uchar>(table[offset]) << 8) | static_cast<uchar>(table[offset + 1]);
	};
	int count = word(2), storage = word(4);
	if (word(0) > 1 || count > (table.size() - 6) / 12 || storage < 6 + count * 12 || storage > table.size()) {
		TraceBorderFontDialog("qt-name invalid-header");
		return;
	}
	if (word(0) == 1) {
		int tags = 6 + count * 12;
		if (tags > table.size() - 2) return;
		int languages = word(tags);
		if (languages > (table.size() - tags - 2) / 4 || storage < tags + 2 + languages * 4) return;
	}
	int logged = 0;
	for (int i = 0; i < count; ++i) {
		int record = 6 + i * 12;
		int platform = word(record), encoding = word(record + 2), language = word(record + 4), id = word(record + 6);
		if (id != 1 && id != 2 && id != 4 && id != 16 && id != 17 && id != 21 && id != 22) continue;
		int length = word(record + 8), offset = word(record + 10);
		if (offset > table.size() - storage || length > table.size() - storage - offset) continue;
		if (logged++ >= 64) { TraceBorderFontDialog("qt-name record-limit=64"); break; }
		QByteArray bytes = table.mid(storage + offset, qMin(length, 256));
		QString text = "<undecoded>";
		bool unicode = (platform == 0 && encoding <= 6) || (platform == 3 && (encoding == 0 || encoding == 1 || encoding == 10));
		if (unicode && !(length % 2)) {
			text.clear();
			for (int j = 0; j < bytes.size(); j += 2) text.append(QChar(word(storage + offset + j)));
		} else if (platform == 1 && encoding == 0) {
			bool ascii = true;
			for (char ch : bytes) { if (static_cast<uchar>(ch) >= 128) { ascii = false; break; } }
			if (ascii) text = QString::fromLatin1(bytes);
		}
		text.replace('\\', "\\\\");
		text.replace('"', "\\\"");
		text.replace('\r', "\\r");
		text.replace('\n', "\\n");
		text.replace('\t', "\\t");
		text.replace(QChar(0), "\\0");
		TraceBorderFontDialog(QString("qt-name record=%1 id=%2 platform=%3 encoding=%4 language=0x%5 text=\"%6\" hex=%7 truncated=%8")
			.arg(i).arg(id).arg(platform).arg(encoding).arg(language, 4, 16, QChar('0')).arg(text)
			.arg(QString::fromLatin1(bytes.toHex())).arg(length > bytes.size()));
	}
}

static void TraceQtBorderFont(const QFont& comboFont, const QString& comboText, const QString& styleHint, int script, double points)
{
	if (!IsDebuggerPresent()) return;
	DWORD lastError = GetLastError();
	QString family = comboFont.family();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QFontDatabase database;
	bool listedBefore = database.families().contains(family, Qt::CaseInsensitive);
	QStringList styles = database.styles(family);
#else
	bool listedBefore = QFontDatabase::families().contains(family, Qt::CaseInsensitive);
	QStringList styles = QFontDatabase::styles(family);
#endif
	QString style = styles.contains(styleHint) ? styleHint : styles.value(0);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	QFont query = database.font(family, style, qMax(1, qRound(points)));
#else
	QFont query = QFontDatabase::font(family, style, qMax(1, qRound(points)));
#endif
	query.setPointSizeF(points);
	QFontInfo info(query);
	QFontDatabase::WritingSystem system = script >= 0 && script < QFontDatabase::WritingSystemsCount ? (QFontDatabase::WritingSystem)script : QFontDatabase::Any;
	QRawFont raw = QRawFont::fromFont(query, system);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	bool listedAfter = database.families().contains(family, Qt::CaseInsensitive);
#else
	bool listedAfter = QFontDatabase::families().contains(family, Qt::CaseInsensitive);
#endif
	TraceBorderFontDialog(QString("qt-family version=%1 platform=%2 combo=\"%3\" family=\"%4\" script=%5 listedBefore=%6 listedAfter=%7 styles=[%8] styleHint=\"%9\" chosenStyle=\"%10\"")
		.arg(qVersion()).arg(QGuiApplication::platformName()).arg(comboText).arg(family).arg(script)
		.arg(listedBefore).arg(listedAfter).arg(styles.join(" | ").left(2048)).arg(styleHint).arg(style));
	// This reports Qt's match only; it does not approve a GDI selection.
	TraceBorderFontDialog(QString("qt-resolved requestFamily=\"%1\" requestStyle=\"%2\" requestedPt=%3 family=\"%4\" style=\"%5\" qtWeight=%6 italic=%7 actualPt=%8 exactMatch=%9")
		.arg(query.family()).arg(query.styleName()).arg(points).arg(info.family()).arg(info.styleName())
		.arg(info.weight()).arg(info.italic()).arg(info.pointSizeF()).arg(info.exactMatch()));
	TraceBorderFontDialog(QString("qt-raw valid=%1").arg(raw.isValid()));
	if (raw.isValid()) {
		TraceBorderFontDialog(QString("qt-raw family=\"%1\" style=\"%2\" qtWeight=%3 pixelSize=%4")
			.arg(raw.familyName()).arg(raw.styleName()).arg(raw.weight()).arg(raw.pixelSize()));
		TraceBorderFontNames(raw.fontTable("name"));
	}
	SetLastError(lastError);
}

CBorderFontDialog::CBorderFontDialog(const SBoxBorderFont& initial, int legacyReference, double points, const QString& appliedValue,
	const std::function<SB_STATUS(const QString&)>& applyFont, QWidget* parent)
	: QDialog(parent), m_LegacyReference(legacyReference), m_InitialUnavailable(!initial.error.isEmpty()),
	m_ApplyFont(applyFont), m_AppliedValue(appliedValue)
{
	if (!appliedValue.isEmpty()) {
		SBoxBorderFont applied = CBoxBorder::ParseLabelFont(appliedValue);
		if (applied.error.isEmpty())
			m_AppliedValue = CBoxBorder::SerializeLabelFont(applied);
	}
	setWindowTitle(COptionsWindow::tr("Border label font"));
	auto layout = new QVBoxLayout(this);
	auto form = new QFormLayout;
	layout->addLayout(form);
	m_Script = new QComboBox(this);
	m_Script->addItem(COptionsWindow::tr("Any"), (int)QFontDatabase::Any);
	for (int i = 1; i < QFontDatabase::WritingSystemsCount; ++i)
		m_Script->addItem(QFontDatabase::writingSystemName((QFontDatabase::WritingSystem)i), i);
	int script = m_Script->findData(initial.writingSystem);
	if (script < 0) {
		m_Script->addItem(COptionsWindow::tr("Unavailable writing system (%1)").arg(initial.writingSystem), initial.writingSystem);
		script = m_Script->count() - 1;
	}
	m_Script->setCurrentIndex(script);
	form->addRow(COptionsWindow::tr("&Writing system:"), m_Script);
	m_Family = new QFontComboBox(this);
	m_Family->setEditable(false);
	int system = initial.writingSystem;
	m_Family->setWritingSystem(system >= 0 && system < QFontDatabase::WritingSystemsCount ? (QFontDatabase::WritingSystem)system : QFontDatabase::Any);
	m_Family->setCurrentFont(QFont(initial.family));
	// Size edits must not approve a family substituted by the combo box.
	m_InitialFamilyUnavailable = m_Family->findText(initial.family, Qt::MatchFixedString) < 0 ||
		m_Family->currentFont().family().compare(initial.family, Qt::CaseInsensitive) != 0;
	m_InitialUnavailable |= m_InitialFamilyUnavailable;
	form->addRow(COptionsWindow::tr("Font &family:"), m_Family);
	m_Style = new QComboBox(this);
	form->addRow(COptionsWindow::tr("Font st&yle:"), m_Style);
	m_Size = new QDoubleSpinBox(this);
	m_Size->setRange(1, 512);
	m_Size->setDecimals(2);
	m_Size->setSingleStep(0.5);
	m_Size->setSuffix(COptionsWindow::tr(" pt"));
	m_Size->setValue(points);
	form->addRow(COptionsWindow::tr("&Size:"), m_Size);
	m_Underline = new QCheckBox(COptionsWindow::tr("&Underline"), this);
	m_Underline->setChecked(initial.underline);
	m_Strikeout = new QCheckBox(COptionsWindow::tr("Stri&keout"), this);
	m_Strikeout->setChecked(initial.strikeout);
	auto effects = new QHBoxLayout;
	effects->addWidget(m_Underline);
	effects->addWidget(m_Strikeout);
	effects->addStretch();
	layout->addLayout(effects);
	m_Preview = new QLabel(this);
	m_Preview->setAlignment(Qt::AlignCenter);
	m_Preview->setAccessibleName(COptionsWindow::tr("Native font preview"));
	m_Preview->setFrameShape(QFrame::StyledPanel);
	layout->addWidget(m_Preview);
	m_Status = new QLabel(this);
	m_Status->setWordWrap(true);
	layout->addWidget(m_Status);
	auto buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel, this);
	m_Ok = buttons->button(QDialogButtonBox::Ok);
	m_Apply = buttons->button(QDialogButtonBox::Apply);
	m_Apply->setToolTip(m_ApplyFont ? COptionsWindow::tr("Save only this font and size immediately. Other pending box options are not applied. Cancel keeps the last applied font.") :
		COptionsWindow::tr("This template is read-only."));
	layout->addWidget(buttons);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	connect(m_Apply, &QPushButton::clicked, this, [this]() {
		if (!m_Apply->isEnabled() || !m_ApplyFont) return;
		QString value = CBoxBorder::SerializeLabelFont(Specification());
		SB_STATUS status = m_ApplyFont(value);
		if (!status) { theGUI->CheckResults(QList<SB_STATUS>() << status, this); return; }
		m_AppliedValue = value;
		m_Apply->setEnabled(false);
	});
	connect(m_Script, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
		m_InitialUnavailable = false;
		int id = m_Script->currentData().toInt();
		m_UpdatingFamily = true;
		m_Family->setWritingSystem(id >= 0 && id < QFontDatabase::WritingSystemsCount ? (QFontDatabase::WritingSystem)id : QFontDatabase::Any);
		m_UpdatingFamily = false;
		UpdateStyles();
	});
	connect(m_Family, &QFontComboBox::currentFontChanged, this, [this](const QFont&) {
		if (m_UpdatingFamily) return;
		m_InitialUnavailable = false;
		UpdateStyles();
	});
	connect(m_Family, QOverload<int>::of(&QComboBox::activated), this, [this](int) {
		if (m_InitialUnavailable || m_InitialFamilyUnavailable) {
			m_InitialUnavailable = false;
			m_InitialFamilyUnavailable = false;
			UpdateStyles();
		}
	});
	connect(m_Style, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { m_InitialUnavailable = false; UpdatePreview(); });
	connect(m_Size, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double) { m_InitialUnavailable = false; UpdatePreview(); });
	connect(m_Underline, &QCheckBox::toggled, this, [this](bool) { UpdatePreview(); });
	connect(m_Strikeout, &QCheckBox::toggled, this, [this](bool) { UpdatePreview(); });
	QWidget::setTabOrder(m_Script, m_Family);
	QWidget::setTabOrder(m_Family, m_Style);
	QWidget::setTabOrder(m_Style, m_Size);
	QWidget::setTabOrder(m_Size, m_Underline);
	QWidget::setTabOrder(m_Underline, m_Strikeout);
	UpdateStyles(&initial);
}

SBoxBorderFont CBorderFontDialog::Specification() const
{
	SBoxBorderFont font;
	font.family = m_Family->currentFont().family();
	font.writingSystem = m_Script->currentData().toInt();
	font.pointSize = m_Size->value();
	font.underline = m_Underline->isChecked();
	font.strikeout = m_Strikeout->isChecked();
	int index = m_Style->currentIndex();
	if (index >= 0 && index < m_Faces.size()) {
		const auto& face = m_Faces[index];
		if (face.family.compare(font.family, Qt::CaseInsensitive)) font.nativeFamily = face.family;
		font.style = face.style;
		font.fullName = face.fullName;
		font.weight = face.weight;
		font.italic = face.italic;
	}
	return font;
}

void CBorderFontDialog::accept()
{
	if (m_Ok->isEnabled()) QDialog::accept();
}

void CBorderFontDialog::UpdateStyles(const SBoxBorderFont* initial)
{
	SBoxBorderFont wanted = initial ? *initial : Specification();
	QSignalBlocker blocker(m_Style);
	m_Style->clear();
	m_Faces = CBoxBorder::EnumLabelFontFaces(m_Family->currentFont().family());
	int selected = -1;
	for (int i = 0; i < m_Faces.size(); ++i) {
		const auto& face = m_Faces[i];
		bool nativeAlias = face.family.compare(m_Family->currentFont().family(), Qt::CaseInsensitive) != 0;
		m_Style->addItem(nativeAlias || face.style.isEmpty() ? face.fullName : face.style);
		bool exact = initial && (!wanted.fullName.isEmpty() || !wanted.style.isEmpty());
		bool match = exact ? (wanted.fullName.isEmpty() || !face.fullName.compare(wanted.fullName, Qt::CaseInsensitive)) &&
			(wanted.style.isEmpty() || !face.style.compare(wanted.style, Qt::CaseInsensitive)) : face.weight == wanted.weight && face.italic == wanted.italic;
		if (selected < 0 && match) selected = i;
	}
	if (!initial && selected < 0 && !m_Faces.isEmpty()) selected = 0;
	m_Style->setCurrentIndex(selected);
	TraceBorderFontDialog(QString("styles family=\"%1\" faces=%2 selected=%3 initialUnavailable=%4 familyUnavailable=%5")
		.arg(m_Family->currentFont().family()).arg(m_Faces.size()).arg(selected).arg(m_InitialUnavailable).arg(m_InitialFamilyUnavailable));
	UpdatePreview();
	if (IsDebuggerPresent()) {
		QString family = m_Family->currentFont().family();
		if (!m_TracedFamilies.contains(family)) {
			m_TracedFamilies.insert(family);
			TraceQtBorderFont(m_Family->currentFont(), m_Family->currentText(), wanted.style, m_Script->currentData().toInt(), m_Size->value());
		}
	}
}

void CBorderFontDialog::UpdatePreview()
{
	m_Ok->setEnabled(false);
	m_Apply->setEnabled(false);
	m_Preview->clear();
	SBoxBorderFont font = Specification();
	if (m_InitialUnavailable || m_InitialFamilyUnavailable || font.writingSystem < 0 || font.writingSystem >= QFontDatabase::WritingSystemsCount || m_Style->currentIndex() < 0) {
		TraceBorderFontDialog(QString("reject stage=selection-unavailable initialUnavailable=%1 script=%2 styleIndex=%3 familyUnavailable=%4")
			.arg(m_InitialUnavailable).arg(font.writingSystem).arg(m_Style->currentIndex()).arg(m_InitialFamilyUnavailable));
		m_Status->setText(COptionsWindow::tr("The saved selection is unavailable. Choose an installed family and a native style."));
		return;
	}
	double ratio = m_Preview->devicePixelRatioF();
	int dpi = qMax(1, qRound(m_Preview->logicalDpiY() * ratio));
	QString error;
	HFONT nativeFont = CBoxBorder::CreateLabelFont(font, m_LegacyReference, dpi, NULL, &error);
	if (!nativeFont) { TraceBorderFontDialog(QString("reject stage=native-font error=\"%1\"").arg(error)); m_Status->setText(error); return; }
	int width = qMax(1, qRound(fontMetrics().averageCharWidth() * 48 * ratio));
	int height = qMax(1, qRound(fontMetrics().lineSpacing() * 5 * ratio));
	BITMAPINFO info = {};
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = -height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;
	HDC dc = CreateCompatibleDC(NULL);
	void* pixels = NULL;
	HBITMAP bitmap = dc ? CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, NULL, 0) : NULL;
	HGDIOBJ oldBitmap = bitmap ? SelectObject(dc, bitmap) : NULL;
	HGDIOBJ oldFont = oldBitmap && oldBitmap != HGDI_ERROR ? SelectObject(dc, nativeFont) : NULL;
	bool valid = pixels && oldFont && oldFont != HGDI_ERROR;
	QImage image;
	if (valid) {
		RECT rect = { 0, 0, width, height };
		QColor background = m_Preview->palette().color(QPalette::Base);
		QColor foreground = m_Preview->palette().color(QPalette::Text);
		HBRUSH brush = CreateSolidBrush(RGB(background.red(), background.green(), background.blue()));
		valid = brush && FillRect(dc, &rect, brush) != 0;
		if (brush) DeleteObject(brush);
		SetBkMode(dc, TRANSPARENT);
		SetTextColor(dc, RGB(foreground.red(), foreground.green(), foreground.blue()));
		QString sample = QFontDatabase::writingSystemSample((QFontDatabase::WritingSystem)font.writingSystem);
		if (sample.isEmpty()) sample = COptionsWindow::tr("Sandbox name");
		valid = valid && DrawTextW(dc, (LPCWSTR)sample.utf16(), sample.size(), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX) != 0;
		GdiFlush();
		image = QImage((const uchar*)pixels, width, height, width * 4, QImage::Format_RGB32).copy();
		for (int y = 0; y < image.height(); ++y) {
			QRgb* row = reinterpret_cast<QRgb*>(image.scanLine(y));
			for (int x = 0; x < image.width(); ++x) row[x] |= 0xff000000;
		}
		image.setDevicePixelRatio(ratio);
	}
	if (oldFont && oldFont != HGDI_ERROR) SelectObject(dc, oldFont);
	if (oldBitmap && oldBitmap != HGDI_ERROR) SelectObject(dc, oldBitmap);
	if (bitmap) DeleteObject(bitmap);
	if (dc) DeleteDC(dc);
	DeleteObject(nativeFont);
	if (!valid || image.isNull()) { TraceBorderFontDialog("reject stage=preview-render"); m_Status->setText(COptionsWindow::tr("Unable to create the native font preview.")); return; }
	m_Preview->setPixmap(QPixmap::fromImage(image));
	m_Status->setText(COptionsWindow::tr("Native GDI preview. Writing system filters the font list; it does not change label language or direction."));
	m_Ok->setEnabled(true);
	m_Apply->setEnabled(m_ApplyFont && CBoxBorder::SerializeLabelFont(font) != m_AppliedValue);
	TraceBorderFontDialog("preview-ready");
}
