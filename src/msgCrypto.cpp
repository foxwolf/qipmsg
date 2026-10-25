// This file is part of QIpMsg.
//
// QIpMsg is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QIpMsg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QIpMsg.  If not, see <http://www.gnu.org/licenses/>.
//

#include "msgCrypto.h"
#include "ipmsg.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QRandomGenerator>
#include <QSaveFile>
#include <QVector>

#include <cstring>

namespace {

const quint32 kOurSym = IPMSG_BLOWFISH_128 | IPMSG_AES_256;
const int kMaxModBytes = 512;

struct RsaPriv {
    QString expHex;
    QString modHex;
    QString privHex;
    int bits;
    bool ok;
    RsaPriv() : bits(0), ok(false) {}
};

struct RsaPub {
    QString expHex;
    QString modHex;
    quint32 caps;
    bool ok;
    RsaPub() : caps(0), ok(false) {}
};

QMutex g_lock;
QString g_testDir;
RsaPriv g_local;
QHash<QString, RsaPub> g_peers;
bool g_peersLoaded = false;

class BigNum
{
public:
    QVector<quint32> w;

    static BigNum fromU32(quint32 v)
    {
        BigNum n;
        if (v) {
            n.w.append(v);
        }
        return n;
    }

    static BigNum fromHex(const QString &hex)
    {
        QByteArray raw = QByteArray::fromHex(hex.toLatin1());
        return fromBe(raw);
    }

    static BigNum fromBe(const QByteArray &be)
    {
        BigNum n;
        int i = 0;
        while (i < be.size() && be.at(i) == 0) {
            ++i;
        }
        int nbytes = be.size() - i;
        if (nbytes <= 0) {
            return n;
        }
        int limbs = (nbytes + 3) / 4;
        n.w.resize(limbs);
        for (int b = 0; b < nbytes; ++b) {
            const int fromStart = nbytes - 1 - b;
            const unsigned char byte = static_cast<unsigned char>(be.at(i + fromStart));
            n.w[b / 4] |= quint32(byte) << ((b % 4) * 8);
        }
        n.trim();
        return n;
    }

    QString toHex() const
    {
        return QString::fromLatin1(toBe().toHex());
    }

    QByteArray toBe(int width = 0) const
    {
        int bytes = (bitLength() + 7) / 8;
        if (bytes < 1 && !isZero()) {
            bytes = 1;
        }
        if (width > bytes) {
            bytes = width;
        }
        if (bytes < 1) {
            bytes = 1;
        }
        QByteArray out(bytes, 0);
        for (int b = 0; b < bytes; ++b) {
            const int limb = b / 4;
            const int shift = (b % 4) * 8;
            unsigned char byte = 0;
            if (limb < w.size()) {
                byte = static_cast<unsigned char>((w.at(limb) >> shift) & 0xffu);
            }
            out[bytes - 1 - b] = char(byte);
        }
        return out;
    }

    bool isZero() const
    {
        return w.isEmpty();
    }

    int bitLength() const
    {
        if (w.isEmpty()) {
            return 0;
        }
        const quint32 top = w.last();
        int bits = (w.size() - 1) * 32;
        quint32 v = top;
        while (v) {
            ++bits;
            v >>= 1;
        }
        return bits;
    }

    int cmp(const BigNum &o) const
    {
        if (w.size() != o.w.size()) {
            return w.size() < o.w.size() ? -1 : 1;
        }
        for (int i = w.size() - 1; i >= 0; --i) {
            if (w.at(i) != o.w.at(i)) {
                return w.at(i) < o.w.at(i) ? -1 : 1;
            }
        }
        return 0;
    }

    void trim()
    {
        while (!w.isEmpty() && w.last() == 0) {
            w.removeLast();
        }
    }

    BigNum add(const BigNum &o) const
    {
        BigNum r;
        const int n = qMax(w.size(), o.w.size());
        r.w.resize(n);
        quint64 carry = 0;
        for (int i = 0; i < n; ++i) {
            const quint64 av = i < w.size() ? w.at(i) : 0;
            const quint64 bv = i < o.w.size() ? o.w.at(i) : 0;
            const quint64 sum = av + bv + carry;
            r.w[i] = quint32(sum);
            carry = sum >> 32;
        }
        if (carry) {
            r.w.append(quint32(carry));
        }
        return r;
    }

    BigNum sub(const BigNum &o) const
    {
        BigNum r;
        r.w.resize(w.size());
        quint64 borrow = 0;
        for (int i = 0; i < w.size(); ++i) {
            const quint64 bv = i < o.w.size() ? o.w.at(i) : 0;
            quint64 av = w.at(i);
            if (av < bv + borrow) {
                av += (quint64(1) << 32);
                r.w[i] = quint32(av - bv - borrow);
                borrow = 1;
            } else {
                r.w[i] = quint32(av - bv - borrow);
                borrow = 0;
            }
        }
        r.trim();
        return r;
    }

    BigNum shl1() const
    {
        BigNum r;
        r.w.resize(w.size() + 1);
        quint32 carry = 0;
        for (int i = 0; i < w.size(); ++i) {
            r.w[i] = (w.at(i) << 1) | carry;
            carry = w.at(i) >> 31;
        }
        r.w[w.size()] = carry;
        r.trim();
        return r;
    }

    BigNum mul(const BigNum &o) const
    {
        if (isZero() || o.isZero()) {
            return BigNum();
        }
        BigNum r;
        r.w.resize(w.size() + o.w.size());
        for (int i = 0; i < w.size(); ++i) {
            quint64 carry = 0;
            for (int j = 0; j < o.w.size(); ++j) {
                const quint64 sum = quint64(r.w.at(i + j))
                        + quint64(w.at(i)) * quint64(o.w.at(j)) + carry;
                r.w[i + j] = quint32(sum);
                carry = sum >> 32;
            }
            r.w[i + o.w.size()] = quint32(carry);
        }
        r.trim();
        return r;
    }

