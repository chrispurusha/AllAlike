# nl3Fixed.h - fixed-point arithmetic

The instrument computes in 24-bit words with 56-bit accumulators. The Arithmetic parameter, per instance, runs the
stages that have been converted that way (so far the filter); Float, the default, is the same laws in doubles.
TiAlike's `tiFixed.h` is the same arithmetic.

## 1. The units
A word is a fraction of full scale, 1.0 = 2²³. An accumulator holds 56 bits, 1.0 = 2⁴⁷: a word sits in its middle 24
bits, with 24 below it and 8 above for headroom.

## 2. Words into accumulators
A word moved into an accumulator fills the middle, the low part cleared and the top extended by the sign. A left
shift keeps 56 bits; anything shifted past the top is lost.

## 3. Multiplying
Two words multiply as fractions, so the product is doubled.

## 4. Moving out
An accumulator moved to a word is limited first - beyond ±1 it gives the largest word of that sign - then the
middle 24 bits are kept, truncating towards minus infinity.

## 5. Rounding
A rounded multiply rounds convergently: exact halves go to the even word.
