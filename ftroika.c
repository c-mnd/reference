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

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>

#include "t27.h"
#include "ftroika.h"
#include "t27.c"
#include "microtime.h"
#include "round_constants.h"

static trit_t ftroika_get(const t27_t* state, const int rowcol, const int slice){
    trit_t val;
    val = t27_get(&state[rowcol], slice);
    return val;
}

static void ftroika_set(t27_t state[27], const int rowcol, const int slice, const trit_t val){
    t27_set(&state[rowcol], slice, val);
}

static void ftroika_set_weak(t27_t state[27], const int rowcol, const int slice, const trit_t val){
    t27_set_weak(&state[rowcol], slice, val);
}

static void ftroika_sub_tryte(t27_t* a,t27_t* b,t27_t* c){
    t27_t d = t27_dec(*c), e, f, g;
    e = t27_add(t27_mul(d, *b), *a);
    f = t27_add(t27_mul(e, *b), d);
    g = t27_add(t27_mul(e, f), *b);
    *a = t27_clean(e);
    *b = t27_clean(f);
    *c = t27_clean(g);
}

static void ftroika_sub_trytes(t27_t* state){
    for (int rowcol = 0; rowcol < SLICESIZE; rowcol += 3){
        ftroika_sub_tryte(&state[rowcol + 2], &state[rowcol + 1], &state[rowcol]);
    }
}

static void ftroika_shift_rows(t27_t *state){
    t27_t new_state[SLICESIZE];
    const int8_t shifts[27] = {0,1,2,3,4,5,6,7,8,   12,13,14,15,16,17,9,10,11,   24,25,26,18,19,20,21,22,23};
    for(int i = 0; i < SLICESIZE; ++i) {
            new_state[shifts[i]] = state[i];
    }
    memcpy(state + 9 , new_state + 9, (SLICESIZE - 9) * sizeof(t27_t));
}

static void ftroika_shift_lanes(t27_t *state){
    const int8_t shifts[27] = {19,13,21,10,24,15,2,9,3,   14,0,6,5,1,25,22,23,20,   7,17,26,12,8,18,16,11,4};
    int rowcol;
    t27_t new_state[SLICESIZE];
    for(rowcol = 0; rowcol < SLICESIZE; ++rowcol) {
        new_state[rowcol] = t27_roll(state[rowcol], shifts[rowcol]);
    }
    memcpy(state, new_state, SLICESIZE * sizeof(t27_t));
}

static void ftroika_get_column_parity(const t27_t *state, t27_t parity[COLUMNS]){
    int col;
    for(col = 0; col < COLUMNS; ++col) {
        parity[col] = t27_add(state[col], t27_add(state[COLUMNS + col], state[2 * COLUMNS + col]));
    }
}

static void ftroika_add_column_parity(t27_t *state, int round){
    int row, col, idx;
    t27_t sum_to_add;
    t27_t parity[COLUMNS];
    ftroika_get_column_parity(state, parity);
    for(col = 0; col < COLUMNS; ++col) {
        t27_t t1 = parity[(col == 0)?(COLUMNS - 1):(col - 1)];
        t27_t t2 = t27_roll(parity[(col == COLUMNS - 1) ? 0 : (col + 1)], SLICES - 1);
        sum_to_add = t27_add(t1, t2);
        for(row = 0; row < ROWS; ++row) {
            idx = COLUMNS * row + col;
            t27_add_in_place(&state[idx], &sum_to_add);
        }
    }
}

static void ftroika_add_round_constant(t27_t *state, int round){
    int col;
    for(col = 0; col < COLUMNS; ++col) {
        t27_add_in_place(&state[col], &fround_constants[round][col]);
    }
}

void ftroika_permutation(t27_t *state, int num_rounds){
    assert(num_rounds <= num_rounds);
    for(int round = 0; round < num_rounds; round++) {
        ftroika_sub_trytes(state);
        ftroika_shift_rows(state);
        ftroika_shift_lanes(state);
        ftroika_add_column_parity(state, round);
        ftroika_add_round_constant(state, round);
    }
}

void ftroika_nullify_state(t27_t* state){
    const uint32_t mask = 0;
    for (int i = 0; i < SLICESIZE; i++){
        state[i].p &= mask;
        state[i].n &= mask;
    }
}
void ftroika_nullify_rate(t27_t* state){
    const uint32_t mask = 0x07fffe00;
    for (int i = 0; i < SLICESIZE; i++){
        state[i].p &= mask;
        state[i].n &= mask;
    }
}
void ftroika_nullify_capacity(t27_t* state){
    const uint32_t mask = 0x000001ff;
    for (int i = 0; i < SLICESIZE; i++){
        state[i].p &= mask;
        state[i].n &= mask;
    }
}

