#pragma once

#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QAbstractButton>
#include <QBrush>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QKeySequenceEdit>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProxyStyle>
#include <QStyleFactory>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStyle>
#include <QStyleOptionButton>
#include <QStyleOptionComboBox>
#include <QStylePainter>
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>

class CPendingComboPaintFilter : public QObject
{
public:
	CPendingComboPaintFilter(QComboBox* pCombo)
		: QObject(pCombo)
	{
	}

protected:
	bool eventFilter(QObject* pObject, QEvent* pEvent) override
	{
		QComboBox* pCombo = qobject_cast<QComboBox*>(pObject);
		if (!pCombo || pEvent->type() != QEvent::Paint || !pCombo->property("pending_value_combo_color").isValid())
			return QObject::eventFilter(pObject, pEvent);

		QStyleOptionComboBox Option;
		Option.initFrom(pCombo);
		Option.editable = false;
		Option.currentText = pCombo->currentText();
		Option.currentIcon = pCombo->itemIcon(pCombo->currentIndex());
		Option.iconSize = pCombo->iconSize();

		QStylePainter Painter(pCombo);
		pCombo->style()->drawComplexControl(QStyle::CC_ComboBox, &Option, &Painter, pCombo);
		QRect EditRect = pCombo->style()->subControlRect(QStyle::CC_ComboBox, &Option, QStyle::SC_ComboBoxEditField, pCombo);
		Painter.fillRect(EditRect, pCombo->property("pending_value_combo_color").value<QColor>());
		pCombo->style()->drawControl(QStyle::CE_ComboBoxLabel, &Option, &Painter, pCombo);
		return true;
	}
};

class CPendingButtonPaintFilter : public QObject
{
public:
	CPendingButtonPaintFilter(QWidget* pButton)
		: QObject(pButton)
	{
	}

protected:
	bool eventFilter(QObject* pObject, QEvent* pEvent) override
	{
		QWidget* pButton = qobject_cast<QWidget*>(pObject);
		if (!pButton || pEvent->type() != QEvent::Paint || !pButton->property("pending_button_color").isValid())
			return QObject::eventFilter(pObject, pEvent);
		QAbstractButton* pAbstractButton = qobject_cast<QAbstractButton*>(pButton);
		if (!pAbstractButton)
			return QObject::eventFilter(pObject, pEvent);

		QStyleOptionButton Option;
		Option.initFrom(pButton);
		Option.text = pAbstractButton->text();
		QStyle::SubElement Element;
		bool bCheckBox;
		if (QCheckBox* pCheck = qobject_cast<QCheckBox*>(pButton)) {
			bCheckBox = true;
			Option.state |= pCheck->checkState() == Qt::PartiallyChecked ? QStyle::State_NoChange
				: pCheck->isChecked() ? QStyle::State_On : QStyle::State_Off;
			Element = QStyle::SE_CheckBoxContents;
		}
		else if (QRadioButton* pRadio = qobject_cast<QRadioButton*>(pButton)) {
			bCheckBox = false;
			Option.state |= pRadio->isChecked() ? QStyle::State_On : QStyle::State_Off;
			Element = QStyle::SE_RadioButtonContents;
		}
		else
			return QObject::eventFilter(pObject, pEvent);

		QStylePainter Painter(pButton);
		Painter.fillRect(pButton->style()->subElementRect(Element, &Option, pButton), pButton->property("pending_button_color").value<QColor>());
		pButton->style()->drawControl(bCheckBox ? QStyle::CE_CheckBox : QStyle::CE_RadioButton, &Option, &Painter, pButton);
		return true;
	}
};

class CPendingEditorStyle : public QProxyStyle
{
public:
	CPendingEditorStyle(QStyle* pBase)
		: QProxyStyle(pBase)
	{
	}

	void drawPrimitive(PrimitiveElement Element, const QStyleOption* pOption, QPainter* pPainter, const QWidget* pWidget = nullptr) const override
	{
		QProxyStyle::drawPrimitive(Element, pOption, pPainter, pWidget);
		if (Element == PE_PanelLineEdit && pWidget && pWidget->property("pending_editor_color").isValid()) {
			QRect Rect = pOption ? pOption->rect : pWidget->rect();
			Rect.adjust(1, 1, -1, -1);
			pPainter->fillRect(Rect, pWidget->property("pending_editor_color").value<QColor>());
		}
	}
};

class CPendingChanges : public QObject
{
public:
	CPendingChanges(QWidget* pRoot, bool* pHoldChange, int TemplateItemRole, bool bTrackKeySequences)
		: m_pRoot(pRoot), m_pHoldChange(pHoldChange), m_TemplateItemRole(TemplateItemRole), m_bTrackKeySequences(bTrackKeySequences)
	{
	}

	void CaptureItemBaselines(QTreeWidget* pExcludedTree, QTreeWidget* pTree = nullptr)
	{
		auto Capture = [this](QTreeWidget* pTree) {
			QSignalBlocker Blocker(pTree);
			QTreeWidgetItemIterator It(pTree);
			while (*It) {
				QTreeWidgetItem* pItem = *It;
				if (!IsTemplateItem(pItem)) {
					SetItemHighlight(pItem, eNone);
					pItem->setData(0, ItemBaselineRole, GetItemSignature(pItem));
				}
				++It;
			}
		};

		if (pTree) {
			Capture(pTree);
			return;
		}

		foreach(QTreeWidget* pCandidate, m_pRoot->findChildren<QTreeWidget*>()) {
			if (pCandidate != pExcludedTree)
				Capture(pCandidate);
		}
	}

