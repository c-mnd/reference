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

#ifndef stroika_h
#define stroika_h

#include <stdio.h>

#include "general.h"
#include "strit.h"
#include "mux.h"

typedef struct stroika_ctx{
    strit_t state[STATESIZE];
    strit_t mux[MUX_LEN];
    int rounds;
    int idx;
} stroika_ctx;


void stroika_mux(strit_t* mux, trit_t** trits, int slot, int length);
void stroika_mux_binary(strit_t* mux, strit_base_t ** stb, int slots,  int length);
void stroika_demux(strit_t* mux, trit_t** trits, int slot, int length);
void stroika_demux_simple(strit_t* mux, trit_t** trits, int slot, int length);
void stroika_demux_binary(strit_t* mux, strit_base_t** stb, int slots,  int length);

void stroika_permutation(strit_t *state, int num_rounds);
void stroika_init(stroika_ctx* ctx, const int rounds);
void stroika_absorb(stroika_ctx* ctx, const strit_t* muxtrits, int length);
void stroika_finalize(stroika_ctx* ctx);
void stroika_squeeze(stroika_ctx* ctx, strit_t* muxtrits, int length);

#endif /* stroika_h */
