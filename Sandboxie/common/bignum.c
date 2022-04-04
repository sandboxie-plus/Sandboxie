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


#include "common/defines.h"
#include "common/bignum.h"


//---------------------------------------------------------------------------
// Compilation Options
//---------------------------------------------------------------------------


#ifdef  BIGNUM_WITH_RANDOM
#include <stdlib.h>
#define BIGNUM_WITH_ADD
#endif  BIGNUM_WITH_RANDOM


#ifdef  BIGNUM_WITHOUT_FREE
#define BigNum_Free(x)
#endif  BIGNUM_WITHOUT_FREE


//#define BIGNUM_WITH_ADD
//#define BIGNUM_WITH_SUBTRACT
//#define BIGNUM_WITH_STRING
//#define BIGNUM_WITH_FROM_STRING
//#define BIGNUM_WITH_TO_STRING


#ifdef  BIGNUM_WITH_STRING
#define BIGNUM_WITH_FROM_STRING
#define BIGNUM_WITH_TO_STRING
#endif  BIGNUM_WITH_STRING


#ifdef  BIGNUM_WITH_FROM_STRING
#define BIGNUM_WITH_ADD
#endif  BIGNUM_WITH_FROM_STRING


//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------


#define BigNum_Assert(bn) if (! bn) goto finish

#define BigNum_FreeAndSetNull(bn) BigNum_Free(bn); bn = NULL

#define BigNum_FreeAndSetTo(bn,bnx) BigNum_Free(bn); bn = bnx

#define BigNum_IsZero(bn) (*bn == 1 && bn[1] == 0)


//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------


static void BigNum_DivideHelper(
                BIGNUM Dividend, BIGNUM Divisor, BIGNUM Quotient);


//---------------------------------------------------------------------------
// BigNum_CreateFromInteger
//---------------------------------------------------------------------------


_FX BIGNUM BigNum_CreateFromInteger(POOL *pool, ULONG Value)
{
    //
    // create a simple bignum with just one word, set to the input value
    //

    BIGNUM BigNum;

    BigNum = Pool_Alloc(pool, sizeof(ULONG) * 2);
    if (BigNum) {

        *BigNum = 1;
        BigNum[1] = Value;
    }

    return BigNum;
}


//---------------------------------------------------------------------------
// BigNum_CreateFromBigNum
//---------------------------------------------------------------------------


_FX BIGNUM BigNum_CreateFromBigNum(POOL *pool, BIGNUM Value)
{
    //
    // create a new bignum from the given input bignum.  note that leading
    // zero words are stripped when creating the new bignum.
    //

    BIGNUM BigNum;
    ULONG Words, WordIndex;

    Words = *Value;
    while (Words > 1 && Value[Words] == 0)
        --Words;

    BigNum = Pool_Alloc(pool, sizeof(ULONG) * (Words + 1));
    if (BigNum) {

        *BigNum = Words;
        for (WordIndex = 1; WordIndex <= Words; ++WordIndex)
            BigNum[WordIndex] = Value[WordIndex];
    }

    return BigNum;
}


//---------------------------------------------------------------------------
// BigNum_Free
//---------------------------------------------------------------------------


#ifndef BigNum_Free


_FX void BigNum_Free(BIGNUM BigNum)
{
    //
    // free the storage used to hold the words of a bignum.
    //
    // if BIGNUM_WITHOUT_FREE is defined, this function is not compiled,
    // and Pool_Delete will be the only way to discard bignum storage.
    //

    Pool_Free(BigNum, sizeof(ULONG) * (*BigNum + 1));
}


#endif BigNum_Free


//---------------------------------------------------------------------------
// BigNum_Compare
//---------------------------------------------------------------------------


_FX int BigNum_Compare(BIGNUM BigNumA, BIGNUM BigNumB)
{
    //
    // compare the two input bignums.  first compare the number of
    // words in each bignum, excluding any leading zero words.
    // the bignum that has a greater number of words is bigger.
    // if both bignums have the same number of words, compare
    // each word in a loop, to determine which bignum is greater.
    //

    ULONG Words1, Words2;
    int Result;

    Words1 = *BigNumA;
    while (Words1 > 1 && BigNumA[Words1] == 0)
        --Words1;

    Words2 = *BigNumB;
    while (Words2 > 1 && BigNumB[Words2] == 0)
        --Words2;

    if (Words1 > Words2)
        Result = +1;
    else if (Words2 > Words1)
        Result = -1;
    else {
        Result = 0;

        while (Words1) {
            if (BigNumA[Words1] > BigNumB[Words1]) {
                Result = +1;
                break;
            }
            if (BigNumB[Words1] > BigNumA[Words1]) {
                Result = -1;
                break;
            }
            --Words1;
        }
    }

    return Result;
}