	void CaptureCheckboxBaselines()
	{
		foreach(QCheckBox* pCheck, m_pRoot->findChildren<QCheckBox*>()) {
			SetCheckboxHighlight(pCheck, false);
			pCheck->setProperty(CheckboxBaselineProperty, (int)pCheck->checkState());
			if (!pCheck->property(CheckboxPaintFilterProperty).toBool()) {
				pCheck->setProperty(CheckboxPaintFilterProperty, true);
				pCheck->installEventFilter(new CPendingButtonPaintFilter(pCheck));
			}
		}
	}

	void CaptureRadioButtonBaselines()
	{
		foreach(QRadioButton* pRadio, m_pRoot->findChildren<QRadioButton*>())
			CaptureRadioButtonBaseline(pRadio);
	}

	void CaptureRadioButtonBaseline(QRadioButton* pRadio)
	{
		SetRadioButtonHighlight(pRadio, false);
		pRadio->setProperty(RadioBaselineProperty, pRadio->isChecked());
		if (!pRadio->property(RadioPaintFilterProperty).toBool()) {
			pRadio->setProperty(RadioPaintFilterProperty, true);
			pRadio->installEventFilter(new CPendingButtonPaintFilter(pRadio));
		}
		if (!pRadio->property(RadioConnectionProperty).toBool()) {
			pRadio->setProperty(RadioConnectionProperty, true);
			connect(pRadio, &QRadioButton::toggled, this, [this](bool) {
				if (m_bEnabled && !IsHoldingChanges())
					UpdateRadioButtonHighlights();
			});
		}
	}

	void ExcludeValue(QWidget* pControl)
	{
		SetValueHighlight(pControl, false);
		pControl->setProperty(ValueExcludedProperty, true);
	}

	void SetEnabled(bool bEnabled, QTreeWidget* pExcludedTree)
	{
		if (m_bEnabled == bEnabled)
			return;

		m_bEnabled = bEnabled;
		if (!m_bEnabled)
			ClearHighlights(pExcludedTree);
	}

	void CaptureValueBaselines()
	{
		foreach(QComboBox* pCombo, m_pRoot->findChildren<QComboBox*>()) {
			if (IsValueExcluded(pCombo))
				continue;
			SetValueHighlight(pCombo, false);
			if (pCombo->isEditable()) {
				QSignalBlocker Blocker(pCombo);
				EnsureComboSnapshot(pCombo);
			}
			pCombo->setProperty(ValueBaselineProperty, ComboValue(pCombo));
			if (!pCombo->isEditable() && !pCombo->property(ValueComboPaintFilterProperty).toBool()) {
				pCombo->setProperty(ValueComboPaintFilterProperty, true);
				pCombo->installEventFilter(new CPendingComboPaintFilter(pCombo));
			}
			if (!pCombo->property(ValueConnectionProperty).toBool()) {
				pCombo->setProperty(ValueConnectionProperty, true);
				connect(pCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, pCombo](int) {
					if (!IsHoldingChanges())
						UpdateValueHighlight(pCombo);
				});
				if (pCombo->isEditable()) {
					connect(pCombo, &QComboBox::currentTextChanged, this, [this, pCombo](const QString&) {
						if (!IsHoldingChanges())
							UpdateValueHighlight(pCombo);
					});
					connect(pCombo, qOverload<int>(&QComboBox::activated), this, [this, pCombo](int) {
						QTimer::singleShot(0, pCombo, [this, pCombo]() {
							if (!IsHoldingChanges())
								UpdateValueHighlight(pCombo);
						});
					});
				}
			}
		}

