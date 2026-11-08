#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

//
// McpTools - the tool surface.
//
// Every tool here is read-only. Nothing in this file writes configuration,
// starts a process, or touches the file system of a box: the write tiers arrive
// with the consent machinery that gates them, and shipping a mutating tool
// before that gate exists would be the one ordering mistake worth avoiding.
//
// Results follow the MCP tool-result shape: a content array, plus
// structuredContent for the machine-readable form, so a model can read either.
// isError marks a tool that ran and failed, which is not the same as a
// protocol-level error.
//
class CMcpDocs;
class CMcpKnowledge;

namespace McpTools
{
	// The documentation pack the docs_* tools read. Not owned here; the server
	// mounts it once and hands it over. NULL means the pack is unavailable, and
	// the docs tools then say so rather than disappearing - a tool that vanishes
	// looks like a protocol fault, while one that explains itself does not.
	void			SetDocs(CMcpDocs* pDocs);
	void			SetKnowledge(CMcpKnowledge* pKnowledge);

	// The tools/list payload: an array of {name, title, description, inputSchema}.
	QJsonArray		Describe();

	// True for the tools that read only what Sandboxie ships - the
	// documentation pack, the settings schema, and which version this is.
	// These need no authorisation, and asking for one would be theatre: the
	// same bytes sit in files any process running as this user can open, and
	// the version is a GetVersionEx call away.
	//
	// Everything else here reads the machine - which sandboxes exist, what is
	// running in them, what they were denied - and that is private, so it
	// waits for pairing.
	bool			IsPublic(const QString& Name);

	bool			Exists(const QString& Name);

	// Runs a tool. Always returns a tool result - a failure inside a tool is
	// reported as isError, never as a JSON-RPC error, because the model should
	// see what went wrong and adjust rather than lose the turn.
	QJsonObject		Call(const QString& Name, const QJsonObject& Arguments);

	// Helpers shared with whatever ends up building results elsewhere.
	QJsonObject		MakeTextResult(const QString& Text, const QJsonObject& Structured = QJsonObject());
	QJsonObject		MakeErrorResult(const QString& Text);

	// Adds a second block of text to a result that has already been built.
	// Used to put a reminder on the answer to the call that made it relevant,
	// which is the only place an agent is certain to read it.
	void			AppendNote(QJsonObject& Result, const QString& Note);
}