    quint32 modSmall(quint32 m) const
    {
        quint64 rem = 0;
        for (int i = w.size() - 1; i >= 0; --i) {
            rem = ((rem << 32) | w.at(i)) % m;
        }
        return quint32(rem);
    }
};

BigNum bnSub(const BigNum &a, const BigNum &b)
{
    return a.sub(b);
}

int bnCmp(const BigNum &a, const BigNum &b)
{
    return a.cmp(b);
}

quint32 invMod32(quint32 a)
{
    quint32 x = 1;
    for (int i = 0; i < 5; ++i) {
        x = quint32(x * (2u - a * x));
    }
    return x;
}

BigNum montMul(const BigNum &a, const BigNum &b, const BigNum &n, quint32 n0neg, int k)
{
    QVector<quint32> t(2 * k + 4, 0);
    for (int i = 0; i < k; ++i) {
        const quint32 av = i < a.w.size() ? a.w.at(i) : 0;
        quint64 carry = 0;
        for (int j = 0; j < k; ++j) {
            const quint32 bv = j < b.w.size() ? b.w.at(j) : 0;
            const quint64 sum = quint64(t.at(i + j)) + quint64(av) * bv + carry;
            t[i + j] = quint32(sum);
            carry = sum >> 32;
        }
        int p = i + k;
        while (carry) {
            if (p >= t.size()) {
                t.resize(p + 1);
            }
            const quint64 sum = quint64(t.at(p)) + carry;
            t[p] = quint32(sum);
            carry = sum >> 32;
            ++p;
        }
    }
    for (int i = 0; i < k; ++i) {
        const quint32 u = quint32(quint64(t.at(i)) * n0neg);
        quint64 carry = 0;
        for (int j = 0; j < k; ++j) {
            const quint32 nv = j < n.w.size() ? n.w.at(j) : 0;
            const quint64 sum = quint64(t.at(i + j)) + quint64(u) * nv + carry;
            t[i + j] = quint32(sum);
            carry = sum >> 32;
        }
        int p = i + k;
        while (carry) {
            if (p >= t.size()) {
                t.resize(p + 1);
            }
            const quint64 sum = quint64(t.at(p)) + carry;
            t[p] = quint32(sum);
            carry = sum >> 32;
            ++p;
        }
    }
    BigNum r;
    if (t.size() > k) {
        r.w = t.mid(k);
    }
    r.trim();
    if (bnCmp(r, n) >= 0) {
        r = bnSub(r, n);
    }
    if (bnCmp(r, n) >= 0) {
        r = bnSub(r, n);
    }
    return r;
}

void divMod(const BigNum &num, const BigNum &den, BigNum *quot, BigNum *rem)
{
    BigNum q;
    BigNum r;
    if (!den.isZero()) {
        const int bits = num.bitLength();
        for (int i = bits - 1; i >= 0; --i) {
            r = r.shl1();
            const int limb = i / 32;
            const int bit = i % 32;
            if (limb < num.w.size() && (num.w.at(limb) & (quint32(1) << bit))) {
                r = r.add(BigNum::fromU32(1));
            }
            if (r.cmp(den) >= 0) {
                r = bnSub(r, den);
                const int need = i / 32 + 1;
                if (q.w.size() < need) {
                    q.w.resize(need);
                }
                q.w[i / 32] |= quint32(1) << bit;
            }
        }
        q.trim();
        r.trim();
    }
    if (quot) {
        *quot = q;
    }
    if (rem) {
        *rem = r;
    }
}

BigNum modReduce(const BigNum &a, const BigNum &n)
{
    if (n.isZero() || a.cmp(n) < 0) {
        return a;
    }
    BigNum rem;
    divMod(a, n, 0, &rem);
    return rem;
}

BigNum modExp(const BigNum &baseIn, const BigNum &exp, const BigNum &n)
{
    if (n.isZero() || (n.w.size() == 1 && n.w.at(0) == 1)) {
        return BigNum();
    }
    const int k = n.w.size();
    const quint32 n0neg = 0u - invMod32(n.w.at(0));
    BigNum r2 = BigNum::fromU32(1);
    const int steps = 2 * k * 32;
    for (int i = 0; i < steps; ++i) {
        r2 = r2.shl1();
        if (r2.cmp(n) >= 0) {
            r2 = bnSub(r2, n);
        }
    }
    BigNum base = baseIn;
    if (base.cmp(n) >= 0) {
        base = modReduce(base, n);
    }
    BigNum one = BigNum::fromU32(1);
    BigNum acc = montMul(one, r2, n, n0neg, k);
    BigNum baseM = montMul(base, r2, n, n0neg, k);
    for (int i = exp.bitLength() - 1; i >= 0; --i) {
        acc = montMul(acc, acc, n, n0neg, k);
        const int limb = i / 32;
        const int bit = i % 32;
        if (limb < exp.w.size() && (exp.w.at(limb) & (quint32(1) << bit))) {
            acc = montMul(acc, baseM, n, n0neg, k);
        }
    }
    return montMul(acc, one, n, n0neg, k);
}

BigNum modInverse(const BigNum &a, const BigNum &m)
{
    BigNum oldR = m;
    BigNum r = a;
    if (r.cmp(m) >= 0) {
        r = modReduce(r, m);
    }
    BigNum oldS;
    BigNum s = BigNum::fromU32(1);
    bool oldSNeg = false;
    bool sNeg = false;
    auto addAbs = [](const BigNum &x, bool xn, const BigNum &y, bool yn,
                     bool *outNeg) -> BigNum {
        if (xn == yn) {
            *outNeg = xn;
            return x.add(y);
        }
        if (x.cmp(y) >= 0) {
            *outNeg = xn;
            return bnSub(x, y);
        }
        *outNeg = yn;
        return bnSub(y, x);
    };
    while (!r.isZero()) {
        BigNum q;
        BigNum rem;
        divMod(oldR, r, &q, &rem);
        const BigNum prod = q.mul(s);
        bool nextSNeg = false;
        const BigNum nextS = addAbs(oldS, oldSNeg, prod, !sNeg, &nextSNeg);
        oldR = r;
        r = rem;
        oldS = s;
        oldSNeg = sNeg;
        s = nextS;
        sNeg = nextSNeg;
    }
    if (oldSNeg) {
        oldS = bnSub(m, modReduce(oldS, m));
    } else if (oldS.cmp(m) >= 0) {
        oldS = modReduce(oldS, m);
    }
    return oldS;
}

bool hexOk(const QString &text)
{
    if (text.isEmpty() || (text.size() % 2) != 0) {
        return false;
    }
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        const bool digit = c.isDigit();
        const bool alpha = (c >= QLatin1Char('a') && c <= QLatin1Char('f'))
                || (c >= QLatin1Char('A') && c <= QLatin1Char('F'));
        if (!digit && !alpha) {
            return false;
        }
    }
    return true;
}

