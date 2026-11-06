#!/usr/bin/python3
# Fits p2Drive notes §9's drive model (tools/drive_model.c drive_run3: an OTA-style gain stage whose gain and ceiling
# are one smooth function of Drive, then a fixed clipper per curve) to the same 55 drive_sweep/ takes and score as
# drive_wave.py (the averaged cycle and the harmonics), and reports it against the engine's §8 fit.
# usage: drive_ota.py <curve 1=TUBE 2=FUZZ> [fit|score] [state.json]
import ctypes, json, os, sys, tempfile

import numpy as np
from scipy.optimize import least_squares

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import drive_shape as ds
import drive_wave as dw
from drive_shape import harmonics, residual, LIB, DP

NAMES = ['I0', 'a', 'b', 's', 'e', 'pos', 'neg', 'hardPos', 'hardNeg', 'couplingHz', 'shelfDirect', 'shelfHz']
LO = [0.0, -12.0, -2.0, 0.05, -1.5, 0.005, 0.005, 0.5, 0.5, 1.0, 0.0, 20.0]
HI = [5.0, 8.0, 10.0, 200.0, 1.5, 0.5, 0.5, 30.0, 30.0, 3000.0, 1.0, 20000.0]
# §8's engine as the start: its gain law as a, b; its clipper and shelf
START = {
    1: [0.0, np.log(10 ** (-6.18 / 20)), np.log(10 ** (19.8 / 20)), 1.0, 0.0, 0.0742 * 1.6, 0.0319 * 1.6, 1.821, 4.019, 98.8, 0.5242, 357.4],
    2: [0.0, np.log(10 ** (15.6 / 20)), np.log(10 ** (21.8 / 20)), 1.0, 0.05, 0.0532 * 1.2, 0.0519 * 1.2, 0.976, 0.932, 627.1, 0.6738, 223.6],
}
# §8's engine, for the comparison (drive_wave's model: shape, gain per Drive, trim per Drive; bias fixed at its bound)
ENGINE8 = {
    1: ([0.0742, 0.0319, 0.6, 98.8, 0.5242, 357.4, 1.821, 4.019], [-6.18, 1.52, 5.52, 10.55, 12.11, 13.62], [5.30, 3.78, 3.07, 2.13, 2.28, 2.43]),
    2: ([0.0532, 0.0519, -0.6, 627.1, 0.6738, 223.6, 0.976, 0.932], [15.60, 22.05, 24.48, 27.86, 32.71, 37.41], [0.98, 1.18, 1.48, 1.78, 1.78, 1.78]),
}
LIB.drive_run3.argtypes = [DP, ctypes.c_int, ctypes.c_double, DP, ctypes.c_double, DP]


def run3(x, rate, p, drive):
    x = np.ascontiguousarray(x, dtype=np.float64)
    out = np.empty_like(x)
    pp = np.ascontiguousarray(p, dtype=np.float64)
    LIB.drive_run3(x.ctypes.data_as(DP), len(x), rate, pp.ctypes.data_as(DP), float(drive), out.ctypes.data_as(DP))
    return out


def take_res(row, p):
    d, _, _, span, _, f0, rate, a, b = row['t']
    y = (run3(span, rate, p, d) * row['gate'])[a:b]
    mc = dw.aligned(dw.cycle(y, rate, f0), row['hw'])
    peak = max(1e-9, np.max(np.abs(row['hw'])))
    wave = (mc - row['hw']) / peak * dw.WAVE_WEIGHT / np.sqrt(dw.N / 16)
    return wave, residual(row['t'], harmonics(y, rate, f0))


def scores(rows, p):
    w, h = [], []
    for r in rows:
        a, b = take_res(r, p)
        w.append(np.sqrt(np.mean((a / dw.WAVE_WEIGHT * np.sqrt(dw.N / 16)) ** 2)) * 100)
        h.append(np.sqrt(np.mean(b ** 2)))
    return np.mean(w), np.mean(h), w, h


def main():
    curve = int(sys.argv[1])
    stage = sys.argv[2] if len(sys.argv) > 2 else 'fit'
    saved = sys.argv[3] if len(sys.argv) > 3 else os.path.join(tempfile.gettempdir(), 'p2_drive_ota_%d.json' % curve)
    ds.APPLY_GATE[0] = True
    rows = dw.build(curve)
    sh, g6, t6 = ENGINE8[curve]
    gains = [float(np.interp(d, dw.P2DRIVE_AT, g6)) for d in ds.DRIVES]
    trims = [float(np.interp(d, dw.P2DRIVE_AT, t6)) for d in ds.DRIVES]
    w8, h8, _, _ = dw.scores(rows, sh, gains, trims)
    print('engine (§8): cycle error %.1f%% of peak, harmonic rms %.2f dB over %d takes' % (w8, h8, len(rows)), flush=True)
    if stage == 'score':
        p = json.load(open(saved))['p']
    else:
        start = [min(max(v, l + 1e-6), u - 1e-6) for v, l, u in zip(START[curve], LO, HI)]
        w0, h0, _, _ = scores(rows, start)
        print('§9 start:    cycle error %.1f%% of peak, harmonic rms %.2f dB' % (w0, h0), flush=True)

        def f(p):
            return np.concatenate([np.concatenate(take_res(r, p)) for r in rows])
        best = None
        for s in (1.0, 4.0, 16.0):                       # the stage's tanh barely bent, half, well into saturation
            st = list(start)
            st[3] = s
            st[1] = start[1] - np.log(s)                 # the same small-signal gain
            fit = least_squares(f, st, bounds=(LO, HI), x_scale='jac', diff_step=1e-3, max_nfev=300)
            print('  from s=%g: cost %.1f' % (s, fit.cost), flush=True)
            if (best is None) or (fit.cost < best.cost):
                best = fit
        p = list(best.x)
        json.dump({'p': p}, open(saved, 'w'))
        print('saved', saved)
    w, h, ws, hs = scores(rows, p)
    print('§9 fit:      cycle error %.1f%% of peak, harmonic rms %.2f dB' % (w, h))
    print('  ' + ', '.join('%s %.4g' % (k, v) for k, v in zip(NAMES, p)))
    for d in ds.DRIVES:
        sel = [i for i, r in enumerate(rows) if r['drive'] == d]
        print('  drive %3d: cycle %5.1f%%  harmonics %5.2f dB  (%d takes)' % (d, np.mean([ws[i] for i in sel]), np.mean([hs[i] for i in sel]), len(sel)))


if __name__ == '__main__':
    main()
