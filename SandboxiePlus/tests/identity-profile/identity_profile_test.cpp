#include <QApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QLockFile>
#include <QMap>
#include <QSet>
#include <QTemporaryDir>
#include <cstdio>
#include <cstdlib>

#include "Helpers/IdentityProfile.h"
#include "Windows/IdentityProfilesDialog.h"

#define CHECK(condition) do { if (!(condition)) { \
	std::fprintf(stderr, "FAIL at line %d: %s\n", __LINE__, #condition); std::exit(1); \
} } while (0)

// Synthetic box section: a multimap of setting -> values, plus template values that must stay untouched.
struct SFakeBox : public CIdentityIniTarget
{
	QMap<QString, QStringList> Values;
	QMap<QString, QStringList> TemplateValues;
	int Processes = 0;
	bool FailWrites = false;
	int Writes = 0;
	bool Deferred = false;		// writes land in Pending until Flush(), like SbieSvc with refresh disabled
	bool DropOnFlush = false;	// Flush() discards the pending writes instead of publishing them
	int Flushes = 0;
	QMap<QString, QStringList> Pending;
	bool HasPending = false;

	QString GetText(const QString& Setting) const { return Values.value(Setting).value(0); }
	QStringList GetTextList(const QString& Setting, bool withTemplates) const
	{
		QStringList List = Values.value(Setting);
		if (withTemplates) List.append(TemplateValues.value(Setting));
		return List;
	}
	QMap<QString, QStringList>& Store() { if (Deferred) { if (!HasPending) { Pending = Values; HasPending = true; } return Pending; } return Values; }
	bool SetText(const QString& Setting, const QString& Value)
	{
		++Writes; if (FailWrites) return false;
		if (Deferred && Values.value(Setting) == (QStringList() << Value)) return true;
		Store()[Setting] = QStringList() << Value; return true;
	}
	bool AppendText(const QString& Setting, const QString& Value)
	{
		++Writes; if (FailWrites) return false;
		Store()[Setting].append(Value); return true;
	}
	bool DelValue(const QString& Setting, const QString& Value)
	{
		++Writes; if (FailWrites) return false;
		QMap<QString, QStringList>& S = Store();
		if (Value.isEmpty()) S.remove(Setting);
		else { S[Setting].removeOne(Value); if (S[Setting].isEmpty()) S.remove(Setting); }
		return true;
	}
	int GetActiveProcessCount() const { return Processes; }
	bool Flush()
	{
		++Flushes;
		if (!Deferred) return true;
		if (!DropOnFlush && HasPending) Values = Pending;
		Pending.clear(); HasPending = false;
		return true;
	}
};

static CIdentityProfile MakeProfile(const QString& Name, int Volumes = 2)
{
	CIdentityProfile Profile;
	Profile.Id = CIdentityProfile::NewId();
	Profile.Name = Name;
	for (int i = 1; i <= Volumes; i++) {
		SIdentityVolume Volume;
		Volume.Device = "HarddiskVolume" + QString::number(i);
		Volume.Serial = QString("%1-%2").arg(i, 4, 16, QChar('0')).arg(0xABCD, 4, 16, QChar('0')).toUpper();
		Profile.Volumes.append(Volume);
	}
	return Profile;
}

static QStringList Sorted(QStringList List) { List.sort(); return List; }

static QSet<QString> Snapshot(const QString& Dir)
{
	QSet<QString> Files;
	QDir Root(Dir);
	foreach(const QString& Entry, Root.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name))
		Files.insert(Entry);
	return Files;
}

//---------------------------------------------------------------------------

static void NormalizeDevice()
{
	CHECK(CIdentityProfile::NormalizeDevice("HarddiskVolume3") == "HarddiskVolume3");
	CHECK(CIdentityProfile::NormalizeDevice("\\Device\\HarddiskVolume12") == "HarddiskVolume12");
	CHECK(CIdentityProfile::NormalizeDevice("  harddiskvolume7 ") == "HarddiskVolume7");
	QString Error;
	CHECK(CIdentityProfile::NormalizeDevice("C:", &Error).isEmpty() && Error.contains("C:"));
	CHECK(CIdentityProfile::NormalizeDevice("HarddiskVolume0").isEmpty());
	CHECK(CIdentityProfile::NormalizeDevice("PhysicalDrive0").isEmpty());
	CHECK(CIdentityProfile::NormalizeDevice("HarddiskVolume1\\x").isEmpty());
	CHECK(CIdentityProfile::NormalizeDevice("").isEmpty());
}

