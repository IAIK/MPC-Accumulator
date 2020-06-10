// RCurve.cpp
//
// The implementation in this file is an extension/modification
// of the Relic wrapper in:
//    https://github.com/ladnir/cryptoTools/blob/master/cryptoTools/Crypto/RCurve.cpp

#include "RCurve.h"
#include <cstring>
#include <sstream>

#ifdef ENABLE_RELIC

#ifdef RAND_SIZE
#undef RAND_SIZE
#endif

#ifdef SEED_SIZE
#undef SEED_SIZE
#endif

extern "C" {
    #include "relic/relic_core.h"
    #include "relic/relic_pp.h"
}

#if !defined(GSL_UNLIKELY)
#define GSL_UNLIKELY(x) x
#endif

#ifndef RLC_EQ
#define RLC_EQ CMP_EQ
#endif
#ifndef RLC_LT
#define RLC_LT CMP_LT
#endif
#ifndef RLC_GT
#define RLC_GT CMP_GT
#endif
#ifndef RLC_FP_BYTES
#define RLC_FP_BYTES FP_BYTES
#endif
#ifndef RLC_BN_SIZE
#define RLC_BN_SIZE BN_SIZE
#endif

namespace osuCrypto
{

    REccNumber::REccNumber(const REccNumber & num)
    {
        init();
        *this = num;
    }

    REccNumber::REccNumber()
    {
        init();
    }

    REccNumber::REccNumber(const i32 & val)
    {
        init();
        *this = val;
    }

    REccNumber::REccNumber(const Scalar& val)
    {
        init();
        fromScalar(val);
    }

    REccNumber::REccNumber(REllipticCurve&, const REccNumber & num)
    {
        init();
        *this = num;
    }

    REccNumber::REccNumber(REllipticCurve&)
    {
        init();
    }

    REccNumber::REccNumber(REllipticCurve&,const i32 & val)
    {
        init();
        *this = val;
    }

    REccNumber::~REccNumber()
    {
        bn_clean(*this);
    }

    REccNumber & REccNumber::operator=(const REccNumber & c)
    {
        *this = c.mVal;
        return *this;
    }

    REccNumber & REccNumber::operator=(const bn_t c)
    {
        bn_copy(*this, c);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic copy error " LOCATION);

        return *this;
    }

    REccNumber & REccNumber::operator=(int i)
    {
        bn_set_dig(mVal, i);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic set int error " LOCATION);

        reduce();
        return *this;
    }

    REccNumber & REccNumber::operator++()
    {
        return *this += 1;
    }

    REccNumber & REccNumber::operator--()
    {
        return *this -= 1;
    }

    REccNumber & REccNumber::operator+=(int i)
    {
        bn_add_dig(*this, *this, i);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic add int error " LOCATION);

        reduce();
        return *this;
    }

    REccNumber & REccNumber::operator-=(int i)
    {
        bn_sub_dig(*this, *this, i);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic sub int error " LOCATION);

        reduce();
        return *this;
    }

    REccNumber & REccNumber::operator+=(const REccNumber & b)
    {
        bn_add(*this, *this, b);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic add error " LOCATION);
        reduce();
        return *this;
    }

    REccNumber & REccNumber::operator-=(const REccNumber & b)
    {
        bn_sub(*this, *this, b);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic sub error " LOCATION);
        reduce();
        return *this;
    }

    REccNumber & REccNumber::operator*=(const REccNumber & b)
    {
        bn_mul(*this, *this, b);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic mul error " LOCATION);

        reduce();
        return *this;
    }

    REccNumber & REccNumber::operator*=(int i)
    {
        bn_mul_dig(*this, *this, i);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic mul error " LOCATION);

        reduce();
        return *this;
    }

    REccNumber & REccNumber::operator/=(const REccNumber & b)
    {
        return (*this *= b.inverse());
    }

    REccNumber & REccNumber::operator/=(int i)
    {
        REccNumber iInv, y, c;


        bn_gcd_ext_dig(c, y, iInv, modulus(), i);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic div error " LOCATION);



        return (*this *= iInv);
    }

