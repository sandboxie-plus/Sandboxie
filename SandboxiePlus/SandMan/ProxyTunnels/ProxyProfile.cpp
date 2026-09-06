// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#ifdef PROXY_TUNNELS_STANDALONE
#include <QtCore>
#else
#include "stdafx.h"
#endif
#include "ProxyProfile.h"
#include <QCoreApplication>
#include <QHostAddress>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QUrl>
#include <QUuid>
#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#endif

namespace
{
	QString Text(const char* Value) { return QCoreApplication::translate("ProxyProfiles", Value); }
	bool Fail(QString& Error, const char* Message) { Error = Text(Message); return false; }
	bool HasControls(const QString& Value)
	{
		for (QChar Ch : Value) {
			if (Ch.unicode() < 32 || Ch.unicode() == 127) return true;
		}
		return false;
	}
	bool ValidId(const QString& Id)
	{
		QUuid Uuid(Id);
		return !Uuid.isNull() && Uuid.toString(QUuid::WithoutBraces) == Id;
	}
#ifdef Q_OS_WIN
	void Wipe(QByteArray& Data)
	{
		Data.detach();
		SecureZeroMemory(Data.data(), size_t(Data.size()));
		Data.clear();
	}
#endif
}

QString SProxyProfile::AdapterName() const { return QStringLiteral("SbieProxy-") + Id; }

QJsonObject SProxyProfile::ToJson() const
{
	return {{"id", Id}, {"name", Name}, {"scheme", Scheme}, {"host", Host},
		{"port", int(Port)}, {"dpapi", QString::fromLatin1(ProtectedAuth.toBase64())}};
}

bool SProxyProfile::FromJson(const QJsonObject& Object, SProxyProfile& Profile, QString& Error)
{
	static const QSet<QString> Keys = {"id", "name", "scheme", "host", "port", "dpapi"};
	for (auto It = Object.begin(); It != Object.end(); ++It) {
		if (!Keys.contains(It.key())) return Fail(Error, "Unknown profile field; the file was not modified.");
	}
	if (!Object.value("port").isDouble() || Object.value("port").toDouble() != Object.value("port").toInt()
		|| !Object.value("dpapi").isString())
		return Fail(Error, "Invalid saved proxy profile.");
	SProxyInput Input;
	Input.Name = Object.value("name").toString();
	Input.Scheme = Object.value("scheme").toString();
	Input.Host = Object.value("host").toString();
	Input.Port = Object.value("port").toInt();
	if (!ValidId(Object.value("id").toString()) || !ProxyProfiles::Validate(Input, Error))
		return Fail(Error, "Invalid saved proxy profile; the file was not modified.");
	const QString Encoded = Object.value("dpapi").toString();
	if (!QRegularExpression(QStringLiteral("^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$")).match(Encoded).hasMatch()
		|| Encoded.size() > 16384)
		return Fail(Error, "Invalid protected credentials.");
	Profile.Id = Object.value("id").toString();
	Profile.Name = Input.Name;
	Profile.Host = Input.Host;
	Profile.Scheme = Input.Scheme;
	Profile.Port = quint16(Input.Port);
	Profile.ProtectedAuth = QByteArray::fromBase64(Encoded.toLatin1());
	return true;
}

bool ProxyProfiles::Validate(const SProxyInput& Input, QString& Error)
{
	Error.clear();
	if (Input.Name.trimmed().isEmpty() || Input.Name.size() > 100 || HasControls(Input.Name))
		return Fail(Error, "Use a profile name of 1 to 100 characters without control characters.");
	if (Input.Scheme != "socks5" && Input.Scheme != "http")
		return Fail(Error, "Only SOCKS5 and HTTP CONNECT proxies are supported.");
	if (Input.Port < 1 || Input.Port > 65535)
		return Fail(Error, "The proxy port must be between 1 and 65535.");
	if (Input.Host.isEmpty() || Input.Host.size() > 253 || HasControls(Input.Host) || Input.Host != Input.Host.trimmed())
		return Fail(Error, "Invalid proxy host.");
	QHostAddress Address;
	if (Address.setAddress(Input.Host)) {
		if (Address == QHostAddress::AnyIPv4 || Address == QHostAddress::AnyIPv6 || Address.isMulticast() || Input.Host.contains('%')
			|| Address == QHostAddress(QStringLiteral("255.255.255.255")))
			return Fail(Error, "Use a unicast proxy address without a scope identifier.");
	} else {
		const QString Host = QString::fromLatin1(QUrl::toAce(Input.Host));
		if (Host.isEmpty() || Host.size() > 253 || QRegularExpression(QStringLiteral("^[0-9.]+$")).match(Host).hasMatch())
			return Fail(Error, "Invalid proxy host.");
		for (const QString& Label : Host.split('.')) {
			if (!QRegularExpression(QStringLiteral("^[A-Za-z0-9](?:[A-Za-z0-9-]{0,61}[A-Za-z0-9])?$")).match(Label).hasMatch())
				return Fail(Error, "Invalid proxy host.");
		}
	}
	if (HasControls(Input.User) || HasControls(Input.Password) || Input.User.toUtf8().size() > 255 || Input.Password.toUtf8().size() > 255)
		return Fail(Error, "Credentials must not contain control characters or exceed 255 UTF-8 bytes.");
	if (Input.User.isEmpty() && !Input.Password.isEmpty())
		return Fail(Error, "A password requires a username.");
	return true;
}

