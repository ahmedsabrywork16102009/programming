# -*- coding: utf-8 -*-
# محرك حل: نظرية ذات الحدين + الأعداد المركبة (جبر - مستر لطفي زهران)
import sys, re, math
from fractions import Fraction
import sympy as sp
from sympy.parsing.sympy_parser import (parse_expr, standard_transformations,
                                        implicit_multiplication_application)

try:
    sys.stdout.reconfigure(encoding='utf-8')
    sys.stdin.reconfigure(encoding='utf-8')
except Exception:
    pass


# ---------------------------------------------------------------- عرض عربي يمين لشمال
# لو المكتبتين مش متثبتين: pip install arabic-reshaper python-bidi
import shutil
try:
    import arabic_reshaper
    try:
        from bidi import get_display
    except ImportError:
        from bidi.algorithm import get_display
    HAS_BIDI = True
except ImportError:
    HAS_BIDI = False

ALIGN_RIGHT = True      # محاذاة السطور لليمين
DIGITS_AR = True        # أرقام هندية ٠١٢٣
_D2AR = str.maketrans('0123456789', '٠١٢٣٤٥٦٧٨٩')
_SUP = str.maketrans('0123456789-', '⁰¹²³⁴⁵⁶⁷⁸⁹⁻')
_AR_RUN = re.compile(r'[\u0621-\u064A\u0640]+')
_AR_WORD = re.compile(r'[\u0621-\u064A]{2,}')
_MATHWORDS = ('جتا', 'جا', 'ظا', 'هـ')


def _arabize(line):
    line = re.sub(r'\bT\(', 'ح(', line)
    line = re.sub(r'\bT(\d)', r'ح\1', line)
    line = re.sub(r'C\((\d+),(\d+)\)', r'ق(\1،\2)', line)
    line = line.replace('C(', 'ق(').replace('cos', 'جتا').replace('sin', 'جا')
    line = re.sub(r'\^(?:\((-?\d+)\)|(-?\d+))', lambda m: (m.group(1) or m.group(2)).translate(_SUP), line)
    if DIGITS_AR:
        line = line.translate(_D2AR)
    return line


def _is_math(tok):
    t = tok
    for w in _MATHWORDS:
        t = t.replace(w, '')
    return not _AR_WORD.search(t)


def _island(text):
    # معادلة: بتتكتب شمال لليمين زي الكتاب، والكلمات العربي جواها بتتشكل وتتعكس
    return _AR_RUN.sub(lambda m: arabic_reshaper.reshape(m.group())[::-1], text)


def shape(line, info=None):
    line = _arabize(line)
    if not HAS_BIDI or not line.strip():
        return line
    segs = []
    for t in line.split():
        k = 'M' if _is_math(t) else 'T'
        if segs and segs[-1][0] == k:
            segs[-1][1].append(t)
        else:
            segs.append([k, [t]])
    parts = []
    for k, toks in reversed(segs):
        txt = ' '.join(toks)
        parts.append(_island(txt) if k == 'M' else get_display(arabic_reshaper.reshape(txt)))
    if info is not None:
        info['math_only'] = (len(segs) == 1 and segs[0][0] == 'M')
    return ' '.join(parts)


def out(text="", end="\n"):
    width = shutil.get_terminal_size((80, 20)).columns - 1
    for line in str(text).split("\n"):
        info = {}
        v = shape(line, info)
        if HAS_BIDI and v.strip():
            if info.get('math_only'):
                v = v.strip().center(width)
            elif ALIGN_RIGHT:
                v = v.rstrip().rjust(width)
        print(v, end=end)


def inp(msg=""):
    return input(shape(msg))


X, Y, Z = sp.symbols('x y z')
TR = standard_transformations + (implicit_multiplication_application,)
AR = str.maketrans('٠١٢٣٤٥٦٧٨٩٫', '0123456789.')


# ---------------------------------------------------------------- أدوات عامة
def norm(s, caret=True):
    s = s.translate(AR)
    pairs = [('جتا', 'cos'), ('جا', 'sin'), ('×', '*'), ('÷', '/'), ('√', 'sqrt'),
             ('−', '-'), ('–', '-'), ('س', 'x'), ('ص', 'y'), ('π', 'pi'), ('ω', 'w')]
    for a, b in pairs:
        s = s.replace(a, b)
    if caret:
        s = s.replace('^', '**')
    return s.strip()


def P(s, extra=None):
    d = {'x': X, 'y': Y, 'z': Z}
    if extra:
        d.update(extra)
    return parse_expr(norm(s), transformations=TR, local_dict=d)


def f(e):
    s = str(e)
    s = s.replace('**', '^')
    s = re.sub(r'\bI\b', 'ت', s)
    s = re.sub(r'\bE\b', 'هـ', s)
    s = s.replace('sqrt', '√').replace('pi', 'π').replace('*', '')
    for a_, b_ in (('x', 'س'), ('y', 'ص'), ('w', 'ω')):
        s = s.replace(a_, b_)
    return s