//---------------------------------------------------------------------------
// BigNum_ShiftRight
//---------------------------------------------------------------------------


_FX BIGNUM BigNum_ShiftRight(POOL *pool, BIGNUM BigNumA, ULONG Bits)
{
    //
    // shift an input bignum by the specified number of bits.  calculate
    // the number of words in the result as the number of signficant words
    // in the input, minus the number of shift bits divided by the size of
    // a bignum word.  if the new length is zero or one words, then use a
    // zero value or the value of the highest word, respectively, to create
    // a single word bignum.  otherwise right-shift each two consecutive
    // words from the input bignum into a single word in the result bignum.
    //

    BIGNUM BigNumB, BigNumC;
    ULONG ShiftBits, WordsToSkip;
    ULONG Words, WordIndex;
    ULONG64 TwoWords;

    Words = *BigNumA;
    while (Words >= 1 && BigNumA[Words] == 0)
        --Words;

    ShiftBits = Bits % 32;
    WordsToSkip = Bits / 32;

    if (Words > WordsToSkip)
        Words -= WordsToSkip;
    else
        Words = 0;

    if (Words <= 1) {

        if (Words == 0 || WordsToSkip >= *BigNumA)
            WordIndex = 0;
        else
            WordIndex = BigNumA[WordsToSkip + 1];
        WordIndex >>= ShiftBits;
        BigNumB = BigNum_CreateFromInteger(pool, WordIndex);

    } else {

        BigNumB = Pool_Alloc(pool, sizeof(ULONG) * (Words + 1));
        if (BigNumB) {

            *BigNumB = Words;

            for (WordIndex = 1; WordIndex <= Words; ++WordIndex) {
                if (WordIndex < Words) {
                    TwoWords = BigNumA[WordIndex + WordsToSkip + 1];
                    TwoWords <<= 32;
                } else
                    TwoWords = 0;
                TwoWords |= BigNumA[WordIndex + WordsToSkip];
                BigNumB[WordIndex] = (ULONG)(TwoWords >> ShiftBits);
            }

            if (BigNumB[Words] == 0) {
                BigNumC = BigNum_CreateFromBigNum(pool, BigNumB);
                BigNum_Free(BigNumB);
                BigNumB = BigNumC;
            }
        }
    }

    return BigNumB;
}


//---------------------------------------------------------------------------
// BigNum_ShiftLeft
//---------------------------------------------------------------------------


_FX BIGNUM BigNum_ShiftLeft(
            POOL *pool, BIGNUM BigNumA, ULONG Bits, ULONG OrValue)
{
    //
    // shift an input bignum by the specified number of bits and 'or' the
    // last parameter into the resulting low order word.  if the number of
    // signficant words in the input is zero, the create a single word
    // bignum using the 'or' value.  otherwise calculatevthe number of
    // words in the result as the number of signficant words in the input,
    // plus the number of shift bits divided by the size of a bignum word.
    // left-shift each two consecutive words from the input bignum into a
    // single word in the result bignum.
    //

    BIGNUM BigNumB, BigNumC;
    ULONG ShiftBits, WordsToAdd;
    ULONG Words, WordIndex;
    ULONG64 TwoWords;

    Words = *BigNumA;
    while (Words >= 1 && BigNumA[Words] == 0)
        --Words;

    if (Words == 0) {

        BigNumB = BigNum_CreateFromInteger(pool, OrValue);

    } else {

        ShiftBits = Bits % 32;
        WordsToAdd = Bits / 32 + 1;

        Words += WordsToAdd;
        BigNumB = Pool_Alloc(pool, sizeof(ULONG) * (Words + 1));
        if (BigNumB) {

            *BigNumB = Words;

            for (WordIndex = Words; WordIndex >= 1; --WordIndex) {
                TwoWords = 0;
                if (WordIndex >= WordsToAdd) {
                    if (WordIndex < Words) {
                        TwoWords = BigNumA[WordIndex - WordsToAdd + 1];
                        TwoWords <<= 32;
                    }
                    if (WordIndex > WordsToAdd)
                        TwoWords |= BigNumA[WordIndex - WordsToAdd];
                }
                BigNumB[WordIndex] = (ULONG)((TwoWords << ShiftBits) >> 32);
            }

            BigNumB[1] |= OrValue;

            if (BigNumB[Words] == 0) {
                BigNumC = BigNum_CreateFromBigNum(pool, BigNumB);
                BigNum_Free(BigNumB);
                BigNumB = BigNumC;
            }
        }
    }

    return BigNumB;
}


