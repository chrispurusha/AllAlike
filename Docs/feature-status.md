# Feature status

Where each plug-in stands: what is **missing**, what is **built but needs more work**, and what is **built but not
yet checked** in a host or against the instrument. Updated 2026-11-06 (Pulse2Alike P001-P015 close by ear except P003, the drive; a dedicated Pulse 2 capture rig).

This is a summary for planning. The working lists stay in each plug-in's `Docs/todo.md` (one line per item) and
`Docs/to-test.md`; the reasoning and measurements in `Docs/findings.md` and `Docs/code-notes/`. When an item here is
done, change it there first, then here.

| | Pulse2Alike | TiAlike | Sn2Alike |
|---|---|---|---|
| State | Beta: the whole instrument plays | Work in progress: the voice is nearly complete, effects partly | Work in progress: oscillators and filter done, most modulation to come |
| Scope | One sound per instance, optional extra voices | One Single per instance, a voice limit, no Multi | One program per instance (to confirm), no performances yet |
| Hardware link | Built: follows the sound, backs up the instrument's sounds, stores one sound to a slot | Built: follows the sound; bank backup and restore built, not yet tried | Built: follows the sound, reads a backup |

## Across all of them

Missing everywhere, or built in one and wanted in the others:

- **Level meters at stages** (oscillators, filter, effects, output), to watch gain staging. G2Alike has them; none of the Alikes do.
- **External audio into the voices.** Pulse2Alike's osc 3 EXT is built (side-chain, 2026-11-06); Sn2Alike's Audio In oscillators (they play as a Saw); G2Alike's side-chain input is the model.
- **Named drop-downs** for modulation sources and destinations, waves and other numbered choices (TiAlike, Sn2Alike).
- **Value text in real units** (times, rates, semitones) where the laws are measured; the panels show raw values now.
- **Writing sounds back to the instrument.** Pulse2Alike's Store to Slot is built (a test store to P450 read back), Restore Presets waits; TiAlike's bank write is built but untried; Sn2Alike only reads.
- **Arithmetic option** (float or the instrument's fixed point, per instance): TiAlike has the filter's biquad; Sn2Alike to follow.
- **Panel pages, in common** (CT 2026-11-06): OSC, FILTER, AMP, ENV, LFO, MOD, then the instrument's own pages (ARP, TABLES beside OSC, the effects), GLOBAL always last. Short names as on the instruments' panels; a new Alike follows the same.
- **The panel code** (`p2Draw.c`, copied as `tiDraw.c` and `sn2Draw.c`) is three near-copies: a fix in one probably belongs in all three until it moves into SynthLib.

## Pulse2Alike

**Missing**
- Restore Presets to Pulse 2 (flash writes, waiting for a go-ahead). Store Sound to Pulse 2 Slot is built (2026-11-06, P001-P499; P500 is the random generator and is never written), not yet tried from a host.

**Needs more work**
- **Drive.** Mono, as on the instrument (one circuit ahead of the two VCAs; `code-notes/p2Plugin.md` §60). The model is at its limit: a rebuild as one gain stage plus a fixed clipper per curve is planned (`code-notes/p2Drive.md` §9). FUZZ is close by ear at low and middle Drive, not at 127, and its even harmonics do not grow with Drive; quiet inputs are not lifted as on the instrument. TUBE at high Drive is a near-switch the present chain cannot make, and is 10 dB loud on noise.
- **Filter.** LP12's stopband 2.5 dB high at 1-2.5 kHz at cutoff 0; level sweeps for its nonlinearity and lower-level resonance sweeps still to take.
- **Oscillators.** Levels against pitch and Level now within 0.4 dB (2026-11-06); the toggle flip (an anomaly) built for the pulse. The triangle at full Level (too rounded at the top, low odd harmonics too strong); pulses at full Level 2.4 dB bright at 16 kHz; the noise now matches at every Level (2026-11-06); the saw's reset and ramp shape against pitch; APW mixed with other sources; X3-PWM with sync.
- **Osc 3 routings.** LEVEL2 1.5-4 dB loud below osc 2 Level 101; the VCF, DRIVE and FB depths are guesses beyond what was measured; FB alone does not sing as the instrument's does.
- **Envelopes and VCA.** Short filter-envelope attacks; the first ~4 ms of a note over a release; the release tail 3-4 dB low at low volume; the VCA thump at Decay 0 too big.
- **Modulation.** The source and target lists match the instrument's; CONTROL-X is built (2026-11-06). The math sources (M-DELAY, M-SMOOTH, M-PRODUCT/MIN/MAX) want captures; the less-used targets want spot checks; Amp Velocity's law away from 64 and 127.
- **Arpeggiator.** What Arp Pattern selects; the SOFT and HARD velocity offsets.
- **Sounds by ear:** P001-P015 close (CT 2026-11-06) apart from P003, the only one with drive on; P016 onwards still to
  hear. The measured residues of the close ones are parked in `todo.md`.

**Built, not yet checked** - the full list is `to-test.md`. The big ones: the plug-in in a real host end to end (load, panel, menus, presets, program changes, project reopen); Drive by ear; the arpeggiator against the instrument; Link and the backup in Live; extra voices through one shared drive.

## TiAlike

**Missing**
- **Effects:** Chorus types other than Classic; Delay Color, Tape types and modulation; Reverb's Feedback modes and Clock; the Distortion Wave Shaper; Character types other than Analog Boost.
- **Arpeggiator, vocoder, input follower, soft knobs:** not offered (the arpeggiator last, if at all).
- **Matrix routes:** LFO 1 to Filter Gain, the Unison destinations, Osc 3 Detune, and the oscillator models' F-Shift and F-Spread.
- **Loading custom wavetables** from common file formats.
- **Fixed point** beyond the filter (oscillators, envelopes, mix, saturation, ladder): a sample-exact null needs all of them.

**Needs more work**
- **Smooth Mode settled on the hardware (2026-11-06):** no mode glides; Off/On/Auto/Note act at once, the Quantise values hold a change to the next tempo division - built.
- **Oscillator models** (Wave PWM, Grain, Formant, built 2026-11-02): with Detune on, the Complex models make a half-frequency component 24 dB down that the reference does not; high Formant or Sync on rich tables keeps up to 2 dB too much at the top. Wavetables 7, 35 and 77 are missing content above the 64th partial.
- **Oscillators generally:** start phases (free-running and fixed) do not yet line up for a null; a cent flat when free-running and 0.3 cents sharp otherwise; about 6 dB down at note 108; Ring at the same pitch; HyperSaw's cross-sync frequency.
- **Filters:** Serial 6's second section carries no resonance (close but not exact above 64); the analog modes' self-oscillation pitch (0.9% low) and high-frequency floor; Osc Volume's upper half into the analog filter.
- **Saturation:** curves 6-14 approximate or blended, their key follow missing; the Rate and Bit Reducers at high drive.
- **Effects built but approximate:** Distortion Mint, Treble Booster, High Cut, Tone; the Phaser's Spread and feedback extremes; the Filter Bank's in-between shapes, Vowel at high resonance, Stereo Phase; EQ's mid cut too wide; the Reverb's top octave 4-6 dB hot; Delay Clock's times wander with tempo.
- **Punch and note starts:** Punch's decay law and sign cases; the first millisecond of a note; late release.
- **Worst library Singles** still unexplained: Theory, RuffBelsBC, DRY DRUM.

**Built, not yet checked** - `to-test.md`: the first build in a host end to end; Link and Filter Link; patch loading and presets; the envelopes and Overdrives by ear; the Virus TI menu's bank backup and restore with the instrument connected.

## Sn2Alike

**Missing**
- **Oscillator:** Sync, Hardness, Skew, Formant Width; the audio-input types.
- **Filter:** Special Width.
- **Modulation:** FM; LFO routes to level, Soften, sync and resonance; LFO speed from Env 3, wheel and pressure; LFO tempo sync; envelope routes to pitch, width, level and resonance; Env 2 Delay; Env 2 and 3 generally.
- **Effects:** the comb, Pan Type, phaser types, the chorus feedback law, the Gated reverbs' gates.
- **Performances:** not read at all.
- **Panel:** only the Filter page is finished; dials for the LFO routes, bend and wheel.
- **Arithmetic option** as TiAlike's.

**Needs more work**
- **Oscillator stretch law:** nulls stop near -31 dB.
- **Level and peaks:** voices peak about 3 dB over the instrument and chords about 1.7x lower in crest factor, so Distortion and the output ceiling are driven differently; A001 22-26 dB dark above 3 kHz; Distortion at low Volume too clean; Volume 127 with Distortion on 2.5 dB short.
- **Envelopes:** Sustain Time's curve; the Trigger values beyond Retrigger and Legato; Glissando and the fixed Glide types.
- **Glide** (2026-11-02, from the instrument by ear): Normal glides with a note held and starts from the pitch reached, mid-glide or not - built. In poly each voice seems to glide from its own last pitch: built that way, still to confirm.
- **Odd sounds:** SuperStrings1 about 18 dB loud and decaying while held; the Vanilla group 3 dB quiet; a handful of outliers (TB1, BubbleArp, China Cymbal, Ride 2, Timpani and others).
- **Voice pool:** notes sum as on the instrument when unison is off (checked on the hardware, 2026-11-02), but the instrument has a fixed number of voices (24 on the unit measured) and heavy unison runs out of them, so its chords grow much less; this plug-in has no pool, so unison chords come out louder.
- **Checks owed:** Filter Overdrive and Q Normalise against the reference again.

**Built, not yet checked** - `to-test.md`: almost everything outside the filter and oscillators has only been run offline: the Filter page in a host, program changes, mono and legato, bend and wheel, the LFO page, Link, and Read from Hardware.

## Nl3Alike

Started 2026-11-02 from Sn2Alike's skeleton, renamed: it builds, validates as an Audio Unit and opens, playing
Sn2Alike's engine as a placeholder. **Everything Nord Lead 3 is missing**: the program format and parameters, the
engine (oscillators with FM and sync, filter, envelopes, LFOs, morph), the panel's pages, the MIDI link's own messages,
and reading the instrument's programs. See `Nl3Alike/Docs/todo.md`.

