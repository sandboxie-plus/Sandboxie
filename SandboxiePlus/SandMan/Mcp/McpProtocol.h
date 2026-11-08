#pragma once
#include <QJsonObject>
#include <QJsonValue>
#include <QStringList>

//
// McpProtocol - JSON-RPC and MCP framing, with no transport and no Sandboxie in
// it, so it can be tested on its own.
//
// Two protocol eras have to be served from one endpoint:
//
//   modern (2026-07-28 and later)  no handshake. Every request carries its
//                                  version, client identity and capabilities in
//                                  _meta, and server/discover is mandatory.
//                                  Protocol-level sessions, the GET stream and
//                                  Last-Event-ID resumability do not exist.
//
//   legacy (2025-11-25 and older)  an initialize handshake establishes a
//                                  session; this is what the clients people
//                                  actually have installed still speak.
//
// A request carrying modern _meta is served statelessly; an initialize request
// selects legacy semantics. Both are explicitly sanctioned by the spec, and
// being dual-era is the only way to be useful now and correct later.
//

namespace McpProtocol
{
	// Protocol versions, newest first. The first entry is what we prefer and
	// what server/discover reports as current.
	const char* const kVersionModern		= "2026-07-28";
	const char* const kVersionLegacyNewest	= "2025-11-25";

	// The pipe name the listener may open, and why it is not free text.
	//
	// [Template_SbieMcpProtect] keeps sandboxed programs off this endpoint with
	// ClosedFilePath=\Device\NamedPipe\SbieMcp*, and that pattern is a literal
	// in a file shipped with the product - it cannot follow a name someone put
	// in their own configuration. A measurement made this concrete: pointed at
	// a name outside the prefix, the endpoint was reachable from inside a box
	// and only the driver gate turned it away. Two layers became one, silently,
	// because of a setting that looks cosmetic.
	//
	// So the listener refuses a name its protection does not cover. Refusing to
	// arm is the right failure: the alternative is arming with one layer gone
	// and nobody being told.
	const char* const kEndpointPrefix = "SbieMcp";

	// Also rejects anything that is not a plain name - a separator or a
	// wildcard in a pipe name is either an escape from the namespace or a
	// pattern the template matches differently than intended.
	bool				IsEndpointNameSafe(const QString& Name, QString* pReason = NULL);

	QStringList			SupportedVersions();
	bool				IsSupportedVersion(const QString& Version);
	bool				IsModernVersion(const QString& Version);

	// _meta keys. These are spelled out rather than built from a prefix because
	// a typo in one would silently turn into "no version declared".
	const char* const kMetaProtocolVersion		= "io.modelcontextprotocol/protocolVersion";
	const char* const kMetaClientInfo			= "io.modelcontextprotocol/clientInfo";
	const char* const kMetaClientCapabilities	= "io.modelcontextprotocol/clientCapabilities";

	enum EErrorCode
	{
		// JSON-RPC
		eParseError			= -32700,
		eInvalidRequest		= -32600,
		eMethodNotFound		= -32601,
		eInvalidParams		= -32602,
		eInternalError		= -32603,

		// MCP's reserved sub-range
		eHeaderMismatch				= -32020,
		eUnsupportedProtocolVersion	= -32022,
	};

	// The HTTP status that must accompany a given JSON-RPC error. The spec is
	// specific about these and they are not all 400: an unknown method is a 404
	// so that the JSON-RPC body distinguishes it from a legacy server that does
	// not host the endpoint at all.
	int					HttpStatusForError(int ErrorCode);

	struct SRequest
	{
		SRequest() : bValid(false), bHasId(false), bNotification(false), bModern(false), bLegacyInitialize(false) {}

		bool		bValid;
		QString		Error;			// why it is not valid, for the log
		int			ErrorCode;		// which JSON-RPC error to answer with

		QJsonValue	Id;
		bool		bHasId;			// absent id means notification
		bool		bNotification;

		QString		Method;
		QJsonObject	Params;
		QJsonObject	Meta;

		QString		ProtocolVersion;	// from _meta, empty if not declared
		QJsonObject	ClientInfo;
		QJsonObject	ClientCapabilities;

		bool		bModern;			// declared a modern version in _meta
		bool		bLegacyInitialize;	// method == "initialize"

		// params.name or params.uri - what the Mcp-Name header must mirror.
		QString		NameOrUri() const;
		// Whether this method requires an Mcp-Name header at all.
		bool		RequiresName() const;
	};

	// Parses one JSON-RPC message. Never throws; check bValid.
	SRequest			ParseRequest(const QByteArray& Body);

	// Validates the mirrored headers against the parsed body, per the spec's
	// server-validation rules. Returns true when they agree. A mismatch, a
	// missing required header or an undecodable value is eHeaderMismatch.
	//
	// Only applies to modern requests: legacy revisions did not define these.
	bool				ValidateHeaders(const SRequest& Request, const QString& HdrProtocolVersion, const QString& HdrMethod, const QString& HdrName, QString* pError);

	// Header values that cannot be plain ASCII arrive as "=?base64?<b64>?=".
	// Returns the decoded value, or the input when the sentinel is absent.
	QString				DecodeHeaderValue(const QString& Value);
	bool				IsHeaderSentinel(const QString& Value);

	// Response builders. Id may be null for an error with no known id.
	QByteArray			MakeResult(const QJsonValue& Id, const QJsonObject& Result);
	QByteArray			MakeError(const QJsonValue& Id, int Code, const QString& Message, const QJsonObject& Data = QJsonObject());

	// The specific shape the spec requires for a version we do not implement.
	QByteArray			MakeUnsupportedVersionError(const QJsonValue& Id, const QString& Requested);
}