const quint32 kSmallPrimes[] = {
    3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,
    101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,
    193,197,199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,
    293,307,311,313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,
    409,419,421,431,433,439,443,449,457,461,463,467,479,487,491,499,503,509,
    521,523,541,547,557,563,569,571,577,587,593,599,601,607,613,617,619,631,
    641,643,647,653,659,661,673,677,683,691,701,709,719,727,733,739,743,751,
    757,761,769,773,787,797,809,811,821,823,827,829,839,853,857,859,863,877,
    881,883,887,907,911,919,929,937,941,947,953,967,971,977,983,991,997
};

bool trialDivides(const BigNum &n)
{
    const int count = int(sizeof(kSmallPrimes) / sizeof(kSmallPrimes[0]));
    for (int i = 0; i < count; ++i) {
        const quint32 p = kSmallPrimes[i];
        if (n.bitLength() < 16 && n.cmp(BigNum::fromU32(p)) == 0) {
            return false;
        }
        if (n.modSmall(p) == 0) {
            return true;
        }
    }
    return false;
}

bool isPrime(const BigNum &n)
{
    if (n.bitLength() < 2) {
        return false;
    }
    if (n.w.size() == 1 && n.w.at(0) == 2) {
        return true;
    }
    if ((n.w.at(0) & 1u) == 0) {
        return false;
    }
    if (trialDivides(n)) {
        return false;
    }
    BigNum one = BigNum::fromU32(1);
    BigNum nm1 = bnSub(n, one);
    int s = 0;
    BigNum d = nm1;
    while (!d.isZero() && (d.w.at(0) & 1u) == 0) {
        quint32 carry = 0;
        for (int i = d.w.size() - 1; i >= 0; --i) {
            const quint32 cur = d.w.at(i);
            d.w[i] = (cur >> 1) | carry;
            carry = (cur & 1u) << 31;
        }
        d.trim();
        ++s;
    }
    const quint32 bases[] = {2, 3, 5, 7, 11, 13, 23, 29};
    for (quint32 a32 : bases) {
        if (n.cmp(BigNum::fromU32(a32)) <= 0) {
            continue;
        }
        BigNum x = modExp(BigNum::fromU32(a32), d, n);
        if (x.cmp(one) == 0 || x.cmp(nm1) == 0) {
            continue;
        }
        bool hit = false;
        for (int r = 1; r < s; ++r) {
            x = modExp(x, BigNum::fromU32(2), n);
            if (x.cmp(nm1) == 0) {
                hit = true;
                break;
            }
            if (x.cmp(one) == 0) {
                return false;
            }
        }
        if (!hit) {
            return false;
        }
    }
    return true;
}

BigNum randomOdd(int bits)
{
    const int limbs = (bits + 31) / 32;
    BigNum n;
    n.w.resize(limbs);
    auto *rng = QRandomGenerator::system();
    for (int i = 0; i < limbs; ++i) {
        n.w[i] = rng->generate();
    }
    int top = bits % 32;
    if (top == 0) {
        top = 32;
    }
    if (top < 32) {
        n.w[limbs - 1] &= (quint32(1) << top) - 1u;
    }
    n.w[limbs - 1] |= quint32(1) << (top - 1);
    n.w[0] |= 1u;
    n.trim();
    return n;
}

BigNum addU32(const BigNum &n, quint32 v)
{
    return n.add(BigNum::fromU32(v));
}

BigNum randomPrime(int bits)
{
    BigNum n = randomOdd(bits);
    for (int i = 0; i < 200000; ++i) {
        if (isPrime(n)) {
            return n;
        }
        n = addU32(n, 2);
        if (n.bitLength() > bits) {
            n = randomOdd(bits);
        }
    }
    return BigNum();
}

QByteArray randomBytes(int n)
{
    QByteArray out(n, 0);
    auto *rng = QRandomGenerator::system();
    for (int i = 0; i < n; ++i) {
        out[i] = char(rng->generate() & 0xffu);
    }
    return out;
}

