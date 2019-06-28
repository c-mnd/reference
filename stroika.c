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

#include "t27.h"
#include "stroika.h"
#include "round_constants.h"

#include "strit.c"
#include "mux.c"

static void stroika_sub_tryte(strit_t* a,strit_t* b,strit_t* c/*, strit_t* dest*/){
    strit_t d = *c, e, f, g;
    strit_dec(&d);
    e = strit_add(strit_mul(d, *b), *a);
    f = strit_add(strit_mul(e, *b), d);
    g = strit_add(strit_mul(e, f), *b);
    *a = e;
    *b = f;
    *c = g;
}

static void stroika_sub_trytes(strit_t* state){
    strit_t new_state[STATESIZE];
    for (int idx = 0; idx < STATESIZE; idx += 3){
        stroika_sub_tryte(&state[idx + 2], &state[idx + 1], &state[idx]);
    }
}

const uint16_t stroika_shift_rows_and_lanes_from_index[STATESIZE] = {
    216,379,164,462,85,329,681,493,656,366,16,584,603,712,65,147,121,203,561,292,50,429,538,269,315,451,641,
    243,406,191,489,112,356,708,520,683,393,43,611,630,10,92,174,148,230,588,319,77,456,565,296,342,478,668,
    270,433,218,516,139,383,6,547,710,420,70,638,657,37,119,201,175,257,615,346,104,483,592,323,369,505,695,
    297,460,245,543,166,410,33,574,8,447,97,665,684,64,146,228,202,284,642,373,131,510,619,350,396,532,722,
    324,487,272,570,193,437,60,601,35,474,124,692,711,91,173,255,229,311,669,400,158,537,646,377,423,559,20,
    351,514,299,597,220,464,87,628,62,501,151,719,9,118,200,282,256,338,696,427,185,564,673,404,450,586,47,
    378,541,326,624,247,491,114,655,89,528,178,17,36,145,227,309,283,365,723,454,212,591,700,431,477,613,74,
    405,568,353,651,274,518,141,682,116,555,205,44,63,172,254,336,310,392,21,481,239,618,727,458,504,640,101,
    432,595,380,678,301,545,168,709,143,582,232,71,90,199,281,363,337,419,48,508,266,645,25,485,531,667,128,
    459,622,407,705,328,572,195,7,170,609,259,98,117,226,308,390,364,446,75,535,293,672,52,512,558,694,155,
    486,649,434,3,355,599,222,34,197,636,286,125,144,253,335,417,391,473,102,562,320,699,79,539,585,721,182,
    513,676,461,30,382,626,249,61,224,663,313,152,171,280,362,444,418,500,129,589,347,726,106,566,612,19,209,
    540,703,488,57,409,653,276,88,251,690,340,179,198,307,389,471,445,527,156,616,374,24,133,593,639,46,236,
    567,1,515,84,436,680,303,115,278,717,367,206,225,334,416,498,472,554,183,643,401,51,160,620,666,73,263,
    594,28,542,111,463,707,330,142,305,15,394,233,252,361,443,525,499,581,210,670,428,78,187,647,693,100,290,
    621,55,569,138,490,5,357,169,332,42,421,260,279,388,470,552,526,608,237,697,455,105,214,674,720,127,317,
    648,82,596,165,517,32,384,196,359,69,448,287,306,415,497,579,553,635,264,724,482,132,241,701,18,154,344,
    675,109,623,192,544,59,411,223,386,96,475,314,333,442,524,606,580,662,291,22,509,159,268,728,45,181,371,
    702,136,650,219,571,86,438,250,413,123,502,341,360,469,551,633,607,689,318,49,536,186,295,26,72,208,398,
    0,163,677,246,598,113,465,277,440,150,529,368,387,496,578,660,634,716,345,76,563,213,322,53,99,235,425,
    27,190,704,273,625,140,492,304,467,177,556,395,414,523,605,687,661,14,372,103,590,240,349,80,126,262,452,
    54,217,2,300,652,167,519,331,494,204,583,422,441,550,632,714,688,41,399,130,617,267,376,107,153,289,479,
    81,244,29,327,679,194,546,358,521,231,610,449,468,577,659,12,715,68,426,157,644,294,403,134,180,316,506,
    108,271,56,354,706,221,573,385,548,258,637,476,495,604,686,39,13,95,453,184,671,321,430,161,207,343,533,
    135,298,83,381,4,248,600,412,575,285,664,503,522,631,713,66,40,122,480,211,698,348,457,188,234,370,560,
    162,325,110,408,31,275,627,439,602,312,691,530,549,658,11,93,67,149,507,238,725,375,484,215,261,397,587,
    189,352,137,435,58,302,654,466,629,339,718,557,576,685,38,120,94,176,534,265,23,402,511,242,288,424,614};