    void REccNumber::init()
    {
        bn_new(mVal);
        bn_init(mVal, static_cast<int>(sizeDigits()));
    }

    void REccNumber::reduce()
    {
        //auto t = *this;
        bn_mod_basic(*this, *this, modulus());

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic mod error " LOCATION);

    }

    const bn_st * REccNumber::modulus() const { return &core_get()->ep_r; }

    REccNumber REccNumber::negate() const
    {
        REccNumber r;
        bn_neg(r, *this);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic neg error " LOCATION);

        r.reduce();
        return r;
    }

    REccNumber REccNumber::inverse() const
    {
        REccNumber bInv,y,c;

        bn_gcd_ext_basic(c, bInv, y, *this, modulus());

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic inverse error " LOCATION);

        bInv.reduce();

        return bInv;
    }

    bool REccNumber::operator==(const REccNumber & cmp) const
    {
        return bn_cmp(*this, cmp) == RLC_EQ;
    }

    bool REccNumber::operator==(const int & cmp) const
    {
        return bn_cmp_dig(*this, cmp) == RLC_EQ;
    }

    bool REccNumber::operator!=(const REccNumber & cmp) const
    {
        return !(*this == cmp);
    }

    bool REccNumber::operator!=(const int & cmp) const
    {
        return !(*this == cmp);
    }

    bool REccNumber::operator>=(const REccNumber & cmp) const
    {
        return  bn_cmp(*this, cmp) != RLC_LT;
    }

    bool REccNumber::operator>=(const int & cmp) const
    {
        return bn_cmp_dig(*this, cmp) != RLC_LT;
    }

    bool REccNumber::operator<=(const REccNumber & cmp) const
    {
        return cmp >= *this;
    }


    bool REccNumber::operator<=(const int & cmp) const
    {
        return bn_cmp_dig(*this, cmp) != RLC_GT;
    }

    bool REccNumber::operator>(const REccNumber & cmp) const
    {
        return bn_cmp(*this, cmp) == RLC_GT;
    }

    bool REccNumber::operator>(const int & cmp) const
    {
        return bn_cmp_dig(*this, cmp) == RLC_GT;
    }

    bool REccNumber::operator<(const REccNumber & cmp) const
    {
        return cmp > *this;
    }

    bool REccNumber::operator<(const int & cmp) const
    {
        return bn_cmp_dig(*this, cmp) == RLC_LT;
    }

    bool REccNumber::isPrime() const
    {
        return bn_is_prime(*this);
    }

    bool REccNumber::iszero() const
    {
        return bn_is_zero(*this);
    }

    bool operator==(const int & cmp1, const REccNumber & cmp2)
    {
        return cmp2 == cmp1;
    }

    bool operator!=(const int & cmp1, const REccNumber & cmp2)
    {
        return cmp2 != cmp1;
    }

    REccNumber operator-(const REccNumber &v)
    {
        return v.negate();
    }

    REccNumber operator+(int i, const REccNumber &v)
    {
        REccNumber r;
        bn_add_dig(r, v, i);


        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic add error " LOCATION);


        r.reduce();
        return r;
    }
    REccNumber operator+(const REccNumber &i, const REccNumber &v)
    {
        REccNumber r;
        bn_add(r, v, i);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic add error " LOCATION);

        r.reduce();
        return r;
    }
    REccNumber operator-(const REccNumber & v, int i)
    {
        REccNumber r;
        bn_sub_dig(r, v, i);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic sub error " LOCATION);

        r.reduce();
        return r;
    }
    REccNumber operator-(int i, const REccNumber &v)
    {
        return i + v.negate();
    }
    REccNumber operator-(const REccNumber &v, const REccNumber &i)
    {
        REccNumber r;
        bn_sub(r, v, i);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic sub error " LOCATION);

        r.reduce();
        return r;
    }
    REccNumber operator*(const REccNumber & v, int i)
    {
        REccNumber r;
        bn_mul_dig(r, v, i);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic mul error " LOCATION);

        r.reduce();
        return r;
    }
    REccNumber operator*(int i, const REccNumber &v)
    {
        return v * i;
    }
    REccNumber operator*(const REccNumber & v, const REccNumber &i)
    {
        REccNumber r;
        bn_mul(r, v, i);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic mul error " LOCATION);

        r.reduce();
        return r;
    }
    REccNumber operator/(const REccNumber & v, int i)
    {
        auto vv = v;
        vv /= i;
        return vv;
    }
    REccNumber operator/(int i, const REccNumber &v)
    {
        return i * v.inverse();
    }
    REccNumber operator/(const REccNumber &i, const REccNumber &v)
    {
        return i * v.inverse();
    }

