# nl3Effects.c - the effects section

## 1. Dry
With effects on, the dry path is linear in its dial and 3 dB above the bypassed signal at full.

## 2. Chorus
Each side has two delay taps and no dry of its own. Each tap sweeps a triangle, from about 0.05 ms up by 0.12 ms per step of Depth, and the two run at different rates, a ratio of 1.18, so the sweep does not repeat. Speed sets the period, measured at three points and joined in the logarithm. Quad Chorus sits 0.2 ms later than Chorus/Flanger. Send is linear.

## 3. Not settled
Feedback is an estimate. Phaser plays as a chorus. Starting phases are those of one render.

## 4. Delay
One line with a tap for each side, 6.592 ms per step of Time, up to 838.8 ms. Ratio shortens one side's tap, and the stored byte is its index + 96. The feedback always comes from the full time. Sync sets the time from the tempo instead, capped at the same length; with no tempo from the host it uses 122 BPM. Send is linear and a first echo at full matches the dry level. Width cross-mixes the taps: each side also hears 1 - Width / 127 of the other. Feedback is Feedback / 128 a repeat, through one pole of low pass set by HF Damp, so the first echo is not damped. The Global Delay switch stops new sound entering, so a tail rings out.

## 5. Reverb
A model, not the instrument's own network: eight lines around a Householder feedback, shaped to measurements of each type. The decay time at four settings of Decay and the time to the fullest point are measured by type and joined in the logarithm; Send is linear. HF Damp shortens the top of the band and leaves the low end, by one pole in the loop chosen from the wanted loss. Early Reflections measured no effect and is not built. The four Gated types use a short decay with no gate yet. The Global Reverb switch stops new sound entering, so a tail rings out.

## 6. Order
On the instrument, playing a program, the effects run side by side whatever order the program stores: measured over six orders with delay and chorus, the echo never changed and a chorus send of 0 never silenced the delay. So a program's order is not read. The Order control is an addition, off at D+R+C: it connects the effects as the names say, each fed by the wet output before it at its own send, and those gains are a choice, not a measurement.

## 7. Distortion
On the voices' mix, after the filter and ahead of the effects, so a chord intermodulates. A soft clip towards a fixed ceiling, its drive and softness measured at six Levels: gentle below 60, a hard clip at 127. Gain Compensation scales with the drive, so it does nothing at Level 0. Output Level is Output / 64 at every Level. Curve measured no effect.

## 8. EQ
Two first-order shelves on the output, about 500 Hz and 2.2 kHz. Bass at 0 removes the shelf's whole band, so it is a high pass.

## 7a. Distortion, replacing the curve in §7
Measured sample against sample, a saw bypassed against through the stage. The voices meet one curve, `c k x / (1 + |k x|^s)^(1/s)`, with k, s and c read at each Level, within 0.0015 at every one. Read with a saw below 0.42 and a dense chord above it, to the top of the range: a saw alone stops at 0.47 and its end looks like a limit that is not there. At Level 0 the line runs straight to the output's ceiling. Output and Dry are plain gains after it. The curve is normalised to 1.93 at Level 0, which is only its own scale: on the instrument, effects on is 3.07 dB above bypass, 1.4227 times, and that is the Dry gain. Compensation keeps its law from the instrument: a level after the curve.
The instrument gets louder with Level, by the table in the code, at Compensation 64: measured against the instrument, as the curve's own level was read with Compensation out of the path.

EQ, replacing §8's points, measured on the instrument: Bass a shelf at 501 Hz, gain dial/64 below centre and 1 + (dial-64)/128 above, within 0.01 dB. Treble a shelf at 2650 Hz, gain and a level trim by dial, the gain slightly negative at the bottom, within 0.2 dB.
