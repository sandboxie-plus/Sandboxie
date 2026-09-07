#include "IdentityProfile.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QUuid>

//---------------------------------------------------------------------------
// CIdentityProfile
//---------------------------------------------------------------------------

QString CIdentityProfile::NewId()
{
	return QUuid::createUuid().toString(QUuid::WithoutBraces).toLower();
}

bool CIdentityProfile::IsValidId(const QString& Id)
{
	static const QRegularExpression Pattern("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$");
	return Pattern.match(Id).hasMatch();
}

QString CIdentityProfile::NormalizeDevice(const QString& Device, QString* pError)
{
	QString Value = Device.trimmed();
	if (Value.startsWith("\\Device\\", Qt::CaseInsensitive))
		Value = Value.mid(8);
	static const QRegularExpression Pattern("^HarddiskVolume([1-9][0-9]{0,3})$", QRegularExpression::CaseInsensitiveOption);
	QRegularExpressionMatch Match = Pattern.match(Value);
	if (!Match.hasMatch()) {
		if (pError) *pError = QString("invalid volume device '%1' (expected HarddiskVolumeN)").arg(Device.trimmed());
		return QString();
	}
	return "HarddiskVolume" + Match.captured(1);
}

QString CIdentityProfile::NormalizeSerial(const QString& Serial, QString* pError)
{
	QString Hex = Serial.trimmed().remove('-').toUpper();
	static const QRegularExpression Pattern("^[0-9A-F]{8}$");
	if (!Pattern.match(Hex).hasMatch()) {
		if (pError) *pError = QString("invalid volume serial '%1' (expected XXXX-XXXX hex)").arg(Serial.trimmed());
		return QString();
	}
	return Hex.left(4) + "-" + Hex.mid(4);
}

QString CIdentityProfile::RandomSerial()
{
	quint32 Value = QRandomGenerator::system()->generate();
	return QString("%1").arg(Value, 8, 16, QChar('0')).toUpper().insert(4, '-');
}

QStringList CIdentityProfile::Validate() const
{
	QStringList Errors;
	if (Version != CurrentVersion)
		Errors.append(QString("unsupported profile version %1").arg(Version));
	if (!IsValidId(Id))
		Errors.append("missing or malformed profile id");
	if (Name.trimmed().isEmpty())
		Errors.append("profile name is empty");
	if (Revision < 0)
		Errors.append("negative revision");
	if (Volumes.isEmpty())
		Errors.append("profile has no volumes");
	QSet<QString> Seen;
	foreach(const SIdentityVolume& Volume, Volumes) {
		QString Error;
		QString Device = NormalizeDevice(Volume.Device, &Error);
		if (Device.isEmpty()) { Errors.append(Error); continue; }
		if (Device != Volume.Device)
			Errors.append(QString("volume device '%1' is not normalized").arg(Volume.Device));
		if (Seen.contains(Device.toLower()))
			Errors.append(QString("duplicate volume device '%1'").arg(Device));
		Seen.insert(Device.toLower());
		QString Serial = NormalizeSerial(Volume.Serial, &Error);
		if (Serial.isEmpty()) Errors.append(Error);
		else if (Serial != Volume.Serial)
			Errors.append(QString("volume serial '%1' is not normalized").arg(Volume.Serial));
	}
	return Errors;
}

QJsonObject CIdentityProfile::ToJson() const
{
	QJsonArray Array;
	foreach(const SIdentityVolume& Volume, Volumes) {
		QJsonObject Entry;
		Entry["device"] = Volume.Device;
		Entry["serial"] = Volume.Serial;
		Array.append(Entry);
	}
	QJsonObject Json;
	Json["format"] = "sandboxie-identity-profile";
	Json["version"] = Version;
	Json["id"] = Id;
	Json["name"] = Name;
	Json["revision"] = Revision;
	Json["created"] = Created;
	Json["updated"] = Updated;
	Json["volumes"] = Array;
	return Json;
}