static void NormalizeSerial()
{
	CHECK(CIdentityProfile::NormalizeSerial("1234-abcd") == "1234-ABCD");
	CHECK(CIdentityProfile::NormalizeSerial("1234ABCD") == "1234-ABCD");
	CHECK(CIdentityProfile::NormalizeSerial(" 0000-0000 ") == "0000-0000");
	CHECK(CIdentityProfile::NormalizeSerial("1234-ABC").isEmpty());
	CHECK(CIdentityProfile::NormalizeSerial("1234-ABCDE").isEmpty());
	CHECK(CIdentityProfile::NormalizeSerial("GGGG-0000").isEmpty());
	CHECK(CIdentityProfile::NormalizeSerial("").isEmpty());
	for (int i = 0; i < 64; i++) {
		QString Random = CIdentityProfile::RandomSerial();
		CHECK(CIdentityProfile::NormalizeSerial(Random) == Random);
	}
	QString A = CIdentityProfile::RandomSerial();
	CHECK(CIdentityProfile::NormalizeSerial(A) == A);
	CHECK(A.size() == 9 && A[4] == '-');
}

static void ValidateInvalid()
{
	CIdentityProfile Good = MakeProfile("good");
	CHECK(Good.IsValid());

	CIdentityProfile P = Good; P.Version = 2;
	CHECK(!P.IsValid());
	P = Good; P.Id = "not-a-uuid";
	CHECK(!P.IsValid());
	P = Good; P.Name = "   ";
	CHECK(!P.IsValid());
	P = Good; P.Volumes.clear();
	CHECK(!P.IsValid());
	P = Good; P.Volumes.append(P.Volumes[0]);
	CHECK(!P.IsValid());
	P = Good; P.Volumes[0].Device = "harddiskvolume1";
	CHECK(!P.IsValid());
	P = Good; P.Volumes[0].Serial = "1234abcd";
	CHECK(!P.IsValid());
	P = Good; P.Volumes[1].Serial = "XYZ";
	CHECK(!P.IsValid());
	P = Good; P.Revision = -1;
	CHECK(!P.IsValid());
}

static void JsonCorrupt()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	QString Id = CIdentityProfile::NewId();
	QString Path = Store.PathFor(Id);
	QDir().mkpath(Dir.path());

	CIdentityProfile Profile; QString Error;
	QFile File(Path); CHECK(File.open(QIODevice::WriteOnly)); File.write("{ not json"); File.close();
	CHECK(!Store.Load(Id, Profile, &Error) && !Error.isEmpty());

	CHECK(File.open(QIODevice::WriteOnly | QIODevice::Truncate)); File.write("[1,2,3]"); File.close();
	CHECK(!Store.Load(Id, Profile, &Error));

	CHECK(File.open(QIODevice::WriteOnly | QIODevice::Truncate)); File.write("{\"format\":\"something-else\",\"version\":1,\"volumes\":[]}"); File.close();
	CHECK(!Store.Load(Id, Profile, &Error) && Error.contains("not an identity profile"));

	CIdentityProfile Other = MakeProfile("other");
	CHECK(File.open(QIODevice::WriteOnly | QIODevice::Truncate)); File.write(QJsonDocument(Other.ToJson()).toJson()); File.close();
	CHECK(!Store.Load(Id, Profile, &Error) && Error.contains("does not match"));

	QJsonObject Json = Other.ToJson(); Json["version"] = 99;
	CHECK(File.open(QIODevice::WriteOnly | QIODevice::Truncate)); File.write(QJsonDocument(Json).toJson()); File.close();
	CHECK(!Store.Load(Id, Profile, &Error) && Error.contains("version"));
}

static void StoreRoundtrip()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfile Profile = MakeProfile("round trip", 3);
	{
		CIdentityProfileStore Store(Dir.path() + "/profiles");
		CHECK(Store.Save(Profile));
		CHECK(Profile.Revision == 1 && !Profile.Created.isEmpty() && Profile.Updated == Profile.Created);
	}
	{
		// A second store instance stands in for a new SandMan process.
		CIdentityProfileStore Store(Dir.path() + "/profiles");
		CIdentityProfile Loaded;
		CHECK(Store.Load(Profile.Id, Loaded));
		CHECK(Loaded.ToJson() == Profile.ToJson());
		CHECK(Loaded.Volumes.size() == 3 && Loaded.Volumes[2].Device == "HarddiskVolume3");
		CHECK(Loaded.DiskSerialNumberValues() == (QStringList() << "HarddiskVolume1,0001-ABCD" << "HarddiskVolume2,0002-ABCD" << "HarddiskVolume3,0003-ABCD"));
		Loaded.Name = "renamed";
		CHECK(Store.Save(Loaded));
		CHECK(Loaded.Revision == 2 && Loaded.Created == Profile.Created);
		CIdentityProfile Again;
		CHECK(Store.Load(Profile.Id, Again) && Again.Name == "renamed" && Again.Revision == 2);
		CHECK(Again.Volumes == Profile.Volumes);
	}
}

static void StoreStaleWriter()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore First(Dir.path()), Second(Dir.path());
	CIdentityProfile Profile = MakeProfile("original");
	CHECK(First.Save(Profile));
	CIdentityProfile Stale; CHECK(Second.Load(Profile.Id, Stale));
	Profile.Volumes[0].Serial = "1234-5678";
	CHECK(First.Save(Profile));
	Stale.Name = "stale rename";
	QString Error;
	CHECK(!Second.Save(Stale, &Error) && Error.contains("changed"));
	CIdentityProfile Loaded; CHECK(First.Load(Profile.Id, Loaded));
	CHECK(Loaded.ToJson() == Profile.ToJson() && Stale.Revision == 1);
	CHECK(Second.Load(Profile.Id, Stale));
	Stale.Name = "fresh rename";
	CHECK(Second.Save(Stale) && Stale.Revision == 3);
	CHECK(First.Remove(Profile.Id));
	CHECK(!Second.Save(Stale, &Error) && !QFile::exists(Second.PathFor(Profile.Id)));
}