//---------------------------------------------------------------------------
// BigNum_Add
//---------------------------------------------------------------------------


#ifdef BIGNUM_WITH_ADD


_FX BIGNUM BigNum_Add(POOL *pool, BIGNUM BigNumA, BIGNUM BigNumB)
{
    //
    // addition of bignum to bignum.  we expect the first bignum to have
    // at least as many bignum words as the second bignum, and switch
    // the arguments if necessary.  a result bignum is allocated with
    // space for one more word than the first bignum.  then we fill the
    // result with the contents of the second, smaller bignum.  then we
    // add each digit of the larger bignum into the result.
    //
    // the result bignum will likely have zero in the high order word,
    // so we create a proper bignum and return it instead.
    //
    //

    BIGNUM BigNum, BigNumFinal;
    ULONG Words, WordIndex, Carry;
    ULONG64 Product;

    if (*BigNumB > *BigNumA) {
        BigNum = BigNumA;
        BigNumA = BigNumB;
        BigNumB = BigNum;
    }

    Words = *BigNumA + 1;
    BigNum = Pool_Alloc(pool, sizeof(ULONG) * (Words + 1));
    if (! BigNum)
        return NULL;
    *BigNum = Words;

    for (WordIndex = 1; WordIndex <= Words; ++WordIndex) {
        if (WordIndex <= *BigNumB)
            BigNum[WordIndex] = BigNumB[WordIndex];
        else
            BigNum[WordIndex] = 0;
    }

    Carry = 0;
    for (WordIndex = 1; WordIndex <= *BigNumA; ++WordIndex) {
        Product = BigNum[WordIndex];
        Product += BigNumA[WordIndex];
        Product += Carry;
        BigNum[WordIndex] = (ULONG)Product;
        Carry = (ULONG)(Product >> 32);
    }
    BigNum[WordIndex] += Carry;

    BigNumFinal = BigNum_CreateFromBigNum(pool, BigNum);
    BigNum_Free(BigNum);
    return BigNumFinal;
}


#endif BIGNUM_WITH_ADD


//---------------------------------------------------------------------------
// BigNum_Subtract
//---------------------------------------------------------------------------


#ifdef BIGNUM_WITH_SUBTRACT


_FX BIGNUM BigNum_Subtract(POOL *pool, BIGNUM BigNumA, BIGNUM BigNumB)
{
    //
    // subtract a bignum from a bignum.  if the first bignum is smaller
    // than the second bignum, return a zero result.  otherwise allocate
    // a result bignum with space for one more word than the first bignum.
    // then we fill the result with the contents of the second, smaller
    // bignum.  then we subtract each digit of the result from each digit
    // of the larger bignum, and place the result into the result space.
    //
    // the result bignum will likely have zero in the high order word,
    // so we create a proper bignum and return it instead.
    //
    //

    BIGNUM BigNum, BigNumFinal;
    ULONG Words, WordIndex, Carry;
    LONG64 Product;

    if (BigNum_Compare(BigNumA, BigNumB) <= 0)
        return BigNum_CreateFromInteger(pool, 0);

    Words = *BigNumA + 1;
    BigNum = Pool_Alloc(pool, sizeof(ULONG) * (Words + 1));
    if (! BigNum)
        return NULL;
    *BigNum = Words;

    for (WordIndex = 1; WordIndex <= Words; ++WordIndex) {
        if (WordIndex <= *BigNumB)
            BigNum[WordIndex] = BigNumB[WordIndex];
        else
            BigNum[WordIndex] = 0;
    }

    Carry = 0;
    for (WordIndex = 1; WordIndex <= *BigNumA; ++WordIndex) {
        Product = BigNumA[WordIndex];
        Product -= BigNum[WordIndex];
        if (Carry) {
            --Product;
            Carry = 0;
        }
        BigNum[WordIndex] = (ULONG)Product;
        if (Product < 0)
            Carry = 1;
    }

    BigNumFinal = BigNum_CreateFromBigNum(pool, BigNum);
    BigNum_Free(BigNum);
    return BigNumFinal;
}


