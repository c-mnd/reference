/*
 * Copyright (c) 2019 c-mnd
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef strit_h
#define strit_h

#include <stdio.h>
#include "general.h"

#ifndef SIMD_SIZE
#define SIMD_SIZE 64
#endif


#if SIMD_SIZE == 256
#include <immintrin.h>
#define SBIT_T __m256i
#define SBIT_BASE_T long long
#define SBIT_SIZE 256
#define SBIT_BASE_SIZE 64
#define LSHIFT(x, y) _mm256_slli_epi64(x, y)
#define RSHIFT(x, y) _mm256_srli_epi64(x, y)
#define ADDR

#elif SIMD_SIZE == 128
#include <immintrin.h>
#define SBIT_T    __m128i
#define SBIT_BASE_T long long
#define SBIT_SIZE 128
#define SBIT_BASE_SIZE 64
#define LSHIFT(x, y) _mm_slli_epi64(x, y)
#define RSHIFT(x, y) _mm_srli_epi64(x, y)
#define ADDR

#elif SIMD_SIZE == 64
#define SBIT_T    uint64_t
#define SBIT_BASE_T uint64_t
#define SBIT_SIZE 64
#define SBIT_BASE_SIZE 64
#define LSHIFT(x, y) ((x)<<(y))
#define RSHIFT(x, y) ((x)>>(y))
#define ADDR &


#elif SIMD_SIZE == 32
#define SBIT_T    uint32_t
#define SBIT_BASE_T uint32_t
#define SBIT_SIZE 32
#define SBIT_BASE_SIZE 32
#define LSHIFT(x, y) ((x)<<(y))
#define RSHIFT(x, y) ((x)>>(y))
#define ADDR &

#endif

typedef struct strit_t{
    SBIT_T p;
    SBIT_T n;
} strit_t;

typedef struct strit_base_t{
    SBIT_BASE_T p;
    SBIT_BASE_T n;
} strit_base_t;

#endif /* strit_h */
