#pragma once

#include "../mischelpers_global.h"

#include <QSyntaxHighlighter>

class MISCHELPERS_EXPORT CCodeEdit : public QWidget
{
	Q_OBJECT
public:
	CCodeEdit(QSyntaxHighlighter* pHighlighter, QWidget* pParent = 0);

	void				SetCode(const QString& Code)	{m_pSourceCode->setPlainText(Code);}
	QString				GetCode()						{return m_pSourceCode->toPlainText();}

signals:
	void				textChanged();

private slots:
	void				OnFind();
	void				OnFindNext();
	void				OnGoTo();

	/*void				OnComment();
	void				OnUnComment();*/

private:
	QGridLayout*		m_pMainLayout;

	QTextEdit*			m_pSourceCode;

	QAction*			m_pFind;
	QAction*			m_pFindNext;
	QAction*			m_pGoTo;

	/*QAction*			m_pComment;
	QAction*			m_pUnComment;*/

	QString				m_CurFind;
};