		foreach(QAbstractSpinBox* pSpinBox, m_pRoot->findChildren<QAbstractSpinBox*>()) {
			if (IsValueExcluded(pSpinBox))
				continue;
			SetValueHighlight(pSpinBox, false);
			pSpinBox->setProperty(ValueBaselineProperty, SpinBoxValue(pSpinBox));
			if (!pSpinBox->property(ValueConnectionProperty).toBool()) {
				pSpinBox->setProperty(ValueConnectionProperty, true);
				if (QSpinBox* pSpin = qobject_cast<QSpinBox*>(pSpinBox))
					connect(pSpin, qOverload<int>(&QSpinBox::valueChanged), this, [this, pSpin](int) {
						if (!IsHoldingChanges())
							UpdateValueHighlight(pSpin);
					});
				else if (QDoubleSpinBox* pSpin = qobject_cast<QDoubleSpinBox*>(pSpinBox))
					connect(pSpin, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, pSpin](double) {
						if (!IsHoldingChanges())
							UpdateValueHighlight(pSpin);
					});
			}
		}

		foreach(QLineEdit* pLineEdit, m_pRoot->findChildren<QLineEdit*>()) {
			if (IsComboLineEdit(pLineEdit))
				continue;
			if (IsValueExcluded(pLineEdit))
				continue;
			SetValueHighlight(pLineEdit, false);
			pLineEdit->setProperty(ValueBaselineProperty, pLineEdit->text());
			pLineEdit->setModified(false);
			if (!pLineEdit->property(ValueConnectionProperty).toBool()) {
				pLineEdit->setProperty(ValueConnectionProperty, true);
				connect(pLineEdit, &QLineEdit::textChanged, this, [this, pLineEdit](const QString&) {
					if (!IsHoldingChanges())
						UpdateValueHighlight(pLineEdit);
				});
			}
		}

		foreach(QPlainTextEdit* pTextEdit, m_pRoot->findChildren<QPlainTextEdit*>()) {
			if (IsValueExcluded(pTextEdit))
				continue;
			SetValueHighlight(pTextEdit, false);
			pTextEdit->setProperty(ValueBaselineProperty, pTextEdit->toPlainText());
			if (!pTextEdit->property(ValueConnectionProperty).toBool()) {
				pTextEdit->setProperty(ValueConnectionProperty, true);
				connect(pTextEdit, &QPlainTextEdit::textChanged, this, [this, pTextEdit]() {
					if (!IsHoldingChanges())
						UpdateValueHighlight(pTextEdit);
				});
			}
		}

		if (m_bTrackKeySequences) {
			foreach(QKeySequenceEdit* pKeyEdit, m_pRoot->findChildren<QKeySequenceEdit*>()) {
				if (IsValueExcluded(pKeyEdit))
					continue;
				SetValueHighlight(pKeyEdit, false);
				pKeyEdit->setProperty(ValueBaselineProperty, pKeyEdit->keySequence().toString(QKeySequence::PortableText));
				if (!pKeyEdit->property(ValueConnectionProperty).toBool()) {
					pKeyEdit->setProperty(ValueConnectionProperty, true);
					connect(pKeyEdit, &QKeySequenceEdit::keySequenceChanged, this, [this, pKeyEdit](const QKeySequence&) {
						if (!IsHoldingChanges())
							UpdateValueHighlight(pKeyEdit);
					});
				}
			}
		}
	}

	void Update(QObject* pSource, QTreeWidget* pExcludedTree)
	{
		if (!m_bEnabled)
			return;

		UpdateItemHighlights(pExcludedTree);
		UpdateCheckboxHighlight(pSource);
		UpdateRadioButtonHighlights();
		UpdateValueHighlight(pSource);
		UpdateValueHighlights();
	}

	void UpdateAll(QTreeWidget* pExcludedTree)
	{
		if (!m_bEnabled)
			return;

		UpdateItemHighlights(pExcludedTree);
		UpdateCheckboxHighlights();
		UpdateRadioButtonHighlights();
		UpdateValueHighlights(true);
	}

