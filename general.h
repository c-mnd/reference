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

#ifndef general_h
#define general_h

#include <stdint.h> // uint8_t ...
#include <string.h> // memset ...
#include "iotafy.h"

#if INTPTR_MAX == INT32_MAX
#define ENV32BIT
#elif INTPTR_MAX == INT64_MAX
#define ENV64BIT
#else
#error "Environment not 32 or 64-bit."
#endif

typedef int8_t trit_t;
typedef int8_t tryte_t;

#define COLUMNS 9
#define ROWS 3
#define SLICES 27
#define SLICESIZE (COLUMNS * ROWS)
#define STATESIZE (COLUMNS * ROWS * SLICES)
#define NUM_SBOXES (SLICES * ROWS * COLUMNS / 3)

#define TROIKA_ROUNDS 24
#define TROIKA_RATE 243

#define TROIKA_PADDING TROIKA_SYMBOL_ONE

#define _P(a) (a->p)
#define _N(a) (a->n)
#define _0(a) (~(a->p ^ a->n))

#endif /* general_h */
