#!/usr/bin/python3
# p2Drive notes §10: the drive with its coupling loop held at the clipper's resting output (drive_model.c drive_run4),
# so a fixed bias ahead of the clipper survives. Fitted to drive_wave.py's 55 sweep takes (averaged cycles and
# harmonics) AND to the two-tone takes (192k/drive_imd/: even and odd products per step at Drive 0/32/64/127, the
# model run on the NONE take). Shape (8), gain and trim per Drive (5 + 5). Reports cycles, harmonics and products
# against the §8 engine. Stage 'expand' fits drive_run5 instead: a cubic ahead of the clipper (shape[8]); 'deadzone'
# drive_run6: the cubic and a one-sided dead zone (shape[9]), starting from a saved fit (P2_FROM=<state.json>).
# 'hysteresis' (drive_run7): feedback h around the clipper (shape[10]), from P2_FROM, starting at P2_START_H.
#   P2_HYST_GAIN=1: drive_run8, the feedback ahead of the gain (loop gain h x gain).
# usage: drive_fit2.py <curve 1=TUBE 2=FUZZ> [fit|expand|deadzone|hysteresis|score] [state.json]
import ctypes, json, os, sys, tempfile

import numpy as np
from scipy.optimize import least_squares

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import drive_shape as ds
import drive_wave as dw
import drive_ota as do
import drive_imd_analyse as di
from drive_shape import harmonics, residual, LIB, DP
from osc_analyse import read

IMD = os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/drive_imd'
IMD_DRIVES = (0, 32, 64, 127)
IMD_WEIGHT = 0.5
QUIET = os.environ.get('P2_CAPTURES', '/Volumes/ext-ssd/Captures/Pulse2') + '/192k/drive_quiet2'
EXTRA_WEIGHT = float(os.environ.get('P2_EXTRA_WEIGHT', '0.5'))   # p2Drive notes §11: FUZZ's quiet single tones (H2, H3) and TUBE's noise bands, dB


def extra_targets(curve):
    from drive_quiet2_analyse import steps
    none_one, r = read(os.path.join(QUIET, 'one_c0_d0.wav')); none_noise, _ = read(os.path.join(QUIET, 'noise_c0_d0.wav'))
    if curve == 2:
        ins = steps(none_one[:, 0], r, 4, 1.0)
        hw = {d: [[di.level(s, r, 2000) - di.level(s, r, 1000), di.level(s, r, 3000) - di.level(s, r, 1000)]
                  for s in steps(read(os.path.join(QUIET, 'one_c2_d%d.wav' % d))[0][:, 0], r, 4, 1.0)] for d in (0, 17, 64)}
    else:
        ins = steps(none_noise[:, 0], r, 2, 1.5)
        hw = {d: [noise_bands(s, r) for s in steps(read(os.path.join(QUIET, 'noise_c1_d%d.wav' % d))[0][:, 0], r, 2, 1.5)] for d in (0, 64, 127)}
    return ins, hw, r


NOISE_CENT = (125, 250, 500, 1000, 2000, 4000, 8000)


def noise_bands(x, r):
    sp = np.abs(np.fft.rfft(x * np.hanning(len(x)))) ** 2
    f = np.fft.rfftfreq(len(x), 1 / r)
    return [10 * np.log10(sp[(f >= c / 2 ** 0.5) & (f < c * 2 ** 0.5)].mean()) for c in NOISE_CENT]


def extra_res(curve, shape, gains, trims, ins, hw, r):
    out = []
    for d, rows in hw.items():
        g = float(np.interp(d, ds.DRIVES, gains)); t = float(np.interp(d, ds.DRIVES, trims))
        for x, want in zip(ins, rows):
            y = run4(np.concatenate([x[:int(0.2 * r)], x]), r, shape, g, t)[int(0.2 * r):]    # 0.2 s to settle first
            if curve == 2:
                got = [di.level(y, r, 2000) - di.level(y, r, 1000), di.level(y, r, 3000) - di.level(y, r, 1000)]
            else:
                got = noise_bands(y, r)
            out += [np.clip(a - b, -20, 20) * EXTRA_WEIGHT for a, b in zip(got, want)]
    return np.array(out)

LIB.drive_run4.argtypes = [DP, ctypes.c_int, ctypes.c_double, DP, ctypes.c_double, ctypes.c_double, DP]
LIB.drive_run5.argtypes = LIB.drive_run4.argtypes
LIB.drive_run6.argtypes = LIB.drive_run4.argtypes
LIB.drive_run7.argtypes = LIB.drive_run4.argtypes
LIB.drive_run8.argtypes = LIB.drive_run4.argtypes
HYST_GAIN = os.environ.get('P2_HYST_GAIN') == '1'   # feedback ahead of the gain (drive_run8)
EXPAND = [False]