bool ProxyProfiles::ParseLine(const QString& Line, SProxyInput& Input, QString& Error)
{
	const QString Value = Line.trimmed();
	Input = SProxyInput();
	Input.Name = Text("Imported proxy");
	if (Value.size() > 4096 || HasControls(Line))
		return Fail(Error, "The entry is too long or contains control characters.");
	if (Value.contains("://")) {
		QUrl Url(Value, QUrl::StrictMode);
		if (!Url.isValid() || !Url.path().isEmpty() || Url.hasQuery() || Url.hasFragment())
			return Fail(Error, "Use a proxy URL without a path, query, or fragment.");
		Input.Scheme = Url.scheme().toLower();
		Input.Host = Url.host();
		Input.Port = Url.port(-1);
		Input.User = Url.userName();
		Input.Password = Url.password();
	} else {
		// IPv6 must use brackets; a legacy password may contain colons.
		const auto Match = QRegularExpression(QStringLiteral("^(?:\\[([^\\]]+)\\]|([^:\\s]+)):([0-9]{1,5})(?::([^:]*):(.*))?$")).match(Value);
		if (!Match.hasMatch()) return Fail(Error, "Use host:port, host:port:user:password, or a SOCKS5/HTTP proxy URL.");
		Input.Host = Match.captured(1).isEmpty() ? Match.captured(2) : Match.captured(1);
		Input.Port = Match.captured(3).toInt();
		Input.User = Match.captured(4);
		Input.Password = Match.captured(5);
	}
	return Validate(Input, Error);
}

QList<SProxyInput> ProxyProfiles::ParseList(const QString& TextValue, QList<SProxyImportError>& Errors)
{
	QList<SProxyInput> Inputs;
	Errors.clear();
	if (TextValue.toUtf8().size() > 1024 * 1024) {
		Errors.append({0, Text("The import exceeds 1 MiB.")});
		return Inputs;
	}
	int Number = 0;
	for (QString Line : TextValue.split('\n')) {
		++Number;
		if (Line.endsWith('\r')) Line.chop(1);
		if (Line.trimmed().isEmpty() || Line.trimmed().startsWith('#')) continue;
		SProxyInput Input;
		QString Error;
		if (Inputs.size() >= 1000) Errors.append({Number, Text("The import limit is 1000 profiles.")});
		else if (!ParseLine(Line, Input, Error)) Errors.append({Number, Error});
		else {
			Input.Name = Text("Proxy %1").arg(Number);
			Inputs.append(Input); // Do not collapse distinct sessions sharing an ingress host.
		}
	}
	return Inputs;
}

bool ProxyProfiles::Protect(const SProxyInput& Input, SProxyProfile& Profile, QString& Error)
{
	if (!Validate(Input, Error)) return false;
	SProxyProfile Updated = Profile;
	if (Updated.Id.isEmpty()) Updated.Id = QUuid::createUuid().toString(QUuid::WithoutBraces);
	if (!ValidId(Updated.Id)) return Fail(Error, "Invalid profile identity.");
	Updated.Name = Input.Name.trimmed();
	Updated.Host = Input.Host;
	Updated.Port = quint16(Input.Port);
	Updated.Scheme = Input.Scheme;
	Updated.ProtectedAuth.clear();
	if (!Input.User.isEmpty()) {
#ifdef Q_OS_WIN
		QByteArray Plain = QJsonDocument(QJsonObject{{"user", Input.User}, {"password", Input.Password}}).toJson(QJsonDocument::Compact);
		QByteArray Entropy = Updated.Id.toUtf8();
		DATA_BLOB Source = {DWORD(Plain.size()), reinterpret_cast<BYTE*>(Plain.data())};
		DATA_BLOB Salt = {DWORD(Entropy.size()), reinterpret_cast<BYTE*>(Entropy.data())};
		DATA_BLOB Result = {};
		BOOL Ok = CryptProtectData(&Source, L"Sandboxie Plus proxy credentials", &Salt, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &Result);
		Wipe(Plain);
		if (!Ok) return Fail(Error, "Windows could not protect the credentials. Nothing was saved.");
		Updated.ProtectedAuth = QByteArray(reinterpret_cast<const char*>(Result.pbData), int(Result.cbData));
		SecureZeroMemory(Result.pbData, Result.cbData);
		LocalFree(Result.pbData);
#else
		return Fail(Error, "Credential storage requires Windows DPAPI.");
#endif
	}
	Profile = Updated;
	return true;
}

