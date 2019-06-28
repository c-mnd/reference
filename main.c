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

#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <stdlib.h>

#include "troika.h"
#include "ftroika.h"
#include "stroika.h"
#include "utils.c"
#include "testsuite.h"

void* test_troika(void* args){
    test_t* t = (test_t *)args;
    if (t->name == 0){ // first call: copy individual properties into the test
        t->name = (char*)&"Troika";
        return NULL;
    } // below: the actual test
    for (int i = 0; i < t->times; i++) {
        TroikaVarRounds((Trit*)t->out[0], t->out_size, (Trit*)t->in[0], t->in_size, t->rounds);
        //memcpy(t->in[0], t->out[0], (t->in_size < t->out_size ? t->in_size : t->out_size) * sizeof(trit_t));
        memcpy(t->in[0], t->out[0], SBIT_BASE_SIZE * sizeof(trit_t));
    }
    return NULL;
}

void* test_ftroika(void* args){
    test_t* t = (test_t*)args;
    if (t->name == 0){
        t->name = (char*)&"fTroika";
        t->isRepeated = 1;
        t->chunks = 2;
        return NULL;
    }
    ftroika_ctx ctx;
    int sum, chunk, chunk_idx;
    for (int i = 0; i < t->times; i++) {
        ftroika_init(&ctx, t->rounds);
        sum = 0; chunk_idx = 0;
        while (sum < t->in_size) {
            chunk = t->chunksAbsorb[chunk_idx++];
            ftroika_absorb(&ctx, t->in[0] + sum, chunk);
            sum += chunk;
        }
        ftroika_finalize(&ctx);
        sum = 0; chunk_idx = 0;
        while (sum < t->out_size) {
            chunk = t->chunksSqueeze[chunk_idx++];
            ftroika_squeeze(&ctx, t->out[0] + sum, chunk);
            sum += chunk;
        }
        memcpy(t->in[0], t->out[0], SBIT_BASE_SIZE * sizeof(trit_t)); // feedback
    }
    return NULL;
}
void* test_stroika(void* args){
    test_t* t = (test_t *)args;
    if (t->name == 0){
        t->name = (char*)&"sTroika";
        t->isRepeated = 1;
        t->simd_size = SBIT_SIZE;
        t->slots = SBIT_SIZE;
        t->binary = 1;
        return NULL;
    }
    stroika_ctx ctx;
    for (int i = 0; i < t->times; i++) {
        stroika_init(&ctx, t->rounds);
        stroika_mux_binary(ctx.mux, t->in_binary, t->slots, t->in_size);
        int sum = 0, chunk, chunk_idx;
        sum = 0; chunk_idx = 0;
        while (sum < t->in_size) {
            chunk = t->chunksAbsorb[chunk_idx++];
            stroika_absorb(&ctx, ctx.mux + sum, chunk);
            sum += chunk;
        }
        stroika_finalize(&ctx);
        sum = 0; chunk_idx = 0;
        while (sum < t->out_size) {
            chunk = t->chunksSqueeze[chunk_idx++];
            stroika_squeeze(&ctx, ctx.mux + sum, chunk);
            sum += chunk;
        }
        stroika_demux_binary(ctx.mux, t->out_binary, t->slots, t->out_size);
        for (int slot = 0; slot < t->slots; slot++){
            t->in_binary[slot][0] = t->out_binary[slot][0];
        }
    }
    return NULL;
}
void* test_stroika_with_trit_t(void* args){
    test_t* t = (test_t *)args;
    if (t->name == 0){
        t->name = (char*)&"sTroika with trit_t";
        t->isRepeated = 1;
        t->simd_size = SBIT_SIZE;
        t->slots = SBIT_SIZE;
        t->binary = 0;
        return NULL;
    }
    stroika_ctx ctx;
    for (int i = 0; i < t->times; i++) {
        stroika_init(&ctx, t->rounds);
        stroika_mux(ctx.mux, t->in, t->slots, t->in_size);
        int sum = 0, chunk, chunk_idx;
        sum = 0; chunk_idx = 0;
        while (sum < t->in_size) {
            chunk = t->chunksAbsorb[chunk_idx++];
            stroika_absorb(&ctx, ctx.mux + sum, chunk);
            sum += chunk;
        }
        stroika_finalize(&ctx);
        sum = 0; chunk_idx = 0;
        while (sum < t->out_size) {
            chunk = t->chunksSqueeze[chunk_idx++];
            stroika_squeeze(&ctx, ctx.mux + sum, chunk);
            sum += chunk;
        }
        stroika_demux(ctx.mux, t->out, t->slots, t->out_size);
        // feedback only for testing
        for (int slot = 0; slot < t->slots; slot++){
            memcpy(t->in[slot], t->out[slot], SBIT_BASE_SIZE * sizeof(trit_t));
        }
    }
    return NULL;
}

