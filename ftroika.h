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

#ifndef ftroika_h
#define ftroika_h

#include "general.h"
#include "t27.h"

typedef struct{
    t27_t state[SLICESIZE];
    int rounds;
    int idx;
    int slice;
    int rowcol;
    uint32_t mask;
} ftroika_ctx;

//void ftroika_permutation(t27_t *state, int num_rounds);
//void ftroika_nullify_state(t27_t* state);
//void ftroika_nullify_rate(t27_t* state);
//void ftroika_nullify_capacity(t27_t* state);
//void ftroika_trits_to_rate(t27_t* state, const trit_t* trits, int len);
//void ftroika_rate_to_trits(const t27_t* state, trit_t* trits, int len);
//int ftroika_trytes_to_state(t27_t* state, const trit_t* trytes, int len);
//int ftroika_compare_states(t27_t* state, t27_t* other);
//void ftroika_increase_state(t27_t* state);
//void get_trytes(const t27_t *state);

void ftroika_init(ftroika_ctx *ctx, const int rounds);
void ftroika_absorb(ftroika_ctx *ctx, const trit_t *trits, int length);
void ftroika_finalize(ftroika_ctx *ctx);
void ftroika_squeeze(ftroika_ctx *ctx, trit_t *trits, int length);

void ftroika(trit_t *out, int outlen, const trit_t *in, const int inlen);
void ftroika_var_rounds(trit_t *out, const int outlen, const trit_t *in, const int inlen, const int rounds);
void ftroika_243_repeated(trit_t *out, const trit_t *in, const int rounds, const int repeat);
void ftroika_242_repeated(trit_t *out, const trit_t *in, const int rounds, const int repeat);

#endif /* ftroika_h */