static void StoreLockedWriter()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("locked");
	CHECK(Store.Save(Profile));
	QLockFile Lock(Store.PathFor(Profile.Id) + ".lock");
	CHECK(Lock.tryLock(0));
	QString Error;
	CHECK(!Store.Save(Profile, &Error) && !Error.isEmpty());
	CHECK(!Store.Remove(Profile.Id, &Error));
	Lock.unlock();
	CHECK(Store.Save(Profile) && Profile.Revision == 2);
}

static void StoreAtomicFailure()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("atomic");
	CHECK(Store.Save(Profile));
	QByteArray Before; { QFile File(Store.PathFor(Profile.Id)); CHECK(File.open(QIODevice::ReadOnly)); Before = File.readAll(); }

	// Make the target path a directory so the replacement cannot be committed.
	CHECK(QFile::remove(Store.PathFor(Profile.Id)));
	CHECK(QDir().mkpath(Store.PathFor(Profile.Id)));
	CIdentityProfile Changed = Profile; Changed.Name = "changed";
	QString Error;
	CHECK(!Store.Save(Changed, &Error) && !Error.isEmpty());
	CHECK(Changed.Revision == Profile.Revision && Changed.Name == "changed");
	CHECK(QDir(Store.PathFor(Profile.Id)).exists());
	CHECK(QDir(Store.PathFor(Profile.Id)).entryList(QDir::Files).isEmpty());
	CHECK(QDir().rmdir(Store.PathFor(Profile.Id)));
	{ QFile File(Store.PathFor(Profile.Id)); CHECK(File.open(QIODevice::WriteOnly)); File.write(Before); }
	CIdentityProfile Loaded;
	CHECK(Store.Load(Profile.Id, Loaded) && Loaded.Name == "atomic" && Loaded.Revision == 1);

	// Every save leaves exactly one file behind, no temporaries.
	CHECK(Store.Save(Loaded));
	CHECK(QDir(Dir.path()).entryList(QDir::Files).size() == 1);
}

static void StoreListSkipsBad()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile A = MakeProfile("a"), B = MakeProfile("b");
	CHECK(Store.Save(A) && Store.Save(B));
	{ QFile File(Dir.path() + "/notes.json"); CHECK(File.open(QIODevice::WriteOnly)); File.write("{}"); }
	{ QFile File(Store.PathFor(CIdentityProfile::NewId())); CHECK(File.open(QIODevice::WriteOnly)); File.write("garbage"); }
	QStringList Problems;
	QList<CIdentityProfile> List = Store.List(&Problems);
	CHECK(List.size() == 2 && Problems.size() == 2);
	QStringList Names; foreach(const CIdentityProfile& P, List) Names.append(P.Name);
	CHECK(Sorted(Names) == (QStringList() << "a" << "b"));
}

static void StoreRemove()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile A = MakeProfile("a");
	CHECK(Store.Save(A));
	QString Error;
	CHECK(!Store.Remove("../etc", &Error));
	CHECK(!Store.Remove(CIdentityProfile::NewId(), &Error) && Error.contains("not exist"));
	CHECK(Store.Remove(A.Id));
	CHECK(!QFile::exists(Store.PathFor(A.Id)));
	CHECK(Store.List().isEmpty());
}

static void RegenerateExplicit()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("regen", 3);
	CHECK(Store.Save(Profile));
	QList<SIdentityVolume> Original = Profile.Volumes;

	// Loading, editing the name and saving keeps every serial: nothing regenerates on its own.
	CIdentityProfile Loaded; CHECK(Store.Load(Profile.Id, Loaded));
	Loaded.Name = "renamed";
	CHECK(Store.Save(Loaded) && Loaded.Volumes == Original);
	CHECK(Store.Load(Profile.Id, Loaded) && Loaded.Volumes == Original);

	Loaded.Regenerate();
	CHECK(Loaded.Volumes.size() == 3);
	for (int i = 0; i < 3; i++) {
		CHECK(Loaded.Volumes[i].Device == Original[i].Device);
		CHECK(Loaded.Volumes[i].Serial != Original[i].Serial);
		CHECK(CIdentityProfile::NormalizeSerial(Loaded.Volumes[i].Serial) == Loaded.Volumes[i].Serial);
	}
	CHECK(Loaded.Id == Profile.Id);
	CHECK(Store.Save(Loaded) && Loaded.Revision == 3);
}