def fc(z):
    z = sp.expand_complex(sp.sympify(z))
    a, b = sp.simplify(sp.re(z)), sp.simplify(sp.im(z))
    if b == 0:
        return f(a)
    neg = bool(b.is_negative)
    bb = -b if neg else b
    core = f(bb)
    if bb == 1:
        coef = ''
    elif bb.is_Add or '/' in core:
        coef = '(' + core + ')'
    else:
        coef = core
    imag = coef + 'ت'
    if a == 0:
        return ('-' if neg else '') + imag
    real = '(' + f(a) + ')' if a.is_Add else f(a)
    return f"{real} {'-' if neg else '+'} {imag}"


def hdr(t):
    out("")
    out("─" * 34)
    out(t)
    out("─" * 34)


def ask(msg):
    return inp(msg).strip()


def ask_int(msg):
    return int(norm(ask(msg)))


# ---------------------------------------------------------------- قراءة المفكوك
def read_bin():
    s = ask("اكتب المفكوك (مثال: (2x+3y)^5 أو (3x^2-1/x)^13) : ")
    s = norm(s, caret=False).replace('**', '^').replace(' ', '')
    m = re.match(r'^\((.*)\)\^\(?(\d+)\)?$', s)
    if not m:
        raise ValueError("الشكل لازم يبقى (حد1 + حد2)^ن")
    inner, n = m.group(1), int(m.group(2))
    cuts, d = [], 0
    for k, ch in enumerate(inner):
        if ch == '(':
            d += 1
        elif ch == ')':
            d -= 1
        elif ch in '+-' and d == 0 and k > 0 and inner[k - 1] not in '*/^(':
            cuts.append(k)
    if len(cuts) != 1:
        raise ValueError("لازم يبقى جوه القوس حدين بس")
    a, b = P(inner[:cuts[0]]), P(inner[cuts[0]:])
    return a, b, n


def term(a, b, n, r):
    return sp.expand(sp.binomial(n, r) * a ** (n - r) * b ** r)


def show_T(a, b, n, r):
    C = sp.binomial(n, r)
    T = term(a, b, n, r)
    out(f"T{r + 1} = C({n},{r}) × ({f(a)})^{n - r} × ({f(b)})^{r}")
    out(f"= {C} × ({f(a ** (n - r))}) × ({f(b ** r)})")
    out(f"∴ T{r + 1} = {f(T)}")
    return T


def main_var(a, b):
    fs = (a + b).free_symbols
    if not fs:
        raise ValueError("مفيش متغير في المفكوك")
    return X if X in fs else sorted(fs, key=str)[0]


# ---------------------------------------------------------------- ذات الحدين
def op_expand(a, b, n):
    hdr("فك المفكوك")
    out(f"عدد الحدود = ن + 1 = {n + 1}")
    if b.as_coeff_Mul()[0] < 0:
        out("إشارة الحد التاني (-) فالإشارات هتتبادل: + - + - ...")
    terms = [show_T(a, b, n, r) for r in range(n + 1)]
    out("\nالمفكوك = " + f(sp.Add(*terms)))


def op_sumcoef(a, b, n):
    hdr("مجموع المعاملات")
    fs = (a + b).free_symbols
    sub = {s: 1 for s in fs}
    if fs:
        out("بنعوض عن كل متغير بـ 1: " + " ، ".join(f"{f(s)} = 1" for s in sorted(fs, key=str)))
    av, bv = a.subs(sub), b.subs(sub)
    out(f"({f(av)} + ({f(bv)}))^{n} = ({f(av + bv)})^{n} = {f((av + bv) ** n)}")


def op_Tk(a, b, n, k):
    hdr(f"الحد رقم {k} من البداية")
    if not 1 <= k <= n + 1:
        raise ValueError(f"رقم الحد لازم بين 1 و {n + 1}")
    out("الحد العام: T(ر+1) = C(ن,ر) × (الأول)^(ن-ر) × (التاني)^ر")
    out(f"ر + 1 = {k}  →  ر = {k - 1}")
    show_T(a, b, n, k - 1)


def op_end(a, b, n, k):
    hdr(f"الحد رقم {k} من النهاية")
    rank = (n + 1) - k + 1
    out(f"عدد الحدود = {n + 1}")
    out(f"رتبة الحد من البداية = عدد الحدود - رتبته من النهاية + 1 = {n + 1} - {k} + 1 = {rank}")
    if not 1 <= rank <= n + 1:
        raise ValueError("رقم الحد برا المفكوك")
    show_T(a, b, n, rank - 1)
    out("\nالحل التاني: نبدل الحدين ونجيب الحد رقم " + str(k) + " عادي من البداية")
    out(f"({f(b)} + {f(a)})^{n}")
    show_T(b, a, n, k - 1)


