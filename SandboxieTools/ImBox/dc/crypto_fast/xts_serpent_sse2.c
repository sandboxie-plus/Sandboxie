/*
    *
    * Copyright (c) 2010-2011
    * ntldr <ntldr@diskcryptor.net> PGP key ID - 0x1B6A24550F33E44A
    *

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if defined(USE_AVX) && !defined(__INTEL_COMPILER)
 #error Please use Intel C++ Compoler
#endif
#include <intrin.h>
#include "serpent.h"
#include "xts_fast.h"
#ifdef USE_AVX
 #include <immintrin.h>
 #include "xts_serpent_avx.h"
#else
 #include <emmintrin.h>
 #include "xts_serpent_sse2.h"
#endif

#define transpose(_B0, _B1, _B2, _B3) {         \
	__m128i _T0 = _mm_unpacklo_epi32(_B0, _B1); \
    __m128i _T1 = _mm_unpacklo_epi32(_B2, _B3); \
	__m128i _T2 = _mm_unpackhi_epi32(_B0, _B1); \
    __m128i _T3 = _mm_unpackhi_epi32(_B2, _B3); \
	_B0 = _mm_unpacklo_epi64(_T0, _T1);         \
    _B1 = _mm_unpackhi_epi64(_T0, _T1);         \
    _B2 = _mm_unpacklo_epi64(_T2, _T3);         \
    _B3 = _mm_unpackhi_epi64(_T2, _T3);         \
}

#define KXf(_B0, _B1, _B2, _B3, _ctx, round)                           \
	_B0 = _mm_xor_si128(_B0, _mm_set1_epi32((_ctx)->expkey[4*round  ])); \
	_B1 = _mm_xor_si128(_B1, _mm_set1_epi32((_ctx)->expkey[4*round+1])); \
	_B2 = _mm_xor_si128(_B2, _mm_set1_epi32((_ctx)->expkey[4*round+2])); \
	_B3 = _mm_xor_si128(_B3, _mm_set1_epi32((_ctx)->expkey[4*round+3]));

#define NOT_SI128(_X) ( \
	_mm_xor_si128(_X, _mm_set1_epi32(0xFFFFFFFF)) )
	
#define ROL_SI128(_X, _rot) ( \
	_mm_or_si128(_mm_slli_epi32(_X, _rot), _mm_srli_epi32(_X, 32-_rot)) )

#define ROR_SI128(_X, _rot) ( ROL_SI128(_X, (32-_rot)) )

#define LTf(_B0, _B1, _B2, _B3)                       \
	_B0 = ROL_SI128(_B0, 13);                         \
	_B2 = ROL_SI128(_B2, 3);                          \
	_B1 = _mm_xor_si128(_B1, _B0);                    \
	_B1 = _mm_xor_si128(_B1, _B2);                    \
	_B3 = _mm_xor_si128(_B3, _B2);                    \
	_B3 = _mm_xor_si128(_B3, _mm_slli_epi32(_B0, 3)); \
	_B1 = ROL_SI128(_B1, 1);                          \
	_B3 = ROL_SI128(_B3, 7);                          \
	_B0 = _mm_xor_si128(_B0, _B1);                    \
	_B0 = _mm_xor_si128(_B0, _B3);                    \
	_B2 = _mm_xor_si128(_B2, _B3);                    \
	_B2 = _mm_xor_si128(_B2, _mm_slli_epi32(_B1, 7)); \
	_B0 = ROL_SI128(_B0, 5);                          \
	_B2 = ROL_SI128(_B2, 22);

#define ITf(_B0, _B1, _B2, _B3)                       \
	_B2 = ROR_SI128(_B2, 22);                         \
	_B0 = ROR_SI128(_B0, 5);                          \
	_B2 = _mm_xor_si128(_B2, _B3);                    \
	_B2 = _mm_xor_si128(_B2, _mm_slli_epi32(_B1, 7)); \
	_B0 = _mm_xor_si128(_B0, _B1);                    \
	_B0 = _mm_xor_si128(_B0, _B3);                    \
	_B3 = ROR_SI128(_B3, 7);                          \
	_B1 = ROR_SI128(_B1, 1);                          \
	_B3 = _mm_xor_si128(_B3, _B2);                    \
	_B3 = _mm_xor_si128(_B3, _mm_slli_epi32(_B0, 3)); \
	_B1 = _mm_xor_si128(_B1, _B0);                    \
	_B1 = _mm_xor_si128(_B1, _B2);                    \
	_B2 = ROR_SI128(_B2, 3);                          \
	_B0 = ROR_SI128(_B0, 13);

#define sE1(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B1;             \
	_B3 = _mm_xor_si128(_B3, _B0); \
    _B1 = _mm_and_si128(_B1, _B3); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B1 = _mm_xor_si128(_B1, _B0); \
	_B0 = _mm_or_si128(_B0, _B3);  \
	_B0 = _mm_xor_si128(_B0, _tt); \
	_tt = _mm_xor_si128(_tt, _B3); \
	_B3 = _mm_xor_si128(_B3, _B2); \
	_B2 = _mm_or_si128(_B2, _B1);  \
	_B2 = _mm_xor_si128(_B2, _tt); \
	_tt = NOT_SI128(_tt);          \
	_tt = _mm_or_si128(_tt, _B1);  \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B1 = _mm_xor_si128(_B1, _tt); \
	_B3 = _mm_or_si128(_B3, _B0);  \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_tt = _mm_xor_si128(_tt, _B3); \
	_B3 = _B0; \
	_B0 = _B1; \
	_B1 = _tt; \
}

#define sE2(_B0, _B1, _B2, _B3) {  \
	__m128i _tt;                   \
	_B0 = NOT_SI128(_B0);          \
	_B2 = NOT_SI128(_B2);          \
    _tt = _B0;                     \
	_B0 = _mm_and_si128(_B0, _B1); \
	_B2 = _mm_xor_si128(_B2, _B0); \
	_B0 = _mm_or_si128(_B0, _B3);  \
	_B3 = _mm_xor_si128(_B3, _B2); \
	_B1 = _mm_xor_si128(_B1, _B0); \
	_B0 = _mm_xor_si128(_B0, _tt); \
	_tt = _mm_or_si128(_tt, _B1);  \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B2 = _mm_or_si128(_B2, _B0);  \
	_B2 = _mm_and_si128(_B2, _tt); \
	_B0 = _mm_xor_si128(_B0, _B1); \
	_B1 = _mm_and_si128(_B1, _B2); \
	_B1 = _mm_xor_si128(_B1, _B0); \
	_B0 = _mm_and_si128(_B0, _B2); \
	_tt = _mm_xor_si128(_tt, _B0); \
	_B0 = _B2; \
	_B2 = _B3; \
	_B3 = _B1; \
	_B1 = _tt; \
}

#define sE3(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B0;             \
	_B0 = _mm_and_si128(_B0, _B2); \
	_B0 = _mm_xor_si128(_B0, _B3); \
	_B2 = _mm_xor_si128(_B2, _B1); \
	_B2 = _mm_xor_si128(_B2, _B0); \
	_B3 = _mm_or_si128(_B3, _tt);  \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B1 = _B3;                     \
	_B3 = _mm_or_si128(_B3, _tt);  \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_B0 = _mm_and_si128(_B0, _B1); \
	_tt = _mm_xor_si128(_tt, _B0); \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B1 = _mm_xor_si128(_B1, _tt); \
	_B0 = _B2;                     \
	_B2 = _B1;                     \
	_B1 = _B3;                     \
	_B3 = NOT_SI128(_tt);          \
}

#define sE4(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B0;             \
	_B0 = _mm_or_si128(_B0, _B3);  \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_B1 = _mm_and_si128(_B1, _tt); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B2 = _mm_xor_si128(_B2, _B3); \
	_B3 = _mm_and_si128(_B3, _B0); \
	_tt = _mm_or_si128(_tt, _B1);  \
	_B3 = _mm_xor_si128(_B3, _tt); \
	_B0 = _mm_xor_si128(_B0, _B1); \
	_tt = _mm_and_si128(_tt, _B0); \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B1 = _mm_or_si128(_B1, _B0);  \
	_B1 = _mm_xor_si128(_B1, _B2); \
	_B0 = _mm_xor_si128(_B0, _B3); \
	_B2 = _B1;                     \
	_B1 = _mm_or_si128(_B1, _B3);  \
	_B0 = _mm_xor_si128(_B0, _B1); \
	_B1 = _B2; \
	_B2 = _B3; \
	_B3 = _tt; \
}

#define sE5(_B0, _B1, _B2, _B3) {  \
	__m128i _tt;                   \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B3 = NOT_SI128(_B3);          \
	_B2 = _mm_xor_si128(_B2, _B3); \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_tt = _B1;                     \
	_B1 = _mm_and_si128(_B1, _B3); \
	_B1 = _mm_xor_si128(_B1, _B2); \
	_tt = _mm_xor_si128(_tt, _B3); \
	_B0 = _mm_xor_si128(_B0, _tt); \
	_B2 = _mm_and_si128(_B2, _tt); \
	_B2 = _mm_xor_si128(_B2, _B0); \
	_B0 = _mm_and_si128(_B0, _B1); \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_tt = _mm_or_si128(_tt, _B1);  \
	_tt = _mm_xor_si128(_tt, _B0); \
	_B0 = _mm_or_si128(_B0, _B3);  \
	_B0 = _mm_xor_si128(_B0, _B2); \
	_B2 = _mm_and_si128(_B2, _B3); \
	_B0 = NOT_SI128(_B0);          \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B2 = _B0; \
	_B0 = _B1; \
	_B1 = _tt; \
}

#define sE6(_B0, _B1, _B2, _B3) {  \
	__m128i _tt;                   \
	_B0 = _mm_xor_si128(_B0, _B1); \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B3 = NOT_SI128(_B3);          \
	_tt = _B1;                     \
	_B1 = _mm_and_si128(_B1, _B0); \
	_B2 = _mm_xor_si128(_B2, _B3); \
	_B1 = _mm_xor_si128(_B1, _B2); \
	_B2 = _mm_or_si128(_B2, _tt);  \
	_tt = _mm_xor_si128(_tt, _B3); \
	_B3 = _mm_and_si128(_B3, _B1); \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_tt = _mm_xor_si128(_tt, _B1); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B2 = _mm_xor_si128(_B2, _B0); \
	_B0 = _mm_and_si128(_B0, _B3); \
	_B2 = NOT_SI128(_B2);          \
	_B0 = _mm_xor_si128(_B0, _tt); \
	_tt = _mm_or_si128(_tt, _B3);  \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B2 = _B0; \
	_B0 = _B1; \
	_B1 = _B3; \
	_B3 = _tt; \
}

#define sE7(_B0, _B1, _B2, _B3) {  \
	__m128i _tt;                   \
	_B2 = NOT_SI128(_B2);          \
	_tt = _B3;                     \
	_B3 = _mm_and_si128(_B3, _B0); \
	_B0 = _mm_xor_si128(_B0, _tt); \
	_B3 = _mm_xor_si128(_B3, _B2); \
	_B2 = _mm_or_si128(_B2, _tt);  \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B2 = _mm_xor_si128(_B2, _B0); \
	_B0 = _mm_or_si128(_B0, _B1);  \
	_B2 = _mm_xor_si128(_B2, _B1); \
	_tt = _mm_xor_si128(_tt, _B0); \
	_B0 = _mm_or_si128(_B0, _B3);  \
	_B0 = _mm_xor_si128(_B0, _B2); \
	_tt = _mm_xor_si128(_tt, _B3); \
	_tt = _mm_xor_si128(_tt, _B0); \
	_B3 = NOT_SI128(_B3);          \
	_B2 = _mm_and_si128(_B2, _tt); \
	_B3 = _mm_xor_si128(_B3, _B2); \
	_B2 = _tt;                     \
}

#define sE8(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B1;             \
	_B1 = _mm_or_si128(_B1, _B2);  \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B2 = _mm_xor_si128(_B2, _B1); \
	_B3 = _mm_or_si128(_B3, _tt);  \
	_B3 = _mm_and_si128(_B3, _B0); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_B1 = _mm_or_si128(_B1, _tt);  \
	_B1 = _mm_xor_si128(_B1, _B0); \
	_B0 = _mm_or_si128(_B0, _tt);  \
	_B0 = _mm_xor_si128(_B0, _B2); \
	_B1 = _mm_xor_si128(_B1, _tt); \
	_B2 = _mm_xor_si128(_B2, _B1); \
	_B1 = _mm_and_si128(_B1, _B0); \
	_B1 = _mm_xor_si128(_B1, _tt); \
	_B2 = NOT_SI128(_B2);          \
	_B2 = _mm_or_si128(_B2, _B0);  \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B2 = _B1; \
	_B1 = _B3; \
	_B3 = _B0; \
	_B0 = _tt; \
}

#define sD1(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B1;             \
	_B2 = NOT_SI128(_B2);          \
	_B1 = _mm_or_si128(_B1, _B0);  \
	_tt = NOT_SI128(_tt);          \
	_B1 = _mm_xor_si128(_B1, _B2); \
	_B2 = _mm_or_si128(_B2, _tt);  \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B0 = _mm_xor_si128(_B0, _tt); \
	_B2 = _mm_xor_si128(_B2, _B0); \
	_B0 = _mm_and_si128(_B0, _B3); \
	_tt = _mm_xor_si128(_tt, _B0); \
	_B0 = _mm_or_si128(_B0, _B1);  \
	_B0 = _mm_xor_si128(_B0, _B2); \
	_B3 = _mm_xor_si128(_B3, _tt); \
	_B2 = _mm_xor_si128(_B2, _B1); \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_B2 = _mm_and_si128(_B2, _B3); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B2 = _B1; \
	_B1 = _tt; \
}

#define sD2(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B1;             \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B3 = _mm_and_si128(_B3, _B1); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_B0 = _mm_or_si128(_B0, _B1);  \
	_B2 = _mm_xor_si128(_B2, _B3); \
	_B0 = _mm_xor_si128(_B0, _tt); \
	_B0 = _mm_or_si128(_B0, _B2);  \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B0 = _mm_xor_si128(_B0, _B1); \
	_B1 = _mm_or_si128(_B1, _B3);  \
	_B1 = _mm_xor_si128(_B1, _B0); \
	_tt = NOT_SI128(_tt);          \
	_tt = _mm_xor_si128(_tt, _B1); \
	_B1 = _mm_or_si128(_B1, _B0);  \
	_B1 = _mm_xor_si128(_B1, _B0); \
	_B1 = _mm_or_si128(_B1, _tt);  \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_B1 = _B0; \
	_B0 = _tt; \
	_tt = _B2; \
	_B2 = _B3; \
	_B3 = _tt; \
}

#define sD3(_B0, _B1, _B2, _B3) {  \
	__m128i _tt;                   \
	_B2 = _mm_xor_si128(_B2, _B3); \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_tt = _B3;                     \
	_B3 = _mm_and_si128(_B3, _B2); \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_B1 = _mm_or_si128(_B1, _B2);  \
	_B1 = _mm_xor_si128(_B1, _tt); \
	_tt = _mm_and_si128(_tt, _B3); \
	_B2 = _mm_xor_si128(_B2, _B3); \
	_tt = _mm_and_si128(_tt, _B0); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B2 = _mm_and_si128(_B2, _B1); \
	_B2 = _mm_or_si128(_B2, _B0);  \
	_B3 = NOT_SI128(_B3);          \
	_B2 = _mm_xor_si128(_B2, _B3); \
	_B0 = _mm_xor_si128(_B0, _B3); \
	_B0 = _mm_and_si128(_B0, _B1); \
	_B3 = _mm_xor_si128(_B3, _tt); \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_B0 = _B1; \
	_B1 = _tt; \
}

#define sD4(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B2;             \
	_B2 = _mm_xor_si128(_B2, _B1); \
	_B0 = _mm_xor_si128(_B0, _B2); \
	_tt = _mm_and_si128(_tt, _B2); \
	_tt = _mm_xor_si128(_tt, _B0); \
	_B0 = _mm_and_si128(_B0, _B1); \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B3 = _mm_or_si128(_B3, _tt);  \
	_B2 = _mm_xor_si128(_B2, _B3); \
	_B0 = _mm_xor_si128(_B0, _B3); \
	_B1 = _mm_xor_si128(_B1, _tt); \
	_B3 = _mm_and_si128(_B3, _B2); \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_B1 = _mm_xor_si128(_B1, _B0); \
	_B1 = _mm_or_si128(_B1, _B2);  \
	_B0 = _mm_xor_si128(_B0, _B3); \
	_B1 = _mm_xor_si128(_B1, _tt); \
	_B0 = _mm_xor_si128(_B0, _B1); \
	_tt = _B0; \
	_B0 = _B2; \
	_B2 = _B3; \
	_B3 = _tt; \
}

#define sD5(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B2;             \
	_B2 = _mm_and_si128(_B2, _B3); \
	_B2 = _mm_xor_si128(_B2, _B1); \
	_B1 = _mm_or_si128(_B1, _B3);  \
	_B1 = _mm_and_si128(_B1, _B0); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_tt = _mm_xor_si128(_tt, _B1); \
	_B1 = _mm_and_si128(_B1, _B2); \
	_B0 = NOT_SI128(_B0);          \
	_B3 = _mm_xor_si128(_B3, _tt); \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_B3 = _mm_and_si128(_B3, _B0); \
	_B3 = _mm_xor_si128(_B3, _B2); \
	_B0 = _mm_xor_si128(_B0, _B1); \
	_B2 = _mm_and_si128(_B2, _B0); \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_B2 = _mm_xor_si128(_B2, _tt); \
	_B2 = _mm_or_si128(_B2, _B3);  \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_B2 = _mm_xor_si128(_B2, _B1); \
	_B1 = _B3; \
	_B3 = _tt; \
}

#define sD6(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B3;             \
	_B1 = NOT_SI128(_B1);          \
	_B2 = _mm_xor_si128(_B2, _B1); \
	_B3 = _mm_or_si128(_B3, _B0);  \
	_B3 = _mm_xor_si128(_B3, _B2); \
	_B2 = _mm_or_si128(_B2, _B1);  \
	_B2 = _mm_and_si128(_B2, _B0); \
	_tt = _mm_xor_si128(_tt, _B3); \
	_B2 = _mm_xor_si128(_B2, _tt); \
	_tt = _mm_or_si128(_tt, _B0);  \
	_tt = _mm_xor_si128(_tt, _B1); \
	_B1 = _mm_and_si128(_B1, _B2); \
	_B1 = _mm_xor_si128(_B1, _B3); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B3 = _mm_and_si128(_B3, _tt); \
	_tt = _mm_xor_si128(_tt, _B1); \
	_B3 = _mm_xor_si128(_B3, _tt); \
	_tt = NOT_SI128(_tt);          \
	_B3 = _mm_xor_si128(_B3, _B0); \
	_B0 = _B1; \
	_B1 = _tt; \
	_tt = _B3; \
	_B3 = _B2; \
	_B2 = _tt; \
}

#define sD7(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B2;             \
	_B0 = _mm_xor_si128(_B0, _B2); \
	_B2 = _mm_and_si128(_B2, _B0); \
	_tt = _mm_xor_si128(_tt, _B3); \
	_B2 = NOT_SI128(_B2);          \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_B2 = _mm_xor_si128(_B2, _B3); \
	_tt = _mm_or_si128(_tt, _B0);  \
	_B0 = _mm_xor_si128(_B0, _B2); \
	_B3 = _mm_xor_si128(_B3, _tt); \
	_tt = _mm_xor_si128(_tt, _B1); \
	_B1 = _mm_and_si128(_B1, _B3); \
	_B1 = _mm_xor_si128(_B1, _B0); \
	_B0 = _mm_xor_si128(_B0, _B3); \
	_B0 = _mm_or_si128(_B0, _B2);  \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_tt = _mm_xor_si128(_tt, _B0); \
	_B0 = _B1; \
	_B1 = _B2; \
	_B2 = _tt; \
}

#define sD8(_B0, _B1, _B2, _B3) {  \
	__m128i _tt = _B2;             \
	_B2 = _mm_xor_si128(_B2, _B0); \
	_B0 = _mm_and_si128(_B0, _B3); \
	_tt = _mm_or_si128(_tt, _B3);  \
	_B2 = NOT_SI128(_B2);          \
	_B3 = _mm_xor_si128(_B3, _B1); \
	_B1 = _mm_or_si128(_B1, _B0);  \
	_B0 = _mm_xor_si128(_B0, _B2); \
	_B2 = _mm_and_si128(_B2, _tt); \
	_B3 = _mm_and_si128(_B3, _tt); \
	_B1 = _mm_xor_si128(_B1, _B2); \
	_B2 = _mm_xor_si128(_B2, _B0); \
	_B0 = _mm_or_si128(_B0, _B2);  \
	_tt = _mm_xor_si128(_tt, _B1); \
	_B0 = _mm_xor_si128(_B0, _B3); \
	_B3 = _mm_xor_si128(_B3, _tt); \
	_tt = _mm_or_si128(_tt, _B0);  \
	_B3 = _mm_xor_si128(_B3, _B2); \
	_tt = _mm_xor_si128(_tt, _B2); \
	_B2 = _B1; \
	_B1 = _B0; \
	_B0 = _B3; \
	_B3 = _tt; \
}


#define serpent256_sse2_encrypt(_B0, _B1, _B2, _B3, _ctx)                      \
	transpose(_B0,_B1,_B2,_B3);                                                \
	KXf(_B0,_B1,_B2,_B3,_ctx, 0); sE1(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 1); sE2(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 2); sE3(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 3); sE4(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 4); sE5(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 5); sE6(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 6); sE7(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 7); sE8(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 8); sE1(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 9); sE2(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3);  \
	KXf(_B0,_B1,_B2,_B3,_ctx, 10); sE3(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 11); sE4(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 12); sE5(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 13); sE6(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 14); sE7(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 15); sE8(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 16); sE1(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 17); sE2(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 18); sE3(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 19); sE4(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 20); sE5(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 21); sE6(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 22); sE7(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 23); sE8(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 24); sE1(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 25); sE2(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 26); sE3(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 27); sE4(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 28); sE5(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 29); sE6(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 30); sE7(_B0,_B1,_B2,_B3); LTf(_B0,_B1,_B2,_B3); \
	KXf(_B0,_B1,_B2,_B3,_ctx, 31); sE8(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 32); \
	transpose(_B0,_B1,_B2,_B3);

#define serpent256_sse2_decrypt(_B0, _B1, _B2, _B3, _ctx)                      \
	transpose(_B0,_B1,_B2,_B3);                                                \
	KXf(_B0,_B1,_B2,_B3,_ctx, 32); sD8(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 31); \
	ITf(_B0,_B1,_B2,_B3); sD7(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 30); \
	ITf(_B0,_B1,_B2,_B3); sD6(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 29); \
	ITf(_B0,_B1,_B2,_B3); sD5(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 28); \
	ITf(_B0,_B1,_B2,_B3); sD4(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 27); \
	ITf(_B0,_B1,_B2,_B3); sD3(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 26); \
	ITf(_B0,_B1,_B2,_B3); sD2(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 25); \
	ITf(_B0,_B1,_B2,_B3); sD1(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 24); \
	ITf(_B0,_B1,_B2,_B3); sD8(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 23); \
	ITf(_B0,_B1,_B2,_B3); sD7(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 22); \
	ITf(_B0,_B1,_B2,_B3); sD6(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 21); \
	ITf(_B0,_B1,_B2,_B3); sD5(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 20); \
	ITf(_B0,_B1,_B2,_B3); sD4(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 19); \
	ITf(_B0,_B1,_B2,_B3); sD3(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 18); \
	ITf(_B0,_B1,_B2,_B3); sD2(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 17); \
	ITf(_B0,_B1,_B2,_B3); sD1(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 16); \
	ITf(_B0,_B1,_B2,_B3); sD8(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 15); \
	ITf(_B0,_B1,_B2,_B3); sD7(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 14); \
	ITf(_B0,_B1,_B2,_B3); sD6(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 13); \
	ITf(_B0,_B1,_B2,_B3); sD5(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 12); \
	ITf(_B0,_B1,_B2,_B3); sD4(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 11); \
	ITf(_B0,_B1,_B2,_B3); sD3(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 10); \
	ITf(_B0,_B1,_B2,_B3); sD2(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 9);  \
	ITf(_B0,_B1,_B2,_B3); sD1(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 8);  \
	ITf(_B0,_B1,_B2,_B3); sD8(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 7);  \
	ITf(_B0,_B1,_B2,_B3); sD7(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 6);  \
	ITf(_B0,_B1,_B2,_B3); sD6(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 5);  \
	ITf(_B0,_B1,_B2,_B3); sD5(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 4);  \
	ITf(_B0,_B1,_B2,_B3); sD4(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 3);  \
	ITf(_B0,_B1,_B2,_B3); sD3(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 2);  \
	ITf(_B0,_B1,_B2,_B3); sD2(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 1);  \
	ITf(_B0,_B1,_B2,_B3); sD1(_B0,_B1,_B2,_B3); KXf(_B0,_B1,_B2,_B3,_ctx, 0);  \
	transpose(_B0,_B1,_B2,_B3);
	

#define sse2_next_tweak(_N, _O) {                        \
	__m128i _tt = _O;                                    \
	__m128i _t2;                                         \
	_tt = _mm_srai_epi16(_tt, 8);                        \
	_tt = _mm_srli_si128(_tt, 15);                       \
	_tt = _mm_and_si128(_tt, _mm_setr_epi32(135,0,0,0)); \
	_t2 = _O;                                            \
	_t2 = _mm_slli_si128(_t2, 8);                        \
	_t2 = _mm_srli_si128(_t2, 7);                        \
	_t2 = _mm_srli_epi64(_t2, 7);                        \
	_N = _O;                                             \
	_N = _mm_slli_epi64(_N, 1);                          \
	_N = _mm_or_si128(_N, _t2);                          \
	_N = _mm_xor_si128(_N, _tt);                         \
}

#ifdef USE_AVX
void _stdcall xts_serpent_avx_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
#else
void _stdcall xts_serpent_sse2_encrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
#endif
{
	__m128i t0, t1, t2, t3;
	__m128i b0, b1, b2, b3;
	__m128i idx;
	int     i;
	
	((unsigned __int64*)&idx)[0] = offset / XTS_SECTOR_SIZE;
	((unsigned __int64*)&idx)[1] = 0;
	do
	{
		// update tweak unit index
		((unsigned __int64*)&idx)[0]++;
		// derive first tweak value
		serpent256_encrypt((unsigned char*)&idx, (unsigned char*)&t0, &key->tweak_k.serpent);
		
		for (i = 0; i < XTS_BLOCKS_IN_SECTOR / 4; i++)
		{
			// derive t1-t3
			sse2_next_tweak(t1, t0);
			sse2_next_tweak(t2, t1);
			sse2_next_tweak(t3, t2);
			// load and tweak 4 blocks
			b0 = _mm_xor_si128(_mm_loadu_si128((const __m128i*)(in + 0 )), t0);
			b1 = _mm_xor_si128(_mm_loadu_si128((const __m128i*)(in + 16)), t1);
			b2 = _mm_xor_si128(_mm_loadu_si128((const __m128i*)(in + 32)), t2);
			b3 = _mm_xor_si128(_mm_loadu_si128((const __m128i*)(in + 48)), t3);
			// encrypt / decrypt
			serpent256_sse2_encrypt(b0, b1, b2, b3, &key->crypt_k.serpent);
			// tweak and store 4 blocks
			_mm_storeu_si128((__m128i*)(out + 0 ), _mm_xor_si128(b0, t0));
			_mm_storeu_si128((__m128i*)(out + 16), _mm_xor_si128(b1, t1));
			_mm_storeu_si128((__m128i*)(out + 32), _mm_xor_si128(b2, t2));
			_mm_storeu_si128((__m128i*)(out + 48), _mm_xor_si128(b3, t3));
			// derive next t0
			sse2_next_tweak(t0, t3);
			// update pointers
			in += XTS_BLOCK_SIZE*4; out += XTS_BLOCK_SIZE*4;
		}
	} while (len -= XTS_SECTOR_SIZE);
}

#ifdef USE_AVX
void _stdcall xts_serpent_avx_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
#else
void _stdcall xts_serpent_sse2_decrypt(const unsigned char *in, unsigned char *out, size_t len, unsigned __int64 offset, xts_key *key)
#endif
{
	__m128i t0, t1, t2, t3;
	__m128i b0, b1, b2, b3;
	__m128i idx;
	int     i;
	
	((unsigned __int64*)&idx)[0] = offset / XTS_SECTOR_SIZE;
	((unsigned __int64*)&idx)[1] = 0;
	do
	{
		// update tweak unit index
		((unsigned __int64*)&idx)[0]++;
		// derive first tweak value
		serpent256_encrypt((unsigned char*)&idx, (unsigned char*)&t0, &key->tweak_k.serpent);
		
		for (i = 0; i < XTS_BLOCKS_IN_SECTOR / 4; i++)
		{
			// derive t1-t3
			sse2_next_tweak(t1, t0);
			sse2_next_tweak(t2, t1);
			sse2_next_tweak(t3, t2);
			// load and tweak 4 blocks
			b0 = _mm_xor_si128(_mm_loadu_si128((const __m128i*)(in + 0 )), t0);
			b1 = _mm_xor_si128(_mm_loadu_si128((const __m128i*)(in + 16)), t1);
			b2 = _mm_xor_si128(_mm_loadu_si128((const __m128i*)(in + 32)), t2);
			b3 = _mm_xor_si128(_mm_loadu_si128((const __m128i*)(in + 48)), t3);
			// encrypt / decrypt
			serpent256_sse2_decrypt(b0, b1, b2, b3, &key->crypt_k.serpent);
			// tweak and store 4 blocks
			_mm_storeu_si128((__m128i*)(out + 0 ), _mm_xor_si128(b0, t0));
			_mm_storeu_si128((__m128i*)(out + 16), _mm_xor_si128(b1, t1));
			_mm_storeu_si128((__m128i*)(out + 32), _mm_xor_si128(b2, t2));
			_mm_storeu_si128((__m128i*)(out + 48), _mm_xor_si128(b3, t3));
			// derive next t0
			sse2_next_tweak(t0, t3);
			// update pointers
			in += XTS_BLOCK_SIZE*4; out += XTS_BLOCK_SIZE*4;
		}
	} while (len -= XTS_SECTOR_SIZE);
}

#ifdef USE_AVX

int _stdcall xts_serpent_avx_available() 
{
	int succs = 0;
	__asm {
		mov eax, 1
		cpuid
		and ecx, 0x18000000 // check 27 bit (OS uses XSAVE/XRSTOR)
		cmp ecx, 0x18000000 // and 28 (AVX supported by CPU)
		jne not_supported
		xor ecx, ecx        // XFEATURE_ENABLED_MASK/XCR0 register number = 0
		xgetbv              // XFEATURE_ENABLED_MASK register is in edx:eax
		and eax, 6
		cmp eax, 6          // check the AVX registers restore at context switch
		jne not_supported
		mov [succs], 1
not_supported:
	}
	return succs;
}

#else
int _stdcall xts_serpent_sse2_available()
{
	int info[4]; __cpuid(info, 1);
	return (info[3] & (1 << 26)) != 0;
}
#endif