private:
	enum EItemState { eNone, eAdded, eEdited };
	enum { ItemBaselineRole = Qt::UserRole + 110, ItemStateRole = Qt::UserRole + 111, ItemBackgroundRole = Qt::UserRole + 112 };
	static constexpr const char* CheckboxBaselineProperty = "pending_checkbox_baseline";
	static constexpr const char* CheckboxPaintFilterProperty = "pending_checkbox_paint_filter";
	static constexpr const char* RadioBaselineProperty = "pending_radio_baseline";
	static constexpr const char* RadioPaintFilterProperty = "pending_radio_paint_filter";
	static constexpr const char* RadioConnectionProperty = "pending_radio_connection";
	static constexpr const char* ButtonPaletteProperty = "pending_button_palette";
	static constexpr const char* ButtonAutoFillProperty = "pending_button_auto_fill";
	static constexpr const char* ButtonColorProperty = "pending_button_color";
	static constexpr const char* ValueBaselineProperty = "pending_value_baseline";
	static constexpr const char* ValuePaletteProperty = "pending_value_palette";
	static constexpr const char* ValueAutoFillProperty = "pending_value_auto_fill";
	static constexpr const char* ValueStyleProperty = "pending_value_style";
	static constexpr const char* ValueViewPaletteProperty = "pending_value_view_palette";
	static constexpr const char* ValueViewportPaletteProperty = "pending_value_viewport_palette";
	static constexpr const char* ValueViewportBaseColorProperty = "pending_value_viewport_base_color";
	static constexpr const char* ValueLineEditAutoFillProperty = "pending_value_line_edit_auto_fill";
	static constexpr const char* ValueSpinBoxLineEditAutoFillProperty = "pending_value_spin_box_line_edit_auto_fill";
	static constexpr const char* ValueTextViewportAutoFillProperty = "pending_value_text_viewport_auto_fill";
	static constexpr const char* ValueTextViewportStyleProperty = "pending_value_text_viewport_style";
	static constexpr const char* ValueEditorStyleProperty = "pending_value_editor_style";
	static constexpr const char* ValueConnectionProperty = "pending_value_connection";
	static constexpr const char* ValueComboBackgroundsProperty = "pending_value_combo_backgrounds";
	static constexpr const char* ValueComboSnapshotProperty = "pending_value_combo_snapshot";
	static constexpr const char* ValueComboColorProperty = "pending_value_combo_color";
	static constexpr const char* ValueComboPaintFilterProperty = "pending_value_combo_paint_filter";
	static constexpr const char* ValueExcludedProperty = "pending_value_excluded";

	bool IsHoldingChanges() const { return m_pHoldChange && *m_pHoldChange; }
	static bool IsValueExcluded(const QWidget* pControl) { return pControl->property(ValueExcludedProperty).toBool(); }
	static QString GetStyleKey(const QStyle* pStyle)
	{
		if (!pStyle)
			return QString();
		QString ClassName = pStyle->metaObject()->className();
		if (ClassName.contains("Windows11", Qt::CaseInsensitive))
			return "windows11";
		if (ClassName.contains("WindowsVista", Qt::CaseInsensitive))
			return "windowsvista";
		if (ClassName.contains("Windows", Qt::CaseInsensitive))
			return "windows";
		if (ClassName.contains("Fusion", Qt::CaseInsensitive))
			return "fusion";
		if (QProxyStyle* pProxy = qobject_cast<QProxyStyle*>(const_cast<QStyle*>(pStyle)))
			return GetStyleKey(pProxy->baseStyle());
		return QString();
	}

	static void InstallEditorStyle(QLineEdit* pLineEdit)
	{
		if (pLineEdit->property(ValueEditorStyleProperty).toBool())
			return;
		QStyle* pBase = QStyleFactory::create(GetStyleKey(pLineEdit->style()));
		if (!pBase)
			return;
		CPendingEditorStyle* pStyle = new CPendingEditorStyle(pBase);
		pStyle->setParent(pLineEdit);
		pLineEdit->setStyle(pStyle);
		pLineEdit->setProperty(ValueEditorStyleProperty, true);
	}
	static bool IsComboLineEdit(const QLineEdit* pLineEdit)
	{
		for (const QWidget* pParent = pLineEdit->parentWidget(); pParent; pParent = pParent->parentWidget()) {
			if (const QComboBox* pCombo = qobject_cast<const QComboBox*>(pParent))
				return pCombo->lineEdit() == pLineEdit;
		}
		return false;
	}

	void ClearHighlights(QTreeWidget* pExcludedTree)
	{
		foreach(QTreeWidget* pTree, m_pRoot->findChildren<QTreeWidget*>()) {
			if (pTree == pExcludedTree)
				continue;
			QTreeWidgetItemIterator It(pTree);
			while (*It) {
				SetItemHighlight(*It, eNone);
				++It;
			}
		}

		foreach(QCheckBox* pCheck, m_pRoot->findChildren<QCheckBox*>())
			SetCheckboxHighlight(pCheck, false);
		foreach(QRadioButton* pRadio, m_pRoot->findChildren<QRadioButton*>())
			SetRadioButtonHighlight(pRadio, false);
		foreach(QComboBox* pCombo, m_pRoot->findChildren<QComboBox*>())
			if (!IsValueExcluded(pCombo)) SetValueHighlight(pCombo, false);
		foreach(QAbstractSpinBox* pSpinBox, m_pRoot->findChildren<QAbstractSpinBox*>())
			if (!IsValueExcluded(pSpinBox)) SetValueHighlight(pSpinBox, false);
		foreach(QLineEdit* pLineEdit, m_pRoot->findChildren<QLineEdit*>())
			if (!IsComboLineEdit(pLineEdit) && !IsValueExcluded(pLineEdit)) SetValueHighlight(pLineEdit, false);
		foreach(QPlainTextEdit* pTextEdit, m_pRoot->findChildren<QPlainTextEdit*>())
			if (!IsValueExcluded(pTextEdit)) SetValueHighlight(pTextEdit, false);
		if (m_bTrackKeySequences)
			foreach(QKeySequenceEdit* pKeyEdit, m_pRoot->findChildren<QKeySequenceEdit*>())
				if (!IsValueExcluded(pKeyEdit)) SetValueHighlight(pKeyEdit, false);
	}

	static QColor PendingColor(const QPalette& Palette, QPalette::ColorRole Role, const QColor& Accent = QColor(255, 179, 0))
	{
		QColor Background = Palette.color(QPalette::Active, Role);
		constexpr int Opacity = 72;
		return QColor((Accent.red() * Opacity + Background.red() * (255 - Opacity)) / 255,
			(Accent.green() * Opacity + Background.green() * (255 - Opacity)) / 255,
			(Accent.blue() * Opacity + Background.blue() * (255 - Opacity)) / 255);
	}

	static QString ComboValue(const QComboBox* pCombo)
	{
		return pCombo->isEditable() ? "T" + pCombo->currentText().toUtf8().toBase64() : QString::number(pCombo->currentIndex());
	}

	static void EnsureComboSnapshot(QComboBox* pCombo)
	{
		const QString Snapshot = pCombo->currentText();
		const QString PreviousSnapshot = pCombo->property(ValueComboSnapshotProperty).toString();
		if (!PreviousSnapshot.isEmpty()) {
			int Index = pCombo->findText(PreviousSnapshot, Qt::MatchExactly);
			if (Index >= 0)
				pCombo->removeItem(Index);
			pCombo->setProperty(ValueComboSnapshotProperty, QVariant());
		}
		if (!Snapshot.isEmpty() && pCombo->findText(Snapshot, Qt::MatchExactly) < 0) {
			pCombo->insertItem(0, Snapshot);
			pCombo->setProperty(ValueComboSnapshotProperty, Snapshot);
		}
		pCombo->setCurrentText(Snapshot);
	}

	static QString SpinBoxValue(const QAbstractSpinBox* pSpinBox)
	{
		if (const QSpinBox* pSpin = qobject_cast<const QSpinBox*>(pSpinBox))
			return QString::number(pSpin->value());
		if (const QDoubleSpinBox* pSpin = qobject_cast<const QDoubleSpinBox*>(pSpinBox))
			return QString::number(pSpin->value(), 'g', 17);
		return pSpinBox->text();
	}

	QString GetItemSignature(const QTreeWidgetItem* pItem) const
	{
		QStringList Values;
		for (int i = 0; i < pItem->columnCount(); i++) {
			Values.append(pItem->text(i).toUtf8().toBase64());
			Values.append(pItem->data(i, Qt::UserRole).toString().toUtf8().toBase64());
			Values.append(QString::number(pItem->checkState(i)));
		}
		return Values.join(QChar(0x1E));
	}

	bool IsTemplateItem(const QTreeWidgetItem* pItem) const
	{
		if (m_TemplateItemRole < 0)
			return false;
		if (pItem->data(0, m_TemplateItemRole).toBool())
			return true;
		for (int i = 0; i < pItem->columnCount(); i++) {
			if (pItem->data(i, Qt::UserRole).toInt() == -1)
				return true;
		}
		return false;
	}

	void SetItemHighlight(QTreeWidgetItem* pItem, int State)
	{
		int OldState = pItem->data(0, ItemStateRole).toInt();
		if (State == OldState)
			return;
		for (int i = 0; i < pItem->columnCount(); i++) {
			if (State != eNone) {
				if (OldState == eNone)
					pItem->setData(i, ItemBackgroundRole, pItem->data(i, Qt::BackgroundRole));
				QColor Color = State == eAdded ? QColor(76, 175, 80) : QColor(255, 179, 0);
				Color.setAlpha(72);
				pItem->setData(i, Qt::BackgroundRole, QBrush(Color));
			}
			else if (OldState != eNone) {
				pItem->setData(i, Qt::BackgroundRole, pItem->data(i, ItemBackgroundRole));
				pItem->setData(i, ItemBackgroundRole, QVariant());
			}
		}
		pItem->setData(0, ItemStateRole, State == eNone ? QVariant() : QVariant(State));
	}

	void UpdateItemHighlights(QTreeWidget* pExcludedTree)
	{
		foreach(QTreeWidget* pTree, m_pRoot->findChildren<QTreeWidget*>()) {
			if (pTree == pExcludedTree)
				continue;
			QSignalBlocker Blocker(pTree);
			QTreeWidgetItemIterator It(pTree);
			while (*It) {
				QTreeWidgetItem* pItem = *It;
				if (!IsTemplateItem(pItem)) {
					QVariant Baseline = pItem->data(0, ItemBaselineRole);
					SetItemHighlight(pItem, !Baseline.isValid() ? eAdded : (Baseline.toString() == GetItemSignature(pItem) ? eNone : eEdited));
				}
				++It;
			}
		}
	}

	void SetCheckboxHighlight(QCheckBox* pCheck, bool bPending)
	{
		SetButtonHighlight(pCheck, bPending);
	}

	void UpdateCheckboxHighlight(QObject* pSource)
	{
		QCheckBox* pCheck = qobject_cast<QCheckBox*>(pSource);
		QVariant Baseline = pCheck ? pCheck->property(CheckboxBaselineProperty) : QVariant();
		if (Baseline.isValid())
			SetCheckboxHighlight(pCheck, Baseline.toInt() != (int)pCheck->checkState());
	}

	void UpdateCheckboxHighlights()
	{
		foreach(QCheckBox* pCheck, m_pRoot->findChildren<QCheckBox*>()) {
			QVariant Baseline = pCheck->property(CheckboxBaselineProperty);
			if (Baseline.isValid())
				SetCheckboxHighlight(pCheck, Baseline.toInt() != (int)pCheck->checkState());
		}
	}

	void SetRadioButtonHighlight(QRadioButton* pRadio, bool bPending)
	{
		SetButtonHighlight(pRadio, bPending);
	}

	void UpdateRadioButtonHighlights()
	{
		foreach(QRadioButton* pRadio, m_pRoot->findChildren<QRadioButton*>()) {
			QVariant Baseline = pRadio->property(RadioBaselineProperty);
			if (Baseline.isValid())
				SetRadioButtonHighlight(pRadio, pRadio->isChecked() && !Baseline.toBool());
		}
	}

	void SetComboPopupHighlight(QComboBox* pCombo, bool bPending)
	{
		QAbstractItemModel* pModel = pCombo->model();
		if (!pModel)
			return;
		if (bPending) {
			QVariantList Backgrounds = pCombo->property(ValueComboBackgroundsProperty).toList();
			if (Backgrounds.isEmpty()) {
				for (int i = 0; i < pCombo->count(); i++) {
					QModelIndex Index = pModel->index(i, pCombo->modelColumn(), pCombo->rootModelIndex());
					Backgrounds.append(pModel->data(Index, Qt::BackgroundRole));
				}
				pCombo->setProperty(ValueComboBackgroundsProperty, Backgrounds);
			}
			QString Baseline = pCombo->property(ValueBaselineProperty).toString();
			int Snapshot = pCombo->isEditable() ? pCombo->findText(QString::fromUtf8(QByteArray::fromBase64(Baseline.mid(1).toUtf8()))) : Baseline.toInt();
			QColor Color = PendingColor(pCombo->palette(), QPalette::Base, QColor(76, 175, 80));
			QColor BaseColor = pCombo->property(ValueViewportBaseColorProperty).value<QColor>();
			for (int i = 0; i < pCombo->count(); i++) {
				QModelIndex Index = pModel->index(i, pCombo->modelColumn(), pCombo->rootModelIndex());
				QVariant Background = Backgrounds.value(i);
				pModel->setData(Index, i == Snapshot ? QVariant::fromValue(QBrush(Color)) : (Background.isValid() ? Background : QVariant::fromValue(QBrush(BaseColor))), Qt::BackgroundRole);
			}
		}
		else {
			QVariantList Backgrounds = pCombo->property(ValueComboBackgroundsProperty).toList();
			for (int i = 0; i < qMin(pCombo->count(), Backgrounds.count()); i++) {
				QModelIndex Index = pModel->index(i, pCombo->modelColumn(), pCombo->rootModelIndex());
				pModel->setData(Index, Backgrounds.value(i), Qt::BackgroundRole);
			}
			pCombo->setProperty(ValueComboBackgroundsProperty, QVariant());
		}
	}

	void SetValueHighlight(QWidget* pControl, bool bPending)
	{
		if (bPending) {
			if (!pControl->property(ValuePaletteProperty).isValid()) {
				pControl->setProperty(ValuePaletteProperty, QVariant::fromValue(pControl->palette()));
				pControl->setProperty(ValueAutoFillProperty, pControl->autoFillBackground());
				pControl->setProperty(ValueStyleProperty, pControl->styleSheet());
				if (QComboBox* pCombo = qobject_cast<QComboBox*>(pControl)) {
					pCombo->setProperty(ValueViewPaletteProperty, QVariant::fromValue(pCombo->view()->palette()));
					pCombo->setProperty(ValueViewportPaletteProperty, QVariant::fromValue(pCombo->view()->viewport()->palette()));
					pCombo->setProperty(ValueViewportBaseColorProperty, pCombo->view()->viewport()->palette().color(QPalette::Active, QPalette::Base));
					if (pCombo->isEditable() && pCombo->lineEdit()) {
						InstallEditorStyle(pCombo->lineEdit());
						pCombo->setProperty(ValueLineEditAutoFillProperty, pCombo->lineEdit()->autoFillBackground());
					}
				}
				else if (QAbstractSpinBox* pSpinBox = qobject_cast<QAbstractSpinBox*>(pControl)) {
					if (QLineEdit* pLineEdit = pSpinBox->findChild<QLineEdit*>()) {
						InstallEditorStyle(pLineEdit);
						pSpinBox->setProperty(ValueSpinBoxLineEditAutoFillProperty, pLineEdit->autoFillBackground());
					}
				}
				else if (QPlainTextEdit* pTextEdit = qobject_cast<QPlainTextEdit*>(pControl)) {
					QWidget* pViewport = pTextEdit->viewport();
					pTextEdit->setProperty(ValueTextViewportAutoFillProperty, pViewport->autoFillBackground());
					pTextEdit->setProperty(ValueTextViewportStyleProperty, pViewport->styleSheet());
				}
			}
			QPalette Palette = pControl->palette();
			QColor BaseColor = PendingColor(Palette, QPalette::Base);
			QColor ButtonColor = PendingColor(Palette, QPalette::Button);
			Palette.setColor(QPalette::Active, QPalette::Base, BaseColor);
			Palette.setColor(QPalette::Inactive, QPalette::Base, BaseColor);
			Palette.setColor(QPalette::Active, QPalette::Button, ButtonColor);
			Palette.setColor(QPalette::Inactive, QPalette::Button, ButtonColor);
			pControl->setPalette(Palette);
			pControl->setAutoFillBackground(true);
			if (QComboBox* pCombo = qobject_cast<QComboBox*>(pControl)) {
				if (!pCombo->isEditable()) {
					pCombo->setProperty(ValueComboColorProperty, ButtonColor);
				}
				else if (QLineEdit* pLineEdit = pCombo->lineEdit()) {
					QPalette LineEditPalette = pLineEdit->palette();
					LineEditPalette.setColor(QPalette::Active, QPalette::Base, BaseColor);
					LineEditPalette.setColor(QPalette::Inactive, QPalette::Base, BaseColor);
					pLineEdit->setPalette(LineEditPalette);
					pLineEdit->setProperty("pending_editor_color", BaseColor);
					pLineEdit->setAutoFillBackground(true);
					pLineEdit->style()->unpolish(pLineEdit);
					pLineEdit->style()->polish(pLineEdit);
					pLineEdit->update();
				}
				pCombo->view()->setPalette(pCombo->property(ValueViewPaletteProperty).value<QPalette>());
				QPalette ViewportPalette = pCombo->property(ValueViewportPaletteProperty).value<QPalette>();
				QColor ViewportBaseColor = pCombo->property(ValueViewportBaseColorProperty).value<QColor>();
				ViewportPalette.setColor(QPalette::Active, QPalette::Base, ViewportBaseColor);
				ViewportPalette.setColor(QPalette::Inactive, QPalette::Base, ViewportBaseColor);
				pCombo->view()->viewport()->setPalette(ViewportPalette);
				SetComboPopupHighlight(pCombo, true);
				pCombo->update();
			}
			else if (m_bTrackKeySequences && qobject_cast<QKeySequenceEdit*>(pControl)) {
				pControl->setStyleSheet(pControl->property(ValueStyleProperty).toString()
					+ "\nQKeySequenceEdit, QKeySequenceEdit QLineEdit { background-color: " + BaseColor.name() + "; }");
			}
			else if (QPlainTextEdit* pTextEdit = qobject_cast<QPlainTextEdit*>(pControl)) {
				QWidget* pViewport = pTextEdit->viewport();
				QPalette ViewportPalette = pViewport->palette();
				ViewportPalette.setColor(QPalette::Active, QPalette::Base, BaseColor);
				ViewportPalette.setColor(QPalette::Inactive, QPalette::Base, BaseColor);
				pViewport->setPalette(ViewportPalette);
				pViewport->setAutoFillBackground(true);
				pViewport->setStyleSheet(pTextEdit->property(ValueTextViewportStyleProperty).toString()
					+ "\nQWidget { background-color: " + BaseColor.name() + "; }");
				pViewport->style()->unpolish(pViewport);
				pViewport->style()->polish(pViewport);
				pViewport->update();
			}
			else if (QAbstractSpinBox* pSpinBox = qobject_cast<QAbstractSpinBox*>(pControl)) {
				if (QLineEdit* pLineEdit = pSpinBox->findChild<QLineEdit*>()) {
					QPalette LineEditPalette = pLineEdit->palette();
					LineEditPalette.setColor(QPalette::Active, QPalette::Base, BaseColor);
					LineEditPalette.setColor(QPalette::Inactive, QPalette::Base, BaseColor);
					pLineEdit->setPalette(LineEditPalette);
					pLineEdit->setProperty("pending_editor_color", BaseColor);
					pLineEdit->setAutoFillBackground(true);
					pLineEdit->style()->unpolish(pLineEdit);
					pLineEdit->style()->polish(pLineEdit);
					pLineEdit->update();
				}
			}
			else {
				pControl->setStyleSheet(pControl->property(ValueStyleProperty).toString()
					+ "\nbackground-color: " + BaseColor.name() + ";");
			}
		}
		else if (pControl->property(ValuePaletteProperty).isValid()) {
			pControl->setStyleSheet(pControl->property(ValueStyleProperty).toString());
			pControl->setPalette(pControl->property(ValuePaletteProperty).value<QPalette>());
			pControl->setAutoFillBackground(pControl->property(ValueAutoFillProperty).toBool());
			if (QAbstractSpinBox* pSpinBox = qobject_cast<QAbstractSpinBox*>(pControl)) {
				if (QLineEdit* pLineEdit = pSpinBox->findChild<QLineEdit*>()) {
					pLineEdit->setProperty("pending_editor_color", QVariant());
					pLineEdit->setPalette(QPalette());
					pLineEdit->setAutoFillBackground(pSpinBox->property(ValueSpinBoxLineEditAutoFillProperty).toBool());
					pLineEdit->style()->unpolish(pLineEdit);
					pLineEdit->style()->polish(pLineEdit);
					pLineEdit->update();
				}
			}
			if (QPlainTextEdit* pTextEdit = qobject_cast<QPlainTextEdit*>(pControl)) {
				QWidget* pViewport = pTextEdit->viewport();
				const QString StyleSheet = pTextEdit->property(ValueTextViewportStyleProperty).toString();
				pViewport->setStyleSheet(QString());
				pViewport->setPalette(QPalette());
				pViewport->setAutoFillBackground(pTextEdit->property(ValueTextViewportAutoFillProperty).toBool());
				pViewport->setStyleSheet(StyleSheet);
				pViewport->style()->unpolish(pViewport);
				pViewport->style()->polish(pViewport);
				pViewport->update();
			}
			if (QComboBox* pCombo = qobject_cast<QComboBox*>(pControl)) {
				if (pCombo->isEditable())
					pCombo->setPalette(QPalette());
				pCombo->setProperty(ValueComboColorProperty, QVariant());
				SetComboPopupHighlight(pCombo, false);
				pCombo->view()->setPalette(pCombo->property(ValueViewPaletteProperty).value<QPalette>());
				pCombo->view()->viewport()->setPalette(pCombo->property(ValueViewportPaletteProperty).value<QPalette>());
				if (pCombo->isEditable() && pCombo->lineEdit()) {
					QLineEdit* pLineEdit = pCombo->lineEdit();
					pLineEdit->setProperty("pending_editor_color", QVariant());
					pLineEdit->setPalette(QPalette());
					pLineEdit->setAutoFillBackground(pCombo->property(ValueLineEditAutoFillProperty).toBool());
					pLineEdit->style()->unpolish(pLineEdit);
					pLineEdit->style()->polish(pLineEdit);
					pLineEdit->update();
				}
				pCombo->update();
			}
			pControl->style()->unpolish(pControl);
			pControl->style()->polish(pControl);
			pControl->update();
			pControl->setProperty(ValuePaletteProperty, QVariant());
			pControl->setProperty(ValueAutoFillProperty, QVariant());
			pControl->setProperty(ValueStyleProperty, QVariant());
			pControl->setProperty(ValueViewPaletteProperty, QVariant());
			pControl->setProperty(ValueViewportPaletteProperty, QVariant());
			pControl->setProperty(ValueViewportBaseColorProperty, QVariant());
			pControl->setProperty(ValueLineEditAutoFillProperty, QVariant());
			pControl->setProperty(ValueSpinBoxLineEditAutoFillProperty, QVariant());
			pControl->setProperty(ValueTextViewportAutoFillProperty, QVariant());
			pControl->setProperty(ValueTextViewportStyleProperty, QVariant());
		}
	}

	void UpdateValueHighlight(QObject* pSource)
	{
		if (!m_bEnabled)
			return;

		if (QWidget* pControl = qobject_cast<QWidget*>(pSource)) {
			if (IsValueExcluded(pControl))
				return;
		}

		if (QComboBox* pCombo = qobject_cast<QComboBox*>(pSource)) {
			QVariant Baseline = pCombo->property(ValueBaselineProperty);
			if (Baseline.isValid()) {
				if (pCombo->isEditable()) {
					const QString Snapshot = QString::fromUtf8(QByteArray::fromBase64(Baseline.toString().mid(1).toUtf8()));
					const bool bSnapshotSelected = pCombo->currentIndex() >= 0
						&& pCombo->itemText(pCombo->currentIndex()) == Snapshot;
					SetValueHighlight(pCombo, !bSnapshotSelected && pCombo->currentText() != Snapshot);
				}
				else
					SetValueHighlight(pCombo, Baseline.toString() != ComboValue(pCombo));
			}
		}
		else if (QAbstractSpinBox* pSpinBox = qobject_cast<QAbstractSpinBox*>(pSource)) {
			QVariant Baseline = pSpinBox->property(ValueBaselineProperty);
			if (Baseline.isValid()) SetValueHighlight(pSpinBox, Baseline.toString() != SpinBoxValue(pSpinBox));
		}
		else if (QLineEdit* pLineEdit = qobject_cast<QLineEdit*>(pSource)) {
			QVariant Baseline = pLineEdit->property(ValueBaselineProperty);
			if (Baseline.isValid()) SetValueHighlight(pLineEdit, pLineEdit->isModified() && Baseline.toString() != pLineEdit->text());
		}
		else if (QPlainTextEdit* pTextEdit = qobject_cast<QPlainTextEdit*>(pSource)) {
			QVariant Baseline = pTextEdit->property(ValueBaselineProperty);
			if (Baseline.isValid()) SetValueHighlight(pTextEdit, Baseline.toString() != pTextEdit->toPlainText());
		}
		else if (m_bTrackKeySequences) {
			if (QKeySequenceEdit* pKeyEdit = qobject_cast<QKeySequenceEdit*>(pSource)) {
				QVariant Baseline = pKeyEdit->property(ValueBaselineProperty);
				if (Baseline.isValid()) SetValueHighlight(pKeyEdit, Baseline.toString() != pKeyEdit->keySequence().toString(QKeySequence::PortableText));
			}
		}
	}

	void UpdateValueHighlights(bool bAll = false)
	{
		foreach(QComboBox* pCombo, m_pRoot->findChildren<QComboBox*>())
			if (!IsValueExcluded(pCombo) && (bAll || pCombo->property(ValuePaletteProperty).isValid())) UpdateValueHighlight(pCombo);
		foreach(QAbstractSpinBox* pSpinBox, m_pRoot->findChildren<QAbstractSpinBox*>())
			if (!IsValueExcluded(pSpinBox) && (bAll || pSpinBox->property(ValuePaletteProperty).isValid())) UpdateValueHighlight(pSpinBox);
		foreach(QLineEdit* pLineEdit, m_pRoot->findChildren<QLineEdit*>())
			if (!IsComboLineEdit(pLineEdit) && !IsValueExcluded(pLineEdit) && (bAll || pLineEdit->property(ValuePaletteProperty).isValid())) UpdateValueHighlight(pLineEdit);
		foreach(QPlainTextEdit* pTextEdit, m_pRoot->findChildren<QPlainTextEdit*>())
			if (!IsValueExcluded(pTextEdit) && (bAll || pTextEdit->property(ValuePaletteProperty).isValid())) UpdateValueHighlight(pTextEdit);
		if (m_bTrackKeySequences)
			foreach(QKeySequenceEdit* pKeyEdit, m_pRoot->findChildren<QKeySequenceEdit*>())
				if (!IsValueExcluded(pKeyEdit) && (bAll || pKeyEdit->property(ValuePaletteProperty).isValid())) UpdateValueHighlight(pKeyEdit);
	}

	void SetButtonHighlight(QAbstractButton* pButton, bool bPending)
	{
		if (bPending) {
			QPalette BasePalette = pButton->palette();
			if (!pButton->property(ButtonPaletteProperty).isValid()) {
				pButton->setProperty(ButtonPaletteProperty, QVariant::fromValue(BasePalette));
				pButton->setProperty(ButtonAutoFillProperty, pButton->autoFillBackground());
			}
			else {
				BasePalette = pButton->property(ButtonPaletteProperty).value<QPalette>();
			}

			QPalette Palette = BasePalette;
			QColor Color = PendingColor(BasePalette, QPalette::Window);
			Palette.setColor(QPalette::Active, QPalette::Window, Color);
			Palette.setColor(QPalette::Inactive, QPalette::Window, Color);
			pButton->setPalette(Palette);
			pButton->setAutoFillBackground(true);
			pButton->setProperty(ButtonColorProperty, Color);
			pButton->update();
		}
		else if (pButton->property(ButtonPaletteProperty).isValid()) {
			pButton->setPalette(pButton->property(ButtonPaletteProperty).value<QPalette>());
			pButton->setAutoFillBackground(pButton->property(ButtonAutoFillProperty).toBool());
			pButton->setProperty(ButtonPaletteProperty, QVariant());
			pButton->setProperty(ButtonAutoFillProperty, QVariant());
			pButton->setProperty(ButtonColorProperty, QVariant());
			pButton->style()->unpolish(pButton);
			pButton->style()->polish(pButton);
			pButton->update();
		}
	}

	QWidget* m_pRoot;
	bool* m_pHoldChange;
	int m_TemplateItemRole;
	bool m_bTrackKeySequences;
	bool m_bEnabled = true;
};