def op_middle(a, b, n):
    hdr("الحد الأوسط")
    if n % 2 == 0:
        k = n // 2 + 1
        out(f"ن = {n} زوجي ← عدد الحدود {n + 1} فردي ← حد أوسط واحد رتبته (ن+2)/2 = {k}")
        show_T(a, b, n, k - 1)
    else:
        k1 = (n + 1) // 2
        out(f"ن = {n} فردي ← عدد الحدود {n + 1} زوجي ← حدين أوسطين رتبتهم (ن+1)/2 = {k1} ، (ن+3)/2 = {k1 + 1}")
        show_T(a, b, n, k1 - 1)
        show_T(a, b, n, k1)
    out("\nملحوظة: لو الحدين الأوسطين متساويين في (س+ص)^ن فإن س = ص")


def op_power(a, b, n, m):
    var = main_var(a, b)
    hdr(f"الحد المشتمل على {f(var)}^{m}" if m != 0 else f"الحد الخالي من {f(var)}")
    ca, ea = a.as_coeff_exponent(var)
    cb, eb = b.as_coeff_exponent(var)
    if ca.has(var) or cb.has(var):
        raise ValueError("كل حد لازم يبقى حد واحد (monomial) في المتغير")
    r = sp.Symbol('ر')
    out("الحد العام: T(ر+1) = C(ن,ر) × (الأول)^(ن-ر) × (التاني)^ر")
    expo = sp.expand(ea * (n - r) + eb * r)
    out(f"أس {f(var)} في الحد العام = ({f(ea)})({n} - ر) + ({f(eb)})(ر) = {f(expo)}")
    out(f"بنساويه بـ {m}:  {f(expo)} = {m}")
    if ea == eb:
        out("الأس ثابت في كل الحدود: " + ("كل الحدود فيها الأس ده" if expo == m else "مفيش حد بالأس ده"))
        return
    sol = sp.solve(sp.Eq(expo, m), r)
    if not sol:
        out("مفيش حل ← مفيش حد بالشكل ده")
        return
    rv = sol[0]
    out(f"ر = {f(rv)}")
    if rv.is_integer and 0 <= rv <= n:
        rv = int(rv)
        out(f"الحد هو الحد رقم {rv + 1}")
        show_T(a, b, n, rv)
    else:
        out(f"ر لازم تبقى عدد صحيح بين 0 و {n} ← مفيش حد بالشكل ده")


def op_ratio(a, b, n, p, q):
    hdr(f"النسبة T{q} / T{p}")
    if not (1 <= p <= n + 1 and 1 <= q <= n + 1):
        raise ValueError("رقم الحد برا المفكوك")
    if abs(p - q) == 1:
        lo, hi = min(p, q), max(p, q)
        r = lo
        out("القانون: T(ر+1) / T(ر) = (ن-ر+1)/ر × (التاني/الأول)")
        res = sp.simplify(sp.Rational(n - r + 1, r) * b / a)
        out(f"T{hi}/T{lo} = ({n}-{r}+1)/{r} × ({f(b)})/({f(a)}) = {f(res)}")
        if q == lo:
            res = sp.simplify(1 / res)
            out(f"ونقلب: T{q}/T{p} = {f(res)}")
    else:
        res = sp.simplify(term(a, b, n, q - 1) / term(a, b, n, p - 1))
        out(f"T{q}/T{p} = {f(res)}")


def parse_vals(s):
    d = {}
    for m in re.finditer(r'([A-Za-z])\s*=\s*(-?[\d./]+)', norm(s, caret=False)):
        d[sp.Symbol(m.group(1))] = sp.Rational(m.group(2))
    return d


def op_greatest(a, b, n, vals):
    hdr("أكبر حد عددياً")
    av, bv = a.subs(vals), b.subs(vals)
    if av.free_symbols or bv.free_symbols:
        raise ValueError("لازم تدي قيمة لكل المتغيرات")
    if av == 0 or bv == 0:
        raise ValueError("القيم مينفعش تخلي حد يساوي صفر")
    out(f"الأول = {f(av)} ، التاني = {f(bv)}")
    rho = sp.Abs(av / bv)
    out("بفرض أن T(ر+1) أكبر حد:  T(ر+1)/T(ر) ≥ 1  و  T(ر+2)/T(ر+1) ≤ 1")
    out("القانون: (ن - النسبة)/(النسبة + 1) ≤ ر ≤ (ن + 1)/(النسبة + 1) ، النسبة = |الأول/التاني|")
    out(f"النسبة = |{f(av)}/{f(bv)}| = {f(rho)}")
    lo, hi = (n - rho) / (rho + 1), sp.Rational(n + 1) / (rho + 1)
    out(f"{f(lo)} ≤ ر ≤ {f(hi)}   (≈ {float(lo):.4f} ≤ ر ≤ {float(hi):.4f})")
    rs = [r for r in range(n + 1) if lo <= r <= hi]
    for r in rs:
        val = sp.binomial(n, r) * av ** (n - r) * bv ** r
        out(f"ر = {r} ← الحد T{r + 1}")
        out(f"T{r + 1} = C({n},{r}) × ({f(av)})^{n - r} × ({f(bv)})^{r} = {f(val)}   (القيمة العددية = {f(sp.Abs(val))})")
    if len(rs) == 2:
        out("فيه حدين متساويين في القيمة العددية وهما الأكبر")


