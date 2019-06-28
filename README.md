fTroika:\
single hashing,\
supports chunks,\
puts a single lane into two dwords of "t27_t"

sTroika:\
built ontop of reference Troika, with data structure like *state[slice][row][column]*,\
SIMD, 32 and 64 trits everywhere, 128 and 256 trits on Intel,\
supports chunks of any size,\
full multiplexing (all (I called them) slots are loaded and hashes stored back).\
faster, if trits were converted into "strit_base_t" (the "binary" format) before,\
because you will re-taylor stroika for your needs anyway, the one-fits-all functions "stroika" and "stroika_var_rounds" were omitted.\
see also: https://github.com/shufps/s-troika, thanks a lot for inspiring.

For usage, see the tests in *main.c*.



Clone it, go to the folder with the *.c* files, run
```
gcc -O3 -Wall -Wno-unused -DSIMD_SIZE=64 main.c *troika.c -o troika 
./troika
```
or on Intel:
```
gcc -O3 -Wall -Wno-unused -DSIMD_SIZE=256 -mavx2 main.c *troika.c -o troika 
./troika
```
CTRL-C when you're done.


Notes (on stuff that maybe I could have designed more selfexplaining, mostly the testing part)

checksum:\
generated from output data on slot 0, which is available in every variant.\
You might wonder, why there's this checksum instead of trytes or trits,\
simply because we only needed to compare results of the reference (troika) to all the others,\
so "uint64_t checksum(trit_t)" was the easiest 3-liner, that supported the "=="-operator.\
And no, if checksums just differ, it doesn't you anything.\
And yes, if a checksum is like "00000000" or "ffffffff", you'll get a strong hint what went wrong.

test_t.good, test_t.bad:\
are input vectors randomly distributed over the slots,\
their distribution becomes the shout and echo (read on).

test_t.shout, test_t.echo, test_t.result_char\
are used for verifying that multiplex/demultiplex worked correctly,\
when hashing is over, echo will be calculated by outputs equal/unequal to slot 0,\
if shout == echo, it's all fine and result_char gets '.', otherwise '?' and printed out in a verbose test

 troikafy/iotafy\
 Troika was created unbalanced ternary, so we have to kinda translate it from/to balanced ternary,\
 my simple solution is to replace symbols, just exchange values of TROIKA_SYMBOL_XXX in "iotafy.h".
 
 
 
 For further information see the [Troika website](https://www.cyber-crypt.com/troika).
 
 
 
 Finally: I crashed my Raspi, so wasn't able to test there,\
 maybe one of you will do and report on the #troika channel of the IOTA discord.\
 Thank You.
 
