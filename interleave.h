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

#include "t27.h"
#include "ftroika.h"
#include "t27.c"
#include "microtime.h"
#include "round_constants.h"

static trit_t ftroika_state_get(const t27_t* state, const int rowcol, const int slice){
    trit_t val;
    val = t27_get(&state[slice], rowcol);
    return val;
}

static void ftroika_state_set(t27_t* state, const int rowcol, const int slice, const trit_t val){
    t27_set(&state[slice], rowcol, val);
}

static void ftroika_state_set_weak(ftroika_ctx* ctx, const trit_t val){
    if (val & 254){
        ctx->state[ctx->slice].n |= ctx->mask;
    } else if (val & 1) {
        ctx->state[ctx->slice].p |= ctx->mask;
    }
    
}
/*
 static trit_t ftroika_buffer_get(const t27_t* buffer, const int rowcol, const int slice){
 trit_t val;
 val = t27_get(&buffer[slice], rowcol);
 return val;
 }
 
 static void ftroika_buffer_set(t27_t buffer[27], const int rowcol, const int slice, const trit_t val){
 t27_set(&buffer[slice], rowcol, val);
 }
 
 static void ftroika_buffer_set_weak(ftroika_ctx* ctx, const trit_t val){
 if (val & 254){
 ctx->buffer[ctx->slice].n |= ctx->mask;
 } else if (val & 1) {
 ctx->buffer[ctx->slice].p |= ctx->mask;
 }
 }
 */
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
    t27_t d = dec(*c), e, f, g;
    e = add(mul(d, *b), *a);
    f = add(mul(e, *b), d);
    g = add(mul(e, f), *b);
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
    for(int i = 9; i < SLICESIZE; ++i) {
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
        parity[col] = add(state[col], add(state[COLUMNS + col], state[2 * COLUMNS + col]));
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
        sum_to_add = add(t1, t2);
        for(row = 0; row < ROWS; ++row) {
            idx = COLUMNS * row + col;
            t27_sum_in_place(&state[idx], &sum_to_add);
        }
    }
}