#endif BIGNUM_WITH_SUBTRACT


//---------------------------------------------------------------------------
// BigNum_Multiply
//---------------------------------------------------------------------------


_FX BIGNUM BigNum_Multiply(POOL *pool, BIGNUM BigNumA, BIGNUM BigNumB)
{
    //
    // multiplication of bignum by bignum.  implemented by repeatedly
    // multiplying each word from the first bignum by every word of
    // the second bignum.  the product is shift according to the loop
    // index, and placed into a result bignum, which has as many words
    // as both input bignums.  note that the product is shifted.
    //
    // the result bignum will likely have zero in high order words,
    // so we create a proper bignum and return it instead.
    //
    // each digit in the example below represents a single bignum word:
    //
    //    1234 <-- 4 bignum words
    // *    56 <-- 2 words
    //    ----
    //    7404 <-- first iteration, no shift
    // + 6170  <-- shifted by one bignum word
    //   -----
    //   69104 <-- result cannot be more than 4+2 = 6 words
    //
    //

    BIGNUM BigNum, BigNumFinal;
    ULONG Words, WordIndex1, WordIndex2, Carry;
    ULONG64 Product;

    Words = *BigNumA + *BigNumB;
    BigNum = Pool_Alloc(pool, sizeof(ULONG) * (Words + 1));
    if (! BigNum)
        return NULL;
    *BigNum = Words;

    for (WordIndex1 = 1; WordIndex1 <= Words; ++WordIndex1)
        BigNum[WordIndex1] = 0;

    for (WordIndex1 = 1; WordIndex1 <= *BigNumA; ++WordIndex1) {
        Carry = 0;
        for (WordIndex2 = 1; WordIndex2 <= *BigNumB; ++WordIndex2) {
            Product = (ULONG64)BigNumA[WordIndex1];
            Product *= BigNumB[WordIndex2];
            Product += BigNum[WordIndex1 + WordIndex2 - 1];
            Product += Carry;
            BigNum[WordIndex1 + WordIndex2 - 1] = (ULONG)Product;
            Carry = (ULONG)(Product >> 32);
        }
        BigNum[WordIndex1 + WordIndex2 - 1] += Carry;
    }

    BigNumFinal = BigNum_CreateFromBigNum(pool, BigNum);
    BigNum_Free(BigNum);
    return BigNumFinal;
}


//---------------------------------------------------------------------------
// BigNum_DivideByInteger
//---------------------------------------------------------------------------


_FX BIGNUM BigNum_DivideByInteger(
            POOL *pool, BIGNUM BigNumA, ULONG Divisor, ULONG *pReminder)
{
    //
    // long division of bignum by integer.  implemented by repeatedly
    // dividing a 64-bit dividend, by the divisor.  The 64-bit dividend
    // is composed of a pair of 32-bit bignum words, initially the two
    // highest order bignum words.  after each step, the quotient goes
    // into the result, and the reminder is used for the high 32-bits
    // of the 64-bit dividend.  the low 32-bits are taken from the next
    // bignum word.
    //
    // every two digits in the example below represent a single bignum word:
    //
    //          12345678 / 43 = 287108 r 34
    //
    // step 1.  1234     / 43 = 28 r 30
    //          12 and 34 are the first two words in the bignum
    //          28 is collcted into the bignum result
    //
    // step 2.  3056     / 43 = 71 r 3
    //          30 is the reminder from step 1.  56 is the next word
    //          71 is collcted into the bignum result
    //
    // step 3.  0378     / 43 = 08 r 34
    //          03 is the reminder from step 2.  78 is the next word
    //          08 is collcted into the bignum result
    //
    // result.  12345678 / 43 = 287108 r 34
    //

    BIGNUM QuotientResult       = NULL;
    BIGNUM BigNumQ1, BigNumQ2;
    ULONG WordIndex, QuotientLow, QuotientHigh;
    ULONG64 Dividend, Quotient, Reminder;

    BigNumQ1 = BigNum_CreateFromInteger(pool, 0);
    BigNum_Assert(BigNumQ1);

    WordIndex = *BigNumA;
    while (WordIndex > 1 && BigNumA[WordIndex] == 0)
        --WordIndex;
    Dividend = (ULONG64)BigNumA[WordIndex];

    while (1) {

        if (WordIndex > 1) {
            Dividend <<= 32;
            --WordIndex;
            Dividend |= BigNumA[WordIndex];
        }

        Quotient = Dividend / Divisor;
        Reminder = Dividend % Divisor;

        QuotientHigh = (ULONG)(Quotient >> 32);
        QuotientLow  = (ULONG)(Quotient & (ULONG)-1);

        if (QuotientHigh) {
            BigNumQ2 = BigNum_ShiftLeft(pool, BigNumQ1, 32, QuotientHigh);
            BigNum_Assert(BigNumQ2);
            BigNum_FreeAndSetTo(BigNumQ1, BigNumQ2);
        }
        BigNumQ2 = BigNum_ShiftLeft(pool, BigNumQ1, 32, QuotientLow);
        BigNum_Assert(BigNumQ2);
        BigNum_FreeAndSetTo(BigNumQ1, BigNumQ2);

        if (WordIndex == 1)
            break;
        Dividend = Reminder;
    }

    if (pReminder)
        *pReminder = (ULONG)Reminder;

    QuotientResult = BigNumQ1;
    BigNumQ1 = NULL;

finish:

    if (BigNumQ1)
        BigNum_Free(BigNumQ1);

    return QuotientResult;
}


