#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CFinder: public QWidget
{
	Q_OBJECT

public:
	CFinder(QObject* pFilterTarget, QWidget *parent = NULL, int iOptions = eRegExp | eCaseSens | eHighLight);
	~CFinder();

	void				SetTree(QTreeView* pTree);
	void				SetModel(QAbstractItemModel* pModel) { m_pModel = pModel; }

	static void			SetDarkMode(bool bDarkMode) { m_DarkMode = bDarkMode; }
	static bool			GetDarkMode()				{ return m_DarkMode; }

	static QWidget*		AddFinder(QTreeView* pTree, QObject* pFilterTarget, int iOptions = eRegExp | eCaseSens | eHighLight, CFinder** ppFinder = NULL);

	QRegularExpression	GetSearchExp() const		{ return m_RegExp; }

	enum EOptions
	{
		eRegExp		= 0x01,
		eCaseSens	= 0x02,
		eHighLight	= 0x04,
		eHighLightDefault = eHighLight | 0x08,
	};

	static QString m_CaseInsensitive;
	static QString m_RegExpStr;
	static QString m_Highlight;
	static QString m_CloseStr;
	static QString m_FindStr;
	static QString m_AllColumns;

signals:
	void				SetFilter(const QString& Exp, int iOptions = 0, int Column = -1);
	void				SelectNext();

public slots:
	void				Open();
	void				Close();

private slots:
	void				OnUpdate();
	void				OnText();
	void				OnReturn();

	void				OnSelectNext();

protected:
	bool				GetCaseSensitive() const	{ return m_pCaseSensitive ? m_pCaseSensitive->isChecked() : false; }
	bool				GetRegExp() const			{ return m_pRegExp ? m_pRegExp->isChecked() : false; }
	bool				GetHighLight() const		{ return m_pHighLight ? m_pHighLight->isChecked() : false; }
	int					GetColumn() const			{ return m_pColumn ? m_pColumn->currentData().toInt() : -1; }

	bool				eventFilter(QObject* source, QEvent* event);

	virtual bool		MatchString(const QString& value);
	bool				MatchCell(QModelIndex idx, int column);
	bool				MatchRow(QModelIndex idx);
	QModelIndex			FindRow(QModelIndex par, int start, bool reverse);
	QModelIndex			FindRow(bool reverse);

private:

	QHBoxLayout*		m_pSearchLayout;

	QLineEdit*			m_pSearch;
	QCheckBox*			m_pCaseSensitive;
	QCheckBox*			m_pRegExp;
	QComboBox*			m_pColumn;
	QCheckBox*			m_pHighLight;

	QRegularExpression	m_RegExp;

	QTreeView*			m_pTree;
	QAbstractItemModel*	m_pModel;

	QTimer*				m_pTimer;

	static bool			m_DarkMode;
};