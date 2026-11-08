#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class CMcpSession;
class CMcpConsent;

//
// McpReport - composing and submitting a troubleshooting report.
//
// This is the end of the loop the whole feature was built for: diagnose, fix,
// and tell the project what happened. It is also the only place where anything
// leaves the machine, so the rules here are stricter than anywhere else.
//
//   * nothing is ever submitted automatically. The agent composes; the user
//     reads the exact bytes and decides
//   * the agent cannot swap the payload between review and send. report_submit
//     takes no body - it sends the draft that was composed and shown, which is
//     the only way "you approved these bytes" can be a true statement
//   * machine-captured and agent-written halves are labelled and never
//     interleaved. A confidently wrong diagnosis costs a maintainer time, and
//     at volume it costs a great deal of it, so a reader has to be able to see
//     at a glance which sentences are evidence
//   * everything passes through McpRedact first, prose included. Agent prose
//     can contain anything the agent saw
//
// A success is worth reporting too, and the project currently never sees one.
// A run that found a cause and applied a working fix is exactly the material a
// knowledge-base entry is made of, which is why a fixed outcome also emits a
// draft KB entry and a flat envelope built for counting rather than reading.
//
namespace McpReport
{
	// Set once when the server arms. NULL disables submission - without a way
	// to ask, nothing may be sent.
	void			SetConsent(CMcpConsent* pConsent);

	QJsonArray		Describe();
	bool			Exists(const QString& Name);

	QJsonObject		Call(const QString& Name, const QJsonObject& Arguments, CMcpSession* pSession);
}
