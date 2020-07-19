/*
 *
 * Copyright (c) 2020, David Xanatos
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "SandBox.h"
#include "../SbieAPI.h"

//struct SSandBox
//{
//};

CSandBox::CSandBox(const QString& BoxName, class CSbieAPI* pAPI) : CSbieIni(BoxName, pAPI)
{
	//m = new SSandBox;

	// when loading a sandbox that is not initialized, initialize it
	int cfglvl = GetNum("ConfigLevel");
	if (cfglvl >= 7)
		return;
	SetNum("ConfigLevel", 7);

	if (cfglvl == 6) {
		//SetDefaultTemplates7(*this);
	}
	else if (cfglvl >= 1) {
		//UpdateTemplates(*this);
	}
	else
	{
		SetBool("AutoRecover", false);
		SetBool("BlockNetworkFiles", true);

		//SetDefaultTemplates6(*this); // why 6?

		InsertText("RecoverFolder", "%Desktop%");
		//InsertText("RecoverFolder", "%Favorites%"); // obsolete
		InsertText("RecoverFolder", "%Personal%");
		InsertText("RecoverFolder", "%{374DE290-123F-4565-9164-39C4925E467B}%"); // %USERPROFILE%\Downloads

		SetText("BorderColor", "#00FFFF,ttl"); // "#00FFFF,off"
	}
}

CSandBox::~CSandBox()
{
	//delete m;
}

void CSandBox::UpdateDetails()
{
}

SB_STATUS CSandBox::RunStart(const QString& Command)
{
	return m_pAPI->RunStart(m_Name, Command);
}

SB_STATUS CSandBox::RunCommand(const QString& Command)
{
	return m_pAPI->RunSandboxed(m_Name, Command);
}

SB_STATUS CSandBox::TerminateAll()
{
	return m_pAPI->TerminateAll(m_Name);
}

SB_STATUS CSandBox::CleanBox()
{
	SB_STATUS Status = m_pAPI->TerminateAll(m_Name);
	if (Status.IsError())
		return Status;

	QProcess* pProcess = new QProcess(this);
	connect(pProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SIGNAL(BoxCleaned()));

	// ToDo-later: do that manually
	Status = m_pAPI->RunStart(m_Name, "delete_sandbox", pProcess);
	
	return Status;
}

SB_STATUS CSandBox::RenameBox(const QString& NewName)
{
	if (QDir(m_FilePath).exists())
		return SB_ERR(tr("A sandbox must be emptied before it can be renamed."));
	if(NewName.length() > 32)
		return SB_ERR(tr("The sandbox name can not be longer than 32 charakters."));
	
	return RenameSection(QString(NewName).replace(" ", "_"));
}

SB_STATUS CSandBox::RemoveBox()
{
	if (QDir(m_FilePath).exists())
		return SB_ERR(tr("A sandbox must be emptied before it can be deleted."));

	return RemoveSection();
}
