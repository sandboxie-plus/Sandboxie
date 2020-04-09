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
// Lock
//---------------------------------------------------------------------------


#include "common/defines.h"
#include "common/lock.h"


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define LOCK_WAIT_DIVISOR 25


//---------------------------------------------------------------------------
// Lock_Delay
//---------------------------------------------------------------------------


ALIGNED void Lock_Delay(const WCHAR *Operation, const WCHAR *LockName)
{
#ifdef KERNEL_MODE
    LARGE_INTEGER time;
    /*
    DbgPrint("Waiting (%S) in pid %d for lock %S\n",
        Operation, PsGetCurrentProcessId(), LockName);
    */
    // DbgPrint("KERNEL WAITING FOR LOCK ==> %S\n", LockName);
    time.QuadPart = -(SECONDS(1) / LOCK_WAIT_DIVISOR);
    KeDelayExecutionThread(KernelMode, FALSE, &time);
#else
    /*
    printf("Waiting (%S) in pid %d for lock %S\n",
        Operation, GetCurrentProcessId(), LockName);
    */
    // OutputDebugString(L"USER WAITING FOR LOCK ==>\n");OutputDebugString(LockName);OutputDebugString(L"\n");
    SleepEx(1000 / LOCK_WAIT_DIVISOR, TRUE);
#endif
}


//---------------------------------------------------------------------------
// Lock_Exclusive
//---------------------------------------------------------------------------


ALIGNED void Lock_Exclusive(LOCK *lockword, const WCHAR *LockName)
{
    LONG oldval, newval;

    while (1) {
        // exclusive lock expects the exclusive bit to be clear
        oldval = (*lockword) & (~LOCK_EXCLUSIVE);
        // and sets the exclusive bit in the incremented count
        newval = LOCK_EXCLUSIVE | (oldval + 1);
        if (InterlockedCompareExchange(lockword, newval, oldval) == oldval)
            break;

        Lock_Delay(L"exc1", LockName);
    }

    while (1) {
        // now we wait for all share holders to unlock
        oldval = LOCK_EXCLUSIVE | 1;
        newval = LOCK_EXCLUSIVE | 1;
        if (InterlockedCompareExchange(lockword, newval, oldval) == oldval)
            break;

        Lock_Delay(L"exc2", LockName);
    }
}


//---------------------------------------------------------------------------
// Lock_Share
//---------------------------------------------------------------------------


ALIGNED void Lock_Share(LOCK *lockword, const WCHAR *LockName)
{
    LONG oldval, newval;

    while (1) {
        // share lock expects the count to have the exclusive bit clear
        oldval = (*lockword) & (~LOCK_EXCLUSIVE);
        // and keeps the exclusive bit clear in the incremented count
        newval = oldval + 1;
        if (InterlockedCompareExchange(lockword, newval, oldval) == oldval)
            break;

        Lock_Delay(L"shr", LockName);
    }
}


//---------------------------------------------------------------------------
// Lock_Unlock
//---------------------------------------------------------------------------


ALIGNED void Lock_Unlock(LOCK *lockword, const WCHAR *LockName)
{
    LONG oldval, newval;

    while (1) {
        oldval = *lockword;
        if (oldval == (LOCK_EXCLUSIVE | 1)) {
            // if the count indicates a single exclusive holder,
            // then clear the exclusive bit in the decremented count
            newval = LOCK_FREE;
        } else {
            // otherwise keep the exclusive bit as is
            // (either set or clear) in the decremented count
            newval = ((oldval & (~LOCK_EXCLUSIVE)) - 1)
                   |  (oldval & LOCK_EXCLUSIVE);
        }
        if (InterlockedCompareExchange(lockword, newval, oldval) == oldval)
            break;

        Lock_Delay(L"unlk", LockName);
    }
}
