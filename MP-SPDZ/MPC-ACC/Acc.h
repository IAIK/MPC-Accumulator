#pragma once

#include "RCurve.h"

// #define DEBUG_ACC

using osuCrypto::REllipticCurve;
using osuCrypto::REccNumber;
using osuCrypto::REccPoint;
using osuCrypto::REccPairingPoint;
using osuCrypto::REccPairingResult;

namespace WITH_KEY
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
    REccNumber sk;
    REccPairingPoint pk;

  public:
    Acc(REllipticCurve&);
    ~Acc() = default;

    Acc(const Acc&) = delete;
    Acc& operator=(const Acc&) = delete;

    void Gen();
    void Eval(Set&, Accumulator&, Auxillary&);
    bool WitCreate(Accumulator&, Auxillary&, Element&, Witness&);
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

  public:
    Auxillary();
    ~Auxillary() = default;
    Auxillary(const Auxillary&) = delete;
    Auxillary& operator=(const Auxillary&) = delete;
    void init(Acc::Set&);
    bool add(Acc::Element&);
    bool del(Acc::Element&);

    UPDATE getUpdate() const;
    void clearUpdate();
    bool inSet(Acc::Element&);
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
}
