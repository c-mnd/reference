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

#include "mux.h"
#include "strit.h"

static inline void fold(strit_t x[SBIT_BASE_SIZE]){
    SBIT_BASE_T mask[6] ={0x00000000ffffffff,0x0000ffff0000ffff,0x00ff00ff00ff00ff,
        0x0f0f0f0f0f0f0f0f,0x3333333333333333,0x5555555555555555};
    int inner = SBIT_BASE_SIZE / 2, depth = (SBIT_BASE_SIZE == 64)?0:1;
    SBIT_T a, b, c, d;
    SBIT_BASE_T m, nm;
    
    while (inner > 0) {
        m = mask[depth];
        nm = ~m;
        for (int o = 0; o < SBIT_BASE_SIZE; o = o + 2 * inner){
            for (int i = 0; i < inner; i++){
                a = x[o + i].p & m;
                b = RSHIFT((x[o + i].p & ~m), inner);
                c = LSHIFT(x[o + i + inner].p & m, inner);
                d = x[o + i + inner].p & ~m;
                x[o + i].p = a | c;
                x[o + i + inner].p = b | d;
                a = x[o + i].n & m;
                b = RSHIFT((x[o + i].n & ~m), inner);
                c = LSHIFT(x[o + i + inner].n & m, inner);
                d = x[o + i + inner].n & ~m;
                x[o + i].n = a | c;
                x[o + i + inner].n = b | d;
            }
        }
        inner /= 2;
        depth++;
    }
}

void stroika_mux(strit_t *mux, trit_t **trits, int slots,  int length){
    memset(mux, 0, MUX_LEN * sizeof(strit_t));
    for (int slot = 0; slot < slots; slot++){
        for (int i = 0; i < length; i++) {
            int slot_div = slot / SBIT_BASE_SIZE, slot_mod = slot % SBIT_BASE_SIZE;
            int i_div = i / SBIT_BASE_SIZE, i_mod = i % SBIT_BASE_SIZE;
            int idx = i_div * SBIT_BASE_SIZE + slot_mod, slt = slot_div * SBIT_BASE_SIZE + i_mod;
            //printf("%d  %d\n", idx, slt);
            strit_set_weak(&mux[i_div * SBIT_BASE_SIZE + slot_mod], slot_div * SBIT_BASE_SIZE + i_mod, trits[slot][i]);
        }
        //printf("\n");
    }
    for (int i = 0; i < length; i = i + SBIT_BASE_SIZE) {
        fold(mux + i);
    }
}
void stroika_mux_binary(strit_t *mux, strit_base_t **txxb, int slots,  int length){
    //memset(mux, 0, MUX_LEN * sizeof(strit_t));
    for (int i = 0; i < length; i = i + SBIT_BASE_SIZE) {
        for (int slot_mod = 0; slot_mod < SBIT_BASE_SIZE; slot_mod++){
            for (int slot_div = 0; slot_div < SBIT_SIZE / SBIT_BASE_SIZE; slot_div++) {
                int i_div = i / SBIT_BASE_SIZE, i_mod = i % SBIT_BASE_SIZE;
                if (slot_mod + SBIT_BASE_SIZE * slot_div < slots){
                    strit_base_t x = txxb[slot_mod + SBIT_BASE_SIZE * slot_div][i_div];
                    (ADDR mux[i_div * SBIT_BASE_SIZE + slot_mod].p)[slot_div] = x.p;
                    (ADDR mux[i_div * SBIT_BASE_SIZE + slot_mod].n)[slot_div] = x.n;
                }
            }
        }
        if (i % SBIT_BASE_SIZE == 0 /*&& slot == 0*/){
            fold(mux + i);
        }
    }
    /*for (int i = 0; i < length; i = i + SBIT_BASE_SIZE) {
     fold(mux + i);
     }*/
}

void stroika_demux_simple(strit_t *mux, trit_t **trits, int slots, int length){
    for (int slot = 0; slot < slots; slot++){
        for (int i = 0; i < length; i++) {
            int slot_div = slot / SBIT_BASE_SIZE, slot_mod = slot % SBIT_BASE_SIZE;
            int i_div = i / SBIT_BASE_SIZE, i_mod = i % SBIT_BASE_SIZE;
            int idx =i_div * SBIT_BASE_SIZE + slot_mod, slt = slot_div * SBIT_BASE_SIZE + i_mod;
            //printf("%d  %d\n", idx, slt);
            trit_t trit = strit_get(&mux[i_div * SBIT_BASE_SIZE + slot_mod], slot_div * SBIT_BASE_SIZE + i_mod);
            trits[slot][i] = trit;
        }
        //printf("\n");
    }
}

void stroika_demux(strit_t *mux, trit_t **trits, int slots, int length){
    /*for (int i = 0; i < length; i = i + SBIT_BASE_SIZE) {
        fold(mux + i);
    }*/
        for (int i = 0; i < length; i++) {
            if (i % SBIT_BASE_SIZE == 0){
                fold(mux + i);
            }
            for (int slot = 0; slot < slots; slot++){
            int slot_div = slot / SBIT_BASE_SIZE, slot_mod = slot % SBIT_BASE_SIZE;
            int i_div = i / SBIT_BASE_SIZE, i_mod = i % SBIT_BASE_SIZE;
            int idx =i_div * SBIT_BASE_SIZE + slot_mod, slt = slot_div * SBIT_BASE_SIZE + i_mod;
            //printf("%d  %d\n", idx, slt);
            trit_t trit = strit_get(&mux[i_div * SBIT_BASE_SIZE + slot_mod], slot_div * SBIT_BASE_SIZE + i_mod);
            trits[slot][i] = trit;
        }
        //printf("\n");
    }
}
void stroika_demux_binary(strit_t *mux, strit_base_t **txxb, int slots,  int length){
    for (int i = 0; i < length; i=i+SBIT_BASE_SIZE) {
        if (i % SBIT_BASE_SIZE == 0)
            fold(mux + i);
        for (int slot = 0; slot < slots; slot++){
            int slot_div = slot / SBIT_BASE_SIZE, slot_mod = slot % SBIT_BASE_SIZE;
            int i_div = i / SBIT_BASE_SIZE;
            strit_base_t* x = &txxb[slot][i_div];
            x->p = (ADDR mux[i_div * SBIT_BASE_SIZE + slot_mod].p)[slot_div];
            x->n = (ADDR mux[i_div * SBIT_BASE_SIZE + slot_mod].n)[slot_div];
        }
    }
}

void stroika_demux_binary2(strit_t *mux, strit_base_t **txxb, int slots,  int length){
    for (int i = 0; i < length; i=i+SBIT_BASE_SIZE) {
        if (i % SBIT_BASE_SIZE == 0)
            fold(mux + i);
        for (int slot_mod = 0; slot_mod < SBIT_BASE_SIZE; slot_mod++){
            for (int slot_div = 0; slot_div < SBIT_SIZE / SBIT_BASE_SIZE; slot_div++){
                if (slot_div * SBIT_BASE_SIZE + slot_mod < slots){
                    int i_div = i / SBIT_BASE_SIZE;
                    strit_base_t* x = &txxb[slot_div * SBIT_BASE_SIZE + slot_mod][i_div];
                    x->p = (ADDR mux[i_div * SBIT_BASE_SIZE + slot_mod].p)[slot_div];
                    x->n = (ADDR mux[i_div * SBIT_BASE_SIZE + slot_mod].n)[slot_div];
                }
            }
        }
    }
}