void* test_stroika_short_mux(void* args){
    test_t* t = (test_t *)args;
    if (t->name == 0){
        t->name = (char*)&"sTroika one slot";
        t->isRepeated = 1;
        t->simd_size = SBIT_SIZE;
        t->binary = 1;
        return NULL;
    }
    test_stroika(args);
    return NULL;
}

// currently not comparable because feedbback size is 64
void* test_repeated243(void* args){
    test_t* t = (test_t *)args;
    if (t->name == 0){
        t->name = (char*)&"repeated 243 fTroika";
        t->isRepeated = 1;
        return NULL;
    }
    ftroika_243_repeated(t->out[0], t->in[0], t->rounds, (int)t->times);
    return NULL;
}

// currently not comparable because feedbback size is 64
void* test_repeated242(void* args){
    test_t* t = (test_t *)args;
    if (t->name == 0){
        t->name = (char*)&"repeated 242 fTroika";
        t->isRepeated = 1;
        return NULL;
    }
    ftroika_242_repeated(t->out[0], t->in[0], t->rounds, (int)t->times);
    return NULL;
}

int main(int argc, const char * argv[]){
    test_t suite, tests[TEST_MAX_IESTS];
    int rounds, times, pause = 1, random = 0x0fffffff & (int)get_microtime(), verbose, chunks, io_size;
    random = 189930602;
AA:for (rounds = TROIKA_ROUNDS; rounds > 0; rounds--){
        // test IOTA transaction size
        suite = setup_suite(tests, rounds, times = 250, 8019/*in_size*/, 243/*out_size*/, chunks = 0, random++, verbose = 1);
        add_test(&suite, &test_troika);
        add_test(&suite, &test_ftroika);
        add_test(&suite, &test_stroika);
        add_test(&suite, &test_stroika_with_trit_t);
        add_test(&suite, &test_stroika_short_mux);
        for (int i = 0; i < 1; i++){
            //assert(run(&suite));
            run(&suite);
            sleep(pause);
        }
        // with input size ef 242, only one permutation is needed
        suite = setup_suite(tests, rounds/*rounds*/, times, 242/*in_size*/, 243/*out_size*/, 0/*chunks*/, random++/*random*/, 1/*verbose*/);
        add_test(&suite, &test_troika);
        add_test(&suite, &test_ftroika);
        add_test(&suite, &test_stroika);
        add_test(&suite, &test_stroika_short_mux);
        //add_test(&suite, &test_repeated242);
        for (int i = 0; i < 1; i++){
            //assert(run(&suite));
            run(&suite);
            sleep(pause);
        }
        suite = setup_suite(tests, rounds, times, 243/*in_size*/, 243/*out_size*/, 0/*chunks*/, random++, 1/*verbose*/);
        add_test(&suite, &test_troika);
        add_test(&suite, &test_ftroika);
        add_test(&suite, &test_stroika);
        add_test(&suite, &test_stroika_short_mux);
        //add_test(&suite, &test_repeated243);
        for (int i = 0; i < 1; i++){
            //assert(run(&suite));
            run(&suite);
            sleep(pause);
        }
        
        // test with randomly sized IO and chunks
        for (int j = 0; j < 27; j++) {
            io_size = rand() % (TEST_MAX_IO_SIZE - 64) + 64;
            suite = setup_suite(tests, rounds, 3 /*times*/, io_size/*in_size*/, io_size/*out_size*/, 1/*chunks*/, random++, verbose = 0);
            add_test(&suite, &test_troika);
            add_test(&suite, &test_ftroika);
            add_test(&suite, &test_stroika);
            for (int i = 0; i < 3; i++){
                run(&suite);
                sleep(pause);
                suite.srand_start++;
            }
        }
    }
    goto AA;
}