void ftroika_trits_to_rate(t27_t* state, const trit_t* trits, int len){
    int rowcol = 0, slice = 0;
    for (int i = 0; i < len; i++){
        if (rowcol == SLICESIZE){
            rowcol = 0; slice++;
        }
        ftroika_set(state, rowcol, slice, trits[i]);
        rowcol++;
    }
}

void ftroika_rate_to_trits(const t27_t* state, trit_t* trits, int len){
    int rowcol = 0, slice = 0;
    for (int i = 0; i < len; i++){
        if (rowcol == SLICESIZE){
            rowcol = 0; slice++;
        }
        trits[i] = ftroika_get(state, rowcol, slice);
        rowcol++;
    }
}
void ftroika_init(ftroika_ctx *ctx, const int rounds){
    memset(ctx, 0, sizeof(ftroika_ctx));
    ctx->rounds = rounds;
}

void ftroika_absorb(ftroika_ctx *ctx, const trit_t* trits, int length){
    int idx, space;
    while (length > 0) {
        if (ctx->idx == 0){
            ftroika_nullify_rate(ctx->state);
        }
        space = TROIKA_RATE - ctx->idx;
        if (length < space)
            space = length;
        for (idx = 0; idx < space; idx++) {
            //ftroika_set_weak(ctx->state, ctx->rowcol++, ctx->slice, trits[idx]);
            ftroika_set_weak(ctx->state, ctx->rowcol++, ctx->slice, trits[idx]);
            ctx->idx++;
            if (ctx->rowcol == SLICESIZE){
                ctx->rowcol = 0;
                ctx->slice++;
            }
        }
        trits += space;
        length -= space;
        if (ctx->idx == TROIKA_RATE){
            ftroika_permutation(ctx->state, ctx->rounds);
            ctx->idx = 0;
            ctx->slice = 0;
            ctx->rowcol = 0;
        }
    }
}

void ftroika_finalize(ftroika_ctx *ctx){
    trit_t pad[1] = {(trit_t)TROIKA_PADDING};
    ftroika_absorb(ctx, pad, 1);
    if (ctx->idx != 0){
        ftroika_permutation(ctx->state, ctx->rounds);
        ctx->idx = 0;
        ctx->slice = 0;
        ctx->rowcol = 0;
    }
}

void ftroika_squeeze(ftroika_ctx *ctx, trit_t *trits, int length){
    int idx, space;
    while (length > 0) {
        if (ctx->idx == TROIKA_RATE){
            ftroika_permutation(ctx->state, ctx->rounds);
            ctx->idx = 0;
            ctx->slice = 0;
            ctx->rowcol = 0;
        }
        space = TROIKA_RATE - ctx->idx;
        if (length < space)
            space = length;
        for (idx = 0; idx < space; idx++) {
            trits[idx] = ftroika_get(ctx->state, ctx->rowcol++, ctx->slice);
            ctx->idx++;
            if (ctx->rowcol == 27){
                ctx->rowcol = 0;
                ctx->slice++;
            }
        }
        trits += space;
        length -= space;
    }
}

void ftroika_var_rounds(trit_t *out, const int outlen, const trit_t * in, const int inlen, const int rounds){
    ftroika_ctx ctx;
    ftroika_init(&ctx, rounds);
    ftroika_absorb(&ctx, in, inlen);
    ftroika_finalize(&ctx);
    ftroika_squeeze(&ctx, out, outlen);
}

void ftroika(trit_t *out, const int outlen, const trit_t *in, const int inlen){
    ftroika_var_rounds(out, outlen, in, inlen, TROIKA_ROUNDS);
}

// one typical usage of Troika will be with 243 trits input and output
// and rehashing of the last output
// this function does it
void ftroika_243_repeated(trit_t *out, const trit_t *in, const int rounds, const int repeat){
    t27_t state[SLICESIZE];
    ftroika_nullify_state(state);
    ftroika_trits_to_rate(state, in, 243);
    for (int i = 0; i < repeat; i++){
        ftroika_nullify_capacity(state);
        ftroika_permutation(state, rounds);
        ftroika_nullify_rate(state);
        ftroika_set(state, 0, 0, TROIKA_PADDING);
        ftroika_permutation(state, rounds);
    }
    ftroika_rate_to_trits(state, out, 243);
}

// or maybe with just 242 trits, like it is today. that speeds up a lot
void ftroika_242_repeated(trit_t *out, const trit_t *in, const int rounds, const int repeat){
    t27_t state[SLICESIZE];
    ftroika_nullify_state(state);
    ftroika_trits_to_rate(state, in, 242);
    for (int i = 0; i < repeat; i++){
        ftroika_nullify_capacity(state);
        ftroika_set(state, 26, 8, TROIKA_PADDING);
        ftroika_permutation(state, rounds);
    }
    ftroika_rate_to_trits(state, out, 243);
}
