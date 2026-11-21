#pragma once
#include "../../QSbieAPI/SbieAPI.h"

//
// CBoxShadows - the shadow lifecycle, lifted out of CWizardEngine.
//
// A shadow is a clone of a box's or a template's *configuration*, written to a
// <Name>_Shadow section whose FileRootPath points at the original's file set.
// A caller changes the clone freely and nothing reaches the user's box until
// ApplyShadowChanges copies the net difference back. Discarding is therefore
// free rather than a rollback: the original was never modified.
//
// Note what is NOT shadowed: the file set is shared on purpose, because testing
// a config fix against the real profile is the entire value. Files written while
// testing land in the original box and a discard does not undo them - that axis
// needs a box snapshot instead.
//
// This class holds no engine, no thread and no script state - the code never
// did, it merely lived on CWizardEngine. Owners hold one by composition, so an
// agent session does not inherit a JS interpreter to get a shadow. Destruction
// discards every shadow not marked applied, which is what makes "do nothing"
// the safe outcome after a crash or an abandoned session.
//

class CBoxShadows
{
public:
	CBoxShadows() {}
	~CBoxShadows();

	// Clones pIni's config to a new <name>_Shadow section sharing its file set.
	// Repeated calls for the same original return the existing shadow.
	QSharedPointer<CSbieIni> MakeShadow(const QSharedPointer<CSbieIni>& pIni);

	// Takes ownership of an entry that is itself new - a box or template the
	// caller just created. It has no original to copy back to, so committing
	// only clears its IsShadow flag.
	void		AddShadow(const QSharedPointer<CSbieIni>& pIni);

	void		SetApplyShadow(const QString& OriginalName, bool bApply = true);
	bool		IsNoAppliedShadow(const QString& OriginalName);

	// The commit. Copies the net difference of every shadow marked for applying
	// back onto its original, then commits the ini.
	bool		ApplyShadowChanges();

	// What the destructor does, callable early - terminates each shadow box and
	// removes its section. Entries already applied are kept.
	void		DiscardAll();

	bool		HasShadows() const				{ return !m_Shadows.isEmpty(); }
	QStringList	GetOriginalNames() const		{ return m_Shadows.keys(); }

	// The <name>_Shadow actually in use for an original, or empty if there is none.
	QString		GetShadowName(const QString& OriginalName) const;

protected:
	struct SBoxShadow {
		SBoxShadow() : iApplyChanges(0) {}
		QSharedPointer<CSbieIni> pShadow;
		QSharedPointer<CSbieIni> pOriginal;
		int iApplyChanges;	// 0: discard, 1: apply on commit, 2: applied or new - keep
	};

	QMap<QString, SBoxShadow> m_Shadows;
};
