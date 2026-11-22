# UltraAlike

A reproduction inspired by the E-mu Ultra samplers (the EOS 4 generation: E4XT Ultra, E5000 Ultra, E6400 Ultra and
their kin), to be a SynthLib plug-in (VST3 + AU) like the other Alikes.

**Alpha.** One preset or sample an instance. It opens:

- EOS banks (`.E4B`)
- Emulator X banks (`.exb`, inside the bank's folder beside its samples)
- SFZ instruments (`.sfz`)
- loose samples: WAV, and AIFF/AIFC

Prev and Next step through a bank's presets and on into the folder's next bank. Zones by key and velocity, loops, release
samples, level and pan play; the amplifier envelope and, with Anti-alias Off, the interpolation are the E5000's, measured.
Still to come: filters, LFOs, the modulation cords and crossfades. See `Docs/design.md` and `Docs/todo.md`.

GPLv3. See [LICENSE](../LICENSE).
