#pragma once

#include <QDialog>
#include <QList>
#include <QSet>
#include <QString>
#include <QVariant>
#include <atomic>
#include <functional>
#include "../QSbieAPI/Sandboxie/BoxBorder.h"
#include "../QSbieAPI/SbieStatus.h"

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QFontComboBox;
class QLabel;
class QPushButton;

class CBorderFontDialog : public QDialog
{
public:
	CBorderFontDialog(const SBoxBorderFont& initial, int legacyReference, double points, const QString& appliedValue,
		const std::function<SB_STATUS(const QString&)>& applyFont, QWidget* parent);
	SBoxBorderFont Specification() const;
	void accept() override;

private:
	void UpdateStyles(const SBoxBorderFont* initial = NULL);
	void UpdatePreview();

	int m_LegacyReference;
	bool m_InitialUnavailable;
	bool m_InitialFamilyUnavailable = false;
	bool m_UpdatingFamily = false;
	QSet<QString> m_TracedFamilies;
	std::function<SB_STATUS(const QString&)> m_ApplyFont;
	QString m_AppliedValue;
	QComboBox* m_Script;
	QFontComboBox* m_Family;
	QComboBox* m_Style;
	QDoubleSpinBox* m_Size;
	QCheckBox* m_Underline;
	QCheckBox* m_Strikeout;
	QLabel* m_Preview;
	QLabel* m_Status;
	QPushButton* m_Ok;
	QPushButton* m_Apply;
	QList<SBoxBorderFontFace> m_Faces;
};
