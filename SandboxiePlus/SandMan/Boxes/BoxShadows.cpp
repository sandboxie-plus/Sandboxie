#include "stdafx.h"
#include "BoxShadows.h"
#include "../SbiePlusAPI.h"

CBoxShadows::~CBoxShadows()
{
	DiscardAll();
}

void CBoxShadows::DiscardAll()
{
	// Nothing to discard means nothing to reload. Every session owns one of
	// these now, so without this the common case - a session that never made a
	// shadow - would reload every box on teardown.
	if (m_Shadows.isEmpty())
		return;

	foreach(const SBoxShadow& pShadow, m_Shadows) {
		if (pShadow.pShadow) {
			if (pShadow.iApplyChanges == 2)
				continue; // this is a new added entry keep it
			CSandBoxPtr pBox = pShadow.pShadow.objectCast<CSandBox>();
			if(!pBox.isNull()) pBox->TerminateAll();
			pShadow.pShadow->RemoveSection();
		}
	}
	m_Shadows.clear();
	theAPI->ReloadBoxes(true);
}

bool CBoxShadows::ApplyShadowChanges()
{
	for (auto I = m_Shadows.begin(); I != m_Shadows.end(); ++I) {
		if (I->iApplyChanges != 1)
			continue;

		if (I->pOriginal.isNull()) {
			// This is a new box or template, not a copy, just clear shadow flag
			I->pShadow->DelValue("IsShadow", "y");
			I->iApplyChanges = 2;
			continue;
		}

		QList<CSbieIni::SbieIniValue> New = I->pShadow->GetIniSection();
		QList<CSbieIni::SbieIniValue> Old = I->pOriginal->GetIniSection();

		// discard unchanged
		for (auto I = New.begin(); I != New.end();) {
			auto II = I++;
			for (auto J = Old.begin(); J != Old.end();) {
				auto JJ = J++;
				if (II->Name == JJ->Name && II->Value == JJ->Value) {
					I = New.erase(II);
					J = Old.erase(JJ);
					break;
				}
			}
		}

		// apply changed
		foreach(auto & O, Old)
			I->pOriginal->DelValue(O.Name, O.Value);
		foreach(auto & N, New) {
			if (N.Name == "FileRootPath" || N.Name == "IsShadow")
				continue; // skip
			if(N.Name == "Template" && IsNoAppliedShadow("Template_" + N.Value))
				continue; // don't copy non-applied shadow templates
			I->pOriginal->AppendText(N.Name, N.Value);
		}
	}

	theAPI->CommitIniChanges();

	return true;
}

QSharedPointer<CSbieIni> CBoxShadows::MakeShadow(const QSharedPointer<CSbieIni>& pIni)
{
	SBoxShadow& pShadow = m_Shadows[pIni->GetName().toLower()];
	if (!pShadow.pShadow) {
		QString ShadowName = pIni->GetName();
		QString Suffix = "_Shadow"; // do not translate must be a valid sandbox name suffix
		ShadowName.truncate(32 - (Suffix.length() + 3)); // BOXNAME_COUNT
		ShadowName = theAPI->MkNewName(ShadowName.append(Suffix));

		QList<CSbieIni::SbieIniValue> Settings = pIni->GetIniSection();
		for (QList<CSbieIni::SbieIniValue>::iterator I = Settings.begin(); I != Settings.end(); ++I)
			theAPI->SbieIniSet(ShadowName, I->Name, I->Value, CSbieIni::eIniInsert, false);

		CSandBoxPtr pBox = pIni.objectCast<CSandBox>();
		if(!pBox.isNull())
			theAPI->SbieIniSet(ShadowName, "FileRootPath", pBox->GetFileRoot(), CSbieIni::eIniUpdate, false);
		theAPI->SbieIniSet(ShadowName, "IsShadow", "y", CSbieIni::eIniUpdate, false);

		theAPI->CommitIniChanges();
		theAPI->ReloadBoxes(true);

		pShadow.pOriginal = pIni;
		if (pBox)
			pShadow.pShadow = theAPI->GetBoxByName(ShadowName);
		else
			pShadow.pShadow = QSharedPointer<CSbieIni>(new CSbieIni(ShadowName, theAPI));
	}
	return pShadow.pShadow;
}

void CBoxShadows::AddShadow(const QSharedPointer<CSbieIni>& pIni)
{
	SBoxShadow& pShadow = m_Shadows[pIni->GetName().toLower()];
	if (!pShadow.pShadow) {

		pIni->SetText("IsShadow", "y");

		pShadow.pShadow = pIni;
	}
}

void CBoxShadows::SetApplyShadow(const QString& OriginalName, bool bApply)
{
	auto I = m_Shadows.find(OriginalName.toLower());
	if(I !=  m_Shadows.end())
		I->iApplyChanges = bApply ? 1 : 0;
}

bool CBoxShadows::IsNoAppliedShadow(const QString& OriginalName)
{
	auto I = m_Shadows.find(OriginalName.toLower());
	if (I != m_Shadows.end())
		return I->iApplyChanges == 0;
	return false;
}

QString CBoxShadows::GetShadowName(const QString& OriginalName) const
{
	auto I = m_Shadows.find(OriginalName.toLower());
	if (I != m_Shadows.end() && I->pShadow)
		return I->pShadow->GetName();
	return QString();
}