//---------------------------------------------------------------------------
// BigNum_DivideHelper
//---------------------------------------------------------------------------


_FX void BigNum_DivideHelper(
                BIGNUM Dividend, BIGNUM Divisor, BIGNUM Quotient)
{
    //
    // long division of a bignum by bignum based on public domain source
    // code which is based on Knuth.  this function should not be invoked
    // directly as it has strict requirements for its parameters.  see the
    // main divide function.
    //
    // the main loop runs j over the n highest words of the dividend,
    // where n is the number of words in the divisor.  in each step, the
    // two words at position j,j-1 from the dividend are divided by the
    // highest word of the divisor to produce q^ and r^, estimats of the
    // number of times the divisor fits in that part of the dividend.
    //
    // q^ and r^ are then further refined by inspecting the three dividend
    // words j,j-1,j-2 against the two highest divisor words.  the refined
    // q^ can be either correct or just one too high.
    //
    // the multiple of q^ * divisor is then subtracted from the n highest
    // words of the dividend starting at position j.  if this produces
    // negative result in the highest word, then q^ was estimated one too
    // high, and one value of divisor is added back to the n highest words
    // of the dividend starting at position j.
    //
    // the (possibly adjusted) q^ is stored into the quotient space.
    //
    // on return, the spaces pointed to by the Quotient and Dividend
    // parameters contain the quotient and reminder, respectively.  the
    // Divisor parameter is not modified.
    //

    ULONG DividendWords, DivisorWords, DivisorH1, DivisorH2;
    ULONG i, j, k;
    ULONG64 p, qhat, rhat, limit;
    LONG64 t;

    DividendWords = *Dividend - 1;
    DivisorWords = *Divisor;

    ++Dividend;
    ++Divisor;
    ++Quotient;

    DivisorH1 = Divisor[DivisorWords - 1];
    DivisorH2 = Divisor[DivisorWords - 2];

    for (j = DividendWords - DivisorWords; j != -1; --j) {

        p = Dividend[j + DivisorWords];
        p = (p << 32) | Dividend[j + DivisorWords - 1];

        qhat = p / DivisorH1;
        rhat = p % DivisorH1;

        while (1) {
            limit = (rhat << 32) | Dividend[j + DivisorWords - 2];
            if (qhat >= ((ULONG64)1 << 32) || qhat * DivisorH2 > limit) {
                --qhat;
                rhat += DivisorH1;
                if (rhat < ((ULONG64)1 << 32))
                    continue;
            }
            break;
        }

        k = 0;
        for (i = 0; i < DivisorWords; i++) {
            p = qhat * Divisor[i];
            t = Dividend[i + j];
            t = t - k - (ULONG)p;
            Dividend[i + j] = (ULONG)t;
            k = (ULONG)(p >> 32) - (ULONG)(t >> 32);
        }
        t = Dividend[i + j] - k;
        Dividend[i + j] = (ULONG)t;

        if (t < 0) {

            --qhat;

            k = 0;
            for (i = 0; i < DivisorWords; i++) {
                t = Dividend[i + j];
                t = t + Divisor[i] - k;
                Dividend[i + j] = (ULONG)t;
                k = (ULONG)(t >> 32);
            }
            Dividend[i + j] += k;
        }

        Quotient[j] = (ULONG)qhat;
    }
}