    REccNumber operator^(const REccNumber & base, const REccNumber & exp)
    {
        REccNumber r;
        bn_mxp_basic(r, base, exp, base.modulus());


        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic exp error " LOCATION);

        return r;
    }

    std::ostream & operator<<(std::ostream & out, const REccNumber & val)
    {
        auto radix = 16;
        auto size = bn_size_str(val, radix);
        std::string str(size, 0);
        bn_write_str(&str[0], size, val, radix);
        out << str;
        return out;
    }

    std::ostream & operator<<(std::ostream & out, const REccPoint & val)
    {
        REccPoint val2;

        ep_norm(val2, val);

        out << "(";
        out << REllipticCurve::FpToString(val2.mVal->x, 16);
        out << ", ";
        out << REllipticCurve::FpToString(val2.mVal->y, 16);
        out << ", ";
        out << REllipticCurve::FpToString(val2.mVal->z, 16);
        out << ")";

        return out;
    }

    std::ostream & operator<<(std::ostream & out, const REccPairingPoint & val)
    {
        auto radix = 16;

        auto print = [radix](std::ostream& out, fp2_t& c) {

            fp_t* tmp = (fp_t*)&c;

            for (int i = 0; i < 2; i++)
            {
                std::string buff = REllipticCurve::FpToString(tmp[i], radix);

                if (i) {
                    if (buff[0] != '0' || buff[1] != '\0') {
                        out << " + i * ";
                        out << buff;
                    }
                }
                else
                    out << buff;
            }
        };

        REccPairingPoint val2;

        ep2_norm(val2, const_cast<REccPairingPoint&>(val));
        out << "(";
        print(out, val2.mVal->x);
        out << ", ";
        print(out, val2.mVal->y);
        out << ", ";
        print(out, val2.mVal->z);
        out << ")";

        return out;
    }

    REccNumber operator+(const REccNumber &v, int i)
    {
        REccNumber r;
        bn_add_dig(r, v, i);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic add error " LOCATION);
        r.reduce();
        return r;
    }

    REllipticCurve::REllipticCurve(bool pairing)
    {
        if (core_get() == nullptr)
            core_init();

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic core init error " LOCATION);

        if (pairing)
            ep_param_set_any_pairf();
        else
            ep_param_set_any();

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic set any error " LOCATION);
    }

    REllipticCurve::REllipticCurve(int curve, int type)
    {
        if (core_get() == nullptr)
            core_init();

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic core init error " LOCATION);

        ep_param_set(curve);
        ep2_curve_set_twist(type);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic set any error " LOCATION);
    }

    REllipticCurve::~REllipticCurve()
    {
    }

    REllipticCurve::Point REllipticCurve::getGenerator() const
    {
        Point g;
        ep_curve_get_gen(g);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic get gen error " LOCATION);

        return g;
    }

    REllipticCurve::PairingPoint REllipticCurve::getPairingGenerator() const
    {
        PairingPoint g;
        ep2_curve_get_gen(g);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic get gen2 error " LOCATION);

        return g;
    }

    std::vector<REllipticCurve::Point> REllipticCurve::getGenerators() const
    {
        return {getGenerator()};
    }

    REccNumber REllipticCurve::getOrder() const
    {
        REccNumber g;
        ep_curve_get_ord(g);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic get order error " LOCATION);
        return g;
    }

    REccPoint & REccPoint::operator=(const REccPoint & copy)
    {
        ep_copy(*this, copy);
        return *this;
    }