QByteArray pkcs1Encrypt(const QByteArray &msg, const BigNum &mod, const BigNum &exp)
{
    const int k = (mod.bitLength() + 7) / 8;
    if (k < msg.size() + 11 || k > kMaxModBytes) {
        return QByteArray();
    }
    QByteArray em(k, 0);
    em[0] = 0;
    em[1] = 2;
    const int ps = k - msg.size() - 3;
    for (int i = 0; i < ps; ++i) {
        unsigned char b = 0;
        while (b == 0) {
            b = static_cast<unsigned char>(QRandomGenerator::system()->generate() & 0xffu);
        }
        em[2 + i] = char(b);
    }
    em[2 + ps] = 0;
    memcpy(em.data() + 3 + ps, msg.constData(), size_t(msg.size()));
    BigNum m = BigNum::fromBe(em);
    BigNum c = modExp(m, exp, mod);
    return c.toBe(k);
}

QByteArray pkcs1Decrypt(const QByteArray &cipher, const BigNum &mod, const BigNum &priv)
{
    const int k = (mod.bitLength() + 7) / 8;
    if (cipher.size() != k) {
        return QByteArray();
    }
    BigNum c = BigNum::fromBe(cipher);
    if (c.cmp(mod) >= 0) {
        return QByteArray();
    }
    BigNum m = modExp(c, priv, mod);
    QByteArray em = m.toBe(k);
    if (em.size() != k || em.at(0) != 0 || em.at(1) != 2) {
        return QByteArray();
    }
    int i = 2;
    int nonzero = 0;
    while (i < em.size() && em.at(i) != 0) {
        ++nonzero;
        ++i;
    }
    if (i >= em.size() || nonzero < 8) {
        return QByteArray();
    }
    return em.mid(i + 1);
}

#include "blowfishTab.inc"

quint32 bfF(const quint32 sbox[4][256], quint32 x)
{
    const quint32 a = (x >> 24) & 0xffu;
    const quint32 b = (x >> 16) & 0xffu;
    const quint32 c = (x >> 8) & 0xffu;
    const quint32 d = x & 0xffu;
    return quint32((quint64(sbox[0][a]) + sbox[1][b]) ^ sbox[2][c]) + sbox[3][d];
}

struct BfKey {
    quint32 p[18];
    quint32 s[4][256];
};

void bfLoadBe(const unsigned char *in, quint32 *xl, quint32 *xr)
{
    *xl = (quint32(in[0]) << 24) | (quint32(in[1]) << 16)
            | (quint32(in[2]) << 8) | quint32(in[3]);
    *xr = (quint32(in[4]) << 24) | (quint32(in[5]) << 16)
            | (quint32(in[6]) << 8) | quint32(in[7]);
}

void bfStoreBe(quint32 xl, quint32 xr, unsigned char *out)
{
    out[0] = char(xl >> 24); out[1] = char(xl >> 16);
    out[2] = char(xl >> 8); out[3] = char(xl);
    out[4] = char(xr >> 24); out[5] = char(xr >> 16);
    out[6] = char(xr >> 8); out[7] = char(xr);
}

void bfEncipher(const BfKey &key, quint32 *xl, quint32 *xr)
{
    quint32 l = *xl;
    quint32 r = *xr;
    for (int i = 0; i < 16; ++i) {
        l ^= key.p[i];
        r ^= bfF(key.s, l);
        const quint32 tmp = l;
        l = r;
        r = tmp;
    }
    const quint32 tmp = l;
    l = r;
    r = tmp;
    r ^= key.p[16];
    l ^= key.p[17];
    *xl = l;
    *xr = r;
}

void bfDecipher(const BfKey &key, quint32 *xl, quint32 *xr)
{
    quint32 l = *xl;
    quint32 r = *xr;
    for (int i = 17; i > 1; --i) {
        l ^= key.p[i];
        r ^= bfF(key.s, l);
        const quint32 tmp = l;
        l = r;
        r = tmp;
    }
    const quint32 tmp = l;
    l = r;
    r = tmp;
    r ^= key.p[1];
    l ^= key.p[0];
    *xl = l;
    *xr = r;
}

BfKey bfSetup(const QByteArray &keyBytes)
{
    BfKey key;
    memcpy(key.p, kBfP, sizeof(key.p));
    memcpy(key.s, kBfS, sizeof(key.s));
    const int klen = keyBytes.size();
    int j = 0;
    for (int i = 0; i < 18; ++i) {
        quint32 word = 0;
        for (int b = 0; b < 4; ++b) {
            word = (word << 8) | quint32(static_cast<unsigned char>(keyBytes.at(j % klen)));
            ++j;
        }
        key.p[i] ^= word;
    }
    quint32 l = 0;
    quint32 r = 0;
    for (int i = 0; i < 18; i += 2) {
        bfEncipher(key, &l, &r);
        key.p[i] = l;
        key.p[i + 1] = r;
    }
    for (int box = 0; box < 4; ++box) {
        for (int i = 0; i < 256; i += 2) {
            bfEncipher(key, &l, &r);
            key.s[box][i] = l;
            key.s[box][i + 1] = r;
        }
    }
    return key;
}

void bfEncryptBlock(const BfKey &key, unsigned char block[8])
{
    quint32 l = 0;
    quint32 r = 0;
    bfLoadBe(block, &l, &r);
    bfEncipher(key, &l, &r);
    bfStoreBe(l, r, block);
}

void bfDecryptBlock(const BfKey &key, unsigned char block[8])
{
    quint32 l = 0;
    quint32 r = 0;
    bfLoadBe(block, &l, &r);
    bfDecipher(key, &l, &r);
    bfStoreBe(l, r, block);
}

