# sn2Dsp.h and the fixed-point voice

## 1. The arithmetic
Words of 24 bits and accumulators of 56, kept sign-extended in `int32_t` and `int64_t`. A product is the two words'
times two; a move out of an accumulator saturates at +-1 (`sn2_lim`), a move in clears the low word. Flags (carry,
overflow, zero, negative, unnormalised, extension) as each operation leaves them, since later instructions test them.

## 2. Memory and addressing
A voice reads and writes X and Y memory of 4096 words each: its parameter block, its states, constants, and the
tables (exponential, reciprocal and the oscillators' edge table). Address registers step linearly, or wrap within a
block of a power of two (`sn2_step`). The voice loop's count (`lc`) tells the code which voice it is; each voice
works out its slow words (filter, pitch) on one sample in four.

## 3. Status (2026-11-22)
The voice's per-sample arithmetic, its slow words included, agrees with the instrument's word for word - registers
and every memory word - on seven filter settings (plain, resonant, parallel, Overdrive 0-127, Resonance 127), with
and without parameter updates, and with glide (mono and poly), LFO delay and envelope delay mid-ramp. A few modes
of the per-voice ramp units, and part of the note-start clear, have not been exercised yet. The tables are
calculated (exponential 2^(i/512)/2, reciprocal 1/(1 + i/512)/2), within 135 and 1 words of the instrument's.

## 4. What it showed the float model
Fed the same words and input, the float filter's structure agreed with the exact one to 52-111 dB at low levels,
but a driven resonance diverged: the level-dependent damping is worked out once a sample from the band as the sample
starts, and the high-pass term is limited to full scale (sn2Filter notes §4). With both, every setting agrees to at
least 66 dB. The level reaching the filter already matched within 0.2 dB.

## 5. Float and fixed check each other
Where the two disagree, either may be wrong: the float model (as the filter's damping was), the fixed one in a mode
not yet exercised (§3), or both. The instrument itself decides - the hardware above all. Comparing the two over many
settings is also how the untried modes will be reached and checked.

## 6. A voice's memory
One voice to a memory of 4096 words of X and 4096 of Y (`tSn2FixedVoice`), as the instrument's voice processor holds
four. An idle voice is built once (`sn2_fixed_template`): the constants the processor sets for itself, the three
tables calculated (§3), the host side's fixed words, then the voice cleared and run idle for 64 samples. Every voice
starts from a copy. The voice's words from the host side are 221 (`SN2_FIXED_WORDS`, its first the flags); they do
not change while a note is held - envelopes, LFOs and glide all run in the voice's own arithmetic.

## 7. A note
The host side writes the 221 words and raises the update in the flags (all of it, with the oscillators restarted)
for one sample: the voice then sets its own envelope, delay and LFO states. Letting go runs the three envelopes'
sustain timers out at once (they then release). A voice that has finished is cleared and free.

## 8. The words
`sn2FixedWords.c` (generated from measurements) computes them from the program's own bytes: most are a field in a
straight line (base + step x value, exact integers); the host side's time laws (envelope attack, decay, release,
sustain time and rate, delay) are calculated, exactly: an exponent e in 1024ths of an octave that walks in straight
runs (107 a step for decay and release; attack 363 for its first 12 values then 107; sustain time 336 for 20 then
80; sustain rate 214 a step each side of its centre), and the word 2^20 halved for each whole octave of e, read
straight across the octave - (2^20 >> n) x (2048 - f) / 2048. Where no such law reproduces every word (LFO soften
amount so far) the word is a table of 128 steps, measured; Env 1's sustain is 512 x value squared; the key is (note - 60) in the word's middle.
A few words combine fields (oscillator pitch, a flags word, the LFO delays and their fade mode) - not all done yet,
nor velocity; measured on real programs, 95% of the words are exact so far (2026-11-22).

## 9. In the voices
Maths FIXED gives each voice a twin (`tSn2Voice.fixed`): note on starts it in the slot the float voices chose,
letting go releases it, and its bus word, times 2.58, is the voice's output - the instrument's output level with
the volume applied after, as the float voices'. A twin silent for 4096 samples after its release frees the slot. The
effects stay in floating point: the instrument does them on another processor. On the base test program FIXED
nulls against the instrument to 44.9 dB, its spectrum the same to 0.01 dB, its level to 0.00 dB.
