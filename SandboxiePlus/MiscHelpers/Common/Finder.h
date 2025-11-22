#pragma once

#include "../mischelpers_global.h"

class MISCHELPERS_EXPORT CFinder: public QWidget
{
	Q_OBJECT

public:
	CFinder(QObject* pFilterTarget, QWidget *parent = NULL, int iOptions = eRegExp | eCaseSens | eHighLight);
	~CFinder();

	QAbstractButton*	GetToggleButton();

	void				SetTree(QTreeView* pTree);
	void				SetModel(QAbstractItemModel* pModel) { m_pModel = pModel; }

	void				SetAlwaysRaw() {m_bAlwaysRaw = true;}

	static void			SetDarkMode(bool bDarkMode) { m_DarkMode = bDarkMode; }
	static bool			GetDarkMode()				{ return m_DarkMode; }

	static QWidget*		AddFinder(QTreeView* pTree, QObject* pFilterTarget, int iOptions = eRegExp | eCaseSens | eHighLight, CFinder** ppFinder = NULL);

	const QRegularExpression& GetSearchExp() const	{ return m_RegExp; }

	enum EOptions
	{
		eRegExp		= 0x01,
		eCaseSens	= 0x02,
		eHighLight	= 0x04,
		eHighLightDefault = eHighLight | 0x08,
		eDefault    = eRegExp | eCaseSens | eHighLight,
	};

	static QString m_CaseInsensitive;
	static QString m_RegExpStr;
	static QString m_Highlight;
	static QString m_CloseStr;
	static QString m_FindStr;
	static QString m_AllColumns;
	static QString m_Placeholder;
	static QString m_ButtonTip;

	static QIcon m_CaseInsensitiveIcon;
	static QIcon m_RegExpStrIcon;
	static QIcon m_HighlightIcon;

signals:
	void				SetFilter(const QRegularExpression& RegExp, int iOptions = 0, int Column = -1);
	void				SelectNext();

public slots:
	void				Open();
	void				Close();
	void				OnToggle(bool checked);

	void				SetProgress(int value, int maximum);
	void				ShowProgress();
	void				HideProgress();

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
	QAbstractButton*	m_pCaseSensitive;
	QAbstractButton*	m_pRegExp;
	QComboBox*			m_pColumn;
	QAbstractButton*	m_pHighLight;
	QProgressBar*		m_pProgressBar;

	QRegularExpression	m_RegExp;
	bool				m_bAlwaysRaw;

	QTreeView*			m_pTree;
	QAbstractItemModel*	m_pModel;

	QTimer*				m_pTimer;

	QToolButton*		m_pBtnSearch;

	static bool			m_DarkMode;
};