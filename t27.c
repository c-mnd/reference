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

#include <stdio.h>
#include "t27.h"
#include "general.h"
#include "iotafy.h"

// trits are stored as two bits, one inside p, the other inside n, both have the same 'index'
// names 'p' (positive) and 'n' (negative) result from using these functions in a lib for balanced ternary before
// so i just left them here

static t27_t t27_new(const uint32_t a, const uint32_t b){
    t27_t r;
    r.p = a;
    r.n = b;
    return  r;
}
static int t27_eq(const t27_t a, const t27_t b){
    if (a.p == b.p && a.n == b.n){
        return 1;
    }
    return 0;
}
static t27_t t27_clean(t27_t a){
    t27_t r;
    r.p = a.p & 0x07ffffff;
    r.n = a.n & 0x07ffffff;
    return r;
}

static void t27_add_in_place(t27_t* a, const t27_t* b){
    uint32_t temp;
    uint32_t a0 = ~a->p & ~a->n;
    temp = ~(a->n ^ b->n) & ~(a0 ^ b->p);
    a->n = ~(a->p ^ b->p) & ~(a0 ^ b->n);
    a->p = temp;
}

static t27_t t27_add(const t27_t a, const t27_t b){
    t27_t r;
    uint32_t a0 = ~a.p & ~a.n;
    r.p = ~(a.n ^ b.n) & ~(a0 ^ b.p);
    r.n = ~(a.p ^ b.p) & ~(a0 ^ b.n);
    return r;
}

static t27_t t27_mul(const t27_t a, const t27_t b){
    t27_t r;
    r.p = (a.p & b.p) | (a.n & b.n);
    r.n = (a.p & b.n) | (a.n & b.p);
    return r;
}

static t27_t t27_not(const t27_t a){
    t27_t r;
    r.p = a.n;
    r.n = a.p;
    return r;
}

static t27_t t27_dec(const t27_t a){
    t27_t r;
    const t27_t minusOne = {0,0x07ffffff};
    r = t27_add(a, minusOne);
    return r;
}

static t27_t t27_roll(t27_t a, const int n){
    t27_t r;
    r.p = ((a.p << n) | (a.p >> (27-n))) & 0x07ffffff;
    r.n = ((a.n << n) | (a.n >> (27-n))) & 0x07ffffff;
    return r;
}

static int8_t t27_get(const t27_t * a, const int pos){
    const uint32_t mask = 1 << pos;
    if (a->n & mask){
        return (trit_t)TROIKA_SYMBOL_TWO;
    } else {
        return (a->p & mask) ? (trit_t)TROIKA_SYMBOL_ONE : (trit_t)TROIKA_SYMBOL_ZERO;
    }
}

// use this only when you know it wasn't set before
static void t27_set_weak(t27_t * a, const int pos, const int8_t val){
    const uint32_t mask = 1 << pos;
    if (val == (trit_t)TROIKA_SYMBOL_TWO){
        a->n |= mask;
        return;
    } else if (val == (trit_t)TROIKA_SYMBOL_ONE) {
        a->p |= mask;
        return;
    }
}

static void t27_set(t27_t * a, const int pos, const int8_t val){
    const uint32_t unmask = ~ (1 << pos);
    a->p = a->p & unmask;
    a->n = a->n & unmask;
    t27_set_weak(a, pos, val);
}
