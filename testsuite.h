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

#ifndef testsuite_h
#define testsuite_h

#define TEST_MAX_IO_SIZE 8019 // transaction size
#define TEST_MAX_IESTS 9
#define TEST_MAX_CHUNKS 81

#include <stdlib.h>
#include "microtime.h"
#include "iotafy.h"
#include "strit.h"

typedef struct test_t{
    int ok;
    int rounds;
    int slots;
    int verbose;
    void*(*f)(void*);
    void* (*fs[TEST_MAX_IESTS])(void*);
    uint64_t times;
    int timesNow;
    char* name;
    int binary;
    uint64_t count;
    uint64_t microSec;
    uint64_t checksum[SIMD_SIZE];
    uint64_t shout[4];
    uint64_t echo[4];
    trit_t* in[SIMD_SIZE];
    strit_base_t* in_binary[SIMD_SIZE];
    tryte_t* in_trytes[SIMD_SIZE];
    trit_t* out[SIMD_SIZE];
    strit_base_t* out_binary[SIMD_SIZE];
    tryte_t* out_trytes[SIMD_SIZE];
    trit_t good_in[TEST_MAX_IO_SIZE];
    trit_t bad_in[TEST_MAX_IO_SIZE];
    int in_size;
    int out_size;
    int chunks;
    int chunksAbsorb[TEST_MAX_CHUNKS];
    int chunksSqueeze[TEST_MAX_CHUNKS];
    int sumAbsorb;
    int sumSqueeze;
    int isRepeated;
    int simd_size; // aka simd size
    //int mux_size; // aka simd size
    uint32_t srand_start;
    uint32_t srand;
    int num_tests;
    struct test_t* tests;
    char result_char;
} test_t;

static test_t setup_suite(test_t* tests, int rounds, int times, int in_size, int out_size, int chunks, int srand, int verbose){
    test_t t;
    memset(&t, 0, sizeof(test_t));
    t.tests = tests;
    memset(t.tests, 0, TEST_MAX_IESTS * sizeof(test_t));
    t.rounds = rounds;
    t.times = times;
    t.in_size = in_size < TEST_MAX_IO_SIZE ? in_size : TEST_MAX_IO_SIZE;
    t.out_size = out_size < TEST_MAX_IO_SIZE ? out_size : TEST_MAX_IO_SIZE;
    t.chunks = chunks < TEST_MAX_CHUNKS ? chunks : TEST_MAX_CHUNKS;
    t.srand_start = srand;
    t.verbose = verbose;
    t.simd_size = 1;
    t.slots = 1;
    return t;
}

static uint64_t checksum(trit_t* trits, int len){
    uint64_t result = 0, tmp;
    for (int i = 0; i < len; i++){
        tmp = 0ull | trits[i];
        result ^= (tmp << (i % 64) | tmp >> (64 - i % 64));
    }
    return result;
}

int test_random_chunk(int max){
    int size = 1 + rand() % 10;
    int mask = 0xffffffff >> (32 - size);
    int result = rand() % mask;
    if (result > max){
        result = max;
    }
    return result;
}

static void setup_test(test_t* suite, test_t* test, int number){
    memcpy(test, suite, sizeof(test_t));
    test->f = test->fs[number];
    uint64_t bla = 1;
    test->f(test); // first call, to copy properties into the test
    test->in[0] = malloc(sizeof(trit_t) * test->in_size);
    test->shout[0] = 1;
    memcpy(test->in[0], test->good_in, sizeof(trit_t) * test->in_size);
    binary_from_trits(&test->in_binary[0], test->in[0], test->in_size);
    for (int slot = 1; slot < test->slots; slot++) {
        test->in[slot] = malloc(sizeof(trit_t) * test->in_size);
        if (rand() % 2){
            memcpy(test->in[slot], test->good_in, sizeof(trit_t) * test->in_size);
            test->shout[slot / 64] |= (bla << (slot % 64));
        } else {
            memcpy(test->in[slot], test->bad_in, sizeof(trit_t) * test->in_size);
        }
        if (test->binary){
            binary_from_trits(&test->in_binary[slot], test->in[slot], test->in_size);
        }
    }
    for (int slot = 0; slot < test->slots; slot++) {
        trit_t* out = malloc(sizeof(trit_t) * test->out_size);
        test->out[slot] = out;
        if (test->binary){
            binary_from_trits(&test->out_binary[slot], test->out[slot], test->out_size);
        }
    }
    if (test->chunks == 0) {
        test->chunksAbsorb[0] = test->in_size;
        test->chunksSqueeze[0] = test->out_size;
    } else {
        srand((unsigned int)get_microtime());
        int i;
        int rest = test->in_size;
        for (i = 0; i < TEST_MAX_CHUNKS - 1; i++) {
            int chunk = test_random_chunk(rest);
            test->chunksAbsorb[i] = chunk;
            rest -= chunk;
        }
        test->chunksAbsorb[i] = rest;
        rest = test->out_size;
        for (i = 0; i < TEST_MAX_CHUNKS - 1; i++) {
            int chunk = test_random_chunk(rest);
            test->chunksSqueeze[i] = chunk;
            rest -= chunk;
        }
        test->chunksSqueeze[i] = rest;
        /*test->sumAbsorb = 1;
        test->sumSqueeze = 1;
        for (i = 0; i < TEST_MAX_CHUNKS; i++) {
            test->sumAbsorb += test->chunksAbsorb[i];
            test->sumSqueeze += test->chunksSqueeze[i];
        }
        printf("A %d S %d\n", test->sumAbsorb, test->sumSqueeze);*/
    }
}

