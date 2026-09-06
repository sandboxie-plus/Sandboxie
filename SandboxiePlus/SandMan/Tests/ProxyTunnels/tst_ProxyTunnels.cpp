// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
#include <QtTest>
#include <QTemporaryDir>
#include <QJsonDocument>
#include <QPushButton>
#include <QTreeWidget>
#include "../../ProxyTunnels/ProxyWindow.h"
#include <atomic>
#include "../../ProxyTunnels/ProxyManager.h"

class CFakeTunnel : public CProxyTunnel
{
public:
	CFakeTunnel(const SProxyProfile& Profile, QObject* Parent, std::shared_ptr<std::atomic<bool>> Failure)
		: CProxyTunnel(Profile, QString(), Parent), Id(Profile.Id), Failure(std::move(Failure)) {}
protected:
	void run() override
	{
		emit StateChanged(Id, Starting, "Fake starting");
		msleep(5);
		if (!isInterruptionRequested()) emit StateChanged(Id, Running, "Fake active, not a network check");
		while (!isInterruptionRequested() && !Failure->load()) msleep(2);
		if (Failure->load()) emit StateChanged(Id, Failed, "Simulated child/proxy/adapter failure");
		else emit StateChanged(Id, Stopped, "Fake stopped");
	}
private:
	QString Id;
	std::shared_ptr<std::atomic<bool>> Failure;
};