//---------------------------------------------------------------------------
// BigNum_Divide
//---------------------------------------------------------------------------


_FX BIGNUM BigNum_Divide(
            POOL *pool, BIGNUM Dividend, BIGNUM Divisor, BIGNUM *pReminder)
{
    //
    // long division of bignum by bignum.  if the divisor is one word
    // long, calls division by integer.  if the divisor is equal to or
    // larger than the dividend, returns early.  otherwise this function
    // sets up the data as requird by the Knuth long division algorithm,
    // implemented by the divide helper function.  the requirements are:
    //
    // - divisor must be left-shifted such that its highest bit is on
    // - dividend must be left-shifted by the same amount, to ensure the
    // final quotient is correct
    // - the adjusted dividend must have one more high order word than
    // the original dividend.  if the shift operation did not satisfy
    // this requirement (i.e. shift was too small), then do it explicitly
    // - allocate space for quotient, number of words is the difference
    // between words in the dividend and words in the divisor, plus 1.
    //
    // this function then calls the divide helper function.  on return,
    // it is only necessary to right-shift the remainder.
    //

    ULONG Index, x, DivisorShift, Words;
    BIGNUM Divisor2, Dividend2, Quotient, Temp;

    //

    if (pReminder)
        *pReminder = NULL;

    x = BigNum_Compare(Divisor, Dividend);

    if (x == 1) {

        Quotient = BigNum_CreateFromInteger(pool, 0);
        if (Quotient && pReminder) {
            Temp = BigNum_CreateFromBigNum(pool, Dividend);
            if (Temp)
                *pReminder = Temp;
            else
                { BigNum_FreeAndSetNull(Quotient); }
        }
        return Quotient;

    } else if (x == 0) {

        Quotient = BigNum_CreateFromInteger(pool, 1);
        if (Quotient && pReminder) {
            Temp = BigNum_CreateFromInteger(pool, 0);
            if (Temp)
                *pReminder = Temp;
            else
                { BigNum_FreeAndSetNull(Quotient); }
        }
        return Quotient;
    }

    //

    Index = *Divisor;
    while (Index > 1 && Divisor[Index] == 0)
        --Index;

    if (Index == 1) {

        Quotient = BigNum_DivideByInteger(pool, Dividend, Divisor[1], &x);
        if (Quotient && pReminder) {
            Temp = BigNum_CreateFromInteger(pool, x);
            if (Temp)
                *pReminder = Temp;
            else
                { BigNum_FreeAndSetNull(Quotient); }
        }
        return Quotient;
    }

    //

    Temp = NULL;

    Dividend2 = NULL;

    Quotient = NULL;

    //

    x = ((ULONG)-1 >> 31) << 31;
    for (DivisorShift = 0; (Divisor[Index] & x) == 0; ++DivisorShift)
        x >>= 1;

    Divisor2 = BigNum_ShiftLeft(pool, Divisor, DivisorShift, 0);
    BigNum_Assert(Divisor2);

    //

    Temp = BigNum_ShiftLeft(pool, Dividend, DivisorShift, 0);
    BigNum_Assert(Temp);

    if (*Temp != *Dividend) {

        Dividend2 = Temp;

    } else {

        Words = *Temp + 1;
        Dividend2 = Pool_Alloc(pool, sizeof(ULONG) * (Words + 1));
        BigNum_Assert(Dividend2);
        *Dividend2 = Words;
        for (Index = 1; Index < Words; ++Index)
            Dividend2[Index] = Temp[Index];
        Dividend2[Index] = 0;
        BigNum_Free(Temp);
    }

    Temp = NULL;

    //

    Words = *Dividend - *Divisor2 + 1;
    Quotient = Pool_Alloc(pool, sizeof(ULONG) * (Words + 1));
    BigNum_Assert(Quotient);
    *Quotient = Words;

    //

    BigNum_DivideHelper(Dividend2, Divisor2, Quotient);

    Temp = BigNum_CreateFromBigNum(pool, Quotient);
    BigNum_FreeAndSetTo(Quotient, Temp);

    if (Quotient && pReminder) {
        Temp = BigNum_ShiftRight(pool, Dividend2, DivisorShift);
        if (Temp)
            *pReminder = Temp;
        else
            { BigNum_FreeAndSetNull(Quotient); }
    }

    Temp = NULL;

    //

finish:

    if (Temp)
        BigNum_Free(Temp);

    if (Dividend2)
        BigNum_Free(Dividend2);

    if (Divisor2)
        BigNum_Free(Divisor2);

    return Quotient;
}


