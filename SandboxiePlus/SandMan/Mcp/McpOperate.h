#pragma once
#include <QJsonArray>
#include <QJsonObject>
#include <QString>

class CMcpSession;
class CMcpConsent;

//
// McpOperate - the tools that change something.
//
// Kept apart from the read-only surface on purpose. Everything here needs a
// session (shadows belong to one) and user consent, and having the mutating
// tools in their own translation unit makes "what can this thing actually do to
// my machine" answerable by reading one file.
//
// One thing is deliberately absent, and its absence is the design: nothing
// deletes a box or its contents. The gain is small - an agent rarely needs to
// delete in order to diagnose - and the cost of getting it wrong is a user's
// data.
//
// box_run, which starts a program in a box, is the one tool here whose risk is
// not bounded by Sandboxie: a sandbox constrains what a program can reach, but
// the agent chose the program, so "it only runs sandboxed" is weaker than it
// sounds. It exists anyway, because a fix that can only be described is a fix
// the user has to test by hand, and the point of this interface is that they
// do not have to. What carries it is the confirmation: the command line is
// shown verbatim, on its own line, and nothing runs until the user agrees to
// that exact text. The constraints in CallBoxRun exist to keep that showing
// honest rather than to vet the command.
//
namespace McpOperate
{
	// Set once when the server arms. NULL disables every tool here, which is
	// the right failure mode: without a way to ask, nothing may change.
	void			SetConsent(CMcpConsent* pConsent);

	QJsonArray		Describe();
	bool			Exists(const QString& Name);

	// pSession is never NULL: shadows are per-session state, and a tool that
	// could not find its session would be a tool acting on nobody's behalf.
	QJsonObject		Call(const QString& Name, const QJsonObject& Arguments, CMcpSession* pSession);

	// Removes <box>_Shadow sections left behind by a crash. Called at startup,
	// where it also covers the wizard, whose shadows leak the same way.
	int				SweepOrphanShadows();
}
