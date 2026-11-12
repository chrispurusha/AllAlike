# Nl3Alike design

Started 2026-11-02 as a reproduction inspired by the Clavia Nord Lead 3, as a SynthLib plug-in (VST3 + AU). Public, in
AllAlike.

## State

The skeleton is Sn2Alike's, renamed: plug-in wrappers, panel, presets, patch bar, MIDI link. The engine behind it is
still Sn2Alike's and plays as a placeholder; every section is to be replaced by the Nord Lead 3's own.

**The filter is the Nord Lead 3's (2026-11-07):** LP and HP at 12 and 24 dB, its coefficient laws and level
compensation, in the instrument's own fixed-point arithmetic or in floating point (the Arithmetic parameter). The
voices run at 44.1 kHz, the instrument's rate. See `code-notes/nl3Filter.md`.

## The instrument

From its manual (v1.2x) and the program dumps, measured on the instrument. 24 voices, four slots (A-D) layered or
split, each slot a Program; four outputs.

**One voice:**

    OSC 1 (+ hidden sync osc) --+                                   +-- Filter Env
                                +-- Osc Mod -- MIXER -- FILTER ------+-- AMPLIFIER -- out
    OSC 2 (+ hidden sync osc) --+   (Noise3)           (Osc 2 bypass)   +-- Amp Env

- **Oscillators 1 and 2:** six waveforms each (Saw, Pulse, Triangle, Sine, Noise / synched noise, Dual Sine FM)
  with a Shape knob whose meaning depends on the waveform; Sync uses a hidden third oscillator per group, so both
  stay audible. Osc 2 adds Coarse (semitones or partials), Fine, KBT off, and noise type/frequency.
- **Oscillator Modulation:** an amount and six types (DM, Dist, Filt FM, FM, DFM and the amplitude/ring type).
- **Mixer:** Osc Mix (1 to 2), and Noise3 in place of Osc 2.
- **Filter:** single modes LP, HP, BP (6, 12, 24 dB), BR (12), Classic (24), Dist LP; multi modes with a second
  cutoff - LP-BP parallel, LP-HP parallel, HP-LP series, BP-BP parallel, Multi Peak, Multi Notch. Keyboard Track
  (on/off), Osc 2 bypass. Filter Env (ADSR, amount, invert, velocity, exponential attack).
- **Amplifier:** Amp Env (ADSR, exponential attack), output level.
- **Modulation:** LFO 1 and 2 per voice (six waves, 12 destinations each, single-cycle or key sync, mono, invert,
  clock sync), Mod Env (attack, decay or release, repeat, invert, 12 destinations).
- **Play:** vibrato, glide, mono/legato/priority, unison, octave shift, arpeggiator, chord memory.
- **Morph:** four sources (wheel, aftertouch or pedal, velocity, keyboard), each an offset for 26 parameters.

**The Program** is the manual's dump format with corrections measured on the instrument (every CC sent and the
program dumped; see `code-notes/nl3Patch.md` when it lands): the 7-bit knob block exactly as documented; each LFO's
Env/KB Sync is 3 bits; one undocumented bit after each LFO's Mono and one before Mod Env Invert; spare 5 is 1 bit;
18 bits before Legato.

## The rebuild

The engine and parameters came from Sn2Alike and do not fit (three oscillators, two mod envelopes, ring mods,
chorus, delay and reverb, which the Nord Lead 3 does not have). The plan:

1. **Parameters = the Program's fields**, one plug-in parameter per field, plus the four morph groups. **Done
   (2026-11-07):** 199 program parameters; programs from the instrument read in and write back byte for byte, and
   the instrument takes a program written from parameters. The voice still plays through an interim mapping
   (`code-notes/nl3Params.md` §5).
2. **Voice:** oscillators (each waveform, Shape, sync) - **the Saw is built (2026-11-07)**, word for word in
   fixed point, with an Anti-alias switch (On, the default, or Off for the instrument's own aliases) - Osc Mod types, mixer and Noise3, the filter's remaining
   modes, envelopes, LFOs and Mod Env with their destinations, then morph.
3. **Play:** glide, vibrato, mono/legato/unison, arpeggiator, chord memory, slots last if at all.

Each law is measured on the instrument and kept in both arithmetics (Float and the instrument's Fixed).

## Validation

The reference is the instrument: on the Elektron TM-1, MIDI channel 2, its audio into QU-24 inputs 9/10. Laws in the
code are there because they reproduce what the instrument plays; this repo states only what was measured.

## To decide

- Scope: one program per instance, as the other Alikes (slots and performances later, if at all).
- The panel, once the parameters are mapped.