def op_greatest_coef(n):
    hdr(f"أكبر معامل في (1+س)^{n}")
    if n % 2 == 0:
        k = n // 2 + 1
        out(f"ن زوجي ← أكبر معامل هو معامل الحد الأوسط T{k}")
        out(f"= C({n},{n // 2}) = {sp.binomial(n, n // 2)}")
    else:
        k = (n + 1) // 2
        out(f"ن فردي ← أكبر معامل هو معامل الحدين الأوسطين T{k} و T{k + 1}")
        out(f"= C({n},{(n - 1) // 2}) = {sp.binomial(n, (n - 1) // 2)}")


def op_oddeven(a, b, n):
    hdr("ضعف الحدود الفردية / الزوجية الرتبة")
    plus = sp.expand((a + b) ** n + (a - b) ** n)
    minus = sp.expand((a + b) ** n - (a - b) ** n)
    odd_cnt = n // 2 + 1 if n % 2 == 0 else (n + 1) // 2
    even_cnt = n // 2 if n % 2 == 0 else (n + 1) // 2
    out(f"({f(a)} + {f(b)})^{n} + ({f(a)} - {f(b)})^{n} = 2 (T1 + T3 + T5 + ...)")
    out(f"= {f(plus)}")
    out(f"مجموع الحدود الفردية الرتبة = {f(sp.expand(plus / 2))}   (عددها {odd_cnt})")
    out(f"\n({f(a)} + {f(b)})^{n} - ({f(a)} - {f(b)})^{n} = 2 (T2 + T4 + T6 + ...)")
    out(f"= {f(minus)}")
    out(f"مجموع الحدود الزوجية الرتبة = {f(sp.expand(minus / 2))}   (عددها {even_cnt})")


def op_reverse(s):
    hdr("رجّع المفكوك لأصله")
    p = P(s)
    var = main_var(p, 0 * X) if p.free_symbols else X
    pl = sp.Poly(p, var)
    n = pl.degree()
    out(f"الأس ن = درجة المقدار = {n}")
    if n < 2:
        raise ValueError("المقدار لازم يبقى درجته 2 أو أكتر")
    lead = pl.LC()
    c1 = pl.coeff_monomial(var ** (n - 1))
    root = sp.root(lead, n)
    out(f"أول حد: A^{n} = {f(lead)} ← A = {f(root)}")
    for A in (root, -root):
        B = sp.simplify(c1 / (n * A ** (n - 1)))
        if sp.expand((A * var + B) ** n - p) == 0:
            out(f"تاني حد: {n} × A^{n - 1} × B = {f(c1)} ← B = {f(B)}")
            out(f"\nالمقدار = ({f(A * var + B)})^{n}")
            out("تحقق: فكينا القوس طلع نفس المقدار ✓")
            return
    out("المقدار ده مش مفكوك لذات حدين على شكل (أس + ب)^ن")
    out("التحليل: " + f(sp.factor(p)))


# ---------------------------------------------------------------- الأعداد المركبة
def parse_c(s, evaluate=True):
    s = norm(s).replace('ت', 'i')
    s = re.sub(r'(cos|sin)\(\s*(-?\d+(?:\.\d+)?)\s*\)', r'\1((\2)*pi/180)', s)
    s = re.sub(r'(cos|sin)\s*(-?\d+(?:\.\d+)?)(?![\d.*/(\w])', r'\1((\2)*pi/180)', s)
    s = re.sub(r'(cos|sin)', r' \1', s)
    s = re.sub(r'(?<![A-Za-z])e\*\*', 'E**', s)
    return parse_expr(s, transformations=TR, evaluate=evaluate,
                      local_dict={'i': sp.I, 'E': sp.E, 'x': X, 'y': Y, 'z': Z})


def read_c(s):
    z = sp.expand_complex(parse_c(s))
    if z.free_symbols:
        raise ValueError("العدد المركب لازم يبقى أرقام بس")
    return z


def snap(x):
    q = round(x * 4) / 4
    if abs(x - q) < 1e-9:
        return sp.Rational(int(round(x * 4)), 4)
    return sp.Float(round(x, 6))


def fa(t):
    if isinstance(t, sp.Rational):
        return f(t) + "°"
    return f"{float(t):.4f}°"


def fap(t):
    s = fa(t)
    return '(' + s + ')' if s.startswith('-') else s


def norm_ang(t):
    t = t % 360
    if t > 180:
        t = t - 360
    return t


def polar(z):
    a, b = sp.re(z), sp.im(z)
    fa_, fb_ = float(a), float(b)
    if abs(fa_) < 1e-12 and abs(fb_) < 1e-12:
        raise ValueError("الصفر ملوش سعة")
    L = sp.simplify(sp.sqrt(sp.expand(a ** 2 + b ** 2)))
    th = snap(math.degrees(math.atan2(fb_, fa_)))
    return a, b, L, th