    REccPoint & REccPoint::operator+=(const REccPoint & addIn)
    {
        ep_add(*this, *this, addIn);

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_add error " LOCATION);
        return *this;
    }

    REccPoint & REccPoint::operator-=(const REccPoint & subtractIn)
    {
        ep_sub(*this, *this, subtractIn);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_sub error " LOCATION);
        return *this;
    }

    REccPoint & REccPoint::operator*=(const REccNumber & multIn)
    {
        ep_mul(*this, *this, multIn);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_mul error " LOCATION);
        return *this;
    }

    REccPoint REccPoint::operator+(const REccPoint & addIn) const
    {
        REccPoint r;
        ep_add(r, *this, addIn);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_add error " LOCATION);
        return r;
    }

    REccPoint REccPoint::operator-(const REccPoint & subtractIn) const
    {
        REccPoint r;
        ep_sub(r, *this, subtractIn);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_sub error " LOCATION);
        return r;
    }

    REccPoint REccPoint::operator*(const REccNumber & multIn) const
    {
        REccPoint r;
        ep_mul(r, *this, multIn);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_mul error " LOCATION);
        return r;
    }

    bool REccPoint::operator==(const REccPoint & cmp) const
    {
        return ep_cmp(*this, cmp) == RLC_EQ;
    }

    bool REccPoint::operator!=(const REccPoint & cmp) const
    {
        return ep_cmp(*this, cmp) != RLC_EQ;
    }

    u64 REccPoint::sizeBytes() const
    {
        return 1 + RLC_FP_BYTES;
    }

    void REccPoint::toBytes(u8 * dest) const
    {
        ep_write_bin(dest, static_cast<int>(sizeBytes()), *this, 1);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_write error " LOCATION);
    }

    std::vector<std::string> REccPoint::toString(int base) const
    {
        REccPoint val2;
        ep_norm(val2, this->mVal);
        auto x = REllipticCurve::FpToString(val2.mVal->x, base);
        auto y = REllipticCurve::FpToString(val2.mVal->y, base);
        return {x, y};
    }

    REccPoint::REccPoint(const Scalar& other)
    {
        ep_new(mVal);
        REccNumber n(other);
        ep_mul_gen(mVal, n);
    }

    REccPoint::REccPoint(int other)
    {
        ep_new(mVal);
        REccNumber n(other);
        ep_mul_gen(mVal, n);
    }

    void REccPoint::fromBytes(u8 * src)
    {
        ep_read_bin(*this, src, static_cast<int>(sizeBytes()));
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_read error " LOCATION);
    }

    void REccPoint::randomize(const block & seed)
    {
        ep_map(*this, (u8*)&seed, sizeof(block));
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_map error " LOCATION);
    }

    void REccPoint::randomize()
    {
        ep_rand(*this);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_rand error " LOCATION);
    }

    bool REccPoint::is_zero() const
    {
        int res = ep_is_infty(*this);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_rand error " LOCATION);
        return res == 1;
    }

    void REccPoint::assign_zero()
    {
        ep_set_infty(*this);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_rand error " LOCATION);
    }

    void REccPoint::pack(octetStream& os) const
    {
        bool zero = is_zero();
        os.serialize(zero);
        if (zero)
            return;
        int len = sizeBytes();
        os.serialize(len);
        toBytes(os.append(len));
    }

    void REccPoint::unpack(octetStream& os)
    {
        bool zero;
        os.unserialize(zero);
        if (zero)
        {
            assign_zero();
            return;
        }
        int len;
        os.unserialize(len);
        fromBytes(os.consume(len));
    }

    REccPoint REccPoint::operator*(const Scalar& other) const
    {
        REccNumber o(other);
        return *this * o;
    }

    REccPairingPoint::REccPairingPoint(const Scalar& other)
    {
        ep2_new(mVal);
        REccNumber n(other);
        ep2_mul_gen(mVal, n);
    }

    REccPairingPoint::REccPairingPoint(int other)
    {
        ep2_new(mVal);
        REccNumber n(other);
        ep2_mul_gen(mVal, n);
    }