bool ProxyProfiles::Reveal(const SProxyProfile& Profile, SProxyInput& Input, QString& Error)
{
	Input = SProxyInput();
	Input.Name = Profile.Name;
	Input.Host = Profile.Host;
	Input.Port = Profile.Port;
	Input.Scheme = Profile.Scheme;
	if (!Profile.ProtectedAuth.isEmpty()) {
#ifdef Q_OS_WIN
		QByteArray Cipher = Profile.ProtectedAuth;
		QByteArray Entropy = Profile.Id.toUtf8();
		DATA_BLOB Source = {DWORD(Cipher.size()), reinterpret_cast<BYTE*>(Cipher.data())};
		DATA_BLOB Salt = {DWORD(Entropy.size()), reinterpret_cast<BYTE*>(Entropy.data())};
		DATA_BLOB Result = {};
		if (!CryptUnprotectData(&Source, nullptr, &Salt, nullptr, nullptr, CRYPTPROTECT_UI_FORBIDDEN, &Result))
			return Fail(Error, "Credentials are unavailable for this Windows user or computer. Re-enter them.");
		QByteArray Plain(reinterpret_cast<const char*>(Result.pbData), int(Result.cbData));
		SecureZeroMemory(Result.pbData, Result.cbData);
		LocalFree(Result.pbData);
		QJsonParseError ParseError;
		const QJsonDocument Doc = QJsonDocument::fromJson(Plain, &ParseError);
		Wipe(Plain);
		if (ParseError.error != QJsonParseError::NoError || !Doc.isObject()
			|| !Doc.object().value("user").isString() || !Doc.object().value("password").isString())
			return Fail(Error, "Invalid protected credential data.");
		Input.User = Doc.object().value("user").toString();
		Input.Password = Doc.object().value("password").toString();
#else
		return Fail(Error, "Credential storage requires Windows DPAPI.");
#endif
	}
	return Validate(Input, Error);
}

QString ProxyProfiles::ProxyUrl(const SProxyInput& Input)
{
	QUrl Url;
	Url.setScheme(Input.Scheme);
	Url.setHost(Input.Host);
	Url.setPort(Input.Port);
	if (!Input.User.isEmpty()) {
		Url.setUserName(Input.User);
		Url.setPassword(Input.Password);
	}
	return Url.toString(QUrl::FullyEncoded);
}

bool ProxyProfiles::Load(const QString& Path, QList<SProxyProfile>& Profiles, QString& Error)
{
	QFile File(Path);
	if (!File.exists()) { Profiles.clear(); return true; }
	if (!File.open(QIODevice::ReadOnly) || File.size() > 4 * 1024 * 1024)
		return Fail(Error, "Cannot read the proxy profile file, or it exceeds 4 MiB.");
	QJsonParseError ParseError;
	const QJsonDocument Doc = QJsonDocument::fromJson(File.readAll(), &ParseError);
	if (ParseError.error != QJsonParseError::NoError || !Doc.isObject() || Doc.object().value("version").toDouble() != 1
		|| !Doc.object().value("profiles").isArray() || Doc.object().value("profiles").toArray().size() > 1000)
		return Fail(Error, "Invalid proxy profile file. The original file was not modified.");
	QList<SProxyProfile> Loaded;
	QSet<QString> Ids;
	for (const auto& Value : Doc.object().value("profiles").toArray()) {
		SProxyProfile Profile;
		if (!Value.isObject()) return Fail(Error, "Invalid saved proxy profile.");
		if (!SProxyProfile::FromJson(Value.toObject(), Profile, Error)) return false;
		if (Ids.contains(Profile.Id)) return Fail(Error, "Duplicate profile identity in the saved file.");
		Ids.insert(Profile.Id);
		Loaded.append(Profile);
	}
	Profiles = Loaded;
	return true;
}

bool ProxyProfiles::Save(const QString& Path, const QList<SProxyProfile>& Profiles, QString& Error)
{
	if (Profiles.size() > 1000) return Fail(Error, "At most 1000 profiles can be saved.");
	QJsonArray Array;
	QSet<QString> Ids;
	for (const SProxyProfile& Profile : Profiles) {
		SProxyProfile Check;
		if (!SProxyProfile::FromJson(Profile.ToJson(), Check, Error) || Ids.contains(Profile.Id))
			return Fail(Error, "Cannot save an invalid or duplicate profile identity.");
		Ids.insert(Profile.Id);
		Array.append(Profile.ToJson());
	}
	const QByteArray Data = QJsonDocument(QJsonObject{{"version", 1}, {"profiles", Array}}).toJson();
	if (Data.size() > 4 * 1024 * 1024) return Fail(Error, "The profile store exceeds 4 MiB.");
	QSaveFile File(Path);
	File.setDirectWriteFallback(false);
	if (!File.open(QIODevice::WriteOnly) || File.write(Data) != Data.size() || !File.commit())
		return Fail(Error, "Cannot save proxy profiles atomically. The previous file was retained.");
	return true;
}
