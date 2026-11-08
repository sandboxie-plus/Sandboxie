#include "stdafx.h"
#include "McpProtocol.h"
#include <QJsonDocument>
#include <QJsonArray>

namespace McpProtocol
{

bool IsEndpointNameSafe(const QString& Name, QString* pReason)
{
	const QString Trimmed = Name.trimmed();

	if (Trimmed.isEmpty()) {
		if (pReason) *pReason = QObject::tr("the endpoint name is empty");
		return false;
	}

	if (Trimmed.length() > 64) {
		if (pReason) *pReason = QObject::tr("the endpoint name is longer than 64 characters");
		return false;
	}

	foreach(const QChar& Char, Trimmed) {
		const bool bPlain = (Char >= QChar('a') && Char <= QChar('z'))
			|| (Char >= QChar('A') && Char <= QChar('Z'))
			|| (Char >= QChar('0') && Char <= QChar('9'))
			|| Char == QChar('_') || Char == QChar('-') || Char == QChar('.');
		if (!bPlain) {
			if (pReason) *pReason = QObject::tr(
				"the endpoint name contains '%1'; only letters, digits, underscore, hyphen and "
				"dot are allowed").arg(Char);
			return false;
		}
	}

	// Case-insensitive, because pipe names are, and so is the path matching
	// the template is compared with.
	if (!Trimmed.startsWith(QString(kEndpointPrefix), Qt::CaseInsensitive)) {
		if (pReason) *pReason = QObject::tr(
			"the endpoint name must start with '%1' - that is the prefix the template which "
			"keeps sandboxed programs away from this pipe matches, and a name outside it would "
			"not be protected").arg(kEndpointPrefix);
		return false;
	}

	return true;
}

QStringList SupportedVersions()
{
	// Newest first. The legacy entries are the initialize-handshake revisions
	// that shipping clients still speak; serving them is what makes this
	// dual-era rather than correct-but-unusable.
	return QStringList()
		<< kVersionModern
		<< kVersionLegacyNewest
		<< "2025-06-18"
		<< "2025-03-26";
}

bool IsSupportedVersion(const QString& Version)
{
	return SupportedVersions().contains(Version);
}

bool IsModernVersion(const QString& Version)
{
	// Versions are YYYY-MM-DD and ordered lexicographically by construction, so
	// "later than or equal to the first modern revision" is a string compare.
	// This deliberately accepts a future revision we have not seen rather than
	// rejecting it as unknown - the caller still checks IsSupportedVersion.
	return !Version.isEmpty() && Version >= QString(kVersionModern);
}

int HttpStatusForError(int ErrorCode)
{
	switch (ErrorCode)
	{
		case eMethodNotFound:			return 404;	// spec: 404 plus a JSON-RPC body
		case eHeaderMismatch:			return 400;
		case eUnsupportedProtocolVersion:	return 400;
		case eParseError:				return 400;
		case eInvalidRequest:			return 400;
		case eInvalidParams:			return 400;
		default:						return 500;
	}
}

QString SRequest::NameOrUri() const
{
	if (Params.contains("name"))
		return Params.value("name").toString();
	if (Params.contains("uri"))
		return Params.value("uri").toString();
	return QString();
}

bool SRequest::RequiresName() const
{
	return Method == "tools/call" || Method == "resources/read" || Method == "prompts/get";
}

SRequest ParseRequest(const QByteArray& Body)
{
	SRequest Request;
	Request.ErrorCode = eParseError;

	QJsonParseError ParseError;
	QJsonDocument Doc = QJsonDocument::fromJson(Body, &ParseError);
	if (ParseError.error != QJsonParseError::NoError) {
		Request.Error = QObject::tr("malformed JSON: %1").arg(ParseError.errorString());
		return Request;
	}
	if (!Doc.isObject()) {
		// Batching was removed from the protocol, so an array is not a batch we
		// should try to honour - it is simply not a request.
		Request.Error = QObject::tr("message is not a JSON object");
		Request.ErrorCode = eInvalidRequest;
		return Request;
	}

	QJsonObject Obj = Doc.object();
	Request.ErrorCode = eInvalidRequest;

	if (Obj.value("jsonrpc").toString() != "2.0") {
		Request.Error = QObject::tr("missing or wrong jsonrpc version");
		return Request;
	}

	Request.Method = Obj.value("method").toString();
	if (Request.Method.isEmpty()) {
		Request.Error = QObject::tr("no method");
		return Request;
	}

	// An id of null is not the same as an absent id: the first is a request
	// with a null id, the second is a notification.
	Request.bHasId = Obj.contains("id");
	Request.Id = Obj.value("id");
	Request.bNotification = !Request.bHasId;

	if (Obj.value("params").isObject())
		Request.Params = Obj.value("params").toObject();

	if (Request.Params.value("_meta").isObject()) {
		Request.Meta = Request.Params.value("_meta").toObject();
		Request.ProtocolVersion = Request.Meta.value(kMetaProtocolVersion).toString();
		Request.ClientInfo = Request.Meta.value(kMetaClientInfo).toObject();
		Request.ClientCapabilities = Request.Meta.value(kMetaClientCapabilities).toObject();
	}

	Request.bModern = IsModernVersion(Request.ProtocolVersion);
	Request.bLegacyInitialize = (Request.Method == "initialize");

	Request.bValid = true;
	Request.ErrorCode = 0;
	Request.Error.clear();
	return Request;
}

bool IsHeaderSentinel(const QString& Value)
{
	// The markers are case-sensitive and must appear exactly.
	return Value.startsWith("=?base64?") && Value.endsWith("?=") && Value.length() >= 11;
}

QString DecodeHeaderValue(const QString& Value)
{
	if (!IsHeaderSentinel(Value))
		return Value;
	QByteArray Encoded = Value.mid(9, Value.length() - 11).toLatin1();
	return QString::fromUtf8(QByteArray::fromBase64(Encoded));
}

bool ValidateHeaders(const SRequest& Request, const QString& HdrProtocolVersion, const QString& HdrMethod, const QString& HdrName, QString* pError)
{
	// Legacy revisions did not define the mirrored headers, so there is nothing
	// to validate and demanding them would lock out every shipping client.
	if (!Request.bModern)
		return true;

	if (HdrProtocolVersion.isEmpty()) {
		if (pError) *pError = QObject::tr("missing MCP-Protocol-Version header");
		return false;
	}
	if (HdrProtocolVersion != Request.ProtocolVersion) {
		if (pError) *pError = QObject::tr("MCP-Protocol-Version header '%1' does not match body value '%2'").arg(HdrProtocolVersion).arg(Request.ProtocolVersion);
		return false;
	}

	if (HdrMethod.isEmpty()) {
		if (pError) *pError = QObject::tr("missing Mcp-Method header");
		return false;
	}
	if (HdrMethod != Request.Method) {
		if (pError) *pError = QObject::tr("Mcp-Method header '%1' does not match body value '%2'").arg(HdrMethod).arg(Request.Method);
		return false;
	}

	if (Request.RequiresName())
	{
		if (HdrName.isEmpty()) {
			if (pError) *pError = QObject::tr("missing Mcp-Name header for %1").arg(Request.Method);
			return false;
		}
		// Compare the decoded value: a name that is not header-safe travels
		// base64-wrapped and would never match the body as sent.
		const QString Decoded = DecodeHeaderValue(HdrName);
		if (Decoded != Request.NameOrUri()) {
			if (pError) *pError = QObject::tr("Mcp-Name header '%1' does not match body value '%2'").arg(Decoded).arg(Request.NameOrUri());
			return false;
		}
	}

	return true;
}

static QByteArray Serialize(const QJsonObject& Message)
{
	return QJsonDocument(Message).toJson(QJsonDocument::Compact);
}

QByteArray MakeResult(const QJsonValue& Id, const QJsonObject& Result)
{
	QJsonObject Message;
	Message["jsonrpc"] = "2.0";
	Message["id"] = Id;
	Message["result"] = Result;
	return Serialize(Message);
}

QByteArray MakeError(const QJsonValue& Id, int Code, const QString& Message, const QJsonObject& Data)
{
	QJsonObject Error;
	Error["code"] = Code;
	Error["message"] = Message;
	if (!Data.isEmpty())
		Error["data"] = Data;

	QJsonObject Out;
	Out["jsonrpc"] = "2.0";
	Out["id"] = Id;
	Out["error"] = Error;
	return Serialize(Out);
}

QByteArray MakeUnsupportedVersionError(const QJsonValue& Id, const QString& Requested)
{
	QJsonArray Supported;
	foreach(const QString& Version, SupportedVersions())
		Supported.append(Version);

	QJsonObject Data;
	Data["supported"] = Supported;
	Data["requested"] = Requested;

	return MakeError(Id, eUnsupportedProtocolVersion, QObject::tr("Unsupported protocol version"), Data);
}

}
