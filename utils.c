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
#include "utils.h"

void binary_from_trits(strit_base_t** txxb, trit_t* trits, int length){
    int size = (length % SBIT_BASE_SIZE == 0) ? (length / SBIT_BASE_SIZE) : (length / SBIT_BASE_SIZE + 1);
    free(*txxb);
    *txxb = (strit_base_t*) malloc(size * sizeof(strit_t));
    if (*txxb != NULL){
        memset(*txxb, 0, size * sizeof(strit_t));
        for (int i = 0; i < length; i++){
            const int idx = i / SBIT_BASE_SIZE;
            SBIT_BASE_T mask = 1ll << (i % SBIT_BASE_SIZE);
            if (trits[i] == (trit_t)TROIKA_SYMBOL_TWO){
                (*txxb)[idx].n |= mask;
            } else if (trits[i] == (trit_t)TROIKA_SYMBOL_ONE) {
                (*txxb)[idx].p |= mask;
            }
        }
    }
}
void trits_from_binary(trit_t** trits, strit_base_t* stb, int length){
    int size = length;
    free(*trits);
    *trits = (trit_t*) malloc(size * sizeof(trit_t));
    if (*trits != NULL && stb != NULL){
        memset(*trits, 0, size * sizeof(trit_t));
        for (int i = 0; i < length; i++){
            //const int idx = i / SBIT_BASE_SIZE;
            SBIT_BASE_T mask = 1ll << (i % SBIT_BASE_SIZE);
            strit_base_t x = stb[i / SBIT_BASE_SIZE];
            if ((x.n & mask) != 0){
                (*trits)[i] = (trit_t)TROIKA_SYMBOL_TWO;
            } else if ((x.p & mask) != 0) {
                (*trits)[i] = (trit_t)TROIKA_SYMBOL_ONE;
            } else {
                (*trits)[i] = (trit_t)TROIKA_SYMBOL_ZERO;
            }
        }
    }
}

// unused and untested
void trytes_from_trits(trit_t** trytes, trit_t* trits, int length){
    const char* alphabet = "9ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    trit_t factor = 1, tryte = 0;
    int size = length / 3;
    free(*trytes);
    *trytes = (trit_t*) malloc(size * sizeof(trit_t));
    if (*trytes != NULL){
        memset(*trytes, 0, size * sizeof(trit_t));
        for (int i = 0; i < length; i++){
            if (trits[i] == TROIKA_SYMBOL_TWO){
                tryte -= factor;
            } else if (trits[i] == TROIKA_SYMBOL_ONE) {
                tryte += factor;
            }
            factor = factor * 3;
            if (factor == 27 || i == length - 1){
                factor = 1;
                (*trytes)[i / 3] = alphabet[tryte + ((tryte<0)?27:0)];
                tryte = 0;
            }
        }
    }
}