//---------------------------------------------------------------------------
// BigNum_ModPow
//---------------------------------------------------------------------------


_FX BIGNUM BigNum_ModPow(
            POOL *pool, BIGNUM Base, BIGNUM Exponent, BIGNUM Modulus)
{
    //
    // modular exponentiation, i.e. raising bignum Base by power Exponent,
    // dividing the result by Modulous, and returning the reminder.
    // implemented using the exponentiation by squaring method which is
    // documented everywhere.  see Wikipedia.
    //

    BIGNUM Exp                  = NULL;
    BIGNUM Result               = NULL;
    BIGNUM Bas, Temp, DivTemp;

    Bas = BigNum_CreateFromBigNum(pool, Base);
    BigNum_Assert(Bas);

    Exp = BigNum_CreateFromBigNum(pool, Exponent);
    BigNum_Assert(Exp);

    Result = BigNum_CreateFromInteger(pool, 1);
    BigNum_Assert(Result);

    while (! BigNum_IsZero(Exp)) {

        if (Exp[1] & 1) {
            Temp = BigNum_Multiply(pool, Result, Bas);
            BigNum_Assert(Temp);
            BigNum_FreeAndSetNull(Result);
            DivTemp = BigNum_Divide(pool, Temp, Modulus, &Result);
            BigNum_Free(Temp);
            if (! DivTemp)
                goto finish;
            BigNum_Free(DivTemp);
        }

        Temp = BigNum_ShiftRight(pool, Exp, 1);
        BigNum_Assert(Temp);
        BigNum_FreeAndSetTo(Exp, Temp);

        Temp = BigNum_Multiply(pool, Bas, Bas);
        BigNum_Assert(Temp);

        BigNum_FreeAndSetNull(Bas);
        DivTemp = BigNum_Divide(pool, Temp, Modulus, &Bas);
        BigNum_Free(Temp);
        if (! DivTemp)
            goto finish;
        BigNum_Free(DivTemp);
    }

finish:

    if (Exp)
        BigNum_Free(Exp);
    if (Bas)
        BigNum_Free(Bas);

    return Result;
}


//---------------------------------------------------------------------------
// BigNum_CreateFromString
//---------------------------------------------------------------------------


#ifdef BIGNUM_WITH_FROM_STRING


_FX BIGNUM BigNum_CreateFromString(POOL *pool, const WCHAR *Value, int Base)
{
    //
    // create a bignum from the specified string which contains digits
    // in the specified number base.  inefficient yet simple logic which
    // multiplies the working result by number base before adding the next
    // input digit.
    //

    BIGNUM BigNumBase           = NULL;
    BIGNUM BigNumResult         = NULL;
    BIGNUM Temp1                = NULL;
    BIGNUM Temp2                = NULL;
    const WCHAR *ValuePtr;
    ULONG Digit;

    BigNumBase = BigNum_CreateFromInteger(pool, Base);
    BigNum_Assert(BigNumBase);

    BigNumResult = BigNum_CreateFromInteger(pool, 0);
    BigNum_Assert(BigNumResult);

    for (ValuePtr = Value; *ValuePtr; ++ValuePtr) {

        if (*ValuePtr >= L'0' && *ValuePtr <= L'9')
            Digit = *ValuePtr - L'0';
        else if (*ValuePtr >= L'A' && *ValuePtr <= L'Z')
            Digit = *ValuePtr - L'A' + 10;
        else if (*ValuePtr >= L'a' && *ValuePtr <= L'z')
            Digit = *ValuePtr - L'a' + 10;
        else
            Digit = 0;

        Temp1 = BigNum_Multiply(pool, BigNumResult, BigNumBase);
        BigNum_Assert(Temp1);
        Temp2 = BigNum_CreateFromInteger(pool, Digit);
        BigNum_Assert(Temp2);

        BigNum_Free(BigNumResult);
        BigNumResult = BigNum_Add(pool, Temp1, Temp2);
        BigNum_Assert(BigNumResult);

        BigNum_FreeAndSetNull(Temp2);
        BigNum_FreeAndSetNull(Temp1);
    }

finish:

    if (Temp2)
        BigNum_Free(Temp2);
    if (Temp1)
        BigNum_Free(Temp1);
    if (BigNumBase)
        BigNum_Free(BigNumBase);

    return BigNumResult;
}