void add_test(test_t* suite, void*(*f)(void*)){
    suite->fs[suite->num_tests] = f;
    suite->num_tests++;
}

void verify_test(test_t* t){
    if (t->binary){
        trits_from_binary(&t->out[0], t->out_binary[0], t->out_size);
    }
    t->checksum[0] = checksum(t->out[0], t->out_size);
    t->echo[0] = 1;
    uint64_t one = 1;
    for (int slot = 1; slot < t->slots; slot++) {
        if (t->binary){
            trits_from_binary(&t->out[slot], t->out_binary[slot], t->out_size);
        }
        t->checksum[slot] = checksum(t->out[slot], t->out_size);
        if (t->checksum[0] == t->checksum[slot]) {
            t->echo[slot/64] |= (one << (slot % 64));
        }
    }
    if (memcmp(t->shout, t->echo, 4 * sizeof(uint64_t)) == 0){
        t->result_char = '.';
    } else {
        t->result_char = '?';
    }
}

static void print_test(test_t* t){
    verify_test(t);
    printf("%24s %6llu ms/10k  (checksum %016llx)%c\n" , t->name, 1LL*t->microSec*10/t->count/t->simd_size, 0LL|t->checksum[0], t->result_char);
}
static void print_test_per_sec(test_t* t){
    verify_test(t);
    printf("%24s %8llu/sec   (checksum %016llx)%c\n" , t->name, 1LL*t->count*t->simd_size*1000000/t->microSec,
           t->checksum[0], t->result_char);
}
static void print_tests(test_t* suite){
    printf("i/o size %d/%d", suite->in_size, suite->out_size);
    if (suite->chunks != 0){
        printf(", with random chunks"); }
    printf(", %d rounds", suite->rounds);
    printf(", srand %d", suite->srand_start);
    printf("\n");
    printf("-------------------------------------------------------------------\n");
    for (int j = 0; j < suite->num_tests; j++) {
        print_test_per_sec(&suite->tests[j]);
    }
}

static void prepare_inout(test_t* t){
    trit_t good, bad;
    int r;
    for (int i = 0; i < t->in_size; i++){
        r = rand();
        switch (r % 3) {
            case 2:
                good = (trit_t)0 | (trit_t)TROIKA_SYMBOL_TWO;
                bad = (trit_t)0 | (trit_t)TROIKA_SYMBOL_ONE;
                break;
            case 1:
                good = (trit_t)0 | (trit_t)TROIKA_SYMBOL_ONE;
                bad = (trit_t)0 | (trit_t)TROIKA_SYMBOL_ZERO;
                break;
            default:
                good = (trit_t)0 | (trit_t)TROIKA_SYMBOL_ZERO;
                bad = (trit_t)0 | (trit_t)TROIKA_SYMBOL_TWO;
                break;
        }
        t->good_in[i] = good;
        t->bad_in[i] = bad;
    }
}

void free_test(test_t* t){
    for (int i = 0; i < SIMD_SIZE; i++) {
        free(t->in[i]);
        free(t->in_binary[i]);
        free(t->in_trytes[i]);
        free(t->out[i]);
        free(t->out_binary[i]);
        free(t->out_trytes[i]);
    }
}

static int run(test_t* suite){
    test_t* test;
    srand(suite->srand_start);
    suite->srand = rand();
    prepare_inout(suite);
    for (int j = 0; j < suite->num_tests; j++) {
        setup_test(suite, &suite->tests[j], j);
    }
    for (int j = 0; j < suite->num_tests; j++) {
        long mt1 = get_microtime();
        test = &suite->tests[j];
//#define THREADED 1
#ifdef THREADED
        pthread_t id;
        pthread_attr_t attr;
        int rc = pthread_attr_init(&attr);
        if (rc == -1) {
            perror("error in pthread_attr_init");
            exit(1);
        }
        size_t s1 = 2048 * 1024;
        rc = pthread_attr_setstacksize(&attr, s1);
        if (rc == -1) {
            perror("error in pthread_attr_setstacksize");
            exit(2);
        }
        pthread_create(&id, &attr, test->f, test);
        pthread_join(id, NULL);
#else
        test->f(test);
#endif
        long mt2 = get_microtime();
        test->microSec += (mt2 - mt1);
        test->count += test->times;
    }
    suite->ok = 1;
    verify_test(&suite->tests[0]);
    for (int j = 1; j < suite->num_tests; j++) {
        verify_test(&suite->tests[j]);
        if (suite->tests[0].checksum[0] != suite->tests[j].checksum[0] || suite->tests[j].result_char != '.'){
            suite->ok = 0;
        }
    }
    if (suite->ok && !suite->verbose){
        printf(".");
    } else {
        printf("\n");
        print_tests(suite);
    }
    if (!suite->ok){
        exit(1);
    }
    for (int j = 0; j < suite->num_tests; j++) {
        free_test(&suite->tests[j]);
    }

    suite->srand_start++;
    return 1;
}

#endif /* testsuite_h */
