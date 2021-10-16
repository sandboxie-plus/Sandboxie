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
// Arbitrary Precision Unsigned Algebera
//---------------------------------------------------------------------------


#ifndef _MY_BIGNUM_H
#define _MY_BIGNUM_H


#include "common/pool.h"


//---------------------------------------------------------------------------
// Types
//---------------------------------------------------------------------------


typedef ULONG *BIGNUM;


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


#ifdef __cplusplus
extern "C" {
#endif


BIGNUM BigNum_CreateFromInteger(POOL *pool, ULONG Value);

BIGNUM BigNum_CreateFromBigNum(POOL *pool, BIGNUM Value);

void BigNum_Free(BIGNUM BigNum);

int BigNum_Compare(BIGNUM BigNumA, BIGNUM BigNumB);

BIGNUM BigNum_ShiftRight(POOL *pool, BIGNUM BigNumA, ULONG Bits);

BIGNUM BigNum_ShiftLeft(
            POOL *pool, BIGNUM BigNumA, ULONG Bits, ULONG OrValue);

BIGNUM BigNum_Add(POOL *pool, BIGNUM BigNumA, BIGNUM BigNumB);

BIGNUM BigNum_Subtract(POOL *pool, BIGNUM BigNumA, BIGNUM BigNumB);

BIGNUM BigNum_Multiply(POOL *pool, BIGNUM BigNumA, BIGNUM BigNumB);

BIGNUM BigNum_DivideByInteger(
            POOL *pool, BIGNUM BigNumA, ULONG Divisor, ULONG *pReminder);

BIGNUM BigNum_Divide(
            POOL *pool, BIGNUM Dividend, BIGNUM Divisor, BIGNUM *pReminder);

BIGNUM BigNum_ModPow(
            POOL *pool, BIGNUM Base, BIGNUM Exponent, BIGNUM Modulus);

BIGNUM BigNum_CreateFromString(POOL *pool, const WCHAR *Value, int Base);

WCHAR *BigNum_ConvertToString(POOL *pool, BIGNUM BigNum, int Base);

BIGNUM BigNum_CreateRandom(POOL *pool, int Digits, int Base);


#ifdef __cplusplus
} // extern "C"
#endif


//---------------------------------------------------------------------------


#endif // _MY_BIGNUM_H
