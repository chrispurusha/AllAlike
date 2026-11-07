# Fits the drive's top ceiling against its bottom per Drive (p2Drive notes §13): from a saved drive_fit2.py fit
# (P2_FROM), every shape value, gain and trim free, plus ratio[i] dB scaling the top ceiling at each of ds.DRIVES.
# P2_EXTRA=1 adds drive_fit2's extra targets (TUBE: noise bands); P2_NO_SHELF=1 holds the shelf open; a P2_FROM
# with 'ratios' starts from them.
# P2_PER=bias: a bias offset per Drive instead; P2_PER=coupling: the coupling frequency x 10^(v/20) per Drive.
# usage: drive_fit_ratio.py <curve 1=TUBE 2=FUZZ> <out.json>; 'score' as a third argument scores out.json per Drive.
import json, os, sys

import numpy as np
from scipy.optimize import least_squares

import drive_fit2 as f
import drive_shape as ds
import drive_wave as dw
import drive_imd_analyse as di


PER = os.environ.get('P2_PER', 'ratio')   # 'ratio': the top ceiling x 10^(v/20); 'bias': v added to the bias


EXTRA = None
CURVE = [1]


def with_ratio(shape, v):
    s = list(shape)
    if PER == 'bias':
        s[2] += v
    elif PER == 'coupling':
        s[3] *= 10 ** (v / 20)
    else:
        s[0] *= 10 ** (v / 20)
    return s


def per_drive(d, values):
    return float(np.interp(d, ds.DRIVES, values))


def residuals(rows, shape, gains, trims, ratios, ins, hw, r):
    out = []
    for rw in rows:
        i = ds.DRIVES.index(rw['drive'])
        a, b = f.take_res(rw, with_ratio(shape, ratios[i]), gains[i], trims[i])
        out += [a, b]
    for d in f.IMD_DRIVES:
        sh = with_ratio(shape, per_drive(d, ratios))
        g, t = per_drive(d, gains), per_drive(d, trims)
        for k, x in enumerate(ins):
            m = di.products(f.run4(x, r, sh, g, t)[int(0.25 * r):], r)[0]
            out += [np.clip(m[0] - hw[d][k][0], -20, 20) * f.IMD_WEIGHT, np.clip(m[1] - hw[d][k][1], -20, 20) * f.IMD_WEIGHT]
    if EXTRA is not None:   # P2_EXTRA=1: drive_fit2's quiet tones (FUZZ) or noise bands (TUBE), each Drive's ratio applied
        xins, xhw, xr = EXTRA
        for d, want_rows in xhw.items():
            out.append(f.extra_res(CURVE[0], with_ratio(shape, per_drive(d, ratios)), [per_drive(d, gains)] * len(ds.DRIVES),
                                   [per_drive(d, trims)] * len(ds.DRIVES), xins, {d: want_rows}, xr))
    return np.concatenate([np.atleast_1d(o) for o in out])


def report(rows, shape, gains, trims, ratios):
    per = {}
    for rw in rows:
        i = ds.DRIVES.index(rw['drive'])
        a, b = f.take_res(rw, with_ratio(shape, ratios[i]), gains[i], trims[i])
        per.setdefault(rw['drive'], []).append((np.sqrt(np.mean((a / dw.WAVE_WEIGHT * np.sqrt(dw.N / 16)) ** 2)) * 100, np.sqrt(np.mean(b ** 2))))
    allv = [v for vs in per.values() for v in vs]
    print('cycle error %.1f%% of peak, harmonic rms %.2f dB' % (np.mean([v[0] for v in allv]), np.mean([v[1] for v in allv])))
    print('  ' + '  '.join('D%d %.1f%%/%.2f' % (d, np.mean([v[0] for v in vs]), np.mean([v[1] for v in vs])) for d, vs in sorted(per.items())), flush=True)


def main():
    curve, saved = int(sys.argv[1]), sys.argv[2]
    ds.APPLY_GATE[0] = True
    global EXTRA
    rows = dw.build(curve)
    ins, hw, r = f.imd_targets(curve)
    n = len(ds.DRIVES)
    CURVE[0] = curve
    if os.environ.get('P2_EXTRA') == '1':
        EXTRA = f.extra_targets(curve)
    if len(sys.argv) > 3:
        st = json.load(open(saved))
        report(rows, st['shape'], st['gains'], st['trims'], st['ratios'])
        return
    st = json.load(open(os.environ['P2_FROM']))
    shape, gains, trims = list(st['shape']), list(st['gains']), list(st['trims'])
    k = len(shape)
    print('from:', end=' '); report(rows, shape, gains, trims, list(st.get('ratios', [0.0] * n)))
    if EXTRA is not None:
        print('  noise/quiet residual rms %.2f dB' % np.sqrt(np.mean((np.concatenate([f.extra_res(curve, with_ratio(shape, per_drive(d, st.get('ratios', [0.0] * n))), [per_drive(d, gains)] * n, [per_drive(d, trims)] * n, EXTRA[0], {d: w}, EXTRA[2]) for d, w in EXTRA[1].items()]) / f.EXTRA_WEIGHT) ** 2)), flush=True)
    lo = [0.005, 0.005, -0.2, 1.0, 0.0, 20.0, 0.5, 0.5, -0.3, -0.2][:k] + [-20.0] * n + [-12.0] * n + ([-0.1] if PER == 'bias' else [-12.0]) * n
    hi = [0.5, 0.5, 0.2, 3000.0, 1.0, 20000.0, 30.0, 30.0, 5.0, 0.2][:k] + [60.0] * n + [12.0] * n + ([0.1] if PER == 'bias' else [12.0]) * n
    r0 = list(st.get('ratios', [0.0] * n))
    if os.environ.get('P2_NO_SHELF') == '1':   # the shelf's direct share held at 1: all straight through
        lo[4], hi[4], shape[4] = 0.995, 1.0, 0.9975
    p0 = [min(max(v, l + 1e-6), u - 1e-6) for v, l, u in zip(shape + gains + trims + r0, lo, hi)]

    def split(p):
        return list(p[:k]), list(p[k:k + n]), list(p[k + n:k + 2 * n]), list(p[k + 2 * n:])

    fit = least_squares(lambda p: residuals(rows, *split(p), ins, hw, r), p0, bounds=(lo, hi), x_scale='jac', diff_step=1e-3, max_nfev=300)
    shape, gains, trims, ratios = split(fit.x)
    json.dump({'shape': shape, 'gains': gains, 'trims': trims, 'ratios': ratios}, open(saved, 'w'))
    print('fit: ', end=' '); report(rows, shape, gains, trims, ratios)
    print('  shape', np.round(shape, 4).tolist(), '\n  gains', np.round(gains, 2).tolist(), '\n  trims', np.round(trims, 2).tolist(), '\n  ratios dB', np.round(ratios, 2).tolist())


if __name__ == '__main__':
    main()
