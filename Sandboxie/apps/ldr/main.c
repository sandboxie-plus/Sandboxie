/*
 * Copyright 2020 David Xanatos, xanasoft.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include "global.h"
#include "common/my_version.h"
#include "msgs/msgs.h"

int __cdecl wmain(int argc, wchar_t **argv)
{
	STARTUPINFOW si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	BOOLEAN isWow64 = FALSE;
	ULONG errlvl;

	if (argc < 2) {
		fprintf(stderr, "Usage: SbieLdr EXE\n");
		fprintf(stderr, "Inject a SbieDll.dll into a process during start up.\n");
		return 1;
	}

	errlvl = SbieDll_InjectLow_InitHelper();
	if (errlvl) {
		fprintf(stderr, "Failed to initialize helper 0x%08X.\n", errlvl);
		return errlvl;
	}

	errlvl = SbieDll_InjectLow_InitSyscalls(FALSE);
	if (errlvl) {
		fprintf(stderr, "Failed to initialize syscalls 0x%08X.\n", errlvl);
		return errlvl;
	}

	si.cb = sizeof(STARTUPINFO);
	if (!CreateProcessW(NULL, argv[1], NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
		fprintf(stderr, "CreateProcess(\"%S\") failed; error code = 0x%08X\n", argv[1], GetLastError());
		return 1;
	}

#ifdef _WIN64
	ULONG_PTR peb32;
	if (!NT_SUCCESS(NtQueryInformationProcess(pi.hProcess, ProcessWow64Information, &peb32, sizeof(ULONG_PTR), NULL))) {
		fprintf(stderr, "NtQueryInformationProcess failed; error code = 0x%08X\n", GetLastError());
		errlvl = 1;
		goto finish;
	}
	isWow64 = !!peb32;
#endif

	errlvl = SbieDll_InjectLow(pi.hProcess, isWow64, 2, FALSE);
	if (errlvl) {
		fprintf(stderr, "inject failed 0x%08X, prcess terminate.\n", errlvl);
		goto finish;
	}

	if (ResumeThread(pi.hThread) == -1) {
		fprintf(stderr, "ResumeThread failed; error code = 0x%08X\n", GetLastError());
		errlvl = 1;
		goto finish;
	}

finish:
	if(errlvl)
		TerminateProcess(pi.hProcess, -1);

	CloseHandle(pi.hProcess);

	return errlvl;
}
