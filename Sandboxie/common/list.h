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
// Double-linked List based on a Cell Pool
//---------------------------------------------------------------------------

#ifndef _MY_LIST_H
#define _MY_LIST_H

#ifdef LIST_WITH_MEMORY_MANAGEMENT
#include "pool.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------------
// Structures
//---------------------------------------------------------------------------

typedef struct LIST_ELEM {
   struct LIST_ELEM *next;
   struct LIST_ELEM *prev;
} LIST_ELEM;

typedef struct {
    LIST_ELEM *head;
    LIST_ELEM *tail;
    int count;
#ifdef LIST_WITH_MEMORY_MANAGEMENT
    POOL *pool;
    int elem_len;
#endif
} LIST;

//---------------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------------

#ifndef LIST_WITH_MEMORY_MANAGEMENT
#include <memory.h>
/* Initializes list structure */
#define List_Init(list) (memset((list),0,sizeof(LIST)))
#endif

/* Return head elem of list LIST */
#define List_Head(list) ((void *)(((LIST *)(list))->head))

/* Return tail elem of list LIST */
#define List_Tail(list) ((void *)(((LIST *)(list))->tail))

/* Return count of elems in list LIST */
#define List_Count(list) (((LIST *)(list))->count)

/* Return elem next to elem ELEM */
#define List_Next(elem) ((void *)(((LIST_ELEM *)(elem))->next))

/* Return elem previous to elem ELEM */
#define List_Prev(elem) ((void *)(((LIST_ELEM *)(elem))->prev))

// Deletes the pool allocated to a list

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

#ifdef LIST_WITH_MEMORY_MANAGEMENT
/* Initializes list structure */
void List_Init(LIST *list);
#endif

/* Insert new elem before that pointed to by ELEM*/
void List_Insert_Before(LIST *list, void *oldelem, void *newelem);

/* Insert new elem after that pointed to by ELEM*/
void List_Insert_After(LIST *list, void *elem, void *newelem);

/* Delete elem pointed to by ELEM */
void List_Remove(LIST *list, void *elem);

#ifdef LIST_WITH_MEMORY_MANAGEMENT
/* These also manage memory for the elements */
void *List_Insert_New_Before(LIST *list, void *elem);
void *List_Insert_After(LIST *list, void *elem);
void List_Delete(LIST *list, void *elem);
#endif

//---------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif /* _MY_LIST_H */
