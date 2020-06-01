#pragma once
#include "../../MiscHelpers/Common/PanelView.h"
#include "../../MiscHelpers/Common/FlexError.h"
#include "../../QSbieAPI/SbieError.h"

class CMultiErrorDialog : public QDialog
{
	Q_OBJECT

public:
	CMultiErrorDialog(const QString& Message, QList<SB_STATUS> Errors, QWidget* parent = 0);
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