def quad_name(a, b):
    sa = 0 if abs(float(a)) < 1e-12 else (1 if float(a) > 0 else -1)
    sb = 0 if abs(float(b)) < 1e-12 else (1 if float(b) > 0 else -1)
    names = {(1, 1): "الربع الأول", (-1, 1): "الربع التاني", (-1, -1): "الربع التالت",
             (1, -1): "الربع الرابع", (1, 0): "على محور السينات الموجب",
             (-1, 0): "على محور السينات السالب", (0, 1): "على محور الصادات الموجب",
             (0, -1): "على محور الصادات السالب"}
    return names[(sa, sb)], sa, sb


def euler_str(L, th):
    if isinstance(th, sp.Rational):
        rad = th / 180 * sp.pi
        return f"{f(L)} هـ^(ت({f(rad)}))"
    return f"{f(L)} هـ^(ت({math.radians(float(th)):.4f}))"


def trig_str(L, ang):
    Ls = f(L)
    if L.is_Add or '/' in Ls:
        Ls = '(' + Ls + ')'
    if Ls == '1':
        Ls = ''
    return f"{Ls}(جتا {fap(ang)} + ت جا {fap(ang)})"


def op_cplx(s):
    hdr("تبسيط عملية على الأعداد المركبة")
    z = parse_c(s, evaluate=False)
    num, den = sp.fraction(z)
    if den == 1 and z.is_Add:
        num, den = sp.fraction(sp.together(z, deep=False))
    z = sp.sympify(z).doit() if False else z
    den_x = sp.expand_complex(sp.expand(den))
    if den != 1 and sp.im(den_x) != 0:
        den_e = sp.expand(den)
        conj = sp.expand(sp.conjugate(den_e))
        num_e = sp.expand(num)
        out(f"البسط = {f(num_e)}")
        out(f"المقام = {f(den_e)} ← مرافقه = {f(conj)}")
        out("بنضرب البسط والمقام في مرافق المقام (العدد × مرافقه = عدد حقيقي):")
        nn, dd = sp.expand(num_e * conj), sp.expand(den_e * conj)
        out(f"البسط الجديد = ({f(num_e)})({f(conj)}) = {f(nn)}")
        out(f"المقام الجديد = ({f(den_e)})({f(conj)}) = {f(dd)}")
        res = sp.expand_complex(nn / dd)
    else:
        out("بنفك الأقواس وبنعوض عن ت² = -1:")
        out(f"= {f(sp.expand(z))}")
        res = sp.expand_complex(z)
    out(f"\nالناتج = {fc(res)}")
    out(f"الجزء الحقيقي = {f(sp.re(res))} ، الجزء التخيلي = {f(sp.im(res))}")
    out(f"المرافق = {fc(sp.conjugate(res))}")


def op_ipow(n):
    hdr(f"ت^{n}")
    q, r = divmod(n, 4)
    vals = {0: "1", 1: "ت", 2: "-1", 3: "-ت"}
    out("ت² = -1 ، ت³ = -ت ، ت⁴ = 1 ← بنقسم الأس على 4 ونشوف الباقي")
    out(f"{n} ÷ 4 = {q} والباقي {r}")
    out({0: "لو قسمة صحيحة ← الناتج 1", 1: "لو الباقي 0.25 ← الناتج ت",
           2: "لو الباقي 0.5 ← الناتج -1", 3: "لو الباقي 0.75 ← الناتج -ت"}[r])
    out(f"ت^{n} = {vals[r]}")


def op_profile(s):
    hdr("ملف العدد المركب")
    z = read_c(s)
    a, b, L, th = polar(z)
    th0 = th % 360
    out(f"ع = {fc(z)}  ← النقطة ({f(a)} , {f(b)}) في مستوى أرجان")
    out(f"المرافق ع̄ = {fc(sp.conjugate(z))}  ← ({f(a)} , {f(-b)})  (متماثلين حول محور السينات)")
    out(f"المعكوس الجمعي -ع = {fc(-z)}  ← ({f(-a)} , {f(-b)})  (متماثلين حول نقطة الأصل)")
    out(f"\nالمقياس |ع| = √(س² + ص²) = √(({f(a)})² + ({f(b)})²) = {f(L)}")
    qn, sa, sb = quad_name(a, b)
    if sa != 0 and sb != 0:
        alpha = snap(math.degrees(math.atan(abs(float(b)) / abs(float(a)))))
        out(f"ظا θ = ص/س = {f(sp.nsimplify(b / a)) if abs(float(a)) > 1e-12 else '-'} ← الزاوية المرجعية = {fa(alpha)}")
    out(f"الربع: {qn}")
    out(f"السعة θ (من 0 لـ 360) = {fa(th0)}")
    out(f"السعة الأساسية (من -180 لـ 180) = {fa(th)}")
    out(f"\nالصورة المثلثية: ع = {trig_str(L, th0)}")
    out(f"الصورة الأساسية: ع = {trig_str(L, th)}")
    out(f"صورة أويلر: ع = {euler_str(L, th)}")
    out("\nخواص السعة (أساسية):")
    out(f"  سعة ع̄ = -θ = {fa(norm_ang(-th))}")
    out(f"  سعة 1/ع = -θ = {fa(norm_ang(-th))}")
    out(f"  سعة -ع = θ - 180 = {fa(norm_ang(th - 180))}")
    out(f"  سعة ت×ع = θ + 90 = {fa(norm_ang(th + 90))}")


