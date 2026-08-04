/*
* Copyright 2020-2021 David Xanatos, xanasoft.com
*
* This program is free software; you can redistribute it and/or
*   modify it under the terms of the GNU Lesser General Public
*   License as published by the Free Software Foundation; either
*   version 3 of the License, or (at your option) any later version.
* 
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   Lesser General Public License for more details.
* 
*   You should have received a copy of the GNU Lesser General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

ULONG Read_BOM(UCHAR** data, ULONG* len)
{
    ULONG encoding;

    if (*len >= 3 && (*data)[0] == 0xEF && (*data)[1] == 0xBB && (*data)[2] == 0xBF) 
    {
        *data += 3;
        *len -= 3;

        encoding = 1;
        //DbgPrint("sbie read ini, found UTF-8 Signature\n");
    }
    else if (*len >= 2 && (*data)[0] == 0xFF && (*data)[1] == 0xFE)
    {
        *data += 2;
        *len -= 2;

        encoding = 0;
        //DbgPrint("sbie read ini, found Unicode (UTF-16 LE) BOM\n");
    }
    else if (*len >= 2 && (*data)[0] == 0xFE && (*data)[1] == 0xFF)
    {
        *data += 2;
        *len -= 2;

        encoding = 2;
        //DbgPrint("sbie read ini, found Unicode (UTF-16 BE) BOM\n");
    }
    else
    {
        // If there is no BOM/Signature try to detect the file type
        // Unicode Little Endian (windows wchar_t) will have the n*2+1 bytes 0 as long, as no higher unicode characters are used
        BOOLEAN LooksUnicodeLE = TRUE;
        // similrly Unicode Big Endian (byte swapped) will have the n*2 bytes 0 as long
        BOOLEAN LooksUnicodeBE = TRUE;
        // UTF-8 shouldn't have null bytes
        for (ULONG pos = 0; (pos + 1) < min(*len, 16); pos += 2) // check first 8 char16's
        {
            if ((*data)[pos] != 0)
                LooksUnicodeBE = FALSE;
            if ((*data)[pos + 1] != 0)
                LooksUnicodeLE = FALSE;
        }

        if (!LooksUnicodeLE && !LooksUnicodeBE)
        {
            encoding = 1;
            //DbgPrint("sbie read ini, looks UTF-8 encoded\n");
        }
        else if (!LooksUnicodeLE && LooksUnicodeBE)
        {
            encoding = 2;
            //DbgPrint("sbie read ini, looks Unicode (UTF-16 BE) encoded\n");
        }
        else
        {
            encoding = 0;
            //if (LooksUnicodeLE && !LooksUnicodeBE)
            //  DbgPrint("sbie read ini, looks Unicode (UTF-16 LE) encoded\n");
            //else
            //  DbgPrint("sbie read ini, encoding looks broken, assuming (UTF-16 LE)\n");
        }
    }

    return encoding;
}