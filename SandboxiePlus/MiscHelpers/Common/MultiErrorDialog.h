#pragma once
#include "PanelView.h"

class MISCHELPERS_EXPORT CMultiErrorDialog : public QDialog
{
	Q_OBJECT

public:
	CMultiErrorDialog(const QString& Title, const QString& Message, const QStringList& Errors, QWidget* parent = 0);
	virtual ~CMultiErrorDialog();

private:
	enum EColumns
	{
		eMessage,
		//eErrorCode,
		//eErrorText,
		eCount
	};

	QGridLayout*			m_pMainLayout;

	CPanelWidgetEx*			m_pErrors;

	QDialogButtonBox *		m_pButtonBox;
};