def run4(x, rate, shape, gain_db, trim_db):
    x = np.ascontiguousarray(x, dtype=np.float64)
    out = np.empty_like(x)
    sh = np.ascontiguousarray(shape, dtype=np.float64)
    ((LIB.drive_run8 if HYST_GAIN else LIB.drive_run7) if len(shape) > 10 else LIB.drive_run6 if len(shape) > 9 else (LIB.drive_run5 if len(shape) > 8 else LIB.drive_run4))(x.ctypes.data_as(DP), len(x), rate, sh.ctypes.data_as(DP), 10 ** (gain_db / 20), trim_db,
                                                          out.ctypes.data_as(DP))
    return out


def imd_targets(curve):
    none, r = read(os.path.join(IMD, 'imd_c0_d0.wav'))
    none = none[:, 0]
    a = int(np.argmax(np.abs(none) > 1e-3))
    ins = [none[a + int((k + 0.05) * r):a + int((k + 0.75) * r)] for k in range(len(di.STEPS))]   # 0.25 s settling first
    hw = {}
    for d in IMD_DRIVES:
        h, _ = read(os.path.join(IMD, 'imd_c%d_d%d.wav' % (curve, d)))
        hw[d] = [di.products(s, r)[0] for s in di.steps(h[:, 0], r)]
    return ins, hw, r


def imd_res(shape, gains, trims, ins, hw, r):
    out = []
    for d in IMD_DRIVES:
        i = ds.DRIVES.index(d) if d in ds.DRIVES else None
        g = float(np.interp(d, ds.DRIVES, gains)); t = float(np.interp(d, ds.DRIVES, trims))
        for k, x in enumerate(ins):
            y = run4(x, r, shape, g, t)[int(0.25 * r):]
            m = di.products(y, r)[0]
            out += [np.clip(m[0] - hw[d][k][0], -20, 20) * IMD_WEIGHT, np.clip(m[1] - hw[d][k][1], -20, 20) * IMD_WEIGHT]
    return np.array(out)


def take_res(row, shape, g, t):
    d, _, _, span, _, f0, rate, a, b = row['t']
    y = (run4(span, rate, shape, g, t) * row['gate'])[a:b]
    mc = dw.aligned(dw.cycle(y, rate, f0), row['hw'])
    peak = max(1e-9, np.max(np.abs(row['hw'])))
    wave = (mc - row['hw']) / peak * dw.WAVE_WEIGHT / np.sqrt(dw.N / 16)
    return wave, residual(row['t'], harmonics(y, rate, f0))


def scores(rows, shape, gains, trims):
    w, h = [], []
    for r in rows:
        i = ds.DRIVES.index(r['drive'])
        a, b = take_res(r, shape, gains[i], trims[i])
        w.append(np.sqrt(np.mean((a / dw.WAVE_WEIGHT * np.sqrt(dw.N / 16)) ** 2)) * 100)
        h.append(np.sqrt(np.mean(b ** 2)))
    return np.mean(w), np.mean(h)


def imd_table(label, shape, gains, trims, ins, hw, r, model=True):
    for d in IMD_DRIVES:
        if model:
            g = float(np.interp(d, ds.DRIVES, gains)); t = float(np.interp(d, ds.DRIVES, trims))
            rows = [di.products(run4(x, r, shape, g, t)[int(0.25 * r):], r)[0] for x in ins]
        else:
            rows = hw[d]
        print('  %-7s Drive %3d: %s' % (label, d, '  '.join('%+4.0f/%+4.0f' % tuple(p) for p in rows)))