const quint8 kSbox[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

quint8 xtime(quint8 x)
{
    return quint8((x << 1) ^ ((x & 0x80) ? 0x1b : 0));
}

quint8 gmul(quint8 a, quint8 b)
{
    quint8 p = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) {
            p = quint8(p ^ a);
        }
        const quint8 hi = quint8(a & 0x80);
        a = quint8(a << 1);
        if (hi) {
            a = quint8(a ^ 0x1b);
        }
        b = quint8(b >> 1);
    }
    return p;
}

void aesSub(quint8 s[16])
{
    for (int i = 0; i < 16; ++i) {
        s[i] = kSbox[s[i]];
    }
}

void aesShift(quint8 s[16])
{
    quint8 t = s[1];
    s[1] = s[5]; s[5] = s[9]; s[9] = s[13]; s[13] = t;
    t = s[2]; s[2] = s[10]; s[10] = t;
    t = s[6]; s[6] = s[14]; s[14] = t;
    t = s[15];
    s[15] = s[11]; s[11] = s[7]; s[7] = s[3]; s[3] = t;
}

void aesInvShift(quint8 s[16])
{
    quint8 t = s[13];
    s[13] = s[9]; s[9] = s[5]; s[5] = s[1]; s[1] = t;
    t = s[2]; s[2] = s[10]; s[10] = t;
    t = s[6]; s[6] = s[14]; s[14] = t;
    t = s[3];
    s[3] = s[7]; s[7] = s[11]; s[11] = s[15]; s[15] = t;
}

void aesMix(quint8 s[16])
{
    for (int c = 0; c < 4; ++c) {
        const int o = 4 * c;
        const quint8 a0 = s[o];
        const quint8 a1 = s[o + 1];
        const quint8 a2 = s[o + 2];
        const quint8 a3 = s[o + 3];
        s[o] = quint8(xtime(a0) ^ (xtime(a1) ^ a1) ^ a2 ^ a3);
        s[o + 1] = quint8(a0 ^ xtime(a1) ^ (xtime(a2) ^ a2) ^ a3);
        s[o + 2] = quint8(a0 ^ a1 ^ xtime(a2) ^ (xtime(a3) ^ a3));
        s[o + 3] = quint8((xtime(a0) ^ a0) ^ a1 ^ a2 ^ xtime(a3));
    }
}

void aesInvMix(quint8 s[16])
{
    for (int c = 0; c < 4; ++c) {
        const int o = 4 * c;
        const quint8 a0 = s[o];
        const quint8 a1 = s[o + 1];
        const quint8 a2 = s[o + 2];
        const quint8 a3 = s[o + 3];
        s[o] = quint8(gmul(a0, 14) ^ gmul(a1, 11) ^ gmul(a2, 13) ^ gmul(a3, 9));
        s[o + 1] = quint8(gmul(a0, 9) ^ gmul(a1, 14) ^ gmul(a2, 11) ^ gmul(a3, 13));
        s[o + 2] = quint8(gmul(a0, 13) ^ gmul(a1, 9) ^ gmul(a2, 14) ^ gmul(a3, 11));
        s[o + 3] = quint8(gmul(a0, 11) ^ gmul(a1, 13) ^ gmul(a2, 9) ^ gmul(a3, 14));
    }
}

void aesExpand(const quint8 key[32], quint8 rk[240])
{
    memcpy(rk, key, 32);
    int bytes = 32;
    quint8 rcon = 1;
    while (bytes < 240) {
        quint8 t0 = rk[bytes - 4];
        quint8 t1 = rk[bytes - 3];
        quint8 t2 = rk[bytes - 2];
        quint8 t3 = rk[bytes - 1];
        if (bytes % 32 == 0) {
            const quint8 tmp = t0;
            t0 = quint8(kSbox[t1] ^ rcon);
            t1 = kSbox[t2];
            t2 = kSbox[t3];
            t3 = kSbox[tmp];
            rcon = xtime(rcon);
        } else if (bytes % 32 == 16) {
            t0 = kSbox[t0];
            t1 = kSbox[t1];
            t2 = kSbox[t2];
            t3 = kSbox[t3];
        }
        rk[bytes] = quint8(rk[bytes - 32] ^ t0);
        rk[bytes + 1] = quint8(rk[bytes - 31] ^ t1);
        rk[bytes + 2] = quint8(rk[bytes - 30] ^ t2);
        rk[bytes + 3] = quint8(rk[bytes - 29] ^ t3);
        bytes += 4;
    }
}

void aesAddRound(quint8 s[16], const quint8 *rk)
{
    for (int i = 0; i < 16; ++i) {
        s[i] = quint8(s[i] ^ rk[i]);
    }
}

void aesEncryptBlock(const quint8 rk[240], quint8 block[16])
{
    quint8 s[16];
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            s[r + 4 * c] = block[r + 4 * c];
        }
    }
    aesAddRound(s, rk);
    for (int round = 1; round <= 13; ++round) {
        aesSub(s);
        aesShift(s);
        aesMix(s);
        aesAddRound(s, rk + round * 16);
    }
    aesSub(s);
    aesShift(s);
    aesAddRound(s, rk + 14 * 16);
    memcpy(block, s, 16);
}

void aesDecryptBlock(const quint8 rk[240], quint8 block[16])
{
    quint8 inv[256];
    for (int i = 0; i < 256; ++i) {
        inv[kSbox[i]] = quint8(i);
    }
    quint8 s[16];
    memcpy(s, block, 16);
    aesAddRound(s, rk + 14 * 16);
    for (int round = 13; round >= 1; --round) {
        aesInvShift(s);
        for (int i = 0; i < 16; ++i) {
            s[i] = inv[s[i]];
        }
        aesAddRound(s, rk + round * 16);
        aesInvMix(s);
    }
    aesInvShift(s);
    for (int i = 0; i < 16; ++i) {
        s[i] = inv[s[i]];
    }
    aesAddRound(s, rk);
    memcpy(block, s, 16);
}

