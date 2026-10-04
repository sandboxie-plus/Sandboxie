// SPDX-License-Identifier: LicenseRef-Sandboxie-Plus
// See ../LICENSE for the SandMan license.
#pragma once
#include <QByteArray>
#include <QJsonObject>
#include <QList>
#include <QString>

struct SProxyInput
{
	QString Name;
	QString Scheme = QStringLiteral("socks5");
	QString Host;
	int Port = 1080;
	QString User;
	QString Password;
};

struct SProxyProfile
{
	QString Id;
	QString Name;
	QString Scheme;
	QString Host;
	quint16 Port = 0;
	QByteArray ProtectedAuth;

	QString AdapterName() const;
	QJsonObject ToJson() const;
	static bool FromJson(const QJsonObject& Object, SProxyProfile& Profile, QString& Error);
};

struct SProxyImportError
{
	int Line = 0;
	QString Reason; // Never contains the input line or credentials.
};

namespace ProxyProfiles
{
	bool Validate(const SProxyInput& Input, QString& Error);
	bool ParseLine(const QString& Line, SProxyInput& Input, QString& Error);
	QList<SProxyInput> ParseList(const QString& Text, QList<SProxyImportError>& Errors);
	bool Protect(const SProxyInput& Input, SProxyProfile& Profile, QString& Error);
	bool Reveal(const SProxyProfile& Profile, SProxyInput& Input, QString& Error);
	QString ProxyUrl(const SProxyInput& Input);
	bool Load(const QString& Path, QList<SProxyProfile>& Profiles, QString& Error);
	bool Save(const QString& Path, const QList<SProxyProfile>& Profiles, QString& Error);
}
