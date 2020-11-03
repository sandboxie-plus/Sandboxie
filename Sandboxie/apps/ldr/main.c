/*
 * Copyright (c) 2020, David Xanatos
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

int __cdecl wmain(int argc, char **argv)
{
	int errlvl = SbieDll_InjectLow_InitHelper();

	if (errlvl) {
		//LogEvent(MSG_9234, 0x9241, errlvl);
		return errlvl;
	}

	SbieDll_InjectLow_InitSyscalls(FALSE);

	return 0;
}