    REccPairingPoint & REccPairingPoint::operator=(const REccPairingPoint & copy)
    {
        ep2_copy(*this, const_cast<REccPairingPoint&>(copy));
        return *this;
    }

    REccPairingPoint & REccPairingPoint::operator+=(const REccPairingPoint & addIn)
    {
        ep2_add(this->mVal, this->mVal, const_cast<REccPairingPoint&>(addIn));

        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_add error " LOCATION);
        return *this;
    }

    REccPairingPoint & REccPairingPoint::operator-=(const REccPairingPoint & subtractIn)
    {
        ep2_sub(*this, *this, const_cast<REccPairingPoint&>(subtractIn));
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_sub error " LOCATION);
        return *this;
    }

    REccPairingPoint & REccPairingPoint::operator*=(const REccNumber & multIn)
    {
        ep2_mul(*this, *this, multIn);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_mul error " LOCATION);
        return *this;
    }

    REccPairingPoint REccPairingPoint::operator+(const REccPairingPoint & addIn) const
    {
        REccPairingPoint r;
        ep2_add(r, const_cast<REccPairingPoint*>(this)->mVal, const_cast<REccPairingPoint&>(addIn));
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_add error " LOCATION);
        return r;
    }

    REccPairingPoint REccPairingPoint::operator-(const REccPairingPoint & subtractIn) const
    {
        REccPairingPoint r;
        ep2_sub(r, const_cast<REccPairingPoint*>(this)->mVal, const_cast<REccPairingPoint&>(subtractIn));
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_sub error " LOCATION);
        return r;
    }

    REccPairingPoint REccPairingPoint::operator*(const REccNumber & multIn) const
    {
        REccPairingPoint r;
        ep2_mul(r, const_cast<REccPairingPoint*>(this)->mVal, multIn);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_mul error " LOCATION);
        return r;
    }

    bool REccPairingPoint::operator==(const REccPairingPoint & cmp) const
    {
        return ep2_cmp(const_cast<REccPairingPoint*>(this)->mVal, const_cast<REccPairingPoint&>(cmp)) == RLC_EQ;
    }

    bool REccPairingPoint::operator!=(const REccPairingPoint & cmp) const
    {
        return ep2_cmp(const_cast<REccPairingPoint*>(this)->mVal, const_cast<REccPairingPoint&>(cmp)) != RLC_EQ;
    }

    u64 REccPairingPoint::sizeBytes() const
    {
        return 2 * RLC_FP_BYTES + 1;
    }

    void REccPairingPoint::toBytes(u8 * dest) const
    {
        ep2_write_bin(dest, static_cast<int>(sizeBytes()), const_cast<REccPairingPoint*>(this)->mVal, 1);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_write error " LOCATION);
    }

    void REccPairingPoint::fromBytes(u8 * src)
    {
        ep2_read_bin(*this, src, static_cast<int>(sizeBytes()));
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_read error " LOCATION);
    }

    void REccPairingPoint::randomize(const block & seed)
    {
        ep2_map(*this, (u8*)&seed, sizeof(block));
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_map error " LOCATION);
    }

    void REccPairingPoint::randomize()
    {
        ep2_rand(*this);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_rand error " LOCATION);
    }

    bool REccPairingPoint::is_zero() const
    {
        int res = ep2_is_infty(const_cast<REccPairingPoint*>(this)->mVal);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_rand error " LOCATION);
        return res == 1;
    }

    void REccPairingPoint::assign_zero()
    {
        ep2_set_infty(*this);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic ep_rand error " LOCATION);
    }

    void REccPairingPoint::pack(octetStream& os) const
    {
        bool zero = is_zero();
        os.serialize(zero);
        if (zero)
            return;
        int len = sizeBytes();
        os.serialize(len);
        toBytes(os.append(len));
    }

    void REccPairingPoint::unpack(octetStream& os)
    {
        bool zero;
        os.unserialize(zero);
        if (zero)
        {
            assign_zero();
            return;
        }
        int len;
        os.unserialize(len);
        fromBytes(os.consume(len));
    }