class CProxyTests : public QObject
{
	Q_OBJECT
private slots:
	void parse_data()
	{
		QTest::addColumn<QString>("line");
		QTest::addColumn<bool>("valid");
		QTest::newRow("host-port") << "proxy.example:1080" << true;
		QTest::newRow("ipv4-auth") << "192.0.2.10:8000:session-a:secret" << true;
		QTest::newRow("password-colons") << "proxy.example:1080:user:a:b:c" << true;
		QTest::newRow("ipv6-brackets") << "[2001:db8::10]:1080:user:secret" << true;
		QTest::newRow("socks-url") << "socks5://user:p%40ss%3Aword@proxy.example:1080" << true;
		QTest::newRow("http-url") << "http://proxy.example:8080" << true;
		QTest::newRow("idn") << QString::fromUtf8("próxy.example:1080") << true;
		QTest::newRow("ipv6-url") << "socks5://[2001:db8::10]:1080" << true;
		QTest::newRow("zero-port") << "proxy.example:0" << false;
		QTest::newRow("overflow-port") << "proxy.example:65536" << false;
		QTest::newRow("negative-port") << "proxy.example:-1" << false;
		QTest::newRow("missing-port") << "socks5://proxy.example" << false;
		QTest::newRow("missing-host") << "socks5://:1080" << false;
		QTest::newRow("unsupported-scheme") << "ftp://proxy.example:1080" << false;
		QTest::newRow("path") << "socks5://proxy.example:1080/other" << false;
		QTest::newRow("query") << "socks5://proxy.example:1080?password=secret" << false;
		QTest::newRow("fragment") << "socks5://proxy.example:1080#secret" << false;
		QTest::newRow("wildcard4") << "0.0.0.0:1080" << false;
		QTest::newRow("wildcard6") << "[::]:1080" << false;
		QTest::newRow("multicast") << "239.0.0.1:1080" << false;
		QTest::newRow("broadcast") << "255.255.255.255:1080" << false;
		QTest::newRow("malformed-numeric") << "999.0.0.1:1080" << false;
		QTest::newRow("empty-label") << "proxy..example:1080" << false;
		QTest::newRow("label-dash") << "-proxy.example:1080" << false;
		QTest::newRow("whitespace-host") << "proxy example:1080" << false;
		QTest::newRow("scope") << "[fe80::1%eth0]:1080" << false;
		QTest::newRow("empty-username") << "proxy.example:1080::secret" << false;
		QTest::newRow("encoded-newline") << "socks5://user:secret%0A@proxy.example:1080" << false;
		QTest::newRow("literal-control") << QString("proxy.example:1080:user:sec\tret") << false;
		QTest::newRow("oversize-credential") << ("proxy.example:1080:user:" + QString(256, 's')) << false;
	}
	void parse()
	{
		QFETCH(QString, line); QFETCH(bool, valid);
		SProxyInput Input; QString Error;
		QCOMPARE(ProxyProfiles::ParseLine(line, Input, Error), valid);
		if (!valid) { QVERIFY(!Error.isEmpty()); QVERIFY(!Error.contains("secret")); }
	}
	void specialCredentials()
	{
		SProxyInput Input; QString Error;
		QVERIFY(ProxyProfiles::ParseLine("socks5://u%40ser:p%40ss%3Aword@proxy.example:1080", Input, Error));
		QCOMPARE(Input.User, QString("u@ser")); QCOMPARE(Input.Password, QString("p@ss:word"));
		SProxyInput Roundtrip;
		QVERIFY(ProxyProfiles::ParseLine(ProxyProfiles::ProxyUrl(Input), Roundtrip, Error));
		QCOMPARE(Roundtrip.User, Input.User); QCOMPARE(Roundtrip.Password, Input.Password);
	}
	void importErrorsAndSessions()
	{
		QList<SProxyImportError> Errors;
		auto Inputs = ProxyProfiles::ParseList("# comment\r\nproxy.example:1080:a:one\r\ninvalid:secret\nproxy.example:1080:b:two\nproxy.example:1081:a:one\n", Errors);
		QCOMPARE(Inputs.size(), 3); QCOMPARE(Errors.size(), 1); QCOMPARE(Errors.first().Line, 3);
		QVERIFY(!Errors.first().Reason.contains("secret"));
		QCOMPARE(Inputs[0].Host, Inputs[1].Host); QVERIFY(Inputs[0].User != Inputs[1].User);
	}
	void uniqueIdentityAndAtomicPersistence()
	{
		QTemporaryDir Dir; QVERIFY(Dir.isValid()); QString Error;
		SProxyInput Input; QVERIFY(ProxyProfiles::ParseLine("proxy.example:1080", Input, Error));
		SProxyProfile A, B;
		QVERIFY(ProxyProfiles::Protect(Input, A, Error)); QVERIFY(ProxyProfiles::Protect(Input, B, Error));
		QVERIFY(A.Id != B.Id); QVERIFY(A.AdapterName() != B.AdapterName());
		QString Path = Dir.filePath("profiles.json");
		QVERIFY(ProxyProfiles::Save(Path, {A, B}, Error));
		QList<SProxyProfile> Loaded; QVERIFY(ProxyProfiles::Load(Path, Loaded, Error)); QCOMPARE(Loaded.size(), 2);
		QFile Saved(Path); QVERIFY(Saved.open(QIODevice::ReadOnly)); QByteArray Original = Saved.readAll(); Saved.close();
		QVERIFY(!ProxyProfiles::Save(Path, {A, A}, Error));
		QVERIFY(Saved.open(QIODevice::ReadOnly)); QCOMPARE(Saved.readAll(), Original); Saved.close();
		QVERIFY(!Original.contains("active")); QVERIFY(!Original.contains("pid")); QVERIFY(!Original.contains("password"));
		QVERIFY(!ProxyProfiles::Save(Dir.filePath("missing/profiles.json"), {A}, Error));
	}
	void corruptStoreIsNotOverwritten()
	{
		QTemporaryDir Dir; QString Path = Dir.filePath("profiles.json"); QFile File(Path);
		QVERIFY(File.open(QIODevice::WriteOnly)); File.write("{broken"); File.close();
		CProxyManager Manager(Path, {}); QVERIFY(!Manager.StorageError().isEmpty());
		SProxyInput Input; QString Error, Id; QVERIFY(ProxyProfiles::ParseLine("proxy.example:1080", Input, Error));
		QVERIFY(!Manager.SaveProfile(Input, Id, Error));
		QVERIFY(File.open(QIODevice::ReadOnly)); QCOMPARE(File.readAll(), QByteArray("{broken"));
	}
	void exclusiveStoreLock()
	{
		QTemporaryDir Dir; QString Path = Dir.filePath("profiles.json");
		CProxyManager First(Path, {}); CProxyManager Second(Path, {});
		QVERIFY(First.StorageError().isEmpty()); QVERIFY(!Second.StorageError().isEmpty());
	}
	void savedDataValidation()
	{
		SProxyInput Input; SProxyProfile Profile, Out; QString Error;
		QVERIFY(ProxyProfiles::ParseLine("proxy.example:1080", Input, Error)); QVERIFY(ProxyProfiles::Protect(Input, Profile, Error));
		auto Json = Profile.ToJson(); Json["password"] = "secret";
		QVERIFY(!SProxyProfile::FromJson(Json, Out, Error)); QVERIFY(!Error.contains("secret"));
		Json = Profile.ToJson(); Json["id"] = "../adapter"; QVERIFY(!SProxyProfile::FromJson(Json, Out, Error));
		Json = Profile.ToJson(); Json["dpapi"] = "!invalid"; QVERIFY(!SProxyProfile::FromJson(Json, Out, Error));
		Json = Profile.ToJson(); Json["port"] = 10.5; QVERIFY(!SProxyProfile::FromJson(Json, Out, Error));
		Json = Profile.ToJson(); Json["dpapi"] = 123; QVERIFY(!SProxyProfile::FromJson(Json, Out, Error));
	}
	void credentialProtection()
	{
		SProxyInput Input; SProxyProfile Profile; QString Error;
		QVERIFY(ProxyProfiles::ParseLine("proxy.example:1080:user:secret", Input, Error));
#ifdef Q_OS_WIN
		QVERIFY2(ProxyProfiles::Protect(Input, Profile, Error), qPrintable(Error));
		QVERIFY(!Profile.ProtectedAuth.contains("secret"));
		SProxyInput Recovered; QVERIFY(ProxyProfiles::Reveal(Profile, Recovered, Error));
		QCOMPARE(Recovered.Password, Input.Password); QCOMPARE(Recovered.User, Input.User);
		Profile.ProtectedAuth[0] = char(Profile.ProtectedAuth[0] ^ 0x80);
		QVERIFY(!ProxyProfiles::Reveal(Profile, Recovered, Error));
#else
		QVERIFY(!ProxyProfiles::Protect(Input, Profile, Error)); QVERIFY(Profile.Id.isEmpty());
		QSKIP("DPAPI round trip and tamper rejection require Windows; Linux refusal was verified.");
#endif
	}
	void controllerLifecycleAndSharedBoxes()
	{
		QTemporaryDir Dir; QString Path = Dir.filePath("profiles.json"), Error, Id;
		QMap<QString, QString> Associations; int Created = 0; bool Allowed = true;
		auto Failure = std::make_shared<std::atomic<bool>>(false);
		SProxyHooks Hooks;
		Hooks.ActivationError = [&]() { return Allowed ? QString() : QString("Policy no longer permits activation"); };
		Hooks.Boxes = [&]() { QList<SProxyBox> Boxes; for (auto It = Associations.begin(); It != Associations.end(); ++It) Boxes.append({It.key(), It.value(), {}}); return Boxes; };
		Hooks.Assign = [&](const QString& Box, const QString& Profile, QString&) { Associations[Box] = Profile; return true; };
		auto Factory = [&](const SProxyProfile& Profile, const QString&, QObject* Parent) -> CProxyTunnel* {
			++Created; return new CFakeTunnel(Profile, Parent, Failure);
		};
		{
			CProxyManager Manager(Path, Hooks, nullptr, Factory);
			SProxyInput Input; QVERIFY(ProxyProfiles::ParseLine("proxy.example:1080", Input, Error));
			QVERIFY(Manager.SaveProfile(Input, Id, Error));
			QVERIFY(Manager.Assign("BoxA", Id, Error)); QVERIFY(Manager.Assign("BoxB", Id, Error));
			QCOMPARE(Manager.Users(Id).size(), 2); QVERIFY(!Manager.Remove(Id, Error));
			QVERIFY(Manager.Start(Id, Error)); QVERIFY(!Manager.Start(Id, Error)); QCOMPARE(Created, 1);
			QTRY_COMPARE(Manager.Runtime(Id).State, int(CProxyTunnel::Running));
			QVERIFY(!Manager.SaveProfile(Input, Id, Error));
			Failure->store(true);
			QTRY_VERIFY(!Manager.HasWorker(Id)); QCOMPARE(Manager.Runtime(Id).State, int(CProxyTunnel::Failed));
			QCOMPARE(Manager.Users(Id).size(), 2); QVERIFY(Manager.Runtime(Id).Egress.isEmpty());
			Failure->store(false); QVERIFY(Manager.Start(Id, Error)); Manager.Stop(Id);
			QTRY_VERIFY(!Manager.HasWorker(Id)); QCOMPARE(Manager.Runtime(Id).State, int(CProxyTunnel::Stopped));
			QVERIFY(Manager.Start(Id, Error)); QTRY_COMPARE(Manager.Runtime(Id).State, int(CProxyTunnel::Running));
			Allowed = false; QTRY_VERIFY(!Manager.HasWorker(Id)); QCOMPARE(Manager.Users(Id).size(), 2);
			QVERIFY(!Manager.Start(Id, Error));
		}
		Allowed = true;
		CProxyManager Restarted(Path, Hooks, nullptr, Factory);
		QCOMPARE(Restarted.Profiles().size(), 1); QCOMPARE(Restarted.Users(Id).size(), 2);
		QCOMPARE(Restarted.Runtime(Id).State, int(CProxyTunnel::Stopped)); QVERIFY(!Restarted.HasWorker(Id));
	}
	void bulkStartStopAndShutdown()
	{
		QTemporaryDir Dir; QString Error; int Created = 0;
		SProxyHooks Hooks; Hooks.ActivationError = []() { return QString(); };
		auto Failure = std::make_shared<std::atomic<bool>>(false);
		CProxyManager Manager(Dir.filePath("profiles.json"), Hooks, nullptr,
			[&](const SProxyProfile& Profile, const QString&, QObject* Parent) -> CProxyTunnel* { ++Created; return new CFakeTunnel(Profile, Parent, Failure); });
		SProxyInput Input; QVERIFY(ProxyProfiles::ParseLine("proxy.example:1080", Input, Error));
		QStringList Ids;
		for (int i = 0; i < 3; ++i) { QString Id; QVERIFY(Manager.SaveProfile(Input, Id, Error)); Ids.append(Id); QVERIFY(Manager.Start(Id, Error)); }
		QCOMPARE(Created, 3); Manager.StopAll();
		for (const auto& Id : Ids) QTRY_VERIFY(!Manager.HasWorker(Id));
		for (const auto& Id : Ids) QVERIFY(Manager.Start(Id, Error));
		// Destruction must cancel and join each owned worker without detached processes.
	}
	void detachRemainsAvailableWhenActivationFails()
	{
		QTemporaryDir Dir; QString Error; bool Detached = false;
		SProxyHooks Hooks;
		Hooks.ActivationError = []() { return QString("Tunnel activation is unavailable"); };
		Hooks.Assign = [&](const QString& Box, const QString& Id, QString&) { Detached = Box == "BoxA" && Id.isEmpty(); return Detached; };
		CProxyManager Manager(Dir.filePath("profiles.json"), Hooks);
		QVERIFY(Manager.Assign("BoxA", QString(), Error));
		QVERIFY(Detached);
	}
	void importLimitsAndPartialCommit()
	{
		QList<SProxyImportError> Errors;
		const auto Inputs = ProxyProfiles::ParseList(QString("proxy.example:1080\n").repeated(1002), Errors);
		QCOMPARE(Inputs.size(), 1000); QCOMPARE(Errors.size(), 2);
		QCOMPARE(Errors.first().Line, 1001);
		QTemporaryDir Dir; CProxyManager Manager(Dir.filePath("profiles.json"), {});
		int Added = -1; QString Error;
		QVERIFY(Manager.Import("proxy.example:1080\ninvalid:secret\nproxy.example:1081", Errors, Added, Error));
		QCOMPARE(Added, 2); QCOMPARE(Errors.size(), 1); QCOMPARE(Manager.Profiles().size(), 2);
		QVERIFY(!Manager.Import("invalid:secret", Errors, Added, Error));
		QCOMPARE(Added, 0); QCOMPARE(Manager.Profiles().size(), 2);
	}
	void defaultBackendCannotActivate()
	{
		SProxyInput Input; SProxyProfile Profile; QString Error;
		QVERIFY(ProxyProfiles::ParseLine("proxy.example:1080", Input, Error));
		QVERIFY(ProxyProfiles::Protect(Input, Profile, Error));
		CProxyTunnel Worker(Profile, QString());
		QSignalSpy States(&Worker, &CProxyTunnel::StateChanged);
		QSignalSpy Checks(&Worker, &CProxyTunnel::EgressChecked);
		Worker.start(); QVERIFY(Worker.wait(2000));
		QCoreApplication::processEvents();
		QVERIFY(States.size() >= 2); QCOMPARE(States.last().at(1).toInt(), int(CProxyTunnel::Failed));
		QCOMPARE(Checks.size(), 0);
	}
	void nativeWindowSmoke()
	{
		QTemporaryDir Dir; QString Error, Id; SProxyInput Input; int BoxQueries = 0;
		SProxyHooks Hooks;
		Hooks.Boxes = [&]() { ++BoxQueries; return QList<SProxyBox>{{"BoxA", Id, "Saved / offline"}, {"BoxB", Id, "Saved / offline"}}; };
		CProxyManager Manager(Dir.filePath("profiles.json"), Hooks);
		QVERIFY(ProxyProfiles::ParseLine("proxy.example:1080", Input, Error));
		Input.Name = "Shared profile"; QVERIFY(Manager.SaveProfile(Input, Id, Error));
		QString SecondId; Input.Name = "Second profile"; QVERIFY(Manager.SaveProfile(Input, SecondId, Error));
		CProxyWindow Window(&Manager);
		BoxQueries = 0; Input.Name = "Shared profile updated"; QVERIFY(Manager.SaveProfile(Input, Id, Error));
		QCOMPARE(BoxQueries, 1);
		const auto Trees = Window.findChildren<QTreeWidget*>();
		QCOMPARE(Trees.size(), 2); QCOMPARE(Trees[0]->topLevelItemCount(), 2); QCOMPARE(Trees[1]->topLevelItemCount(), 2);
		QCOMPARE(Trees[0]->topLevelItem(0)->text(4), QString("BoxA, BoxB"));
		QVERIFY(Window.windowTitle().contains("experimental"));
		int DisabledActions = 0;
		for (auto Button : Window.findChildren<QPushButton*>()) {
			if (Button->text() == "Start selected" || Button->text() == "Start all" || Button->text() == "Check exit IP") {
				QVERIFY(!Button->isEnabled());
				++DisabledActions;
			}
		}
		QCOMPARE(DisabledActions, 3);
		Window.show(); QTest::qWait(20); BoxQueries = 0; Window.hide(); QTest::qWait(1100); QCOMPARE(BoxQueries, 0);
	}
	void absentIntegrationFailsClosed()
	{
		QTemporaryDir Dir; QString Error, Id; SProxyInput Input;
		CProxyManager Manager(Dir.filePath("profiles.json"), {});
		QVERIFY(ProxyProfiles::ParseLine("proxy.example:1080", Input, Error)); QVERIFY(Manager.SaveProfile(Input, Id, Error));
		QVERIFY(!Manager.Start(Id, Error)); QVERIFY(!Manager.Assign("BoxA", Id, Error)); QVERIFY(!Manager.HasWorker(Id));
	}
};
QTEST_MAIN(CProxyTests)
#include "tst_ProxyTunnels.moc"
