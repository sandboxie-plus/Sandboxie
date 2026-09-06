// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#ifdef PROXY_TUNNELS_STANDALONE
#include <QtCore>
#else
#include "stdafx.h"
#endif
#include "ProxyWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QHostAddress>
#include <QLineEdit>
#include <QLabel>
#include <QTreeWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QSpinBox>
#include <QPlainTextEdit>


CProxyWindow::CProxyWindow(CProxyManager* Manager, QWidget* Parent) : QDialog(Parent), m_Manager(Manager)
{
	setWindowTitle(tr("Proxy tunnels (experimental)"));
	resize(1000, 620);
	auto Layout = new QVBoxLayout(this);
	m_Notice = new QLabel(this);
	m_Notice->setTextFormat(Qt::PlainText);
	m_Notice->setWordWrap(true);
	Layout->addWidget(m_Notice);
	m_Profiles = new QTreeWidget(this);
	m_Profiles->setHeaderLabels({tr("Profile"), tr("Endpoint"), tr("State"), tr("Last IPv4 TCP exit check"), tr("Sandboxes")});
	m_Profiles->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_Profiles->setRootIsDecorated(false);
	m_Profiles->setAllColumnsShowFocus(true);
	Layout->addWidget(m_Profiles, 3);
	auto Buttons = new QHBoxLayout();
	Layout->addLayout(Buttons);
	auto AddButton = [&](const QString& Text, std::function<void()> Action) {
		auto Button = new QPushButton(Text, this);
		connect(Button, &QPushButton::clicked, this, [Action]() { Action(); });
		Buttons->addWidget(Button);
		return Button;
	};
	AddButton(tr("Add"), [this]() { Edit(true); });
	AddButton(tr("Edit"), [this]() { Edit(false); });
	AddButton(tr("Remove"), [this]() { Remove(); });
	AddButton(tr("Import list"), [this]() { Import(); });
	m_ActivationButtons.append(AddButton(tr("Start selected"), [this]() { Run(0, false); }));
	AddButton(tr("Stop selected"), [this]() { Run(1, false); });
	m_ActivationButtons.append(AddButton(tr("Check exit IP"), [this]() { Run(2, false); }));
	m_ActivationButtons.append(AddButton(tr("Start all"), [this]() { Run(0, true); }));
	AddButton(tr("Stop all"), [this]() { Run(1, true); });
	m_Boxes = new QTreeWidget(this);
	m_Boxes->setHeaderLabels({tr("Sandbox"), tr("Proxy profile"), tr("Association status")});
	m_Boxes->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_Boxes->setRootIsDecorated(false);
	Layout->addWidget(m_Boxes, 2);
	Buttons = new QHBoxLayout();
	Layout->addLayout(Buttons);
	m_ActivationButtons.append(AddButton(tr("Assign selected profile to selected sandboxes"), [this]() { Assign(false); }));
	AddButton(tr("Detach selected sandboxes"), [this]() { Assign(true); });
	AddButton(tr("Install dependencies"), [this]() { QString Message; if (!m_Manager->InstallDependencies(Message)) Error(Message); });
	Buttons->addStretch();
	AddButton(tr("Close"), [this]() { close(); });
	connect(m_Manager, &CProxyManager::Changed, this, &CProxyWindow::Refresh);
	connect(&m_Timer, &QTimer::timeout, this, &CProxyWindow::Refresh);
	Refresh();
}

void CProxyWindow::showEvent(QShowEvent* Event)
{
	QDialog::showEvent(Event);
	m_Timer.start(1000);
	Refresh();
}

void CProxyWindow::hideEvent(QHideEvent* Event)
{
	m_Timer.stop();
	QDialog::hideEvent(Event);
}

void CProxyWindow::Error(const QString& Message)
{
	QMessageBox Box(QMessageBox::Warning, tr("Proxy tunnels"), QString(), QMessageBox::Ok, this);
	Box.setTextFormat(Qt::PlainText);
	Box.setText(Message);
	Box.exec();
}

QStringList CProxyWindow::Selected() const
{
	QStringList Ids;
	for (const auto Item : m_Profiles->selectedItems()) Ids.append(Item->data(0, Qt::UserRole).toString());
	return Ids;
}