TABLE = {
    ('+', '+', True): ("θ", lambda t: t),
    ('+', '+', False): ("90 - θ", lambda t: 90 - t),
    ('-', '+', True): ("180 - θ", lambda t: 180 - t),
    ('-', '+', False): ("90 + θ", lambda t: 90 + t),
    ('-', '-', True): ("θ - 180", lambda t: t - 180),
    ('-', '-', False): ("-90 - θ", lambda t: -90 - t),
    ('+', '-', True): ("-θ", lambda t: -t),
    ('+', '-', False): ("θ - 90", lambda t: t - 90),
}
QN = {('+', '+'): "أول", ('-', '+'): "تاني", ('-', '-'): "تالت", ('+', '-'): "رابع"}


def op_standard(s):
    hdr("تحويل لصورة مثلثية قياسية")
    t = norm(s).replace('ت', 'i').replace(' ', '').replace('**', '^')
    num = r'\(?(-?\d+(?:\.\d+)?)\)?'
    m = re.match(r'^(.*?)\(([+-]?)(cos|sin)' + num + r'([+-])\*?i\*?(cos|sin)' + num + r'\)$', t)
    if not m:
        raise ValueError("اكتبها بالشكل: 4(-cos45-i sin45) أو 6(sin240+i cos240)")
    Ls, s1, f1, t1, s2, f2, t2 = m.groups()
    L = P(Ls.rstrip('*')) if Ls.strip('*') else sp.Integer(1)
    if t1 != t2:
        raise ValueError("الزاويتين لازم يبقوا نفس الزاوية")
    th = sp.Rational(t1)
    s1 = s1 or '+'
    if f1 == f2:
        raise ValueError("الدالتين من نفس النوع، مش قياسية ولا معكوسة")
    correct = (f1 == 'cos')
    key = (s1, s2, correct)
    txt, fn = TABLE[key]
    out(f"المعامل: {f(L)} ، θ = {f(th)}°")
    out(f"إشارة الجزء الحقيقي ({s1}) وإشارة التخيلي ({s2}) ← ربع {QN[(s1, s2)]}")
    out("الدوال: " + ("مظبوطة (جتا ثم جا)" if correct else "معكوسة (جا ثم جتا)"))
    ang = fn(th)
    out(f"السعة = {txt} = {f(ang)}°")
    ang2 = norm_ang(ang)
    if ang2 != ang:
        out(f"السعة الأساسية = {f(ang)} + 360 k = {f(ang2)}°")
    ap = f"({f(ang2)}°)" if ang2 < 0 else f"{f(ang2)}°"
    out(f"\nع = {f(L)}(جتا {ap} + ت جا {ap})")


def get_polar_any(s):
    z = read_c(s)
    a, b, L, th = polar(z)
    return z, L, th


def op_polar_ops(s1, s2, op, n=None):
    hdr("ضرب / قسمة / قوة بالصورة المثلثية")
    z1, L1, t1 = get_polar_any(s1)
    out(f"ع١ = {fc(z1)} = {trig_str(L1, t1)}")
    if op in '*/':
        z2, L2, t2 = get_polar_any(s2)
        out(f"ع٢ = {fc(z2)} = {trig_str(L2, t2)}")
    if op == '*':
        L, ang, z = sp.simplify(L1 * L2), t1 + t2, sp.expand_complex(z1 * z2)
        out(f"\nع١ × ع٢ = ل١ل٢ [جتا(θ١+θ٢) + ت جا(θ١+θ٢)]")
        out(f"السعة = {fa(t1)} + {fap(t2)} = {fa(ang)}")
    elif op == '/':
        L, ang, z = sp.simplify(L1 / L2), t1 - t2, sp.expand_complex(z1 / z2)
        out(f"\nع١ ÷ ع٢ = (ل١/ل٢) [جتا(θ١-θ٢) + ت جا(θ١-θ٢)]")
        out(f"السعة = {fa(t1)} - {fap(t2)} = {fa(ang)}")
    else:
        L, ang, z = sp.simplify(L1 ** n), t1 * n, sp.expand_complex(z1 ** n)
        out(f"\nديموافر: عن = لن [جتا ن θ + ت جا ن θ]")
        out(f"السعة = {n} × {fa(t1)} = {fa(ang)}")
    out(f"المقياس = {f(L)}")
    out(f"الناتج = {trig_str(L, norm_ang(ang))}")
    out(f"بالصورة الجبرية = {fc(z)}")


