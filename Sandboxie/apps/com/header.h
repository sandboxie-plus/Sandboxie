/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC
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


//---------------------------------------------------------------------------
// Service Control Manager handles
//---------------------------------------------------------------------------


#define SC_HANDLE_MIN           ((SC_HANDLE)0x12345670)
#define SC_HANDLE_BITS          ((SC_HANDLE)0x12345671)
#define SC_HANDLE_RPCSS         ((SC_HANDLE)0x12345672)
#define SC_HANDLE_MSISERVER     ((SC_HANDLE)0x12345673)
#define SC_HANDLE_EVENTSYSTEM   ((SC_HANDLE)0x12345674)
#define SC_HANDLE_MAX           ((SC_HANDLE)0x12345679)

#define SC_HANDLE_IS_FAKE(h)    ((h) > SC_HANDLE_MIN && (h) < SC_HANDLE_MAX)