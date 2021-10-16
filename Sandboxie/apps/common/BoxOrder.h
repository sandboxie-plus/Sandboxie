/*
 * Copyright 2004-2020 Sandboxie Holdings, LLC 
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
// Box Order Utility
//---------------------------------------------------------------------------


#ifndef _MY_BOXORDER_H
#define _MY_BOXORDER_H


#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------------
// Types and Structures
//---------------------------------------------------------------------------


typedef struct _BOX_ORDER_ENTRY {

    struct _BOX_ORDER_ENTRY *next;
    struct _BOX_ORDER_ENTRY *children;
    WCHAR name[1];

} BOX_ORDER_ENTRY;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


BOX_ORDER_ENTRY *BoxOrder_Alloc(const WCHAR *name);

void BoxOrder_Free(BOX_ORDER_ENTRY *entry);

void BoxOrder_Append(BOX_ORDER_ENTRY *parent, BOX_ORDER_ENTRY *child);

BOX_ORDER_ENTRY *BoxOrder_Read(void);

BOX_ORDER_ENTRY *BoxOrder_ReadDefault(void);

int BoxOrder_Write(BOX_ORDER_ENTRY *root);



//---------------------------------------------------------------------------


#ifdef __cplusplus
}
#endif


#endif _MY_BOXORDER_H