bool CIdentityProfile::FromJson(const QJsonObject& Json, CIdentityProfile& Profile, QStringList& Errors)
{
	Errors.clear();
	if (Json.value("format").toString() != "sandboxie-identity-profile")
		Errors.append("not an identity profile document");
	if (!Json.value("version").isDouble())
		Errors.append("missing profile version");
	if (!Json.value("volumes").isArray())
		Errors.append("missing volume list");
	if (!Errors.isEmpty())
		return false;

	CIdentityProfile Result;
	Result.Version = Json.value("version").toInt(-1);
	Result.Id = Json.value("id").toString();
	Result.Name = Json.value("name").toString();
	Result.Revision = Json.value("revision").toInt(-1);
	Result.Created = Json.value("created").toString();
	Result.Updated = Json.value("updated").toString();
	foreach(const QJsonValue& Value, Json.value("volumes").toArray()) {
		if (!Value.isObject()) { Errors.append("volume entry is not an object"); continue; }
		QJsonObject Entry = Value.toObject();
		SIdentityVolume Volume;
		Volume.Device = Entry.value("device").toString();
		Volume.Serial = Entry.value("serial").toString();
		Result.Volumes.append(Volume);
	}
	Errors.append(Result.Validate());
	if (!Errors.isEmpty())
		return false;
	Profile = Result;
	return true;
}

QStringList CIdentityProfile::DiskSerialNumberValues() const
{
	QStringList Values;
	foreach(const SIdentityVolume& Volume, Volumes)
		Values.append(Volume.Device + "," + Volume.Serial);
	return Values;
}

void CIdentityProfile::Regenerate()
{
	for (int i = 0; i < Volumes.size(); i++)
		Volumes[i].Serial = RandomSerial();
}

CIdentityProfile CIdentityProfile::Clone(const QString& NewName) const
{
	CIdentityProfile Copy = *this;
	Copy.Id = NewId();
	Copy.Name = NewName;
	Copy.Revision = 0;
	Copy.Created.clear();
	Copy.Updated.clear();
	return Copy;
}

//---------------------------------------------------------------------------
// CIdentityProfileStore
//---------------------------------------------------------------------------

CIdentityProfileStore::CIdentityProfileStore(const QString& Dir)
	: m_Dir(QDir::cleanPath(Dir))
{
}

QString CIdentityProfileStore::PathFor(const QString& Id) const
{
	return m_Dir + "/" + Id + ".json";
}

QList<CIdentityProfile> CIdentityProfileStore::List(QStringList* pProblems) const
{
	QList<CIdentityProfile> Profiles;
	QDir Dir(m_Dir);
	foreach(const QString& Entry, Dir.entryList(QStringList() << "*.json", QDir::Files, QDir::Name)) {
		QString Id = Entry.left(Entry.size() - 5);
		if (!CIdentityProfile::IsValidId(Id)) {
			if (pProblems) pProblems->append(QString("%1: file name is not a profile id").arg(Entry));
			continue;
		}
		CIdentityProfile Profile;
		QString Error;
		if (!Load(Id, Profile, &Error)) {
			if (pProblems) pProblems->append(QString("%1: %2").arg(Entry, Error));
			continue;
		}
		Profiles.append(Profile);
	}
	return Profiles;
}

bool CIdentityProfileStore::Load(const QString& Id, CIdentityProfile& Profile, QString* pError) const
{
	if (!CIdentityProfile::IsValidId(Id)) {
		if (pError) *pError = "malformed profile id";
		return false;
	}
	CIdentityProfile Loaded;
	if (!ReadFile(PathFor(Id), Loaded, pError))
		return false;
	if (Loaded.Id != Id) {
		if (pError) *pError = "profile id does not match its file name";
		return false;
	}
	Profile = Loaded;
	return true;
}

bool CIdentityProfileStore::Save(CIdentityProfile& Profile, QString* pError)
{
	QStringList Errors = Profile.Validate();
	if (!Errors.isEmpty()) {
		if (pError) *pError = Errors.join("; ");
		return false;
	}
	if (!QDir().mkpath(m_Dir)) {
		if (pError) *pError = QString("cannot create %1").arg(m_Dir);
		return false;
	}
	CIdentityProfile Next = Profile;
	QString Now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
	if (Next.Created.isEmpty()) Next.Created = Now;
	Next.Updated = Now;
	Next.Revision = Profile.Revision + 1;
	if (!WriteFile(PathFor(Next.Id), Next, pError))
		return false;
	CIdentityProfile Check;
	if (!ReadFile(PathFor(Next.Id), Check, pError))
		return false;
	if (Check.ToJson() != Next.ToJson()) {
		if (pError) *pError = "profile read back differs from what was written";
		return false;
	}
	Profile = Next;
	return true;
}

bool CIdentityProfileStore::Remove(const QString& Id, QString* pError)
{
	if (!CIdentityProfile::IsValidId(Id)) {
		if (pError) *pError = "malformed profile id";
		return false;
	}
	QFile File(PathFor(Id));
	if (!File.exists()) {
		if (pError) *pError = "profile does not exist";
		return false;
	}
	if (!File.remove()) {
		if (pError) *pError = File.errorString();
		return false;
	}
	return true;
}