static void CloneNewId()
{
	CIdentityProfile Profile = MakeProfile("source", 2);
	Profile.Revision = 5; Profile.Created = "2026-01-01T00:00:00Z"; Profile.Updated = Profile.Created;
	CIdentityProfile Copy = Profile.Clone("copy");
	CHECK(Copy.Id != Profile.Id && CIdentityProfile::IsValidId(Copy.Id));
	CHECK(Copy.Name == "copy" && Copy.Revision == 0 && Copy.Created.isEmpty());
	CHECK(Copy.Volumes == Profile.Volumes);
	CHECK(Copy.IsValid());
}

static void ImportNewId()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path() + "/store");
	CIdentityProfile Profile = MakeProfile("exported", 2);
	CHECK(Store.Save(Profile));
	QString Path = Dir.path() + "/exported.json";
	CHECK(Store.Export(Profile, Path));

	CIdentityProfile Imported; QString Error;
	CHECK(Store.Import(Path, Imported, &Error));
	CHECK(Imported.Id != Profile.Id && Imported.Revision == 0 && Imported.Volumes == Profile.Volumes);
	CHECK(Imported.Name == "exported");
	CHECK(Store.Save(Imported));
	CHECK(Store.List().size() == 2);
	CIdentityProfile Original; CHECK(Store.Load(Profile.Id, Original) && Original.Revision == 1);

	{ QFile File(Path); CHECK(File.open(QIODevice::WriteOnly | QIODevice::Truncate)); File.write("{\"format\":\"sandboxie-identity-profile\",\"version\":1,\"id\":\"x\",\"name\":\"bad\",\"volumes\":[{\"device\":\"C:\",\"serial\":\"1\"}]}"); }
	CHECK(!Store.Import(Path, Imported, &Error) && Error.contains("invalid volume device"));
	CHECK(!Store.Import(Dir.path() + "/missing.json", Imported, &Error));
}

static void ExportAtomic()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("export");
	QString Path = Dir.path() + "/out/profile.json";
	QString Error;
	CHECK(!Store.Export(Profile, Path, &Error));	// unsaved profile with revision 0 is still valid, but the directory is missing
	CHECK(!QFile::exists(Path));
	CHECK(QDir().mkpath(Dir.path() + "/out"));
	CHECK(Store.Export(Profile, Path));
	CHECK(QDir(Dir.path() + "/out").entryList(QDir::Files) == (QStringList() << "profile.json"));
	CIdentityProfile Invalid = Profile; Invalid.Volumes.clear();
	CHECK(!Store.Export(Invalid, Path, &Error) && Error.contains("no volumes"));
}

//---------------------------------------------------------------------------

static void BindApply()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("bind", 2);
	CHECK(Store.Save(Profile));

	SFakeBox Box;
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eUnbound);
	QString Error;
	CHECK(CIdentityProfileBinding::Apply(Box, Profile, &Error));
	CHECK(Box.Values["IdentityProfile"] == (QStringList() << Profile.Id));
	CHECK(Box.Values["IdentityProfileRevision"] == (QStringList() << "1"));
	CHECK(Box.Values["HideDiskSerialNumber"] == (QStringList() << "y"));
	CHECK(Sorted(Box.Values["DiskSerialNumber"]) == Sorted(Profile.DiskSerialNumberValues()));
	CHECK(Box.Values.size() == 4);

	SIdentityBindingState State = CIdentityProfileBinding::Read(Box, Store);
	CHECK(State.State == SIdentityBindingState::eApplied && State.AppliedRevision == 1 && State.ProfileRevision == 1);

	// Re-applying the same revision is idempotent.
	int Writes = Box.Writes;
	CHECK(CIdentityProfileBinding::Apply(Box, Profile, &Error));
	CHECK(Box.Values["DiskSerialNumber"].size() == 2);
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eApplied);
	CHECK(Box.Writes > Writes);
}

static void BindTwoBoxes()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile A = MakeProfile("a", 1), B = MakeProfile("b", 2);
	B.Volumes[0].Serial = "DEAD-BEEF";
	CHECK(Store.Save(A) && Store.Save(B));

	SFakeBox BoxA, BoxB, BoxC;
	CHECK(CIdentityProfileBinding::Apply(BoxA, A));
	CHECK(CIdentityProfileBinding::Apply(BoxB, B));
	CHECK(CIdentityProfileBinding::Apply(BoxC, B));	// two boxes may share one profile
	CHECK(BoxA.Values["DiskSerialNumber"] == (QStringList() << "HarddiskVolume1,0001-ABCD"));
	CHECK(BoxB.Values["DiskSerialNumber"] == (QStringList() << "HarddiskVolume1,DEAD-BEEF" << "HarddiskVolume2,0002-ABCD"));
	CHECK(BoxC.Values["DiskSerialNumber"] == BoxB.Values["DiskSerialNumber"]);
	CHECK(BoxA.Values["IdentityProfile"] != BoxB.Values["IdentityProfile"]);
	CHECK(CIdentityProfileBinding::Read(BoxA, Store).State == SIdentityBindingState::eApplied);
	CHECK(CIdentityProfileBinding::Read(BoxB, Store).State == SIdentityBindingState::eApplied);

	// Regenerating B only affects boxes bound to B, and only after they are re-applied.
	B.Regenerate(); CHECK(Store.Save(B));
	CHECK(CIdentityProfileBinding::Read(BoxA, Store).State == SIdentityBindingState::eApplied);
	CHECK(CIdentityProfileBinding::Read(BoxB, Store).State == SIdentityBindingState::eStale);
	CHECK(BoxB.Values["DiskSerialNumber"][0] == "HarddiskVolume1,DEAD-BEEF");
	CHECK(CIdentityProfileBinding::Apply(BoxB, B));
	CHECK(BoxB.Values["DiskSerialNumber"] == B.DiskSerialNumberValues());
	CHECK(BoxC.Values["DiskSerialNumber"][0] == "HarddiskVolume1,DEAD-BEEF");
	CHECK(CIdentityProfileBinding::Read(BoxC, Store).State == SIdentityBindingState::eStale);
}

