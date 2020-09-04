// RCurve.h
//
// The implementation in this file is an extension/modification
// of the Relic wrapper in:
//    https://github.com/ladnir/cryptoTools/blob/master/cryptoTools/Crypto/RCurve.h


#pragma once

#include "Defines.h"
#include "Math/gfp.h"

#ifdef ENABLE_RELIC

extern "C" {
    #include <relic/relic_epx.h>
}

#ifdef SEED
#undef SEED
#endif

#ifdef HASHD
#undef HASHD
#endif


#ifdef ALIGN
#undef ALIGN
#endif

#ifdef MONTY
#undef MONTY
#endif


namespace osuCrypto
{

    class REllipticCurve;
    class REccPoint;
    class REccPairingPoint;
    class REccPairingResult;

    class REccNumber
    {
    public:
        typedef gfp_<100, 4> Scalar;
        REccNumber();
        REccNumber(const REccNumber& num);
        REccNumber(const i32& val);
        REccNumber(const Scalar& num);

        void fromScalar(const Scalar& in);
        Scalar toScalar();

        // backwards compatible constructors
        REccNumber(REllipticCurve&);
        REccNumber(REllipticCurve&, const REccNumber& num);
        REccNumber(REllipticCurve&, const i32& val);

        ~REccNumber();

        REccNumber& operator=(const REccNumber& c);
        REccNumber& operator=(const bn_t c);
        REccNumber& operator=(int i);


        REccNumber& operator++();
        REccNumber& operator--();
        REccNumber& operator+=(int i);
        REccNumber& operator-=(int i);
        REccNumber& operator+=(const REccNumber& b);
        REccNumber& operator-=(const REccNumber& b);
        REccNumber& operator*=(const REccNumber& b);
        REccNumber& operator*=(int i);
        REccNumber& operator/=(const REccNumber& b);
        REccNumber& operator/=(int i);
        //void inplaceNegate();
        REccNumber negate() const;
        REccNumber inverse() const;

        bool operator==(const REccNumber& cmp) const;
        bool operator==(const int& cmp)const;
        friend bool operator==(const int& cmp1, const REccNumber& cmp2);
        bool operator!=(const REccNumber& cmp)const;
        bool operator!=(const int& cmp)const;
        friend bool operator!=(const int& cmp1, const REccNumber& cmp2);

        bool operator>=(const REccNumber& cmp)const;
        bool operator>=(const int& cmp)const;

        bool operator<=(const REccNumber& cmp)const;
        bool operator<=(const int& cmp)const;

        bool operator>(const REccNumber& cmp)const;
        bool operator>(const int& cmp)const;

        bool operator<(const REccNumber& cmp)const;
        bool operator<(const int& cmp)const;

        bool isPrime() const;
        bool iszero() const;

        //const REccNumber& modulus() const;

        friend REccNumber operator-(const REccNumber&);
        friend REccNumber operator+(const REccNumber&, int);
        friend REccNumber operator+(int, const REccNumber&);
        friend REccNumber operator+(const REccNumber&, const REccNumber&);

        friend REccNumber operator-(const REccNumber&, int);
        friend REccNumber operator-(int, const REccNumber&);
        friend REccNumber operator-(const REccNumber&, const REccNumber&);

        friend REccNumber operator*(const REccNumber&, int);
        friend REccNumber operator*(int, const REccNumber&);
        friend REccNumber operator*(const REccNumber&, const REccNumber&);

        friend REccNumber operator/(const REccNumber&, int);
        friend REccNumber operator/(int, const REccNumber&);
        friend REccNumber operator/(const REccNumber&, const REccNumber&);

        friend REccNumber operator^(const REccNumber& base, const REccNumber& exp);

        u64 sizeDigits() const;
        u64 sizeBytes() const;
        void toBytes(u8* dest) const;
        std::string toString(int base) const;
        void fromBytes(const u8* src);
        void fromHex(const char* src);
        //void fromDec(const char* src);

        void randomize();
        void randomize(const block& seed);

        operator bn_t& () { return mVal; }
        operator const bn_t& () const { return mVal; }

    private:

        void init();
        void reduce();

        const bn_st* modulus()const;

    public:
        bn_t  mVal;

        friend class REllipticCurve;
        friend REccPoint;
        friend std::ostream& operator<<(std::ostream& out, const REccNumber& val);
    };
    std::ostream& operator<<(std::ostream& out, const REccNumber& val);


    class REccPoint : public ValueInterface
    {
    public:
        // for sharing
        typedef REccNumber::Scalar Scalar;

        typedef REccPoint next;
        typedef void Square;

        bool is_zero() const;
        void assign_zero();
        void pack(octetStream& os) const;
        void unpack(octetStream& os);
        template<int T>
        void add(octetStream& os) { *this += os.get<REccPoint>(); }
        static int size() { return 0; }
        static string type_string() { return "REccPoint"; }
        REccPoint operator*(const Scalar& other) const;
        void mul(const REccPoint& x, const Scalar& y) { *this = x * y; }
        void mul(const Scalar& x, const REccPoint& y) { *this = y * x; }
        void add(const REccPoint& x, const REccPoint& y) { *this = x + y; }
        static char type_char() { return 'G'; }
        // end for sharing

        REccPoint() { ep_new(mVal); assign_zero(); };
        REccPoint(const Scalar& other);
        REccPoint(int other);
        REccPoint(const REccPoint& copy) { ep_new(mVal); ep_copy(*this, copy); }

        // backwards compatible constructors
        REccPoint(REllipticCurve&) { ep_new(mVal); };
        REccPoint(REllipticCurve&, const REccPoint& copy) { ep_new(mVal); ep_copy(*this, copy);}