bool CIdentityProfileStore::Export(const CIdentityProfile& Profile, const QString& Path, QString* pError) const
{
	QStringList Errors = Profile.Validate();
	if (!Errors.isEmpty()) {
		if (pError) *pError = Errors.join("; ");
		return false;
	}
	return WriteFile(Path, Profile, pError);
}

bool CIdentityProfileStore::Import(const QString& Path, CIdentityProfile& Profile, QString* pError) const
{
	CIdentityProfile Loaded;
	if (!ReadFile(Path, Loaded, pError))
		return false;
	// An import is a new record: the serials are preserved, the identity of the record is not.
	Loaded.Id = CIdentityProfile::NewId();
	Loaded.Revision = 0;
	Loaded.Created.clear();
	Loaded.Updated.clear();
	Profile = Loaded;
	return true;
}

bool CIdentityProfileStore::ReadFile(const QString& Path, CIdentityProfile& Profile, QString* pError)
{
	QFile File(Path);
	if (!File.open(QIODevice::ReadOnly)) {
		if (pError) *pError = File.errorString();
		return false;
	}
	QJsonParseError ParseError;
	QJsonDocument Document = QJsonDocument::fromJson(File.readAll(), &ParseError);
	if (ParseError.error != QJsonParseError::NoError || !Document.isObject()) {
		if (pError) *pError = ParseError.error != QJsonParseError::NoError ? ParseError.errorString() : QString("document is not an object");
		return false;
	}
	QStringList Errors;
	if (!CIdentityProfile::FromJson(Document.object(), Profile, Errors)) {
		if (pError) *pError = Errors.join("; ");
		return false;
	}
	return true;
}

bool CIdentityProfileStore::WriteFile(const QString& Path, const CIdentityProfile& Profile, QString* pError)
{
	QSaveFile File(Path);
	if (!File.open(QIODevice::WriteOnly)) {
		if (pError) *pError = File.errorString();
		return false;
	}
	QByteArray Data = QJsonDocument(Profile.ToJson()).toJson(QJsonDocument::Indented);
	if (File.write(Data) != Data.size() || !File.commit()) {
		if (pError) *pError = File.errorString();
		return false;
	}
	return true;
}

//---------------------------------------------------------------------------
// CIdentityProfileBinding
//---------------------------------------------------------------------------

const char* CIdentityProfileBinding::ProfileSetting = "IdentityProfile";
const char* CIdentityProfileBinding::RevisionSetting = "IdentityProfileRevision";
const char* CIdentityProfileBinding::HideSetting = "HideDiskSerialNumber";
const char* CIdentityProfileBinding::SerialSetting = "DiskSerialNumber";

QString SIdentityBindingState::Describe() const
{
	switch (State)
	{
	case eUnbound:	return "no identity profile";
	case eManual:	return QString("manual DiskSerialNumber entries (%1)").arg(BoxSerials.size());
	case eApplied:	return QString("profile revision %1 applied").arg(AppliedRevision);
	case eStale:	return QString("profile revision %1 applied, profile is at revision %2").arg(AppliedRevision).arg(ProfileRevision);
	case eMissing:	return QString("bound profile %1 is missing").arg(ProfileId);
	case eDiverged:	return QString("box DiskSerialNumber entries differ from profile revision %1").arg(AppliedRevision);
	}
	return QString();
}

static QStringList SortedCopy(QStringList List)
{
	List.sort(Qt::CaseInsensitive);
	return List;
}

SIdentityBindingState CIdentityProfileBinding::Read(const CIdentityIniTarget& Target, const CIdentityProfileStore& Store)
{
	SIdentityBindingState State;
	State.ProfileId = Target.GetText(ProfileSetting).trimmed();
	State.HideSerial = Target.GetText(HideSetting).trimmed().compare("y", Qt::CaseInsensitive) == 0;
	State.BoxSerials = Target.GetTextList(SerialSetting, false);
	QStringList All = Target.GetTextList(SerialSetting, true);
	foreach(const QString& Value, State.BoxSerials)
		All.removeOne(Value);
	State.TemplateSerials = All;

	if (State.ProfileId.isEmpty()) {
		State.State = State.BoxSerials.isEmpty() ? SIdentityBindingState::eUnbound : SIdentityBindingState::eManual;
		return State;
	}

	bool Ok = false;
	State.AppliedRevision = Target.GetText(RevisionSetting).trimmed().toInt(&Ok);
	if (!Ok) State.AppliedRevision = -1;

	CIdentityProfile Profile;
	if (!Store.Load(State.ProfileId, Profile)) {
		State.State = SIdentityBindingState::eMissing;
		return State;
	}
	State.ProfileRevision = Profile.Revision;
	if (State.AppliedRevision != Profile.Revision)
		State.State = SIdentityBindingState::eStale;
	else if (SortedCopy(State.BoxSerials) != SortedCopy(Profile.DiskSerialNumberValues()) || !State.HideSerial)
		State.State = SIdentityBindingState::eDiverged;
	else
		State.State = SIdentityBindingState::eApplied;
	return State;
}