static void BindRefusesRunning()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("running");
	CHECK(Store.Save(Profile));
	SFakeBox Box; Box.Processes = 1;
	QString Error;
	CHECK(!CIdentityProfileBinding::Apply(Box, Profile, &Error) && Error.contains("stop all processes"));
	CHECK(Box.Values.isEmpty() && Box.Writes == 0);
	Box.Processes = 0;
	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	Box.Processes = 2;
	CHECK(!CIdentityProfileBinding::Unbind(Box, &Error) && Error.contains("stop all processes"));
	CHECK(Box.Values["IdentityProfile"] == (QStringList() << Profile.Id));
}

static void BindRefusesManual()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("manual");
	CHECK(Store.Save(Profile));
	SFakeBox Box;
	Box.Values["DiskSerialNumber"] = QStringList() << "HarddiskVolume9,1111-2222";
	Box.Values["HideDiskSerialNumber"] = QStringList() << "y";
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eManual);
	QString Error;
	CHECK(!CIdentityProfileBinding::Apply(Box, Profile, &Error) && Error.contains("manual DiskSerialNumber"));
	CHECK(Box.Values["DiskSerialNumber"] == (QStringList() << "HarddiskVolume9,1111-2222"));
	CHECK(Box.Writes == 0);
	CHECK(!CIdentityProfileBinding::Unbind(Box, &Error) && Error.contains("no identity profile"));
	CHECK(Box.Values["DiskSerialNumber"] == (QStringList() << "HarddiskVolume9,1111-2222"));
}

static void BindStaleAfterRegenerate()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("stale");
	CHECK(Store.Save(Profile));
	SFakeBox Box;
	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	QStringList Applied = Box.Values["DiskSerialNumber"];

	Profile.Regenerate(); CHECK(Store.Save(Profile));
	SIdentityBindingState State = CIdentityProfileBinding::Read(Box, Store);
	CHECK(State.State == SIdentityBindingState::eStale && State.AppliedRevision == 1 && State.ProfileRevision == 2);
	CHECK(Box.Values["DiskSerialNumber"] == Applied);	// nothing changed silently
	CHECK(State.Describe().contains("revision 1") && State.Describe().contains("revision 2"));

	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	CHECK(Box.Values["DiskSerialNumber"] != Applied);
	CHECK(Sorted(Box.Values["DiskSerialNumber"]) == Sorted(Profile.DiskSerialNumberValues()));
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eApplied);
}

static void BindDiverged()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("diverged");
	CHECK(Store.Save(Profile));
	SFakeBox Box;
	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	Box.Values["DiskSerialNumber"].append("HarddiskVolume5,5555-5555");	// raw INI edit behind SandMan's back
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eDiverged);
	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	CHECK(Sorted(Box.Values["DiskSerialNumber"]) == Sorted(Profile.DiskSerialNumberValues()));
	Box.Values["HideDiskSerialNumber"] = QStringList() << "n";
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eDiverged);
}

static void BindMissing()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("missing");
	CHECK(Store.Save(Profile));
	SFakeBox Box;
	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	CHECK(Store.Remove(Profile.Id));
	SIdentityBindingState State = CIdentityProfileBinding::Read(Box, Store);
	CHECK(State.State == SIdentityBindingState::eMissing && State.ProfileId == Profile.Id);
	CHECK(Box.Values["DiskSerialNumber"].size() == 2);	// the configured values stay in place
	CHECK(CIdentityProfileBinding::Unbind(Box));
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eUnbound);
}

static void BindUnbind()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("unbind");
	CHECK(Store.Save(Profile));
	SFakeBox Box;
	Box.Values["Enabled"] = QStringList() << "y";
	Box.Values["HideFirmwareInfo"] = QStringList() << "y";
	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	CHECK(CIdentityProfileBinding::Unbind(Box));
	CHECK(!Box.Values.contains("IdentityProfile") && !Box.Values.contains("IdentityProfileRevision"));
	CHECK(!Box.Values.contains("DiskSerialNumber"));
	CHECK(Box.Values["HideDiskSerialNumber"] == (QStringList() << "y"));	// left to the existing checkbox
	CHECK(Box.Values["Enabled"] == (QStringList() << "y") && Box.Values["HideFirmwareInfo"] == (QStringList() << "y"));
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eUnbound);
}

