# ProphAlike todo

- Replace the placeholder oscillator with the Standard set (saw, pulse), then the level into the filter (design: Status)
- Amp EG times and segment curves (approximate now: envelope notes §10)
- Filter 2, filter routing and Input Trim (Filter 1 is done: filter notes)
- Cutoff law at fine resolution for modulation (the instrument computes it; the 100-step table is interpolated now)
- Cutoff key tracking, EG, LFO and modulation-source inputs to the filters
- The mixer, the amp stage before it and the output stage
- Fixed-point and anti-aliased floating-point paths for each new stage, as the filter has (Maths, Anti-alias)
- Define a repeatable Prophecy capture set and document its calibration and signal path
- Replace inherited program handling with a Prophecy-appropriate state and patch workflow
- Rebuild the remaining panel pages around the Prophecy signal structure (OSC, FILTER, AMP EG and GLOBAL are up)