bool CIdentityProfileBinding::Apply(CIdentityIniTarget& Target, const CIdentityProfile& Profile, QString* pError)
{
	QStringList Errors = Profile.Validate();
	if (!Errors.isEmpty()) {
		if (pError) *pError = Errors.join("; ");
		return false;
	}
	if (Target.GetActiveProcessCount() > 0) {
		if (pError) *pError = "stop all processes in this sandbox before applying an identity profile; running processes keep their current values";
		return false;
	}
	QString Bound = Target.GetText(ProfileSetting).trimmed();
	QStringList Existing = Target.GetTextList(SerialSetting, false);
	if (Bound.isEmpty() && !Existing.isEmpty()) {
		if (pError) *pError = "this sandbox has manual DiskSerialNumber entries; remove them before binding an identity profile";
		return false;
	}

	QStringList Wanted = Profile.DiskSerialNumberValues();
	QStringList ToAdd = Wanted;
	foreach(const QString& Value, Existing) {
		if (!ToAdd.removeOne(Value)) {
			if (!Target.DelValue(SerialSetting, Value)) {
				if (pError) *pError = QString("failed to remove DiskSerialNumber=%1").arg(Value);
				return false;
			}
		}
	}
	foreach(const QString& Value, ToAdd) {
		if (!Target.AppendText(SerialSetting, Value)) {
			if (pError) *pError = QString("failed to add DiskSerialNumber=%1").arg(Value);
			return false;
		}
	}
	if (!Target.SetText(HideSetting, "y")) {
		if (pError) *pError = "failed to set HideDiskSerialNumber";
		return false;
	}
	if (!Target.SetText(ProfileSetting, Profile.Id) || !Target.SetText(RevisionSetting, QString::number(Profile.Revision))) {
		if (pError) *pError = "failed to record the profile binding";
		return false;
	}

	if (SortedCopy(Target.GetTextList(SerialSetting, false)) != SortedCopy(Wanted)
	 || Target.GetText(ProfileSetting).trimmed() != Profile.Id
	 || Target.GetText(RevisionSetting).trimmed().toInt() != Profile.Revision
	 || Target.GetText(HideSetting).trimmed().compare("y", Qt::CaseInsensitive) != 0) {
		if (pError) *pError = "configuration read back does not match the profile";
		return false;
	}
	return true;
}

bool CIdentityProfileBinding::Unbind(CIdentityIniTarget& Target, QString* pError)
{
	if (Target.GetActiveProcessCount() > 0) {
		if (pError) *pError = "stop all processes in this sandbox before removing its identity profile";
		return false;
	}
	if (Target.GetText(ProfileSetting).trimmed().isEmpty()) {
		if (pError) *pError = "this sandbox has no identity profile";
		return false;
	}
	foreach(const QString& Value, Target.GetTextList(SerialSetting, false)) {
		if (!Target.DelValue(SerialSetting, Value)) {
			if (pError) *pError = QString("failed to remove DiskSerialNumber=%1").arg(Value);
			return false;
		}
	}
	if (!Target.DelValue(RevisionSetting) || !Target.DelValue(ProfileSetting)) {
		if (pError) *pError = "failed to remove the profile binding";
		return false;
	}
	return true;
}

QString CIdentityProfileBinding::CoverageText()
{
	return "Configured coverage: volume serial numbers returned by the GetVolumeInformationByHandleW hook (HideDiskSerialNumber + DiskSerialNumber). "
		"This is the volume serial, not the physical disk serial. GetVolumeInformationW/A, native NtQueryVolumeInformationFile, WMI and storage device queries are not covered by this profile version. "
		"A saved profile is not an applied configuration, and an applied configuration is not verified protection.";
}