        ~REccPoint() { ep_free(mVal); }

        REccPoint& operator=(const REccPoint& copy);
        REccPoint& operator+=(const REccPoint& addIn);
        REccPoint& operator-=(const REccPoint& subtractIn);
        REccPoint& operator*=(const REccNumber& multIn);


        REccPoint operator+(const REccPoint& addIn) const;
        REccPoint operator-(const REccPoint& subtractIn) const;
        REccPoint operator*(const REccNumber& multIn) const;

        bool operator==(const REccPoint& cmp) const;
        bool operator!=(const REccPoint& cmp) const;

        u64 sizeBytes() const;
        void toBytes(u8* dest) const;
        std::vector<std::string> toString(int base) const;
        void fromBytes(u8* src);
        //void fromHex(char* x, char* y);
        //void fromDec(char* x, char* y);
        //void fromNum(REccNumber& x, REccNumber& y);

        void randomize(const block& seed);
        void randomize();

        operator ep_t& () { return mVal; }
        operator const ep_t& () const { return mVal; }

        ep_t mVal;
    private:

        friend REccNumber;
        friend std::ostream& operator<<(std::ostream& out, const REccPoint& val);
    };

    class REccPairingPoint  : public ValueInterface
    {
    public:
        // for sharing
        typedef REccNumber::Scalar Scalar;

        typedef REccPairingPoint next;
        typedef void Square;

        bool is_zero() const;
        void assign_zero();
        void pack(octetStream& os) const;
        void unpack(octetStream& os);
        template<int T>
        void add(octetStream& os) { *this += os.get<REccPairingPoint>(); }
        static int size() { return 0; }
        static string type_string() { return "REccPairingPoint"; }
        REccPairingPoint operator*(const Scalar& other) const;
        void mul(const REccPairingPoint& x, const Scalar& y) { *this = x * y; }
        void mul(const Scalar& x, const REccPairingPoint& y) { *this = y * x; }
        void add(const REccPairingPoint& x, const REccPairingPoint& y) { *this = x + y; }
        static char type_char() { return 'G'; }
        // end for sharing

        REccPairingPoint() { ep2_new(mVal); assign_zero(); };
        REccPairingPoint(const Scalar& other);
        REccPairingPoint(int other);
        REccPairingPoint(const REccPairingPoint& copy) { ep2_new(mVal); ep2_copy(*this, const_cast<REccPairingPoint&>(copy)); }

        // backwards compatible constructors
        REccPairingPoint(REllipticCurve&) { ep2_new(mVal); };
        REccPairingPoint(REllipticCurve&, const REccPairingPoint& copy) { ep2_new(mVal); ep2_copy(*this, const_cast<REccPairingPoint&>(copy));}

        ~REccPairingPoint() { ep2_free(mVal); }

        REccPairingPoint& operator=(const REccPairingPoint& copy);
        REccPairingPoint& operator+=(const REccPairingPoint& addIn);
        REccPairingPoint& operator-=(const REccPairingPoint& subtractIn);
        REccPairingPoint& operator*=(const REccNumber& multIn);


        REccPairingPoint operator+(const REccPairingPoint& addIn) const;
        REccPairingPoint operator-(const REccPairingPoint& subtractIn) const;
        REccPairingPoint operator*(const REccNumber& multIn) const;

        bool operator==(const REccPairingPoint& cmp) const;
        bool operator!=(const REccPairingPoint& cmp) const;

        u64 sizeBytes() const;
        void toBytes(u8* dest) const;
        void fromBytes(u8* src);
        //void fromHex(char* x, char* y);
        //void fromDec(char* x, char* y);
        //void fromNum(REccNumber& x, REccNumber& y);

        void randomize(const block& seed);
        void randomize();


        operator ep2_t& () { return mVal; }
        operator const ep2_t& () const { return mVal; }

        ep2_t mVal;
    private:

        friend REccNumber;
        friend std::ostream& operator<<(std::ostream& out, const REccPairingPoint& val);
    };

    std::ostream& operator<<(std::ostream& out, const REccPairingPoint& val);

    class REccPairingResult
    {
    public:
        REccPairingResult() { fp12_new(mVal); };
        REccPairingResult(const REccPairingResult& copy) { fp12_new(mVal); fp12_copy(*this, const_cast<REccPairingResult&>(copy)); }

        // backwards compatible constructors
        REccPairingResult(REllipticCurve&) { fp12_new(mVal); };
        REccPairingResult(REllipticCurve&, const REccPairingResult& copy) { fp12_new(mVal); fp12_copy(*this, const_cast<REccPairingResult&>(copy));}

        REccPairingResult& operator=(const REccPairingResult& copy);

        ~REccPairingResult() { fp12_free(mVal); }

        static bool compare(REccPairingResult&, REccPairingResult&);
        bool operator==(const REccPairingResult& cmp) const;
        bool operator!=(const REccPairingResult& cmp) const;

        operator fp12_t& () { return mVal; }
        operator const fp12_t& () const { return mVal; }

        fp12_t mVal;
    };

    class REllipticCurve
    {
    public:
        typedef REccNumber::Scalar Scalar;
        typedef REccPoint Point;
        typedef REccPairingPoint PairingPoint;

        REllipticCurve(bool paring = false);
        REllipticCurve(int, int);
        ~REllipticCurve();

        Point getGenerator() const;
        PairingPoint getPairingGenerator() const;
        std::vector<Point> getGenerators() const;
        REccNumber getOrder() const;

        void printParam();

        void init_field();

        REccPairingResult pair(Point&, PairingPoint&);

        static std::string FpToString(fp_t&, int radix);

    private:

        friend Point;
        friend PairingPoint;
        friend REccNumber;
    };
}

#endif