static void ftroika_add_round_constant(t27_t *state, int round){
    int col;
    for(col = 0; col < COLUMNS; ++col) {
        t27_sum_in_place(&state[col], &fround_constants[round][col]);
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
    ctx->mask = 1;
}

void ftroika_absorb(ftroika_ctx *ctx, const trit_t* trits, int length){
    int idx, space;
    while (length > 0) {
        if (ctx->idx == TROIKA_RATE){
            interleave(ctx->state);
            //memset(ctx->buffer, 0, 9 * sizeof(t27_t));
            ftroika_permutation(ctx->state, ctx->rounds);
            interleave(ctx->state);
            ctx->idx = 0;
            ctx->slice = 0;
            ctx->rowcol = 0;
            //ftroika_nullify_rate(ctx->state);
            memset(ctx->state, 0, 9 * sizeof(t27_t));
        }
        space = TROIKA_RATE - ctx->idx;
        if (length < space)
            space = length;
        uint64_t mt1 = getMicrotime();
        for (idx = 0; idx < space; idx++) {
            ftroika_state_set_weak(ctx, trits[idx]);
            ctx->rowcol++;
            ctx->idx++;
            ctx->mask *= 2;
            if (ctx->rowcol == SLICESIZE){
                ctx->mask = 1;
                ctx->rowcol = 0;
                ctx->slice++;
            }
        }
        //uint64_t mt1 = getMicrotime();
        uint64_t mt2 = getMicrotime();
        //printf("write %llu\n", mt2-mt1);
        trits += space;
        length -= space;
    }
}

void ftroika_finalize(ftroika_ctx *ctx){
    trit_t pad[1] = {TROIKA_PADDING};
    ftroika_absorb(ctx, pad, 1);
    interleave(ctx->state);
    ftroika_permutation(ctx->state, ctx->rounds);
    interleave(ctx->state);
    ctx->idx = 243;
    ctx->slice = 0;
    ctx->rowcol = 0;
}

void ftroika_squeeze(ftroika_ctx *ctx, trit_t *trits, int length){
    int idx, space;
    while (length > 0) {
        if (ctx->idx == TROIKA_RATE){
            interleave(ctx->state);
            ftroika_permutation(ctx->state, ctx->rounds);
            interleave(ctx->state);
            ctx->idx = 0;
            ctx->slice = 0;
            ctx->rowcol = 0;
        }
        space = TROIKA_RATE - ctx->idx;
        if (length < space)
            space = length;
        uint64_t mt1 = getMicrotime();
        for (idx = 0; idx < space; idx++) {
            trits[idx] = ftroika_state_get(ctx->state, ctx->rowcol++, ctx->slice);
            ctx->idx++;
            if (ctx->rowcol == 27){
                ctx->rowcol = 0;
                ctx->slice++;
            }
        }
        //uint64_t mt1 = getMicrotime();
        uint64_t mt2 = getMicrotime();
        //printf("read %llu\n", mt2-mt1);
        trits += space;
        length -= space;
    }
    int ii = 1;
    ii++;
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
    ftroika_rate_to_trits(state, out, 242);
}

void interleave(t27_t* state){
    t27_t s[27], ns[27];
    memcpy(s, state, 27 * sizeof(t27_t));
    memset(ns, 0, 27 * sizeof(t27_t));
    uint32_t mask[3][3] = {{0x000001ff,0x000003fe00,0x0007fc0000}, {0x001c0e07,0x00e07038,0x070381c0}, {0x01249249,0x02492492,0x04924924}};
    int total = 27,base, width = 3, chunk = width / 3,
    depth = 2, line, other, y, dy, x, dx, shift, msk;
    while (chunk <= 9) {
        line = 0;
        other =  line + 2 * width;
        for (base = 0; base < total; base += width){
            for (dy = 0; dy < width; dy += chunk){
                for (line = base + dy; line < base + dy + chunk; line++) {
                    for (dx = 0; dx < width; dx += chunk) {
                        int sx = dy, sy = dx;
                        other = line - dy + dx;
                        shift = dx - dy;
                        msk = mask[depth][dy/chunk];
                        //if (other < 9){
                        if (shift < 0){
                            ns[line].p |= ((s[other].p & msk) >> (-shift));
                            ns[line].n |= ((s[other].n & msk) >> (-shift));
                            //printf("MACRO1(%d, %d, %d, %d, %d);\n",line, other, depth, dy/chunk, -shift);
                        } else {
                            ns[line].p |= ((s[other].p & msk) << (shift));
                            ns[line].n |= ((s[other].n & msk) << (shift));
                            //printf("MACRO2(%d, %d, %d, %d, %d);\n",line, other, depth, dy/chunk, shift);
                        }//}
                    }
                }
                //line++; other++;
            }
            //other -= (2 * width);
        }
        //printf("\n");
        width *= 3;
        chunk *= 3;
        depth--;
        memcpy(s, ns, 27 * sizeof(t27_t));
        memset(ns, 0, 27 * sizeof(t27_t));
    }
    memcpy(state, s, 27 * sizeof(t27_t));
}
void interleave_buffer2(t27_t* buffer){
    t27_t s[9], ns[9];
    memcpy(s, buffer, 9 * sizeof(t27_t));
    memset(ns, 0, 9 * sizeof(t27_t));
    uint32_t mask[3][3] = {{0x000001ff,0x000003fe00,0x0007fc0000}, {0x001c0e07,0x00e07038,0x070381c0}, {0x01249249,0x02492492,0x04924924}};
    int total = 9, base, width = 3, chunk = width / 3,
    depth = 2, line, other, y, dy, x, dx, shift, msk;
    while (chunk <= 3) {
        line = 0;
        other =  line + 2 * width;
        for (base = 0; base < total; base += width){
            for (dy = 0; dy < width; dy += chunk){
                for (line = base + dy; line < base + dy + chunk; line++) {
                    for (dx = 0; dx < width; dx += chunk) {
                        int sx = dy, sy = dx;
                        other = line - dy + dx;
                        shift = dx - dy;
                        msk = mask[depth][dy/chunk];
                        if (other < 9){
                            if (shift < 0){
                                ns[line].p |= ((s[other].p & msk) >> (-shift));
                                ns[line].n |= ((s[other].n & msk) >> (-shift));
                                //printf("MACRO1(%d, %d, %d, %d, %d);\n",line, other, depth, dy/chunk, -shift);
                            } else {
                                ns[line].p |= ((s[other].p & msk) << (shift));
                                ns[line].n |= ((s[other].n & msk) << (shift));
                                //printf("MACRO2(%d, %d, %d, %d, %d);\n",line, other, depth, dy/chunk, shift);
                            }}
                    }
                }
                //line++; other++;
            }
            //other -= (2 * width);
        }
        //printf("\n");
        width *= 3;
        chunk *= 3;
        depth--;
        memcpy(s, ns, 9 * sizeof(t27_t));
        memset(ns, 0, 9 * sizeof(t27_t));
    }
    memcpy(buffer, s, 9 * sizeof(t27_t));
}
#define MACRO1(a,b,c,d,e) ns[a].p |= ((s[b].p & (mask[c][d])) >> (e));ns[a].n |= ((s[b].n & mask[c][d]) >> (e));
#define MACRO2(a,b,c,d,e) ns[a].p |= ((s[b].p & (mask[c][d])) << (e));ns[a].n |= ((s[b].n & mask[c][d]) << (e));

void b2s(t27_t* state, t27_t* buffer){
    uint64_t mt1 = getMicrotime();
    uint32_t mask[3] = {0x000001ff,0x000003fe00,0x0007fc0000};
    int i, m = 0, b = 0;
    interleave_buffer2(buffer);
    for (i = 0; i < 27; i++) {
        state[i].p |= ((buffer[b].p & mask[m]) >> (9 * m));
        state[i].n |= ((buffer[b].n & mask[m]) >> (9 * m));
        if (++b == 9){
            b = 0;
            m++;;
        };
    }
    uint64_t mt2 = getMicrotime();
    //printf("b2s %llu\n", mt2-mt1);
}
void s2b(t27_t* state, t27_t* buffer){
    uint64_t mt1 = getMicrotime();
    uint32_t mask = 0x000001ff;
    int i, m = 0, b = 0;
    for (i = 0; i < 9; i++) {
        buffer[i].p = (state[i].p & mask) | ((state[i+9].p & mask) << 9) | ((state[i+18].p & mask) << 18);
        buffer[i].n = (state[i].n & mask) | ((state[i+9].n & mask) << 9) | ((state[i+18].n & mask) << 18);
        if (++b == 9){
            b = 0;
            m++;;
        };
    }
    interleave_buffer2(buffer);
    uint64_t mt2 = getMicrotime();
    //printf("s2b %llu\n", mt2-mt1);
}
void interleave2(t27_t* state, t27_t* buffer){
    t27_t s[27], ns[27];
    memset(s, 0, 27 * sizeof(t27_t));
    memcpy(s, state, 27 * sizeof(t27_t));
    memset(ns, 0, 27 * sizeof(t27_t));
    uint32_t mask[3][3] = {{0x000001ff,0x000003fe00,0x0007fc0000}, {0x001c0e07,0x00e07038,0x070381c0}, {0x01249249,0x02492492,0x04924924}};
    int total = 27,base = 27, width = total, chunk = width / 3,
    len = base / width, depth = 0, line, other, y, dy, x, dx, shift, msk;
    
    MACRO2(0, 0, 0, 0, 0);
    MACRO2(0, 9, 0, 0, 9);
    MACRO2(0, 18, 0, 0, 18);
    MACRO2(1, 1, 0, 0, 0);
    MACRO2(1, 10, 0, 0, 9);
    MACRO2(1, 19, 0, 0, 18);
    MACRO2(2, 2, 0, 0, 0);
    MACRO2(2, 11, 0, 0, 9);
    MACRO2(2, 20, 0, 0, 18);
    MACRO2(3, 3, 0, 0, 0);
    MACRO2(3, 12, 0, 0, 9);
    MACRO2(3, 21, 0, 0, 18);
    MACRO2(4, 4, 0, 0, 0);
    MACRO2(4, 13, 0, 0, 9);
    MACRO2(4, 22, 0, 0, 18);
    MACRO2(5, 5, 0, 0, 0);
    MACRO2(5, 14, 0, 0, 9);
    MACRO2(5, 23, 0, 0, 18);
    MACRO2(6, 6, 0, 0, 0);
    MACRO2(6, 15, 0, 0, 9);
    MACRO2(6, 24, 0, 0, 18);
    MACRO2(7, 7, 0, 0, 0);
    MACRO2(7, 16, 0, 0, 9);
    MACRO2(7, 25, 0, 0, 18);
    MACRO2(8, 8, 0, 0, 0);
    MACRO2(8, 17, 0, 0, 9);
    MACRO2(8, 26, 0, 0, 18);
    memcpy(s, ns, 27 * sizeof(t27_t));
    memset(ns, 0, 27 * sizeof(t27_t));
    
    
    MACRO2(0, 0, 1, 0, 0);
    MACRO2(0, 3, 1, 0, 3);
    MACRO2(0, 6, 1, 0, 6);
    MACRO2(1, 1, 1, 0, 0);
    MACRO2(1, 4, 1, 0, 3);
    MACRO2(1, 7, 1, 0, 6);
    MACRO2(2, 2, 1, 0, 0);
    MACRO2(2, 5, 1, 0, 3);
    MACRO2(2, 8, 1, 0, 6);
    MACRO1(3, 0, 1, 1, 3);
    MACRO2(3, 3, 1, 1, 0);
    MACRO2(3, 6, 1, 1, 3);
    MACRO1(4, 1, 1, 1, 3);
    MACRO2(4, 4, 1, 1, 0);
    MACRO2(4, 7, 1, 1, 3);
    MACRO1(5, 2, 1, 1, 3);
    MACRO2(5, 5, 1, 1, 0);
    MACRO2(5, 8, 1, 1, 3);
    MACRO1(6, 0, 1, 2, 6);
    MACRO1(6, 3, 1, 2, 3);
    MACRO2(6, 6, 1, 2, 0);
    MACRO1(7, 1, 1, 2, 6);
    MACRO1(7, 4, 1, 2, 3);
    MACRO2(7, 7, 1, 2, 0);
    MACRO1(8, 2, 1, 2, 6);
    MACRO1(8, 5, 1, 2, 3);
    MACRO2(8, 8, 1, 2, 0);
    memcpy(s, ns, 27 * sizeof(t27_t));
    memset(ns, 0, 27 * sizeof(t27_t));
    
    
    MACRO2(0, 0, 2, 0, 0);
    MACRO2(0, 1, 2, 0, 1);
    MACRO2(0, 2, 2, 0, 2);
    MACRO1(1, 0, 2, 1, 1);
    MACRO2(1, 1, 2, 1, 0);
    MACRO2(1, 2, 2, 1, 1);
    MACRO1(2, 0, 2, 2, 2);
    MACRO1(2, 1, 2, 2, 1);
    MACRO2(2, 2, 2, 2, 0);
    MACRO2(3, 3, 2, 0, 0);
    MACRO2(3, 4, 2, 0, 1);
    MACRO2(3, 5, 2, 0, 2);
    MACRO1(4, 3, 2, 1, 1);
    MACRO2(4, 4, 2, 1, 0);
    MACRO2(4, 5, 2, 1, 1);
    MACRO1(5, 3, 2, 2, 2);
    MACRO1(5, 4, 2, 2, 1);
    MACRO2(5, 5, 2, 2, 0);
    MACRO2(6, 6, 2, 0, 0);
    MACRO2(6, 7, 2, 0, 1);
    MACRO2(6, 8, 2, 0, 2);
    MACRO1(7, 6, 2, 1, 1);
    MACRO2(7, 7, 2, 1, 0);
    MACRO2(7, 8, 2, 1, 1);
    MACRO1(8, 6, 2, 2, 2);
    MACRO1(8, 7, 2, 2, 1);
    MACRO2(8, 8, 2, 2, 0);
    memcpy(s, ns, 27 * sizeof(t27_t));
    memset(ns, 0, 27 * sizeof(t27_t));
    
    memcpy(buffer, s, 9 * sizeof(t27_t));
}
void interleave3(t27_t* state, t27_t* buffer){
    t27_t s[27], ns[27];
    memset(s, 0, 27 * sizeof(t27_t));
    memset(ns, 0, 27 * sizeof(t27_t));
    uint32_t mask[3][3] = {{0x000001ff,0x000003fe00,0x0007fc0000}, {0x001c0e07,0x00e07038,0x070381c0}, {0x01249249,0x02492492,0x04924924}};
    int total = 27,base = 27, width = total, chunk = width / 3,
    len = base / width, depth = 0, line, other, y, dy, x, dx, shift, msk;
    
    memcpy(s, buffer, 9 * sizeof(t27_t));
    
    MACRO2(0, 0, 2, 0, 0);
    MACRO2(0, 1, 2, 0, 1);
    MACRO2(0, 2, 2, 0, 2);
    MACRO1(1, 0, 2, 1, 1);
    MACRO2(1, 1, 2, 1, 0);
    MACRO2(1, 2, 2, 1, 1);
    MACRO1(2, 0, 2, 2, 2);
    MACRO1(2, 1, 2, 2, 1);
    MACRO2(2, 2, 2, 2, 0);
    MACRO2(3, 3, 2, 0, 0);
    MACRO2(3, 4, 2, 0, 1);
    MACRO2(3, 5, 2, 0, 2);
    MACRO1(4, 3, 2, 1, 1);
    MACRO2(4, 4, 2, 1, 0);
    MACRO2(4, 5, 2, 1, 1);
    MACRO1(5, 3, 2, 2, 2);
    MACRO1(5, 4, 2, 2, 1);
    MACRO2(5, 5, 2, 2, 0);
    MACRO2(6, 6, 2, 0, 0);
    MACRO2(6, 7, 2, 0, 1);
    MACRO2(6, 8, 2, 0, 2);
    MACRO1(7, 6, 2, 1, 1);
    MACRO2(7, 7, 2, 1, 0);
    MACRO2(7, 8, 2, 1, 1);
    MACRO1(8, 6, 2, 2, 2);
    MACRO1(8, 7, 2, 2, 1);
    MACRO2(8, 8, 2, 2, 0);
    memcpy(s, ns, 27 * sizeof(t27_t));
    memset(ns, 0, 27 * sizeof(t27_t));
    
    MACRO2(0, 0, 1, 0, 0);
    MACRO2(0, 3, 1, 0, 3);
    MACRO2(0, 6, 1, 0, 6);
    MACRO2(1, 1, 1, 0, 0);
    MACRO2(1, 4, 1, 0, 3);
    MACRO2(1, 7, 1, 0, 6);
    MACRO2(2, 2, 1, 0, 0);
    MACRO2(2, 5, 1, 0, 3);
    MACRO2(2, 8, 1, 0, 6);
    MACRO1(3, 0, 1, 1, 3);
    MACRO2(3, 3, 1, 1, 0);
    MACRO2(3, 6, 1, 1, 3);
    MACRO1(4, 1, 1, 1, 3);
    MACRO2(4, 4, 1, 1, 0);
    MACRO2(4, 7, 1, 1, 3);
    MACRO1(5, 2, 1, 1, 3);
    MACRO2(5, 5, 1, 1, 0);
    MACRO2(5, 8, 1, 1, 3);
    MACRO1(6, 0, 1, 2, 6);
    MACRO1(6, 3, 1, 2, 3);
    MACRO2(6, 6, 1, 2, 0);
    MACRO1(7, 1, 1, 2, 6);
    MACRO1(7, 4, 1, 2, 3);
    MACRO2(7, 7, 1, 2, 0);
    MACRO1(8, 2, 1, 2, 6);
    MACRO1(8, 5, 1, 2, 3);
    MACRO2(8, 8, 1, 2, 0);
    memcpy(s, ns, 27 * sizeof(t27_t));
    memset(ns, 0, 27 * sizeof(t27_t));
    
    MACRO2(0, 0, 0, 0, 0);
    MACRO2(1, 1, 0, 0, 0);
    MACRO2(2, 2, 0, 0, 0);
    MACRO2(3, 3, 0, 0, 0);
    MACRO2(4, 4, 0, 0, 0);
    MACRO2(5, 5, 0, 0, 0);
    MACRO2(6, 6, 0, 0, 0);
    MACRO2(7, 7, 0, 0, 0);
    MACRO2(8, 8, 0, 0, 0);
    MACRO1(9, 0, 0, 1, 9);
    MACRO1(10, 1, 0, 1, 9);
    MACRO1(11, 2, 0, 1, 9);
    MACRO1(12, 3, 0, 1, 9);
    MACRO1(13, 4, 0, 1, 9);
    MACRO1(14, 5, 0, 1, 9);
    MACRO1(15, 6, 0, 1, 9);
    MACRO1(16, 7, 0, 1, 9);
    MACRO1(17, 8, 0, 1, 9);
    MACRO1(18, 0, 0, 2, 18);
    MACRO1(19, 1, 0, 2, 18);
    MACRO1(20, 2, 0, 2, 18);
    MACRO1(21, 3, 0, 2, 18);
    MACRO1(22, 4, 0, 2, 18);
    MACRO1(23, 5, 0, 2, 18);
    MACRO1(24, 6, 0, 2, 18);
    MACRO1(25, 7, 0, 2, 18);
    MACRO1(26, 8, 0, 2, 18);
    memcpy(s, ns, 27 * sizeof(t27_t));
    memset(ns, 0, 27 * sizeof(t27_t));
    
    for (int i = 0; i < 27; i++) {
        state[i].p |= s[i].p;
        state[i].n |= s[i].n;
    }
    //memcpy(state, s, 27 * sizeof(t27_t));
}
void interleave_buffer(t27_t* buffer){
    t27_t s[9], ns[9];
    memset(s, 0, 9 * sizeof(t27_t));
    memset(ns, 0, 9 * sizeof(t27_t));
    uint32_t mask[3][3] = {{0x000001ff,0x000003fe00,0x0007fc0000}, {0x001c0e07,0x00e07038,0x070381c0}, {0x01249249,0x02492492,0x04924924}};
    int total = 27,base = 27, width = total, chunk = width / 3,
    len = base / width, depth = 0, line, other, y, dy, x, dx, shift, msk;
    
    memcpy(s, buffer, 9 * sizeof(t27_t));
    
    MACRO2(0, 0, 2, 0, 0);
    MACRO2(0, 1, 2, 0, 1);
    MACRO2(0, 2, 2, 0, 2);
    MACRO1(1, 0, 2, 1, 1);
    MACRO2(1, 1, 2, 1, 0);
    MACRO2(1, 2, 2, 1, 1);
    MACRO1(2, 0, 2, 2, 2);
    MACRO1(2, 1, 2, 2, 1);
    MACRO2(2, 2, 2, 2, 0);
    MACRO2(3, 3, 2, 0, 0);
    MACRO2(3, 4, 2, 0, 1);
    MACRO2(3, 5, 2, 0, 2);
    MACRO1(4, 3, 2, 1, 1);
    MACRO2(4, 4, 2, 1, 0);
    MACRO2(4, 5, 2, 1, 1);
    MACRO1(5, 3, 2, 2, 2);
    MACRO1(5, 4, 2, 2, 1);
    MACRO2(5, 5, 2, 2, 0);
    MACRO2(6, 6, 2, 0, 0);
    MACRO2(6, 7, 2, 0, 1);
    MACRO2(6, 8, 2, 0, 2);
    MACRO1(7, 6, 2, 1, 1);
    MACRO2(7, 7, 2, 1, 0);
    MACRO2(7, 8, 2, 1, 1);
    MACRO1(8, 6, 2, 2, 2);
    MACRO1(8, 7, 2, 2, 1);
    MACRO2(8, 8, 2, 2, 0);
    memcpy(s, ns, 9 * sizeof(t27_t));
    memset(ns, 0, 9 * sizeof(t27_t));
    
    MACRO2(0, 0, 1, 0, 0);
    MACRO2(0, 3, 1, 0, 3);
    MACRO2(0, 6, 1, 0, 6);
    MACRO2(1, 1, 1, 0, 0);
    MACRO2(1, 4, 1, 0, 3);
    MACRO2(1, 7, 1, 0, 6);
    MACRO2(2, 2, 1, 0, 0);
    MACRO2(2, 5, 1, 0, 3);
    MACRO2(2, 8, 1, 0, 6);
    MACRO1(3, 0, 1, 1, 3);
    MACRO2(3, 3, 1, 1, 0);
    MACRO2(3, 6, 1, 1, 3);
    MACRO1(4, 1, 1, 1, 3);
    MACRO2(4, 4, 1, 1, 0);
    MACRO2(4, 7, 1, 1, 3);
    MACRO1(5, 2, 1, 1, 3);
    MACRO2(5, 5, 1, 1, 0);
    MACRO2(5, 8, 1, 1, 3);
    MACRO1(6, 0, 1, 2, 6);
    MACRO1(6, 3, 1, 2, 3);
    MACRO2(6, 6, 1, 2, 0);
    MACRO1(7, 1, 1, 2, 6);
    MACRO1(7, 4, 1, 2, 3);
    MACRO2(7, 7, 1, 2, 0);
    MACRO1(8, 2, 1, 2, 6);
    MACRO1(8, 5, 1, 2, 3);
    MACRO2(8, 8, 1, 2, 0);
    memcpy(s, ns, 9 * sizeof(t27_t));
    memset(ns, 0, 9 * sizeof(t27_t));
    
    memcpy(buffer, s, 9 * sizeof(t27_t));
}