static void BindKeepsTemplates()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("templates", 1);
	CHECK(Store.Save(Profile));
	SFakeBox Box;
	Box.TemplateValues["DiskSerialNumber"] = QStringList() << "HarddiskVolume1,7777-7777";
	Box.Values["Template"] = QStringList() << "SomeTemplate";
	SIdentityBindingState Before = CIdentityProfileBinding::Read(Box, Store);
	CHECK(Before.State == SIdentityBindingState::eUnbound && Before.TemplateSerials == Box.TemplateValues["DiskSerialNumber"]);
	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	CHECK(Box.TemplateValues["DiskSerialNumber"] == (QStringList() << "HarddiskVolume1,7777-7777"));
	CHECK(Box.Values["Template"] == (QStringList() << "SomeTemplate"));
	CHECK(Box.Values["DiskSerialNumber"] == Profile.DiskSerialNumberValues());
	SIdentityBindingState After = CIdentityProfileBinding::Read(Box, Store);
	CHECK(After.State == SIdentityBindingState::eApplied && After.TemplateSerials.size() == 1);
	CHECK(CIdentityProfileBinding::Unbind(Box));
	CHECK(Box.TemplateValues["DiskSerialNumber"].size() == 1 && Box.Values["Template"].size() == 1);
}

static void BindWriteFailure()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("failure");
	CHECK(Store.Save(Profile));
	SFakeBox Box; Box.FailWrites = true;
	QString Error;
	CHECK(!CIdentityProfileBinding::Apply(Box, Profile, &Error) && Error.contains("failed to add DiskSerialNumber"));
	CHECK(Box.Values.isEmpty());
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eUnbound);
	Box.FailWrites = false;
	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	Box.FailWrites = true;
	CHECK(!CIdentityProfileBinding::Unbind(Box, &Error) && Error.contains("failed to remove"));
	CHECK(Box.Values["IdentityProfile"] == (QStringList() << Profile.Id));
}

static void HostUntouched()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	QString Outside = Dir.path() + "/outside";
	CHECK(QDir().mkpath(Outside));
	{ QFile File(Outside + "/host.txt"); CHECK(File.open(QIODevice::WriteOnly)); File.write("host"); }
	QSet<QString> Before = Snapshot(Dir.path());
	QSet<QString> OutsideBefore = Snapshot(Outside);

	CIdentityProfileStore Store(Dir.path() + "/profiles");
	CIdentityProfile Profile = MakeProfile("host");
	CHECK(Store.Save(Profile));
	Profile.Regenerate(); CHECK(Store.Save(Profile));
	CIdentityProfile Copy = Profile.Clone("copy"); CHECK(Store.Save(Copy));
	SFakeBox Box;
	CHECK(CIdentityProfileBinding::Apply(Box, Profile));
	CHECK(CIdentityProfileBinding::Unbind(Box));
	CHECK(Store.Remove(Copy.Id));

	QSet<QString> After = Snapshot(Dir.path());
	After.remove("profiles");
	CHECK(After == Before);
	CHECK(Snapshot(Outside) == OutsideBefore);
	CHECK(Snapshot(Dir.path() + "/profiles") == (QSet<QString>() << Profile.Id + ".json"));
	// Only the box section was written; a global section would be a different target object.
	CHECK(Box.Values.size() == 1 && Box.Values.contains("HideDiskSerialNumber"));
}

static void BindDeferredCommit()
{
	// SandMan's options dialog batches INI writes and the service publishes them on commit only;
	// the read-back must run after Flush(), otherwise a correct apply is reported as a mismatch.
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("deferred", 2);
	CHECK(Store.Save(Profile));
	SFakeBox Box; Box.Deferred = true;
	QString Error;
	CHECK(CIdentityProfileBinding::Apply(Box, Profile, &Error));
	CHECK(Box.Flushes == 1 && !Box.HasPending);
	CHECK(Sorted(Box.Values["DiskSerialNumber"]) == Sorted(Profile.DiskSerialNumberValues()));
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eApplied);
	CHECK(CIdentityProfileBinding::Unbind(Box, &Error));
	CHECK(Box.Flushes == 2 && !Box.Values.contains("IdentityProfile") && !Box.Values.contains("DiskSerialNumber"));

	// A commit that loses the writes is reported, not hidden.
	SFakeBox Lossy; Lossy.Deferred = true; Lossy.DropOnFlush = true;
	CHECK(!CIdentityProfileBinding::Apply(Lossy, Profile, &Error) && Error.contains("read back"));
	CHECK(Lossy.Values.isEmpty());
}