void CProxyWindow::Refresh()
{
	if (!isVisible()) return;
	const QStringList ProfileSelection = Selected();
	QStringList BoxSelection;
	for (const auto Item : m_Boxes->selectedItems()) BoxSelection.append(Item->text(0));
	const QString Gate = m_Manager->ActivationError();
	for (auto Button : m_ActivationButtons) {
		Button->setEnabled(Gate.isEmpty());
		Button->setToolTip(Gate);
	}
	m_Notice->setText(Gate.isEmpty()
		? tr("Experimental IPv4 TCP tunnels. Profiles and associations are saved; running state is not. DNS/IPv6/UDP coverage is restricted. A successful IP check is not proof of leak protection.")
		: tr("Activation unavailable: %1\nProfiles can still be reviewed. No tunnel is considered active just because its configuration is saved.").arg(Gate));
	const QList<SProxyBox> Boxes = m_Manager->Boxes();
	QMap<QString, QStringList> Users;
	for (const SProxyBox& Box : Boxes) if (!Box.ProfileId.isEmpty()) Users[Box.ProfileId].append(Box.Name);
	m_Profiles->clear();
	QMap<QString, QString> Names;
	for (const SProxyProfile& Profile : m_Manager->Profiles()) {
		Names.insert(Profile.Id, Profile.Name);
		const auto Runtime = m_Manager->Runtime(Profile.Id);
		const QString Endpoint = Profile.Host.contains(':') ? QString("[%1]:%2").arg(Profile.Host).arg(Profile.Port) : QString("%1:%2").arg(Profile.Host).arg(Profile.Port);
		const bool IPv6Only = QHostAddress(Profile.Host).protocol() == QAbstractSocket::IPv6Protocol;
		const QString State = IPv6Only && Runtime.State == CProxyTunnel::Stopped
			? tr("Stored only (laboratory backend requires IPv4 ingress)") : CProxyTunnel::StateText(Runtime.State);
		QString Egress = Runtime.Egress;
		if (!Egress.isEmpty()) Egress += "  " + Runtime.CheckedAt.toLocalTime().toString(Qt::ISODate);
		auto Item = new QTreeWidgetItem(m_Profiles, {Profile.Name, Profile.Scheme + "://" + Endpoint,
			State, Egress, Users.value(Profile.Id).join(", ")});
		Item->setData(0, Qt::UserRole, Profile.Id);
		if (IPv6Only) Item->setToolTip(1, tr("This profile is valid for storage, but the laboratory backend requires an IPv4-reachable proxy endpoint."));
		Item->setToolTip(2, Runtime.Detail.toHtmlEscaped());
		Item->setSelected(ProfileSelection.contains(Profile.Id));
	}
	m_Boxes->clear();
	for (const SProxyBox& Box : Boxes) {
		auto Item = new QTreeWidgetItem(m_Boxes, {Box.Name, Box.ProfileId.isEmpty() ? tr("Unassigned") : Names.value(Box.ProfileId, tr("Missing profile")), Box.Detail});
		Item->setSelected(BoxSelection.contains(Box.Name));
	}
	for (int i = 0; i < m_Profiles->columnCount(); ++i) m_Profiles->resizeColumnToContents(i);
	for (int i = 0; i < m_Boxes->columnCount(); ++i) m_Boxes->resizeColumnToContents(i);
}

