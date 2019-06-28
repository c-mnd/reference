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

#include "strit.h"

static  strit_t strit_new(const SBIT_T a, const SBIT_T b){
    strit_t r;
    r.p = a;
    r.n = b;
    return  r;
}

/*static  int strit_eq(const strit_t a, const strit_t b){
 if (0 == memcmp(&a, &b, sizeof(strit_t)))
 {
 return 1;
 }
 return 0;
 }*/

static  strit_t strit_mul(const strit_t a, const strit_t b){
    strit_t r;
    r.p = (a.p & b.p) | (a.n & b.n);
    r.n = (a.p & b.n) | (a.n & b.p);
    return r;
}

static  strit_t strit_add(const strit_t a, const strit_t b){
    strit_t r;
    SBIT_T a0 = ~a.p & ~a.n;
    r.p = ~(a.n ^ b.n) & ~(a0 ^ b.p);
    r.n = ~(a.p ^ b.p) & ~(a0 ^ b.n);
    return r;
}

static  void strit_add_in_place(strit_t* a, const strit_t* b){
    SBIT_T temp;
    SBIT_T a0 = ~a->p & ~a->n;
    temp = ~(a->n ^ b->n) & ~(a0 ^ b->p);
    a->n = ~(a->p ^ b->p) & ~(a0 ^ b->n);
    a->p = temp;
}

static const strit_t strit_one(){
    strit_t result;
    memset(&result.p, 0xff, sizeof(SBIT_T));
    memset(&result.n, 0, sizeof(SBIT_T));
    return result;
}
static const strit_t strit_minus(){
    strit_t result;
    memset(&result.p, 0, sizeof(SBIT_T));
    memset(&result.n, 0xff, sizeof(SBIT_T));
    return result;
}
static const strit_t strit_zero(){
    strit_t result;
    memset(&result.p, 0, sizeof(SBIT_T));
    memset(&result.n, 0, sizeof(SBIT_T));
    return result;
}

static  void strit_inc(strit_t* a){
    const strit_t one = strit_one();
    strit_add_in_place(a, &one);
}

static  void strit_dec(strit_t* a){
    const strit_t minus = strit_minus();
    strit_add_in_place(a, &minus);
}

/*static  strit_t strit_not(const strit_t a){
 strit_t r;
 r.p = a.n;
 r.n = a.p;
 return r;
 }*/

static int8_t strit_get(const strit_t * a, const int pos){
#if SBIT_SIZE > 0
    const int idx = pos / 64;
    const long long mask = 1ll << pos % 64;
    SBIT_BASE_T* apx = (SBIT_BASE_T*)&a->p;
    SBIT_BASE_T* anx = (SBIT_BASE_T*)&a->n;
    if (anx[idx] & mask){
        return (trit_t)TROIKA_SYMBOL_TWO;
    } else {
        return (apx[idx] & mask) ? (trit_t)TROIKA_SYMBOL_ONE : (trit_t)TROIKA_SYMBOL_ZERO;
    }
#else
#endif
    return TROIKA_SYMBOL_ONE;
}

// use this only when you know it wasn't set before
static void strit_set_weak(strit_t * a, const int pos, const int8_t val){
    const int idx = pos / 64;
    const long long mask = 1ll << pos % 64;
    SBIT_BASE_T* apx = (SBIT_BASE_T*)&a->p;
    SBIT_BASE_T* anx = (SBIT_BASE_T*)&a->n;
    if (val == (trit_t)TROIKA_SYMBOL_TWO){
        anx[idx] |= mask;
        return;
    } else if (val == (trit_t)TROIKA_SYMBOL_ONE) {
        apx[idx] |= mask;
        return;
    }
}

static void strit_base_set_weak(strit_base_t * a, const int pos, const int8_t val){
    const int idx = pos / 64;
    const long long mask = 1ll << pos % 64;
    SBIT_BASE_T* apx = (SBIT_BASE_T*)&a->p;
    SBIT_BASE_T* anx = (SBIT_BASE_T*)&a->n;
    if (val == (trit_t)TROIKA_SYMBOL_TWO){
        anx[idx] |= mask;
        return;
    } else if (val == (trit_t)TROIKA_SYMBOL_ONE) {
        apx[idx] |= mask;
        return;
    }
}