static void BindDeferredCheckboxSave()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("checkbox save", 2);
	CHECK(Store.Save(Profile));

	SFakeBox Legacy; Legacy.Deferred = true;
	Legacy.Values["HideDiskSerialNumber"] = QStringList() << "y";
	CHECK(Legacy.DelValue("HideDiskSerialNumber", QString()));
	QString Error;
	CHECK(!CIdentityProfileBinding::Apply(Legacy, Profile, &Error) && Error.contains("read back"));
	CHECK(!Legacy.Values.contains("HideDiskSerialNumber"));

	SFakeBox Box; Box.Deferred = true;
	Box.Values["HideDiskSerialNumber"] = QStringList() << "y";
	QString Selected = Profile.Id;
	QString Bound;
	bool HideSerialChecked = false;
	if (!CIdentityProfileBinding::WillApplyProfile(Selected, Bound, SIdentityBindingState::eUnbound)) {
		if (HideSerialChecked) CHECK(Box.SetText("HideDiskSerialNumber", "y"));
		else CHECK(Box.DelValue("HideDiskSerialNumber", QString()));
	}
	CHECK(CIdentityProfileBinding::Apply(Box, Profile, &Error));
	CHECK(Box.Values["HideDiskSerialNumber"] == (QStringList() << "y"));
	CHECK(CIdentityProfileBinding::Read(Box, Store).State == SIdentityBindingState::eApplied);
}

static void BindMissingCheckboxSave()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	SFakeBox Box; Box.Deferred = true;
	QString MissingId = CIdentityProfile::NewId();
	Box.Values["IdentityProfile"] = QStringList() << MissingId;
	Box.Values["HideDiskSerialNumber"] = QStringList() << "y";
	SIdentityBindingState State = CIdentityProfileBinding::Read(Box, Store);
	CHECK(State.State == SIdentityBindingState::eMissing);

	QString Selected = State.ProfileId;
	QString Bound = State.ProfileId;
	if (!CIdentityProfileBinding::WillApplyProfile(Selected, Bound, State.State))
		CHECK(Box.DelValue("HideDiskSerialNumber", QString()));
	CHECK(Box.Flush());
	CHECK(!Box.Values.contains("HideDiskSerialNumber"));
	CHECK(Box.Values["IdentityProfile"] == (QStringList() << MissingId));
}

//---------------------------------------------------------------------------

static void DialogNewEdit()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Draft;
	Draft.Id = CIdentityProfile::NewId();
	CIdentityProfileEditDialog Editor(Store, Draft);
	CHECK(!Editor.Save());	// no name, no volumes
	Editor.SetName("edited");
	CHECK(!Editor.Save() && Editor.GetLastError().contains("no volumes"));
	Editor.AddVolume("\\Device\\HarddiskVolume2", "abcd1234");
	Editor.AddVolume("HarddiskVolume2", "0000-0000");
	CHECK(!Editor.Save() && Editor.GetLastError().contains("duplicate"));
	Editor.RemoveSelectedVolume();	// nothing selected: no-op
	CHECK(Editor.GetVolumesText().size() == 2);
	Editor.AddVolume("HarddiskVolume3", "not-a-serial");
	CHECK(!Editor.Save() && Editor.GetLastError().contains("invalid volume serial"));
	CIdentityProfileEditDialog BadDevice(Store, Draft);
	BadDevice.SetName("bad device");
	BadDevice.AddVolume("D:", "0000-0000");
	CHECK(!BadDevice.Save() && BadDevice.GetLastError().contains("invalid volume device"));
	CHECK(!QFile::exists(Store.PathFor(Draft.Id)));

	CIdentityProfileEditDialog Clean(Store, Draft);
	Clean.SetName("edited");
	Clean.AddVolume("\\Device\\HarddiskVolume2", "abcd1234");
	CHECK(Clean.Save());
	CHECK(Clean.GetProfile().Volumes.size() == 1);
	CHECK(Clean.GetProfile().Volumes[0].Device == "HarddiskVolume2" && Clean.GetProfile().Volumes[0].Serial == "ABCD-1234");
	CIdentityProfile Loaded;
	CHECK(Store.Load(Draft.Id, Loaded) && Loaded.Name == "edited" && Loaded.Revision == 1);

	CIdentityProfileEditDialog Again(Store, Loaded);
	CHECK(Again.GetVolumesText() == (QStringList() << "HarddiskVolume2,ABCD-1234"));
	Again.SetName("edited twice");
	CHECK(Again.Save());
	CHECK(Store.Load(Draft.Id, Loaded) && Loaded.Name == "edited twice" && Loaded.Revision == 2 && Loaded.Volumes[0].Serial == "ABCD-1234");

	CIdentityProfilesDialog List(Store, nullptr);
	CHECK(List.GetProfileCount() == 1);
	List.SelectId(Draft.Id);
	CHECK(List.GetSelectedId() == Draft.Id);
}

