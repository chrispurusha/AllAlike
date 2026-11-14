/*
 * UltraAlike - a sampler (a placeholder on a sample player's engine), as a VST3 and Audio Unit instrument.
 *
 * Copyright (C) 2026 Chris Turner <chris_purusha@icloud.com>
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <https://www.gnu.org/licenses/>.
 */
// Notes: Docs/code-notes/uaStretch.md - "// notes §k" refers there.

#include <algorithm>
#include <cstring>
#include <vector>

#include "rubberband/RubberBandStretcher.h"

#include "uaStretch.h"

using RubberBand::RubberBandStretcher;

#define BLOCK    (4096)

bool ua_stretch_render(const float * left, const float * right, uint32_t inFrames, double inRate, double outRate,
                       double pitchScale, float * outLeft, float * outRight, uint32_t outFrames,
                       bool (*cancelled)(void * user), void * user) {
    if ((inFrames == 0) || (outFrames == 0)) {
        return false;
    }
    size_t channels = (right != nullptr) ? 2 : 1;

    try
    {
        // notes §2: offline (two passes), the R3 engine, smooth and transient-keeping
        RubberBandStretcher rb((size_t)inRate, channels,
                               RubberBandStretcher::OptionProcessOffline | RubberBandStretcher::OptionEngineFiner |
                               RubberBandStretcher::OptionPitchHighQuality | RubberBandStretcher::OptionChannelsTogether);

        // notes §3: the time ratio in frames, so out at outRate lasts as asked; the pitch corrected for the rate
        rb.setTimeRatio((double)outFrames / (double)inFrames);
        rb.setPitchScale(pitchScale * inRate / outRate);
        rb.setExpectedInputDuration(inFrames);

        const float *       in[2];

        for (uint32_t at = 0; at < inFrames; at += BLOCK) {
            size_t n = std::min<size_t>(BLOCK, inFrames - at);

            if ((cancelled != nullptr) && cancelled(user)) {
                return false;
            }
            in[0] = left + at;
            in[1] = (right != nullptr) ? right + at : nullptr;
            rb.study(in, n, (at + n) >= inFrames);
        }

        std::vector<float>  scratch[2];
        float *             out[2];
        uint32_t            written = 0;

        auto                drain   = [&](void) {
            int available;

            while ((available = rb.available()) > 0) {
                size_t n    = (size_t)available;

                for (size_t c = 0; c < channels; c++) {
                    scratch[c].resize(n);
                    out[c] = scratch[c].data();
                }

                n        = rb.retrieve(out, n);

                size_t keep = std::min<size_t>(n, outFrames - written);

                std::memcpy(outLeft + written, out[0], keep * sizeof(float));

                if (outRight != nullptr) {
                    std::memcpy(outRight + written, out[channels - 1], keep * sizeof(float));
                }
                written += (uint32_t)keep;
            }
        };

        for (uint32_t at = 0; at < inFrames; at += BLOCK) {
            size_t n = std::min<size_t>(BLOCK, inFrames - at);

            if ((cancelled != nullptr) && cancelled(user)) {
                return false;
            }
            in[0] = left + at;
            in[1] = (right != nullptr) ? right + at : nullptr;
            rb.process(in, n, (at + n) >= inFrames);
            drain();
        }

        drain();

        // the ratio is met to within a few frames; the rest is silence
        std::memset(outLeft + written, 0, (outFrames - written) * sizeof(float));

        if (outRight != nullptr) {
            std::memset(outRight + written, 0, (outFrames - written) * sizeof(float));
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}
