#pragma once
#include <QString>
#include <QStringList>

//
// McpRedact - strips identifying detail out of diagnostic text.
//
// Diagnostic output is full of things that identify a machine: profile paths
// carry the user's name, traces carry SIDs and the computer name, temp paths
// carry per-session GUIDs. An agent will quote this material back, and a report
// submitted upstream will carry it to a bug tracker.
//
// The substitution is **canonicalising**, not merely masking. The same input
// always becomes the same token, so two reports of the same fault from two
// machines come out textually comparable - which is what makes the corpus
// clusterable later. Replacing each occurrence with a random or counted
// placeholder would protect the user just as well and destroy that property.
//
// This is a reduction in exposure, not anonymisation. It removes what is
// predictably identifying; it cannot catch a user who named a sandbox after
// themselves, and it does not pretend to.
//
namespace McpRedact
{
	// Applies every substitution. Safe on empty input.
	QString			Apply(const QString& Text);
	QStringList		Apply(const QStringList& Lines);

	// Which substitutions are in force, so a report can state plainly what was
	// removed rather than leaving the reader to wonder what %User% was.
	QStringList		Describe();
}
