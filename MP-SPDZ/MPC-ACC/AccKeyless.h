#pragma once

#include "RCurve.h"

// #define DEBUG_ACC_KEYLESS

using osuCrypto::REllipticCurve;
using osuCrypto::REccNumber;
using osuCrypto::REccPoint;
using osuCrypto::REccPairingPoint;
using osuCrypto::REccPairingResult;

namespace KEYLESS
{
  class Witness;
  class Auxillary;

  class Acc
  {
  public:
    typedef REccNumber Element;
    typedef std::vector<Element> Set;
    typedef REccPoint Accumulator;
    typedef REccPoint Wit;

  private:
    REllipticCurve& curve;
    std::vector<REccPoint> pk;
    REccPairingPoint pk2;
    uint32_t t;

    void Eval(Set&, Accumulator&, Element&);

  public:
    Acc(REllipticCurve&, uint32_t t);
    ~Acc() = default;

    Acc(const Acc&) = delete;
    Acc& operator=(const Acc&) = delete;

    void Gen();
    void Eval(Set&, Accumulator&, Auxillary&);
    bool WitCreate(Auxillary&, Element&, Witness&);
    bool Verify(Accumulator&, Element&, Witness&);
    bool Add(Accumulator&, Auxillary&, Element&);
    bool Delete(Accumulator&, Auxillary&, Element&);
    bool WitUpdate(Witness&, Auxillary&, Element&);

    static void genRandomSet(Set&, int);
  };

  class Auxillary
  {
  public:
    enum UPDATE
    {
      None,
      Add,
      Delete
    };

  private:
    Acc::Set set;
    UPDATE op;
    Acc::Element r;
    Acc::Accumulator acc;

  public:
    Auxillary();
    ~Auxillary() = default;
    Auxillary(const Auxillary&) = delete;
    Auxillary& operator=(const Auxillary&) = delete;
    void init(Acc::Set&, Acc::Element&);
    bool add(Acc::Element&);
    bool del(Acc::Element&);

    UPDATE getUpdate() const;
    void clearUpdate();
    Acc::Set& getSet();
    Acc::Accumulator& getAcc();
    void setAcc(Acc::Accumulator&);

    Acc::Element& getR();
    bool inSet(Acc::Element&);
    bool inSet(Acc::Element&, Acc::Set&);
  };

  class Witness
  {
    private:
      Acc::Wit wit;
      Acc::Element el;

    public:
      Witness() = default;
      ~Witness() = default;
      Witness(const Witness&) = delete;
      Witness& operator=(const Witness&) = delete;

      Acc::Wit getWitness() const;
      Acc::Element getElement() const;

      void init(Acc::Wit&, Acc::Element&);
      void update(Acc::Wit&);
  };

  template<typename T, typename... Args>
  std::unique_ptr<T> make_unique(Args&&... args) {
      return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  class Polynomial
  {
    private:
      std::vector<Acc::Element> coeffs;
      int deg;

      int powX(int);

    public:
      Polynomial(int degree);
      ~Polynomial() = default;
      Polynomial(const Polynomial&) = delete;
      Polynomial& operator=(const Polynomial&) = delete;

      Acc::Element& getCoefficient(int);
      void setCoefficient(int, Acc::Element&);
      int degree() const;

      Acc::Element& operator[](int i);

      static std::unique_ptr<Polynomial> expand(Acc::Set&);
  };
}
