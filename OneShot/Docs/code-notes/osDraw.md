# osDraw.c

## 1. The panel and its instance
The panel is shared by every instance in the process (SynthLib's panel model); `os_draw_set_user()` names the one
being drawn on each sync, and the panel reaches its sample only through `tOsSampleHost`.

## 2. Prev / Next
Step through the audio files in the current sample's folder, sorted case-insensitively, wrapping. A file that will
not load is passed over silently. The folder is listed again only when it changes. With no sample, Prev/Next opens
the file browser.

## 3. Waveform
The 512-point min/max overview computed at load (osSample), of the left and right averaged. A click on it plays the sample.

## 4. Keys
With no popup open: Left/Up previous, Right/Down next, Space plays. A host may keep some of these for itself.

## 5. The envelope on the waveform
The envelope is stepped over the sample's own length (32 steps a column, so a short attack shows) as though the note
were held throughout. In Gate mode a nominal note off is marked halfway along and the release drawn from it, so the
Release dials show; the real release comes at the real note off. The original sample is drawn dim, the sample
shaped by the envelope over it, and the envelope as a green line. Tune and Keys are not taken into account.

## 6. Start on the waveform
A yellow line where the note starts - snapped as the voice snaps it, through the same `os_voices_start_frame()` - with
the part before it dim and the envelope starting there. Gate's nominal note off is halfway from the start to the end.
The controls are 50 units wide so that PITCH, PLAY and OUTPUT (eleven controls) fit one row.