#endif BIGNUM_WITH_FROM_STRING


//---------------------------------------------------------------------------
// BigNum_ConvertToString
//---------------------------------------------------------------------------


#ifdef BIGNUM_WITH_TO_STRING


_FX WCHAR *BigNum_ConvertToString(POOL *pool, BIGNUM BigNum, int Base)
{
    //
    // converts a bignum to string form in the specified number base.
    // simple logic which works by multiplying the input bignum by number
    // base before and storing the reminder as the next output digit.
    //
    // the output string should be freed using
    //      Pool_Free(string, sizeof(WCHAR) * (wcslen(string) + 1))
    //

    BIGNUM Temp1                = NULL;
    WCHAR *String               = NULL;
    WCHAR *StringResult         = NULL;
    BIGNUM Temp2;
    WCHAR *StringPtr;
    ULONG Digits, Digit;

    Digits = 0;

    Temp1 = BigNum_CreateFromBigNum(pool, BigNum);
    BigNum_Assert(Temp1);
    while (! BigNum_IsZero(Temp1)) {
        Temp2 = BigNum_DivideByInteger(pool, Temp1, Base, NULL);
        BigNum_Assert(Temp2);
        BigNum_FreeAndSetTo(Temp1, Temp2);
        ++Digits;
    }
    BigNum_FreeAndSetNull(Temp1);

    if (Digits == 0) {
        String = Pool_Alloc(pool, sizeof(WCHAR) * 2);
        if (String) {
            String[0] = L'0';
            String[1] = L'\0';
        }
        return String;
    }

    String = Pool_Alloc(pool, sizeof(WCHAR) * (Digits + 1));
    if (! String)
        return NULL;
    StringPtr = String + Digits;
    *StringPtr = L'\0';
    --StringPtr;

    Temp1 = BigNum_CreateFromBigNum(pool, BigNum);
    BigNum_Assert(Temp1);
    while (! BigNum_IsZero(Temp1)) {
        Temp2 = BigNum_DivideByInteger(pool, Temp1, Base, &Digit);
        BigNum_Assert(Temp2);
        BigNum_FreeAndSetTo(Temp1, Temp2);

        if (Digit <= 9)
            *StringPtr = (WCHAR)(Digit + L'0');
        else
            *StringPtr = (WCHAR)((Digit - 10) + L'A');
        --StringPtr;
    }

    StringResult = String;
    String = NULL;

finish:

    if (Temp1)
        BigNum_Free(Temp1);

    if (String)
        Pool_Free(String, sizeof(WCHAR) * (Digits + 1));

    return StringResult;
}


#endif BIGNUM_WITH_TO_STRING


//---------------------------------------------------------------------------
// BigNum_CreateRandom
//---------------------------------------------------------------------------


#ifdef BIGNUM_WITH_RANDOM


_FX BIGNUM BigNum_CreateRandom(POOL *pool, int Digits, int Base)
{
    //
    // creates a psuedo-random bignum of the specified number of digits
    // in the specified base.  inefficient yet simple logic which
    // multiplies the working result by number base before adding
    // the next random digit.
    //

    BIGNUM BigNumBase           = NULL;
    BIGNUM BigNumResult         = NULL;
    BIGNUM Temp1                = NULL;
    BIGNUM Temp2                = NULL;

    BigNumBase = BigNum_CreateFromInteger(pool, Base);
    BigNum_Assert(BigNumBase);

    BigNumResult = BigNum_CreateFromInteger(pool, 0);
    BigNum_Assert(BigNumResult);

    for (; Digits; --Digits) {

        Temp1 = BigNum_Multiply(pool, BigNumResult, BigNumBase);
        BigNum_Assert(Temp1);
        Temp2 = BigNum_CreateFromInteger(pool, rand() % Base);
        BigNum_Assert(Temp2);

        BigNum_Free(BigNumResult);
        BigNumResult = BigNum_Add(pool, Temp1, Temp2);
        BigNum_Assert(BigNumResult);

        BigNum_FreeAndSetNull(Temp2);
        BigNum_FreeAndSetNull(Temp1);
    }

    //

finish:

    if (Temp2)
        BigNum_Free(Temp2);
    if (Temp1)
        BigNum_Free(Temp1);
    if (BigNumBase)
        BigNum_Free(BigNumBase);

    return BigNumResult;
}


#endif BIGNUM_WITH_RANDOM
