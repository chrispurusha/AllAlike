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
sustain timers out and clears their stages, as the host side does, so each releases at once (until 2026-11-22 the
stages were left, and a key let go mid-attack or mid-decay played on). One Shot runs the timers out as the note starts. A voice that has finished is cleared and free.

## 8. The words
`sn2FixedWords.c` computes them from the program's own bytes as the instrument's host side does. Checked 2026-11-22 for
all 1424 programs at five keys and velocities, and field by field over each whole range (4072 settings): every word is
exact, apart from unison's detune and volume and field values no program holds. Most are a field in a straight line; times are an exponent
in 1024ths of an octave (`octave_word`, `exponent`); depths on pitch and LFO to cutoff are squared with their sign;
velocity is ((v v >> 7) v) >> 7 about each envelope's depth; a range of an LFO multiplies its speed by the square root
of ten; an LFO locked to the tempo has no speed of its own; word $6D is the byte of routes (FM, Filter OSCs Bypass, the
LFOs' fades), and where a slot is FM its level and depths are squared. The tempo is the processor's X:$5, 120 beats a minute
at $312700. A few tables remain (LFO soften and sync).

## 9. In the voices
Maths FIXED gives each voice a twin (`tSn2Voice.fixed`): note on starts it in the slot the float voices chose,
letting go releases it, and its bus word, times 2.58 x 63 / 84 (the volume the gain was set at), is the voice's
output. The program's volume is one of the voice's own words ($BE), so it is not applied again after, as it is to the
float voices: applying it twice took a program at volume 9 some 19 dB too low. A volume change reaches the next note. A twin silent for 4096 samples after its release frees the slot. The
effects stay in floating point: the instrument does them on another processor. On the base test program FIXED
nulls against the instrument to 44.9 dB, its spectrum the same to 0.01 dB, its level to 0.00 dB.

## 10. Free-running LFOs
Each LFO's running state is four of the voice's words (LFO 1 at Y:$86, $89, $8A and $9A; LFO 2 five on, and $9B), none
of them written from the host side, so a note's clear starts both LFOs from the beginning. That is what Key does. For an LFO set
to Free the four words are carried over from the voice started last, so the LFO runs on from note to note as the
float voices' shared free LFO does. The delay and fade are not among them and start again with each note. While no
voice sounds, FIXED's free LFO waits where it was; the float voices' and the instrument's run on.

## 11. Free oscillators
The update at a note's start restarts the phases only for Osc Trigger Percussive. For Ensemble that part of it is left
out and the phases and windows are kept over the clear (until 2026-11-22 every note restarted them, so detuned
oscillators beat from the same place on every key). A voice's phases wait while it is silent; the instrument's run on.
The windows (sn2Osc notes §7) start open in the template, as on a voice that has played: they began shut, which muted
the first half cycle of every note.

## 12. While it sounds
The wheel (word $D7), each oscillator's bend and the tempo are kept up to date. Mono: a note that restarts is started
as any other; a legato one changes only the key and the glide. A glide starts at the pitch the float voice worked out.
