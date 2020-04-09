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
// Double-linked List
//---------------------------------------------------------------------------

#include "list.h"

#ifndef NULL
#define NULL (void *)0
#endif

//---------------------------------------------------------------------------
// List_Init
//---------------------------------------------------------------------------

#ifdef LIST_WITH_MEMORY_MANAGEMENT
void List_Init(LIST *list)
{
   list->head = NULL;
   list->tail = NULL;
   list->count = 0;
#ifdef LIST_WITH_MEMORY_MANAGEMENT
   list->pool = pool;
   list->elem_len = elemlen;
#endif
}
#endif

//---------------------------------------------------------------------------
// List_Insert_Before
//---------------------------------------------------------------------------

void List_Insert_Before(LIST *list, void *oldelem, void *newelem)
{
    LIST_ELEM *prev, *curr, *next, *nelm;

    curr = (LIST_ELEM *)oldelem;
    nelm = (LIST_ELEM *)newelem;
    ++list->count;

    if (curr == list->head || curr == NULL) {     /* Insert head elem */
      next = list->head;
      list->head = nelm;
      nelm->prev = NULL;
      nelm->next = next;
      if (next == NULL)                         /* Old head is NULL? */
         list->tail = nelm;
      else
         next->prev = nelm;

    } else {                                  /* Insert non-head elem */
      prev = curr->prev;
      prev->next = nelm;
      nelm->prev = prev;
      curr->prev = nelm;
      nelm->next = curr;
    }
}

//---------------------------------------------------------------------------
// List_Insert_After
//---------------------------------------------------------------------------

void List_Insert_After(LIST *list, void *elem, void *newelem)
{
    LIST_ELEM *prev, *curr, *next, *nelm;

    curr = (LIST_ELEM *)elem;
    nelm = (LIST_ELEM *)newelem;
    ++list->count;

    if (curr == list->tail || curr == NULL) {     /* Insert tail elem */
      prev = list->tail;
      list->tail = nelm;
      nelm->prev = prev;
      nelm->next = NULL;
      if (prev == NULL)                         /* Old tail is NULL? */
         list->head = nelm;
      else
         prev->next = nelm;

    } else {                                  /* Insert non-tail elem */
      next = curr->next;
      next->prev = nelm;
      nelm->next = next;
      curr->next = nelm;
      nelm->prev = curr;
    }
}

//---------------------------------------------------------------------------
// List_Remove
//---------------------------------------------------------------------------

void List_Remove(LIST *list, void *elem)
{
    LIST_ELEM *prev, *curr, *next;

    curr = (LIST_ELEM *)elem;
    prev = curr->prev;
    next = curr->next;
    --list->count;

    if (prev == NULL && next == NULL) {         /* Last elem in list? */
        list->tail = NULL;
        list->head = NULL;

    } else if (prev == NULL) {                       /* Elem is head? */
        list->head = next;
        next->prev = NULL;

    } else if (next == NULL) {                       /* Elem is tail? */
        list->tail = prev;
        prev->next = NULL;

    } else {
        prev->next = next;              /* Elem is non-head, non-tail */
        next->prev = prev;
    }
}

#ifdef LIST_WITH_MEMORY_MANAGEMENT

//---------------------------------------------------------------------------
// List_Insert_New_Before
//---------------------------------------------------------------------------

void List_Insert_New_Before(LIST *list, void *oldelem)
{
    LIST_ELEM *newelem = Pool_Get_Bytes(list->pool, list->elem_len);
    return List_Insert_Before(list, oldelem, newelem);
}

//---------------------------------------------------------------------------
// List_Insert_New_After
//---------------------------------------------------------------------------

void List_Insert_New_After(LIST *list, void *oldelem)
{
    LIST_ELEM *newelem = Pool_Get_Bytes(list->pool, list->elem_len);
    return List_Insert_After(list, oldelem, newelem);
}

//---------------------------------------------------------------------------
// List_Delete
//---------------------------------------------------------------------------

void List_Delete(LIST *list, void *elem)
{
    List_Remove(list, elem);
    Pool_Free_Bytes(elem, list->elem_len);
}

#endif
