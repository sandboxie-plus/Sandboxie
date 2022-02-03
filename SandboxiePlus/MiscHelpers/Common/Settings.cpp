#include "stdafx.h"
#include "Settings.h"
//#include "qzlib.h"
#include "Common.h"

bool TestWriteRight(const QString& Path)
{
	QFile TestFile(Path + "/~test-" + GetRand64Str() + ".tmp");
	if(!TestFile.open(QFile::WriteOnly))
		return false;
	TestFile.close();
	return TestFile.remove();
}

CSettings::CSettings(const QString& AppName, bool bShared, QMap<QString, SSetting> DefaultValues, QObject* qObject) : QObject(qObject)
{
	m_ConfigDir = QCoreApplication::applicationDirPath();
	if (!(m_bPortable = QFile::exists(m_ConfigDir + "/" + AppName + ".ini")))
	{
		QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
		if (dirs.isEmpty())
			m_ConfigDir = QDir::homePath() + "/." + AppName;
		//
		// if shared is set a new ini is created in the shared location 
		// and if present take precedence over an ini in a user location
		// howeever if the only existing ini is in a user location it will be used
		//
		else if(bShared && dirs.count() > 2 && (
		  QFile::exists(dirs[1] + "/" + AppName + "/" + AppName + ".ini") ||
		 !QFile::exists(dirs[0] + "/" + AppName + "/" + AppName + ".ini") ))
			m_ConfigDir = dirs[1] + "/" + AppName;
		else
			m_ConfigDir = dirs[0] + "/" + AppName;
		QDir().mkpath(m_ConfigDir);
	}

	m_pConf = new QSettings(m_ConfigDir + "/" + AppName + ".ini", QSettings::IniFormat, this);

	m_pConf->sync();

	m_DefaultValues = DefaultValues;
	foreach (const QString& Key, m_DefaultValues.uniqueKeys())
	{
		const SSetting& Setting = m_DefaultValues[Key];
		if(!m_pConf->contains(Key) || !Setting.Check(m_pConf->value(Key)))
		{
			if(Setting.IsBlob())
				m_pConf->setValue(Key, Setting.Value.toByteArray().toBase64().replace("+","-").replace("/","_").replace("=",""));
			else
				m_pConf->setValue(Key, Setting.Value);
		}
	}
}

CSettings::~CSettings()
{
	m_pConf->sync();
}

void CSettings::DelValue(const QString& key)
{
	QMutexLocker Locker(&m_Mutex);

	m_pConf->remove(key);

	m_ValueCache.clear();
}

bool CSettings::SetValue(const QString &key, const QVariant &value)
{
	QMutexLocker Locker(&m_Mutex);

	if (!m_DefaultValues.isEmpty())
	{
		ASSERT(m_pConf->contains(key));
#ifndef _DEBUG
		if (!m_DefaultValues[key].Check(value))
			return false;
#endif
	}

	m_pConf->setValue(key, value);

	m_ValueCache.clear();
	return true;
}

QVariant CSettings::GetValue(const QString &key, const QVariant& preset)
{
	QMutexLocker Locker(&m_Mutex);

	ASSERT(m_DefaultValues.isEmpty() || m_pConf->contains(key));	

	return m_pConf->value(key, preset);
}

void CSettings::SetBlob(const QString& key, const QByteArray& value)
{
	QString str;
	//QByteArray data = Pack(value);
	//if(data.size() < value.size())
	//	str = ":PackedArray:" + data.toBase64().replace("+","-").replace("/","_").replace("=","");
	//else
		str = ":ByteArray:" + value.toBase64().replace("+","-").replace("/","_").replace("=","");
	SetValue(key, str);
}

QByteArray CSettings::GetBlob(const QString& key)
{
	QByteArray value;
	QByteArray str = GetValue(key).toByteArray();
	if(str.left(11) == ":ByteArray:")
		value = QByteArray::fromBase64(str.mid(11).replace("-","+").replace("_","/"));
	//else if(str.left(13) == ":PackedArray:")
	//	value = Unpack(QByteArray::fromBase64(str.mid(13).replace("-","+").replace("_","/")));
	return value;
}

const QStringList CSettings::ListKeys(const QString& Root)
{
	QMutexLocker Locker(&m_Mutex); 
	QStringList Keys;
	foreach(const QString& Key, m_pConf->allKeys())
	{
		QStringList Path = Key.split("/");
		ASSERT(Path.count() == 2);
		if(Path[0] == Root)
			Keys.append(Path[1]);
	}
	return Keys;
}
