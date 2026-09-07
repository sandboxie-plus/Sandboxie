#pragma once

#include <QJsonObject>
#include <QList>
#include <QString>
#include <QStringList>

struct SIdentityVolume
{
	QString Device;	// native device component, e.g. HarddiskVolume1
	QString Serial;	// volume serial in XXXX-XXXX form

	bool operator==(const SIdentityVolume& other) const { return Device == other.Device && Serial == other.Serial; }
};

class CIdentityProfile
{
public:
	enum { CurrentVersion = 1 };

	int Version = CurrentVersion;
	QString Id;
	QString Name;
	int Revision = 0;
	QString Created;
	QString Updated;
	QList<SIdentityVolume> Volumes;

	static QString NewId();
	static bool IsValidId(const QString& Id);
	static QString NormalizeDevice(const QString& Device, QString* pError = nullptr);
	static QString NormalizeSerial(const QString& Serial, QString* pError = nullptr);
	static QString RandomSerial();

	QStringList Validate() const;
	bool IsValid() const { return Validate().isEmpty(); }

	QJsonObject ToJson() const;
	static bool FromJson(const QJsonObject& Json, CIdentityProfile& Profile, QStringList& Errors);

	QStringList DiskSerialNumberValues() const;

	void Regenerate();
	CIdentityProfile Clone(const QString& NewName) const;
};

class CIdentityProfileStore
{
public:
	explicit CIdentityProfileStore(const QString& Dir);

	QString GetDir() const { return m_Dir; }
	QString PathFor(const QString& Id) const;

	QList<CIdentityProfile> List(QStringList* pProblems = nullptr) const;
	bool Load(const QString& Id, CIdentityProfile& Profile, QString* pError = nullptr) const;
	bool Save(CIdentityProfile& Profile, QString* pError = nullptr);
	bool Remove(const QString& Id, QString* pError = nullptr);

	bool Export(const CIdentityProfile& Profile, const QString& Path, QString* pError = nullptr) const;
	bool Import(const QString& Path, CIdentityProfile& Profile, QString* pError = nullptr) const;

private:
	static bool ReadFile(const QString& Path, CIdentityProfile& Profile, QString* pError);
	static bool WriteFile(const QString& Path, const CIdentityProfile& Profile, QString* pError);

	QString m_Dir;
};

// Minimal view of a box section; SandMan adapts CSbieIni, tests inject storage.
class CIdentityIniTarget
{
public:
	virtual ~CIdentityIniTarget() {}
	virtual QString GetText(const QString& Setting) const = 0;
	virtual QStringList GetTextList(const QString& Setting, bool withTemplates) const = 0;
	virtual bool SetText(const QString& Setting, const QString& Value) = 0;
	virtual bool AppendText(const QString& Setting, const QString& Value) = 0;
	virtual bool DelValue(const QString& Setting, const QString& Value = QString()) = 0;
	virtual int GetActiveProcessCount() const = 0;
	// Publish pending writes so that the reads below observe them; a no-op for synchronous targets.
	virtual bool Flush() { return true; }
};

struct SIdentityBindingState
{
	enum EState { eUnbound, eManual, eApplied, eStale, eMissing, eDiverged };

	EState State = eUnbound;
	QString ProfileId;
	int AppliedRevision = -1;
	int ProfileRevision = -1;
	bool HideSerial = false;
	QStringList BoxSerials;			// DiskSerialNumber values in the box section itself
	QStringList TemplateSerials;	// DiskSerialNumber values contributed by templates or globals

	QString Describe() const;
};

class CIdentityProfileBinding
{
public:
	static const char* ProfileSetting;
	static const char* RevisionSetting;
	static const char* HideSetting;
	static const char* SerialSetting;

	static SIdentityBindingState Read(const CIdentityIniTarget& Target, const CIdentityProfileStore& Store);
	static bool Apply(CIdentityIniTarget& Target, const CIdentityProfile& Profile, QString* pError = nullptr);
	static bool Unbind(CIdentityIniTarget& Target, QString* pError = nullptr);

	static QString CoverageText();
};