const uint16_t* X = stroika_shift_rows_and_lanes_from_index;

static void stroika_add_column_parity(strit_t *state, int round){
    int slice, row, col, idx;
    strit_t new_state[STATESIZE];
    strit_t parity_next_slice[COLUMNS], parity_this_slice[COLUMNS], parity_left_neighbour, parity_this, sum_to_add;
    slice = 0;
    for (col = 0; col < COLUMNS; col++) {
        parity_next_slice[col] = strit_add(strit_add(state[X[SLICESIZE*slice + col]], state[X[SLICESIZE*slice + COLUMNS + col]]), state[X[SLICESIZE*slice + COLUMNS*2 + col]]);
    }
    for (slice = SLICES - 1; slice >= 0; slice--) {
        col = COLUMNS - 1;
        parity_left_neighbour = strit_add(strit_add(state[X[SLICESIZE*slice + col]], state[X[SLICESIZE*slice + COLUMNS + col]]), state[X[SLICESIZE*slice + COLUMNS*2 + col]]);
        for (col = 0; col < COLUMNS; ++col) {
            parity_this = strit_zero();
            for (row = 0; row < ROWS; ++row) {
                idx = SLICESIZE*slice + COLUMNS*row + col;
                sum_to_add = strit_add(parity_left_neighbour, parity_next_slice[(col + 1) % 9]);
                strit_t x = state[stroika_shift_rows_and_lanes_from_index[idx]];
                new_state[idx] = strit_add(x, sum_to_add);
                parity_this = strit_add(parity_this, x);
            }
            parity_left_neighbour = parity_this;
            parity_this_slice[col] = parity_this;
        }
        memcpy(parity_next_slice, parity_this_slice, COLUMNS * sizeof(strit_t));
    }
    memcpy(state, new_state, STATESIZE * sizeof(strit_t));
}

static void stroika_add_round_constant(strit_t *state, int round){
    int col, idx;
    round = round%24;
    uint32_t slice;
    for (slice = 0; slice < SLICES; ++slice) {
        for (col = 0; col < COLUMNS; ++col) {
            idx = SLICESIZE * slice + col;
            if (fround_constants[round][col].p & (1 << slice)){
                strit_inc(&state[idx]);
            } else if (fround_constants[round][col].n & (1 << slice)){
                strit_dec(&state[idx]);
            }
        }
    }
}

void stroika_permutation(strit_t state[STATESIZE], int num_rounds){
    for(int round = 0; round < num_rounds; round++) {
        stroika_sub_trytes(state);
        stroika_add_column_parity(state, round);
        stroika_add_round_constant(state, round);
    }
}

void stroika_nullify_rate(strit_t* state){
    memset(state, 0, TROIKA_RATE * sizeof(strit_t));
}

void stroika_init(stroika_ctx *ctx, const int rounds){
    memset(ctx, 0, sizeof(stroika_ctx));
    ctx->rounds = rounds;
}

void stroika_absorb(stroika_ctx *ctx, const strit_t *muxtrits, int length){
    int idx, space, slot;
    while (length > 0) {
        if (ctx->idx == TROIKA_RATE){
            stroika_permutation(ctx->state, ctx->rounds);
            ctx->idx = 0;
            stroika_nullify_rate(ctx->state);
        }
        space = TROIKA_RATE - ctx->idx;
        if (length < space)
            space = length;
        for (idx = 0; idx < space; idx++) {
            ctx->state[ctx->idx++] = muxtrits[idx];
        }
        muxtrits += space;
        length -= space;
    }
}

void stroika_finalize(stroika_ctx *ctx){
    strit_t pad[1];
    pad[0] = strit_one();
    stroika_absorb(ctx, pad, 1);
    ctx->idx = TROIKA_RATE;
    stroika_permutation(ctx->state, ctx->rounds);
    ctx->idx = 0;
}

void stroika_squeeze(stroika_ctx *ctx, strit_t *muxtrits, int length){
    int idx, space;
    while (length > 0) {
        if (ctx->idx == TROIKA_RATE){
            stroika_permutation(ctx->state, ctx->rounds);
            ctx->idx = 0;
        }
        space = TROIKA_RATE - ctx->idx;
        if (length < space)
            space = length;
        for (idx = 0; idx < space; idx++) {
            muxtrits[idx] = ctx->state[ctx->idx++];
        }
        muxtrits += space;
        length -= space;
    }
}
