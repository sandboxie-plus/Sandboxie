#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CFinder: public QWidget
{
	Q_OBJECT

public:
	CFinder(QObject* pFilterTarget, QWidget *parent = NULL, bool HighLightOption = true);
	~CFinder();

	static void			SetDarkMode(bool bDarkMode) { m_DarkMode = bDarkMode; }
	static bool			GetDarkMode() { return m_DarkMode; }

	static QWidget* AddFinder(QWidget* pList, QObject* pFilterTarget, bool HighLightOption = true, CFinder** ppFinder = NULL);

	QRegExp GetRegExp() const;
	bool GetHighLight() const	{ return m_pHighLight ? m_pHighLight->isChecked() : false; }
	int GetColumn() const		{ return m_pColumn ? m_pColumn->currentData().toInt() : -1; }

signals:
	void				SetFilter(const QRegExp& Exp, bool bHighLight = false, int Column = -1);
	void				SelectNext();

public slots:
	void				Open();
	void				Close();

private slots:
	void				OnUpdate();
	void				OnText();
	void				OnReturn();

protected:
	bool				eventFilter(QObject* source, QEvent* event);

private:

	QHBoxLayout*		m_pSearchLayout;

	QLineEdit*			m_pSearch;
	QCheckBox*			m_pCaseSensitive;
	QCheckBox*			m_pRegExp;
	QComboBox*			m_pColumn;
	QCheckBox*			m_pHighLight;

	QSortFilterProxyModel* m_pSortProxy;

	QTimer*				m_pTimer;

	static bool			m_DarkMode;
};