QByteArray pkcs7Pad(const QByteArray &in, int block)
{
    const int n = block - (in.size() % block);
    QByteArray out = in;
    out.append(QByteArray(n, char(n)));
    return out;
}

bool pkcs7Unpad(QByteArray *data, int block)
{
    if (!data || data->isEmpty() || (data->size() % block) != 0) {
        return false;
    }
    const int n = static_cast<unsigned char>(data->at(data->size() - 1));
    if (n < 1 || n > block || n > data->size()) {
        return false;
    }
    for (int i = 0; i < n; ++i) {
        if (static_cast<unsigned char>(data->at(data->size() - 1 - i)) != n) {
            return false;
        }
    }
    data->chop(n);
    return true;
}

QByteArray bfCbc(const QByteArray &plain, const QByteArray &key, bool enc)
{
    BfKey sched = bfSetup(key);
    QByteArray data = enc ? pkcs7Pad(plain, 8) : plain;
    if ((data.size() % 8) != 0) {
        return QByteArray();
    }
    unsigned char iv[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    QByteArray out(data.size(), 0);
    for (int off = 0; off < data.size(); off += 8) {
        unsigned char block[8];
        memcpy(block, data.constData() + off, 8);
        if (enc) {
            for (int i = 0; i < 8; ++i) {
                block[i] = quint8(block[i] ^ iv[i]);
            }
            bfEncryptBlock(sched, block);
            memcpy(iv, block, 8);
        } else {
            unsigned char cur[8];
            memcpy(cur, block, 8);
            bfDecryptBlock(sched, block);
            for (int i = 0; i < 8; ++i) {
                block[i] = quint8(block[i] ^ iv[i]);
            }
            memcpy(iv, cur, 8);
        }
        memcpy(out.data() + off, block, 8);
    }
    if (!enc && !pkcs7Unpad(&out, 8)) {
        return QByteArray();
    }
    return out;
}

QByteArray aesCbc(const QByteArray &plain, const QByteArray &key, bool enc)
{
    if (key.size() != 32) {
        return QByteArray();
    }
    quint8 rk[240];
    aesExpand(reinterpret_cast<const quint8 *>(key.constData()), rk);
    QByteArray data = enc ? pkcs7Pad(plain, 16) : plain;
    if ((data.size() % 16) != 0) {
        return QByteArray();
    }
    unsigned char iv[16];
    memset(iv, 0, sizeof(iv));
    QByteArray out(data.size(), 0);
    for (int off = 0; off < data.size(); off += 16) {
        unsigned char block[16];
        memcpy(block, data.constData() + off, 16);
        if (enc) {
            for (int i = 0; i < 16; ++i) {
                block[i] = quint8(block[i] ^ iv[i]);
            }
            aesEncryptBlock(rk, block);
            memcpy(iv, block, 16);
        } else {
            unsigned char cur[16];
            memcpy(cur, block, 16);
            aesDecryptBlock(rk, block);
            for (int i = 0; i < 16; ++i) {
                block[i] = quint8(block[i] ^ iv[i]);
            }
            memcpy(iv, cur, 16);
        }
        memcpy(out.data() + off, block, 16);
    }
    if (!enc && !pkcs7Unpad(&out, 16)) {
        return QByteArray();
    }
    return out;
}

QString storageDir()
{
    if (!g_testDir.isEmpty()) {
        return g_testDir;
    }
    return QDir::homePath() + QStringLiteral("/.qipmsg");
}

QString privatePath()
{
    return storageDir() + QStringLiteral("/rsa_private.json");
}

QString peerPath()
{
    return storageDir() + QStringLiteral("/peer_pubkeys.json");
}

bool writePrivate(int mode)
{
    QDir().mkpath(storageDir());
    QJsonObject obj;
    obj.insert(QStringLiteral("v"), 1);
    obj.insert(QStringLiteral("bits"), g_local.bits);
    obj.insert(QStringLiteral("e"), g_local.expHex);
    obj.insert(QStringLiteral("n"), g_local.modHex);
    obj.insert(QStringLiteral("d"), g_local.privHex);
    QSaveFile file(privatePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    if (!file.commit()) {
        return false;
    }
    QFile::setPermissions(privatePath(),
                          QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    Q_UNUSED(mode);
    return true;
}

bool readPrivate()
{
    QFile file(privatePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
    const QString e = obj.value(QStringLiteral("e")).toString();
    const QString n = obj.value(QStringLiteral("n")).toString();
    const QString d = obj.value(QStringLiteral("d")).toString();
    if (!hexOk(e) || !hexOk(n) || !hexOk(d)) {
        return false;
    }
    g_local.expHex = e.toLower();
    g_local.modHex = n.toLower();
    g_local.privHex = d.toLower();
    g_local.bits = obj.value(QStringLiteral("bits")).toInt();
    g_local.ok = true;
    return true;
}

void loadPeers()
{
    if (g_peersLoaded) {
        return;
    }
    g_peersLoaded = true;
    QFile file(peerPath());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    const QStringList keys = root.keys();
    foreach (const QString &ip, keys) {
        const QJsonObject obj = root.value(ip).toObject();
        RsaPub pub;
        pub.expHex = obj.value(QStringLiteral("e")).toString().toLower();
        pub.modHex = obj.value(QStringLiteral("n")).toString().toLower();
        pub.caps = quint32(obj.value(QStringLiteral("caps")).toVariant().toULongLong());
        if (hexOk(pub.expHex) && hexOk(pub.modHex)) {
            pub.ok = true;
            g_peers.insert(ip, pub);
        }
    }
}

bool savePeers()
{
    QDir().mkpath(storageDir());
    QJsonObject root;
    QHashIterator<QString, RsaPub> it(g_peers);
    while (it.hasNext()) {
        it.next();
        if (!it.value().ok) {
            continue;
        }
        QJsonObject obj;
        obj.insert(QStringLiteral("e"), it.value().expHex);
        obj.insert(QStringLiteral("n"), it.value().modHex);
        obj.insert(QStringLiteral("caps"), double(it.value().caps));
        root.insert(it.key(), obj);
    }
    QSaveFile file(peerPath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    return file.commit();
}

quint32 localCaps()
{
    quint32 rsa = IPMSG_RSA_1024;
    if (g_local.bits > 1024) {
        rsa = IPMSG_RSA_2048;
    }
    return rsa | kOurSym;
}

bool parseAnswer(const QString &body, RsaPub *pub)
{
    const int colon = body.indexOf(QLatin1Char(':'));
    if (colon <= 0 || !pub) {
        return false;
    }
    bool ok = false;
    const quint32 caps = body.left(colon).toUInt(&ok, 16);
    if (!ok) {
        return false;
    }
    const QString rest = body.mid(colon + 1).section(QLatin1Char(':'), 0, 0);
    const int dash = rest.indexOf(QLatin1Char('-'));
    if (dash <= 0) {
        return false;
    }
    const QString exp = rest.left(dash).trimmed().toLower();
    const QString mod = rest.mid(dash + 1).trimmed().toLower();
    if (!hexOk(exp) || !hexOk(mod)) {
        return false;
    }
    if (mod.size() < 64 || mod.size() > kMaxModBytes * 2) {
        return false;
    }
    pub->caps = caps;
    pub->expHex = exp;
    pub->modHex = mod;
    pub->ok = true;
    return true;
}

quint32 chooseSym(quint32 peerCaps)
{
    if (peerCaps & IPMSG_AES_256) {
        return IPMSG_AES_256;
    }
    if (peerCaps & IPMSG_BLOWFISH_128) {
        return IPMSG_BLOWFISH_128;
    }
    return 0;
}

quint32 rsaFlagForMod(const BigNum &mod)
{
    const int bits = mod.bitLength();
    if (bits <= 1024) {
        return IPMSG_RSA_1024;
    }
    if (bits <= 2048) {
        return IPMSG_RSA_2048;
    }
    return 0;
}

} // namespace

const int MsgCrypto::kLocalBits = 1024;

void MsgCrypto::setStorageDirForTest(const QString &dir)
{
    QMutexLocker locker(&g_lock);
    g_testDir = dir;
    g_local = RsaPriv();
    g_peers.clear();
    g_peersLoaded = false;
}

void MsgCrypto::forgetMemoryForTest()
{
    QMutexLocker locker(&g_lock);
    g_local = RsaPriv();
    g_peers.clear();
    g_peersLoaded = false;
}

bool MsgCrypto::cipherSelfCheck()
{
    const QByteArray zeroKey(8, 0);
    const QByteArray zeroBlock(8, 0);
    BfKey key = bfSetup(zeroKey);
    unsigned char block[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    bfEncryptBlock(key, block);
    const QByteArray got(reinterpret_cast<char *>(block), 8);
    if (got.toHex() != QByteArray("4ef997456198dd78")) {
        return false;
    }
    const QByteArray hello("Hello\0", 6);
    const QByteArray bfKey = QByteArray::fromHex("00112233445566778899aabbccddeeff");
    const QByteArray sealed = bfCbc(hello, bfKey, true);
    if (sealed.toHex() != QByteArray("0237718181b47a83")) {
        return false;
    }
    if (bfCbc(sealed, bfKey, false) != hello) {
        return false;
    }
    quint8 rk[240];
    quint8 zkey[32];
    memset(zkey, 0, sizeof(zkey));
    aesExpand(zkey, rk);
    unsigned char ablock[16];
    memset(ablock, 0, sizeof(ablock));
    aesEncryptBlock(rk, ablock);
    if (QByteArray(reinterpret_cast<char *>(ablock), 16).toHex()
            != QByteArray("dc95c078a2408989ad48a21492842087")) {
        return false;
    }
    const QByteArray msg(16, 0);
    const QByteArray akey(32, 0);
    if (aesCbc(aesCbc(msg, akey, true), akey, false) != msg) {
        return false;
    }
    const BigNum n = BigNum::fromU32(3233);
    const BigNum e = BigNum::fromU32(17);
    const BigNum d = BigNum::fromU32(2753);
    const BigNum c = modExp(BigNum::fromU32(123), e, n);
    if (c.cmp(BigNum::fromU32(855)) != 0) {
        return false;
    }
    if (modExp(c, d, n).cmp(BigNum::fromU32(123)) != 0) {
        return false;
    }
    const BigNum inv = modInverse(BigNum::fromU32(17), BigNum::fromU32(3120));
    if (inv.cmp(BigNum::fromU32(2753)) != 0) {
        return false;
    }
    return true;
}

bool MsgCrypto::generateKey(int bits)
{
    if (bits < 512 || bits > 2048 || (bits % 2) != 0) {
        return false;
    }
    const BigNum exp = BigNum::fromU32(65537);
    for (int attempt = 0; attempt < 8; ++attempt) {
        const BigNum p = randomPrime(bits / 2);
        BigNum q = randomPrime(bits / 2);
        if (p.isZero() || q.isZero()) {
            return false;
        }
        if (p.cmp(q) == 0) {
            continue;
        }
        const BigNum n = p.mul(q);
        const BigNum p1 = bnSub(p, BigNum::fromU32(1));
        const BigNum q1 = bnSub(q, BigNum::fromU32(1));
        const BigNum phi = p1.mul(q1);
        if (phi.modSmall(65537) == 0) {
            continue;
        }
        const BigNum d = modInverse(exp, phi);
        const BigNum check = modExp(modExp(BigNum::fromU32(65), exp, n), d, n);
        if (check.cmp(BigNum::fromU32(65)) != 0) {
            continue;
        }
        QMutexLocker locker(&g_lock);
        g_local.expHex = exp.toHex();
        g_local.modHex = n.toHex();
        g_local.privHex = d.toHex();
        g_local.bits = n.bitLength();
        g_local.ok = true;
        return writePrivate(0600);
    }
    return false;
}

bool MsgCrypto::ensureLocalKey()
{
    {
        QMutexLocker locker(&g_lock);
        if (g_local.ok) {
            return true;
        }
        if (readPrivate()) {
            return true;
        }
    }
    return generateKey(kLocalBits);
}

bool MsgCrypto::hasLocalKey()
{
    QMutexLocker locker(&g_lock);
    if (g_local.ok) {
        return true;
    }
    return readPrivate();
}

QString MsgCrypto::localAnswerBody()
{
    if (!ensureLocalKey()) {
        return QString();
    }
    QMutexLocker locker(&g_lock);
    return QString::number(localCaps(), 16) + QLatin1Char(':')
            + g_local.expHex + QLatin1Char('-') + g_local.modHex;
}

bool MsgCrypto::rememberPeerAnswer(const QString &ip, const QString &body)
{
    if (ip.isEmpty()) {
        return false;
    }
    RsaPub pub;
    if (!parseAnswer(body, &pub)) {
        return false;
    }
    QMutexLocker locker(&g_lock);
    loadPeers();
    g_peers.insert(ip, pub);
    savePeers();
    return true;
}

bool MsgCrypto::hasPeerKey(const QString &ip)
{
    QMutexLocker locker(&g_lock);
    loadPeers();
    return g_peers.value(ip).ok;
}

quint32 MsgCrypto::peerCaps(const QString &ip)
{
    QMutexLocker locker(&g_lock);
    loadPeers();
    return g_peers.value(ip).caps;
}

bool MsgCrypto::sealPlain(const QByteArray &plain, const QString &modHex,
                          const QString &expHex, quint32 peerCaps,
                          QString *envelope)
{
    if (!envelope || plain.isEmpty() || !hexOk(modHex) || !hexOk(expHex)) {
        return false;
    }
    const quint32 sym = chooseSym(peerCaps);
    if (sym == 0) {
        return false;
    }
    const BigNum mod = BigNum::fromHex(modHex);
    const BigNum exp = BigNum::fromHex(expHex);
    const quint32 rsa = rsaFlagForMod(mod);
    // 32-byte AES key plus PKCS#1 v1.5 overhead needs 43 bytes (336 bits).
    if (rsa == 0 || mod.bitLength() < 336) {
        return false;
    }
    const int keyLen = (sym == IPMSG_AES_256) ? 32 : 16;
    const QByteArray session = randomBytes(keyLen);
    const QByteArray wrapped = pkcs1Encrypt(session, mod, exp);
    if (wrapped.isEmpty()) {
        return false;
    }
    const QByteArray cipher = (sym == IPMSG_AES_256)
            ? aesCbc(plain, session, true)
            : bfCbc(plain, session, true);
    if (cipher.isEmpty()) {
        return false;
    }
    const quint32 caps = rsa | sym;
    *envelope = QString::number(caps, 16) + QLatin1Char(':')
            + QString::fromLatin1(wrapped.toHex()) + QLatin1Char(':')
            + QString::fromLatin1(cipher.toHex());
    return true;
}

bool MsgCrypto::sealForPeer(const QString &ip, const QByteArray &plain,
                            QString *envelope)
{
    RsaPub pub;
    {
        QMutexLocker locker(&g_lock);
        loadPeers();
        pub = g_peers.value(ip);
    }
    if (!pub.ok) {
        return false;
    }
    return sealPlain(plain, pub.modHex, pub.expHex, pub.caps, envelope);
}

bool MsgCrypto::openEnvelope(const QString &envelope, QByteArray *plain)
{
    if (!plain) {
        return false;
    }
    const QStringList parts = envelope.split(QLatin1Char(':'));
    if (parts.size() < 3) {
        return false;
    }
    bool ok = false;
    const quint32 caps = parts.at(0).toUInt(&ok, 16);
    if (!ok || !hexOk(parts.at(1)) || !hexOk(parts.at(2))) {
        return false;
    }
    RsaPriv local;
    {
        QMutexLocker locker(&g_lock);
        if (!g_local.ok && !readPrivate()) {
            return false;
        }
        local = g_local;
    }
    const BigNum mod = BigNum::fromHex(local.modHex);
    const BigNum priv = BigNum::fromHex(local.privHex);
    const QByteArray wrapped = QByteArray::fromHex(parts.at(1).toLatin1());
    const QByteArray session = pkcs1Decrypt(wrapped, mod, priv);
    if (session.isEmpty()) {
        return false;
    }
    const QByteArray cipher = QByteArray::fromHex(parts.at(2).toLatin1());
    QByteArray opened;
    if ((caps & IPMSG_AES_256) && session.size() == 32) {
        opened = aesCbc(cipher, session, false);
    } else if ((caps & IPMSG_BLOWFISH_128) && session.size() == 16) {
        opened = bfCbc(cipher, session, false);
    } else {
        return false;
    }
    if (opened.isNull()) {
        return false;
    }
    *plain = opened;
    return true;
}