static void DialogCloneRegenerate()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Profile = MakeProfile("origin", 2);
	CHECK(Store.Save(Profile));

	CIdentityProfilesDialog Dialog(Store, nullptr);
	Dialog.SelectId(Profile.Id);
	CHECK(Dialog.CloneSelected("copy"));
	QString CopyId = Dialog.GetSelectedId();
	CHECK(CopyId != Profile.Id && Dialog.GetProfileCount() == 2);
	CIdentityProfile Copy; CHECK(Store.Load(CopyId, Copy));
	CHECK(Copy.Volumes == Profile.Volumes && Copy.Name == "copy" && Copy.Revision == 1);

	CHECK(Dialog.RegenerateSelected());
	CIdentityProfile Regenerated; CHECK(Store.Load(CopyId, Regenerated));
	CHECK(Regenerated.Revision == 2 && Regenerated.Volumes != Profile.Volumes);
	CIdentityProfile Origin; CHECK(Store.Load(Profile.Id, Origin));
	CHECK(Origin.Volumes == Profile.Volumes && Origin.Revision == 1);	// the source is untouched

	CIdentityProfileEditDialog Editor(Store, Origin);
	Editor.RegenerateSerials();
	CHECK(Editor.GetVolumesText() != Origin.DiskSerialNumberValues());
	CHECK(Store.Load(Profile.Id, Origin) && Origin.Volumes == Profile.Volumes);	// draft only until saved
}

static void DialogImportExport()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path() + "/store");
	CIdentityProfile Profile = MakeProfile("shared", 2);
	CHECK(Store.Save(Profile));
	CIdentityProfilesDialog Dialog(Store, nullptr);
	Dialog.SelectId(Profile.Id);
	QString Path = Dir.path() + "/shared.json";
	CHECK(Dialog.ExportSelected(Path) && QFile::exists(Path));
	QString NewId;
	CHECK(Dialog.ImportFile(Path, &NewId));
	CHECK(NewId != Profile.Id && Dialog.GetSelectedId() == NewId && Dialog.GetProfileCount() == 2);
	CIdentityProfile Imported; CHECK(Store.Load(NewId, Imported));
	CHECK(Imported.Volumes == Profile.Volumes && Imported.Revision == 1);
	CHECK(!Dialog.ImportFile(Dir.path() + "/nope.json") && !Dialog.GetLastError().isEmpty());
	CHECK(Dialog.GetProfileCount() == 2);
}

static void DialogDeleteGuard()
{
	QTemporaryDir Dir; CHECK(Dir.isValid());
	CIdentityProfileStore Store(Dir.path());
	CIdentityProfile Used = MakeProfile("used"), Free = MakeProfile("free");
	CHECK(Store.Save(Used) && Store.Save(Free));
	QString UsedId = Used.Id;
	CIdentityProfilesDialog Dialog(Store, [UsedId](const QString& Id) { return Id == UsedId ? QStringList() << "DefaultBox" << "Other" : QStringList(); });
	Dialog.SelectId(Used.Id);
	CHECK(!Dialog.DeleteSelected(false) && Dialog.GetLastError().contains("DefaultBox"));
	CHECK(Dialog.GetProfileCount() == 2 && QFile::exists(Store.PathFor(Used.Id)));
	Dialog.SelectId(Free.Id);
	CHECK(Dialog.DeleteSelected(false));
	CHECK(Dialog.GetProfileCount() == 1 && !QFile::exists(Store.PathFor(Free.Id)));
	Dialog.SelectId(Used.Id);
	CHECK(Dialog.DeleteSelected(true));
	CHECK(Dialog.GetProfileCount() == 0);
}

int main(int argc, char** argv)
{
	QApplication App(argc, argv);
	QMap<QString, void(*)()> Cases = {
		{"normalize-device", NormalizeDevice}, {"normalize-serial", NormalizeSerial},
		{"validate-invalid", ValidateInvalid}, {"json-corrupt", JsonCorrupt},
		{"store-roundtrip", StoreRoundtrip}, {"store-atomic-failure", StoreAtomicFailure},
		{"store-stale-writer", StoreStaleWriter}, {"store-locked-writer", StoreLockedWriter},
		{"store-list-skips-bad", StoreListSkipsBad}, {"store-remove", StoreRemove},
		{"regenerate-explicit", RegenerateExplicit}, {"clone-new-id", CloneNewId},
		{"import-new-id", ImportNewId}, {"export-atomic", ExportAtomic},
		{"bind-apply", BindApply}, {"bind-two-boxes", BindTwoBoxes},
		{"bind-refuses-running", BindRefusesRunning}, {"bind-refuses-manual", BindRefusesManual},
		{"bind-stale-after-regenerate", BindStaleAfterRegenerate}, {"bind-diverged", BindDiverged},
		{"bind-missing", BindMissing}, {"bind-unbind", BindUnbind},
		{"bind-keeps-templates", BindKeepsTemplates}, {"bind-write-failure", BindWriteFailure},
		{"host-untouched", HostUntouched}, {"bind-deferred-commit", BindDeferredCommit},
		{"bind-deferred-checkbox-save", BindDeferredCheckboxSave},
		{"bind-missing-checkbox-save", BindMissingCheckboxSave},
		{"dialog-new-edit", DialogNewEdit}, {"dialog-clone-regenerate", DialogCloneRegenerate},
		{"dialog-import-export", DialogImportExport}, {"dialog-delete-guard", DialogDeleteGuard}
	};
	if (argc != 2 || !Cases.contains(QString::fromUtf8(argv[1]))) return 2;
	Cases.value(QString::fromUtf8(argv[1]))();
	std::printf("PASS: %s\n", argv[1]);
	return 0;
}
