#pragma once
#include <QDialog>
#include "McpClientSetup.h"

class QListWidget;
class QLabel;
class QPlainTextEdit;
class QPushButton;
class QCheckBox;

//
// CMcpConnectDialog - "connect an agent", the entry point for someone who has
// not done this before.
//
// Built in code rather than as a .ui file because it is a list and four labels,
// and a form that small costs more to maintain in two places than it saves.
//
class CMcpConnectDialog : public QDialog
{
	Q_OBJECT

public:
	CMcpConnectDialog(QWidget* parent = NULL);

private slots:
	void			OnClientChanged();
	void			OnConfigure();
	void			OnLaunch();
	void			OnCopy();
	void			OnToggleServer();
	void			OnForgetRemembered();

private:
	void			Refresh();
	CMcpClientSetup::SClient	CurrentClient() const;

	QList<CMcpClientSetup::SClient>	m_Clients;

	QListWidget*	m_pList;
	QLabel*			m_pStatus;
	QLabel*			m_pExplain;
	QPlainTextEdit*	m_pSnippet;
	QPushButton*	m_pConfigure;
	QPushButton*	m_pLaunch;
	QPushButton*	m_pCopy;
	QPushButton*	m_pToggle;
	QPushButton*	m_pForget;
};