void CProxyWindow::Edit(bool Add)
{
	QString Id, Message;
	SProxyInput Input;
	if (!Add) {
		const QStringList Ids = Selected();
		if (Ids.size() != 1) { Error(tr("Select exactly one profile to edit.")); return; }
		Id = Ids.first();
		for (const auto& Profile : m_Manager->Profiles()) {
			if (Profile.Id == Id && !ProxyProfiles::Reveal(Profile, Input, Message))
				Error(Message + tr("\nRe-enter the credentials. Saving will replace the unavailable protected authentication."));
		}
	}
	QDialog Dialog(this);
	Dialog.setWindowTitle(Add ? tr("Add proxy profile") : tr("Edit proxy profile"));
	auto Layout = new QFormLayout(&Dialog);
	QLineEdit Name(Input.Name), Host(Input.Host), User(Input.User), Password(Input.Password);
	QComboBox Scheme; Scheme.addItems({"socks5", "http"}); Scheme.setCurrentText(Input.Scheme);
	QSpinBox Port; Port.setRange(1, 65535); Port.setValue(Input.Port);
	Password.setEchoMode(QLineEdit::Password);
	Layout->addRow(tr("Name"), &Name); Layout->addRow(tr("Protocol"), &Scheme);
	Layout->addRow(tr("Server"), &Host); Layout->addRow(tr("Port"), &Port);
	Layout->addRow(tr("Username"), &User); Layout->addRow(tr("Password"), &Password);
	QLabel Notice(tr("Authentication is protected for this Windows user, not stored as plaintext. SOCKS5/HTTP authentication is not inherently encrypted on the network."));
	Notice.setWordWrap(true); Layout->addRow(&Notice);
	QDialogButtonBox Buttons(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
	Layout->addRow(&Buttons);
	connect(&Buttons, &QDialogButtonBox::rejected, &Dialog, &QDialog::reject);
	connect(&Buttons, &QDialogButtonBox::accepted, &Dialog, [&]() {
		Input.Name = Name.text(); Input.Scheme = Scheme.currentText(); Input.Host = Host.text();
		Input.Port = Port.value(); Input.User = User.text(); Input.Password = Password.text();
		if (!m_Manager->SaveProfile(Input, Id, Message)) { Error(Message); return; }
		Dialog.accept();
	});
	Dialog.exec();
	Password.clear(); User.clear(); Input.Password.fill(QChar(0)); Input.User.clear();
}

void CProxyWindow::Import()
{
	QDialog Dialog(this); Dialog.setWindowTitle(tr("Import proxy list")); Dialog.resize(650, 430);
	auto Layout = new QVBoxLayout(&Dialog);
	QLabel Help(tr("One entry per line: host:port, host:port:user:password, or socks5:// / http:// URLs. Use brackets for IPv6 and percent-encoding for special URL credentials. Invalid lines are reported by number without echoing secrets."));
	Help.setWordWrap(true); Layout->addWidget(&Help);
	QPlainTextEdit Text; Layout->addWidget(&Text);
	QDialogButtonBox Buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel); Layout->addWidget(&Buttons);
	connect(&Buttons, &QDialogButtonBox::rejected, &Dialog, &QDialog::reject);
	connect(&Buttons, &QDialogButtonBox::accepted, &Dialog, [&]() {
		QList<SProxyImportError> Errors; int Added = 0; QString Message;
		const bool Saved = m_Manager->Import(Text.toPlainText(), Errors, Added, Message);
		QString Report = Saved ? tr("Saved %1 profiles.").arg(Added) : Message;
		for (int i = 0; i < qMin(100, int(Errors.size())); ++i) Report += tr("\nLine %1: %2").arg(Errors[i].Line).arg(Errors[i].Reason);
		if (Errors.size() > 100) Report += tr("\n%1 additional invalid lines.").arg(Errors.size() - 100);
		Error(Report);
		if (Saved) Dialog.accept();
	});
	Dialog.exec(); Text.clear();
}

void CProxyWindow::Remove()
{
	const auto Ids = Selected();
	if (Ids.isEmpty()) { Error(tr("Select a profile first.")); return; }
	if (QMessageBox::question(this, tr("Remove profiles"), tr("Remove the selected profiles? Assigned profiles cannot be removed.")) != QMessageBox::Yes) return;
	QStringList Errors;
	for (const auto& Id : Ids) { QString Message; if (!m_Manager->Remove(Id, Message)) Errors.append(Message); }
	if (!Errors.isEmpty()) Error(Errors.join('\n'));
}

void CProxyWindow::Run(int Action, bool All)
{
	QStringList Ids = Selected();
	if (All) { Ids.clear(); for (const auto& Profile : m_Manager->Profiles()) Ids.append(Profile.Id); }
	if (Ids.isEmpty()) { Error(tr("Select a profile first.")); return; }
	QStringList Errors;
	for (const auto& Id : Ids) {
		if (Action == 1) m_Manager->Stop(Id);
		else if (Action == 2) m_Manager->Check(Id);
		else { QString Message; if (!m_Manager->Start(Id, Message)) Errors.append(Message); }
	}
	Errors.removeDuplicates(); if (!Errors.isEmpty()) Error(Errors.join('\n'));
}

void CProxyWindow::Assign(bool Detach)
{
	QString Id;
	if (!Detach) { const auto Ids = Selected(); if (Ids.size() != 1) { Error(tr("Select one proxy profile.")); return; } Id = Ids.first(); }
	QStringList Boxes;
	for (auto Item : m_Boxes->selectedItems()) Boxes.append(Item->text(0));
	if (Boxes.isEmpty()) { Error(tr("Select one or more sandboxes.")); return; }
	if (QMessageBox::question(this, tr("Change sandbox network association"), Detach
		? tr("Detach these stopped sandboxes? Removing the tunnel binding permits the normal host connection again, subject to other sandbox rules.")
		: tr("Associate these stopped sandboxes with the selected profile? The experimental guard blocks ordinary DNS, UDP, and ICMP. Applications need their own tunneled encrypted DNS. Read the coverage document first.")) != QMessageBox::Yes) return;
	QStringList Errors;
	for (const auto& Box : Boxes) { QString Message; if (!m_Manager->Assign(Box, Id, Message)) Errors.append(Box + ": " + Message); }
	if (!Errors.isEmpty()) Error(Errors.join('\n'));
}