    REccPairingPoint REccPairingPoint::operator*(const Scalar& other) const
    {
        REccNumber o(other);
        return *this * o;
    }

    u64 REccNumber::sizeDigits() const
    {
        return bn_size_raw(modulus());
    }

    u64 REccNumber::sizeBytes() const
    {
        return
            bn_size_bin(modulus());
    }

    void REccNumber::toBytes(u8 * dest) const
    {
        bn_write_bin(dest, static_cast<int>(sizeBytes()), *this);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic write error " LOCATION);
    }

    std::string REccNumber::toString(int base) const
    {
       auto size = bn_size_str(*this, base);
       std::string str(size + 1, '\0');
       bn_write_str(&str[0], size, *this, base);
       str.resize(str.find('\0'));
       return str;
    }

    void REccNumber::fromBytes(const u8 * src)
    {
        bn_read_bin(*this, src, static_cast<int>(sizeBytes()));
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic read error " LOCATION);
    }

    void REccNumber::fromHex(const char * src)
    {
        auto len = std::strlen(src);
        bn_read_str(*this, src, static_cast<int>(len), 16);
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic read error " LOCATION);
    }

    void REccNumber::randomize()
    {
        REccNumber g;
        ep_curve_get_ord(g);
        bn_rand_mod(mVal, g);
    }

    void REccNumber::randomize(const block & seed)
    {
        u8* pointer = (u8*)&seed;
        rand_seed(pointer, sizeof(block));
        randomize();
    }

    void REccNumber::fromScalar(const Scalar& in)
    {
        int size = in.size();
        std::vector<unsigned char> tmp(size);
        unsigned char* ptr = (unsigned char*) in.get_ptr();
        for (int i = 0; i < size; i++)
            tmp[i] = ptr[size - i - 1];
        fromBytes(tmp.data());
    }

    REccNumber::Scalar REccNumber::toScalar()
    {
        std::string s = toString(10);
        return bigint(s);
    }

    void REllipticCurve::printParam()
    {
        std::cout << "Using curve:";
        ep_param_print();
        std::cout << "Embedding: " << ep_param_embed() << std::endl;
        if (!ep_curve_is_pairf())
            std::cout << "Curve is NOT pairing friendly!" << std::endl;
    }

    void REllipticCurve::init_field()
    {
        std::string m = getOrder().toString(10);
        std::cout << "Initializing field to p = " << m << std::endl;
        Scalar::init_field(m.c_str(), false);
        Scalar::next::init_field(m.c_str(), false);
    }

    REccPairingResult REllipticCurve::pair(Point& p1, PairingPoint& p2)
    {
        REccPairingResult e;
        pp_map_k12(e.mVal, p1, p2);
        return e;
    }

    bool REccPairingResult::compare(REccPairingResult& pr1, REccPairingResult& pr2)
    {
        return (fp12_cmp(pr1.mVal, pr2.mVal) == RLC_EQ);
    }

    bool REccPairingResult::operator==(const REccPairingResult & cmp) const
    {
        return fp12_cmp(const_cast<REccPairingResult*>(this)->mVal, const_cast<REccPairingResult&>(cmp).mVal) == RLC_EQ;
    }

    bool REccPairingResult::operator!=(const REccPairingResult & cmp) const
    {
        return fp12_cmp(const_cast<REccPairingResult*>(this)->mVal, const_cast<REccPairingResult&>(cmp).mVal) != RLC_EQ;
    }

    REccPairingResult & REccPairingResult::operator=(const REccPairingResult & c)
    {
        fp12_copy(*this, const_cast<REccPairingResult&>(c));
        return *this;
    }

    std::string REllipticCurve::FpToString(fp_t& in, int radix)
    {
        std::string buff(fp_size_str(in, radix) + 1, '\0');

        fp_write_str(&buff[0], static_cast<int>(buff.size()), in, radix);

        buff.resize(buff.find('\0'));
        if (GSL_UNLIKELY(err_get_code()))
            throw std::runtime_error("Relic write error " LOCATION);
        return buff;
    }

}

#endif