def fmt_num(re_, im_):
    sgn = '-' if im_ < 0 else '+'
    return f"{re_:.4f} {sgn} {abs(im_):.4f}i"


def alg_of(Lr, ang):
    if isinstance(ang, sp.Rational):
        try:
            ex = sp.expand_complex(Lr * (sp.cos(sp.pi * ang / 180) + sp.I * sp.sin(sp.pi * ang / 180)))
            if not ex.has(sp.cos, sp.sin) and len(f(ex)) <= 60:
                return "= " + fc(ex)
        except Exception:
            pass
    r, rad = float(Lr), math.radians(float(ang))
    return "≈ " + fmt_num(r * math.cos(rad), r * math.sin(rad))


def op_roots(s, n):
    hdr(f"الجذور الـ {n} للعدد المركب")
    if n < 2:
        raise ValueError("ن لازم تبقى 2 أو أكتر")
    z = read_c(s)
    a, b, L, th = polar(z)
    th0 = th % 360
    out(f"ع = {fc(z)} = {trig_str(L, th0)}")
    out("ديموافر: ع^(1/ن) = ل^(1/ن) [جتا((θ + 360 ر)/ن) + ت جا((θ + 360 ر)/ن)]")
    Lr = sp.simplify(L ** sp.Rational(1, n))
    out(f"ل^(1/ن) = {f(Lr)}")
    out(f"الزيادة بين كل جذر والتاني = 360 ÷ {n} = {f(sp.Rational(360, n))}°")
    for k in range(n):
        ang = (th0 + 360 * k) / n
        line = f"ر = {k} ← ع{k + 1} = {trig_str(Lr, ang)}"
        p = norm_ang(ang)
        if p != ang:
            line += f" = {trig_str(Lr, p)}"
        out(line)
        out(f"        {alg_of(Lr, p)}")


def op_sqrt(s):
    hdr("الجذر التربيعي للعدد المركب")
    z = read_c(s)
    a, b = sp.re(z), sp.im(z)
    out(f"ع = {fc(z)}")
    if b == 0:
        r = sp.sqrt(sp.Abs(a))
        out("العدد حقيقي: " + (f"الجذران: ±{f(r)}" if a >= 0 else f"الجذران: ±{f(r)}ت"))
        return
    xs, ys = sp.symbols('X Y', real=True)
    out("\nالطريقة التانية: نفرض (س + ص ت)² = ع ونربع الطرفين:")
    out(f"س² - ص² = {f(a)}  ......(1)")
    out(f"2 س ص = {f(b)}  ......(2)")
    sols = sp.solve([xs ** 2 - ys ** 2 - a, 2 * xs * ys - b], [xs, ys], dict=True)
    sols = [d for d in sols if d[xs].is_real and d[ys].is_real]
    for d in sols:
        out(f"  س = {f(sp.simplify(d[xs]))} ، ص = {f(sp.simplify(d[ys]))} ← {fc(d[xs] + sp.I * d[ys])}")
    L = sp.simplify(sp.sqrt(a ** 2 + b ** 2))
    sg = '+' if b > 0 else '-'
    out("\nالطريقة الخاصة (للاختياري):  ±( √((المقياس + الحقيقي)/2) ، إشارة الأوسط ، √((المقياس - الحقيقي)/2) ت )")
    p1 = sp.simplify(sp.sqrt((L + a) / 2))
    p2 = sp.simplify(sp.sqrt((L - a) / 2))
    out(f"المقياس = {f(L)}")
    out(f"√(({f(L)} + {f(a)})/2) = {f(p1)}   ،   √(({f(L)} - {f(a)})/2) = {f(p2)}")
    out(f"إشارة الأوسط = إشارة الجزء التخيلي ({sg})")
    r1 = p1 + (1 if b > 0 else -1) * p2 * sp.I
    out(f"الجذران: ±({fc(r1)})")


W = sp.Symbol('w')
WV = (-1 + sp.sqrt(3) * sp.I) / 2


def op_omega(s):
    hdr("الجذور التكعيبية للواحد (ω)")
    out("الجذور: 1 ، ω = (-1 + √3 ت)/2 ، ω² = (-1 - √3 ت)/2")
    out("ω³ = 1 ، 1 + ω + ω² = 0 ، ω² = 1/ω = مرافق ω ، 1 + ω = -ω² ، 1 + ω² = -ω ، ω - ω² = √3 ت")
    expr = P(s, {'w': W})
    red = expr.replace(lambda e: e.is_Pow and e.base == W and e.exp.is_Integer,
                       lambda e: W ** (int(e.exp) % 3))
    out(f"\nالمقدار = {f(expr)}")
    if red != expr:
        out("الأس لو بيقبل القسمة على 3 يبقى الناتج 1، وغير كده نقسم الأس على 3 ونشوف الباقي:")
        out(f"بعد اختزال الأسس = {f(red)}")
    vn = sp.N(red.subs(W, WV), 60)
    if vn.has(sp.zoo, sp.nan, sp.oo):
        raise ValueError("المقدار ده مقامه بيساوي صفر")
    def to_rat(x):
        x = sp.N(x, 50)
        fr = Fraction(str(x)).limit_denominator(10 ** 9)
        return sp.Rational(fr.numerator, fr.denominator)
    re_ = to_rat(sp.re(vn))
    im_ = to_rat(sp.im(vn) / sp.sqrt(3)) * sp.sqrt(3)
    val = sp.expand_complex(re_ + sp.I * im_)
    out(f"\nالناتج = {fc(val)}")
    names = {sp.Integer(1): "1", sp.expand_complex(WV): "ω", sp.expand_complex(WV ** 2): "ω²"}
    for k, v in names.items():
        if sp.simplify(val - k) == 0:
            out(f"أي أن الناتج = {v}")
            break


