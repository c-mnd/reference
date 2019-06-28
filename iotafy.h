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

#ifndef iotafy_h
#define iotafy_h

// define TROIKA_SYMBOL_XXX as you want (of course the values should be different)
#define TROIKA_SYMBOL_ZERO (0)
#define TROIKA_SYMBOL_ONE (1)
#define TROIKA_SYMBOL_TWO (-1)
#define TROIKA_IOTAFY(x, T) (((x) == 2) ? (T)TROIKA_SYMBOL_TWO : ((x) == 1 ? (T)TROIKA_SYMBOL_ONE : (T)TROIKA_SYMBOL_ZERO))
#define TROIKA_TROIKAFY(x, T) ((((T)x) == (T)TROIKA_SYMBOL_TWO) ? 2 : (((T)x) == (T)TROIKA_SYMBOL_ONE ? 1 : 0))

#endif /* iotafy_h */