def main():
    curve = int(sys.argv[1])
    stage = sys.argv[2] if len(sys.argv) > 2 else 'fit'
    saved = sys.argv[3] if len(sys.argv) > 3 else os.path.join(tempfile.gettempdir(), 'p2_drive_fit2_%d.json' % curve)
    ds.APPLY_GATE[0] = True
    rows = dw.build(curve)
    ins, hw, r = imd_targets(curve)
    use_extra = os.environ.get('P2_EXTRA') == '1'
    if use_extra:
        xins, xhw, _ = extra_targets(curve)
    sh8, g6, t6 = do.ENGINE8[curve]
    g0 = [float(np.interp(d, dw.P2DRIVE_AT, g6)) for d in ds.DRIVES]
    t0 = [float(np.interp(d, dw.P2DRIVE_AT, t6)) for d in ds.DRIVES]
    w8, h8, _, _ = dw.scores(rows, sh8, g0, t0)
    print('engine (§8): cycle error %.1f%% of peak, harmonic rms %.2f dB' % (w8, h8), flush=True)
    n = len(ds.DRIVES)
    EXPAND[0] = (stage == 'expand')
    dead = (stage == 'deadzone')
    hyst = (stage == 'hysteresis')
    k = 11 if hyst else (10 if dead else (9 if EXPAND[0] else 8))
    if stage == 'score':
        st = json.load(open(saved)); shape, gains, trims = st['shape'], st['gains'], st['trims']
    else:
        start = list(sh8) + ([0.3] if EXPAND[0] else []); start[2] = float(os.environ.get('P2_START_BIAS', '0'))   # a start away from 0: the fit stays near where it begins
        lo = [0.005, 0.005, -0.2, 1.0, 0.0, 20.0, 0.5, 0.5] + ([-0.3] if EXPAND[0] else []) + [-20.0] * n + [-12.0] * n
        hi = [0.5, 0.5, 0.2, 3000.0, 1.0, 20000.0, 30.0, 30.0] + ([5.0] if EXPAND[0] else []) + [60.0] * n + [12.0] * n
        if dead and os.environ.get('P2_FROM_FULL'):   # continue a saved deadzone fit as it is
            st = json.load(open(os.environ['P2_FROM_FULL']))
            start, g0, t0 = list(st['shape']), st['gains'], st['trims']
            lo = [0.005, 0.005, -0.2, 1.0, 0.0, 20.0, 0.5, 0.5, -0.3, -0.2] + [-20.0] * n + [-12.0] * n
            hi = [0.5, 0.5, 0.2, 3000.0, 1.0, 20000.0, 30.0, 30.0, 5.0, 0.2] + [60.0] * n + [12.0] * n
            start = [min(max(v, l + 1e-6), u - 1e-6) for v, l, u in zip(start, lo, hi)]
        elif dead:   # from a saved fit, with no cubic yet and the dead zone's width as given (sign = side)
            st = json.load(open(os.environ['P2_FROM']))
            start = list(st['shape'][:8]) + [st['shape'][8] if len(st['shape']) > 8 else 0.0, float(os.environ.get('P2_START_W', '0.01'))]
            g0, t0 = st['gains'], st['trims']
            lo = [0.005, 0.005, -0.2, 1.0, 0.0, 20.0, 0.5, 0.5, -0.3, -0.2] + [-20.0] * n + [-12.0] * n
            hi = [0.5, 0.5, 0.2, 3000.0, 1.0, 20000.0, 30.0, 30.0, 5.0, 0.2] + [60.0] * n + [12.0] * n
        if hyst:   # p2Drive notes §12: from a saved fit, plus feedback h around the clipper (P2_START_H)
            st = json.load(open(os.environ['P2_FROM']))
            sh = list(st['shape']) + [0.0] * (10 - len(st['shape']))
            start = sh[:10] + [float(os.environ.get('P2_START_H', '0.5'))]
            g0, t0 = st['gains'], st['trims']
            lo = [0.005, 0.005, -0.2, 1.0, 0.0, 20.0, 0.5, 0.5, -0.3, -0.2, 0.0] + [-20.0] * n + [-12.0] * n
            hi = [0.5, 0.5, 0.2, 3000.0, 1.0, 20000.0, 30.0, 30.0, 5.0, 0.2, 3.0] + [60.0] * n + [12.0] * n
        if os.environ.get('P2_NO_SHELF') == '1':   # the shelf's direct share held at 1: all straight through
            lo[4], hi[4], start[4] = 0.995, 1.0, 0.9975
        p0 = [min(max(v, l + 1e-6), u - 1e-6) for v, l, u in zip(start + g0 + t0, lo, hi)]

        def f(p):
            shape, gains, trims = list(p[:k]), list(p[k:k + n]), list(p[k + n:])
            out = []
            for rw in rows:
                i = ds.DRIVES.index(rw['drive'])
                a, b = take_res(rw, shape, gains[i], trims[i])
                out += [a, b]
            out.append(imd_res(shape, gains, trims, ins, hw, r))
            if use_extra:
                out.append(extra_res(curve, shape, gains, trims, xins, xhw, r))
            return np.concatenate(out)
        fit = least_squares(f, p0, bounds=(lo, hi), x_scale='jac', diff_step=1e-3, max_nfev=300)
        shape, gains, trims = list(fit.x[:k]), list(fit.x[k:k + n]), list(fit.x[k + n:])
        json.dump({'shape': shape, 'gains': gains, 'trims': trims}, open(saved, 'w'))
        print('saved', saved)
    if use_extra:
        print('quiet tones / noise residual rms: %.2f dB' % (np.sqrt(np.mean((extra_res(curve, shape, gains, trims, xins, xhw, r) / EXTRA_WEIGHT) ** 2))))
    w, h = scores(rows, shape, gains, trims)
    print('§10 fit:     cycle error %.1f%% of peak, harmonic rms %.2f dB' % (w, h))
    print('  shape', np.round(shape, 4).tolist(), '\n  gains', np.round(gains, 2).tolist(), '\n  trims', np.round(trims, 2).tolist())
    print('two-tone even/odd products, steps -36..-12 dBFS:')
    for d in IMD_DRIVES:
        print('  Pulse 2 Drive %3d: %s' % (d, '  '.join('%+4.0f/%+4.0f' % tuple(p) for p in hw[d])))
        g = float(np.interp(d, ds.DRIVES, gains)); t = float(np.interp(d, ds.DRIVES, trims))
        print('  model   Drive %3d: %s' % (d, '  '.join('%+4.0f/%+4.0f' % tuple(di.products(run4(x, r, shape, g, t)[int(0.25 * r):], r)[0]) for x in ins)))


if __name__ == '__main__':
    main()