# ---------------------------------------------------------------- القائمة
MENU = """
=============== محرك المنهج: ذات الحدين + الأعداد المركبة ===============
-- نظرية ذات الحدين --
 1) فك مفكوك بالخطوات
 2) مجموع المعاملات
 3) الحد رقم k من البداية (T_k)
 4) الحد رقم k من النهاية
 5) الحد الأوسط
 6) الحد المشتمل على س^m أو الخالي من س
 7) النسبة بين حدين
 8) أكبر حد عددياً (بقيم س و ص)
 9) أكبر معامل في (1+س)^ن
10) ضعف الحدود الفردية / الزوجية الرتبة
11) رجّع المفكوك لأصله
-- الأعداد المركبة --
12) تبسيط عملية (جمع / ضرب / قسمة)
13) ت^ن
14) ملف عدد (مرافق، مقياس، سعة، ربع، مثلثية، أويلر)
15) تحويل لصورة مثلثية قياسية
16) ضرب / قسمة / قوة بالصورة المثلثية
17) الجذور النونية (ديموافر)
18) الجذر التربيعي
19) الجذور التكعيبية للواحد (ω)
 0) خروج
"""


def run(ch):
    if ch == '1':
        a, b, n = read_bin(); op_expand(a, b, n)
    elif ch == '2':
        a, b, n = read_bin(); op_sumcoef(a, b, n)
    elif ch == '3':
        a, b, n = read_bin(); op_Tk(a, b, n, ask_int("رقم الحد : "))
    elif ch == '4':
        a, b, n = read_bin(); op_end(a, b, n, ask_int("رقم الحد من النهاية : "))
    elif ch == '5':
        a, b, n = read_bin(); op_middle(a, b, n)
    elif ch == '6':
        a, b, n = read_bin()
        m = ask("الأس المطلوب (اكتب 0 للخالي من س) : ")
        op_power(a, b, n, sp.Rational(norm(m)))
    elif ch == '7':
        a, b, n = read_bin()
        op_ratio(a, b, n, ask_int("رقم الحد المقسوم عليه (المقام) : "), ask_int("رقم الحد في البسط : "))
    elif ch == '8':
        a, b, n = read_bin()
        op_greatest(a, b, n, parse_vals(ask("القيم (مثال: x=3, y=2) : ")))
    elif ch == '9':
        op_greatest_coef(ask_int("ن = "))
    elif ch == '10':
        a, b, n = read_bin(); op_oddeven(a, b, n)
    elif ch == '11':
        op_reverse(ask("اكتب المقدار (مثال: x^4+16x^3+96x^2+256x+256) : "))
    elif ch == '12':
        op_cplx(ask("اكتب العملية (مثال: 5/(3+2i) أو (1+i)^2/(1-i)) : "))
    elif ch == '13':
        op_ipow(ask_int("الأس ن : "))
    elif ch == '14':
        op_profile(ask("اكتب العدد (مثال: -2+2i) : "))
    elif ch == '15':
        op_standard(ask("اكتب الصورة (مثال: 4(-cos45-i sin45)) : "))
    elif ch == '16':
        s1 = ask("ع١ (مثال: 1+i أو 3(cos75+i sin75)) : ")
        op = ask("العملية (* أو / أو ^) : ")
        if op == '^':
            op_polar_ops(s1, None, '^', ask_int("الأس ن : "))
        else:
            op_polar_ops(s1, ask("ع٢ : "), op)
    elif ch == '17':
        op_roots(ask("اكتب العدد (مثال: 16 أو -32 أو 16(cos60+i sin60)) : "), ask_int("الجذر رقم كام (ن) : "))
    elif ch == '18':
        op_sqrt(ask("اكتب العدد (مثال: 2-2sqrt(3)i) : "))
    elif ch == '19':
        op_omega(ask("اكتب المقدار بـ w (مثال: (1-w-w^2)^3) : "))
    else:
        out("اختيار مش موجود")


def main():
    while True:
        out(MENU)
        ch = norm(ask("اختار رقم : "))
        if ch == '0':
            break
        try:
            run(ch)
        except Exception as e:
            out(f"في مشكلة في الإدخال أو الحل: {e}")
        inp("\nدوس Enter ترجع للقائمة ...")


if __name__ == "__main__":
    main()