#pragma once

#include "Processor/Data_Files.h"

#include "ECExtension.h"

// #define DEBUG_MPCACC

using osuCrypto::REllipticCurve;
using osuCrypto::REccNumber;
using osuCrypto::REccPoint;
using osuCrypto::REccPairingPoint;
using osuCrypto::REccPairingResult;

namespace MPC_ACC
{
  template<template<class U> class T>
  class Witness;

  template<template<class U> class T>
  class Auxillary;

  template<template<class U> class T>
  class Acc
  {
  public:
    typedef T<REllipticCurve::Scalar> sShare;
    typedef T<REccPoint> pShare;
    typedef T<REccPairingPoint> ppShare;
    typedef REllipticCurve::Scalar Element;
    typedef std::vector<Element> Set;
    typedef REccPoint Accumulator;
    typedef REccPoint Wit;

  private:
    REllipticCurve& curve;
    SubProcessor<sShare>& proc;
    typename pShare::Direct_MC MCp;
    typename ppShare::Direct_MC MCpp;
    Player& P;
    int batchsize;

    sShare getRandom(SubProcessor<sShare>& proc);

    sShare sk_share;
    REccPairingPoint pk;

  public:
    Acc<T>(REllipticCurve&, SubProcessor<sShare>&, int);
    ~Acc<T>() = default;

    Acc<T>(const Acc<T>&) = delete;
    Acc<T>& operator=(const Acc<T>&) = delete;

    void Gen();
    void Eval(Set&, Accumulator&, Auxillary<T>&);
    bool WitCreate(Accumulator&, Auxillary<T>&, Element&, Witness<T>&);
    bool Verify(Accumulator&, Element&, Witness<T>&);
    bool Add(Accumulator&, Auxillary<T>&, Element&);
    bool Delete(Accumulator&, Auxillary<T>&, Element&);
    bool WitUpdate(Witness<T>&, Auxillary<T>&, Element&);

    void preprocessing(int inverses = 0);
    SubProcessor<sShare>& getProc() const { return proc; };
    static void genSet(Set&, int);
    static void genElement(Element&, int);
  };

  //----------------------------------------------------------------------

  template<template<class U> class T>
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
    typename Acc<T>::Set set;
    UPDATE op;

  public:
    Auxillary();
    ~Auxillary() = default;
    Auxillary(const Auxillary&) = delete;
    Auxillary& operator=(const Auxillary&) = delete;
    void init(typename Acc<T>::Set&);
    bool add(typename Acc<T>::Element&);
    bool del(typename Acc<T>::Element&);

    UPDATE getUpdate() const;
    void clearUpdate();
    bool inSet(typename Acc<T>::Element&);
  };

  //----------------------------------------------------------------------

  template<template<class U> class T>
  class Witness
  {
    private:
      typename Acc<T>::Wit wit;
      typename Acc<T>::Element el;

    public:
      Witness() = default;
      ~Witness() = default;
      Witness(const Witness&) = delete;
      Witness& operator=(const Witness&) = delete;

      typename Acc<T>::Wit getWitness() const;
      typename Acc<T>::Element getElement() const;

      void init(typename Acc<T>::Wit&, typename Acc<T>::Element&);
      void update(typename Acc<T>::Wit&);
  };

  //----------------------------------------------------------------------

  template<template<class U> class T>
  Auxillary<T>::Auxillary() : set(), op(None)
  {

  }

  template<template<class U> class T>
  void Auxillary<T>::init(typename Acc<T>::Set& set)
  {
    this->set.clear();
    this->set.reserve(set.size());
    for (auto& el : set)
      this->set.push_back(el);
  }

  template<template<class U> class T>
  bool Auxillary<T>::add(typename Acc<T>::Element& el)
  {
    if (inSet(el))
      return false;

    set.push_back(el);
    op = Add;
    return true;
  }

  template<template<class U> class T>
  bool Auxillary<T>::del(typename Acc<T>::Element& el)
  {
    for (auto it = set.begin(); it != set.end(); it++)
    {
      if (*it == el)
      {
        set.erase(it);
        op = Delete;
        return true;
      }
    }
    return false;
  }

  template<template<class U> class T>
  typename Auxillary<T>::UPDATE Auxillary<T>::getUpdate() const
  {
    return op;
  }

  template<template<class U> class T>
  void Auxillary<T>::clearUpdate()
  {
    op = UPDATE::None;
  }

  template<template<class U> class T>
  bool Auxillary<T>::inSet(typename Acc<T>::Element& el)
  {
    for (auto& e : set)
    {
      if (e == el)
        return true;
    }
    return false;
  }

  //----------------------------------------------------------------------

  template<template<class U> class T>
  typename Acc<T>::Wit Witness<T>::getWitness() const
  {
    return wit;
  }

  template<template<class U> class T>
  typename Acc<T>::Element Witness<T>::getElement() const
  {
    return el;
  }

  template<template<class U> class T>
  void Witness<T>::init(typename Acc<T>::Wit& wit, typename Acc<T>::Element& el)
  {
    this->wit = wit;
    this->el = el;
  }

  template<template<class U> class T>
  void Witness<T>::update(typename Acc<T>::Wit& wit)
  {
    this->wit = wit;
  }

  //----------------------------------------------------------------------

  template<template<class U> class T>
  Acc<T>::Acc(REllipticCurve& curve, SubProcessor<sShare>& proc, int batchsize) : curve(curve), proc(proc), MCp(proc.MC.get_alphai()), MCpp(proc.MC.get_alphai()), P(proc.P), batchsize(batchsize)
  {
  }

  template<template<class U> class T>
  typename Acc<T>::sShare Acc<T>::getRandom(SubProcessor<sShare>& proc)
  {
    sShare out,_;
    auto& prep = proc.DataF;
    prep.get_two(DATA_INVERSE, out, _);
    return out;
  }

  template<template<class U> class T>
  void Acc<T>::preprocessing(int n_inverses)
  {
  #ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
  #endif

    auto& prep = proc.DataF;
  #ifdef DEBUG_MPCACC
    size_t start = P.sent + prep.data_sent();
  #endif
    if (n_inverses)
    {
      prep.buffer_triples();
      OnlineOptions::singleton.batch_size = n_inverses;
      prep.buffer_inverses();
      OnlineOptions::singleton.batch_size = batchsize;
    }
    else
    {
      prep.buffer_triples();
    }

  #ifdef DEBUG_MPCACC
    cout << "Generated " << batchsize << " triples in " << timer.elapsed()
            << " seconds, throughput " << batchsize / timer.elapsed() << ", "
            << 1e-3 * (P.sent + prep.data_sent() - start) / batchsize
            << " kbytes per triple" << endl;
    if (n_inverses)
      cout << "Reserverd " << n_inverses << " triples for random elements or inverses" << endl;
  #endif
  }

  template<template<class U> class T>
  void Acc<T>::genSet(Set& set, int num)
  {
  #ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
  #endif
    set.reserve(num);

    Element k, d;

    k = bigint("33515653444815246004843726576806650060199450033327940487234795206578493484707");
    d = bigint("33285192329523448485054943497169911344402135879957193677198722611089929530215");

    for (int i = 0; i < num; i++)
    {
      set.push_back(k * i + d);
    }
  #ifdef DEBUG_MPCACC
    std::cout << "Generated a set of " << set.size() << " Elements in " << timer.elapsed() * 1e3 << " ms for " << std::endl;
  #endif
  }

  template<template<class U> class T>
  void Acc<T>::genElement(Element& el, int num)
  {
    Element k, d;

    k = bigint("33515653444815246004843726576806650060199450033327940487234795206578493484707");
    d = bigint("33285192329523448485054943497169911344402135879957193677198722611089929530215");

    el = (k * num + d);
  }

  template<template<class U> class T>
  void Acc<T>::Gen()
  {
#ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
    auto stats = P.comm_stats;
#endif
    sk_share = getRandom(proc);

    ppShare pk_share = mul(curve.getPairingGenerator(), sk_share);
    pk = MCpp.open(pk_share, P);
    MCpp.Check(P);

#ifdef DEBUG_MPCACC
    std::cout << "Gen took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "sk share: " << sk_share << std::endl;
    std::cout << "pk: " << pk << std::endl;
    (P.comm_stats - stats).print(true);
#endif
  }

  template<template<class U> class T>
  void Acc<T>::Eval(Set& elements, Accumulator& acc, Auxillary<T>& aux)
  {
  #ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
    auto stats = P.comm_stats;
  #endif
    auto& protocol = proc.protocol;
    auto alpha_i = proc.MC.get_alphai();

    sShare r = getRandom(proc);

    aux.init(elements);

    std::vector<sShare> multree;
    multree.reserve(elements.size() + 1);
    multree.push_back(r);
    for (auto& el : elements)
      multree.push_back(sk_share + sShare::constant(el, P.my_num(), alpha_i));

    auto len = multree.size();

    // tree mul
    while (len != 1)
    {
      auto new_len = len / 2;
      protocol.init_mul(&proc);
      for (unsigned int i = 0; i < new_len; i++) {
        protocol.prepare_mul(multree[2 * i], multree[2 * i + 1]);
      }
      protocol.exchange();
      for (unsigned int i = 0; i < new_len; i++)
        multree[i] = protocol.finalize_mul();
      if (len % 2)
      {
        multree[new_len] = multree[len - 1];
        new_len++;
      }
      // multree.resize(new_len);
      len = new_len;
    }

    // non tree mul
    // for (unsigned int i = 1; i < len; i++)
    // {
    //   protocol.init_mul(&proc);
    //   protocol.prepare_mul(multree[0], multree[i]);
    //   protocol.exchange();
    //   multree[0] = protocol.finalize_mul();
    // }

    pShare acc_share = mul(curve.getGenerator(), multree[0]);
    acc = MCp.open(acc_share, P);
    proc.MC.Check(P);
    MCp.Check(P);

  #ifdef DEBUG_MPCACC
    std::cout << "Eval took " << timer.elapsed() * 1e3 << " ms for " << elements.size() << " Elements" << std::endl;
    std::cout << "acc: " << acc << std::endl;
    (P.comm_stats - stats).print(true);
  #endif
  }

  template<template<class U> class T>
  bool Acc<T>::WitCreate(Accumulator& acc, Auxillary<T>& aux, Element& el, Witness<T>& wit)
  {
  #ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
    auto stats = P.comm_stats;
  #endif
    auto& protocol = proc.protocol;
    auto alpha_i = proc.MC.get_alphai();

    bool found = aux.inSet(el);
    if (!found)
    {
  #ifdef DEBUG_MPCACC
      std::cout << "WitCreate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element not found in auxillary data." << std::endl;
  #endif
      return false;
    }

    sShare q = sk_share + sShare::constant(el, P.my_num(), alpha_i);

    sShare r = getRandom(proc);

    protocol.init_mul(&proc);
    protocol.prepare_mul(r, q);
    protocol.exchange();
    auto sigma = proc.MC.open(protocol.finalize_mul(), P);
    sigma.invert();
    q = sigma * r;

    pShare wit_share = mul(acc, q);
    Acc::Wit w = MCp.open(wit_share, P);
    wit.init(w, el);

    proc.MC.Check(P);
    MCp.Check(P);

  #ifdef DEBUG_MPCACC
    std::cout << "WitCreate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "Witness: " << wit.getWitness() << std::endl;
    (P.comm_stats - stats).print(true);
  #endif
    return true;
  }

  template<template<class U> class T>
  bool Acc<T>::Verify(Accumulator& acc, Element& el, Witness<T>& wit)
  {
  #ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
  #endif

    REccPairingPoint g2 = curve.getPairingGenerator();
    REccPairingResult rp1 = curve.pair(acc, g2);
    g2 *= el;
    g2 += pk;

    Acc::Wit w = wit.getWitness();
    REccPairingResult rp2 = curve.pair(w, g2);

    // bool result = (wit.getElement == el) && (rp1 == rp2);
    bool result = (rp1 == rp2);
  #ifdef DEBUG_MPCACC
    std::cout << "Verify took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "result: " << result << std::endl;
  #endif
    return result;
  }

  template<template<class U> class T>
  bool Acc<T>::Add(Accumulator& acc, Auxillary<T>& aux, Element& el)
  {
  #ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
    auto stats = P.comm_stats;
  #endif
    auto alpha_i = proc.MC.get_alphai();

    bool res = aux.add(el);
    if (!res)
    {
  #ifdef DEBUG_MPCACC
      std::cout << "Add took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element already found in auxillary data." << std::endl;
  #endif
      return false;
    }

    sShare q = sk_share + sShare::constant(el, P.my_num(), alpha_i);
    pShare acc_share = mul(acc, q);
    acc = MCp.open(acc_share, P);

    MCp.Check(P);

  #ifdef DEBUG_MPCACC
    std::cout << "Add took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "acc: " << acc << std::endl;
    (P.comm_stats - stats).print(true);
  #endif
    return true;
  }

  template<template<class U> class T>
  bool Acc<T>::Delete(Accumulator& acc, Auxillary<T>& aux, Element& el)
  {
  #ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
    auto stats = P.comm_stats;
  #endif
    auto& protocol = proc.protocol;
    auto alpha_i = proc.MC.get_alphai();

    bool res = aux.del(el);
    if (!res)
    {
  #ifdef DEBUG_MPCACC
      std::cout << "Delete took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element not found in auxillary data." << std::endl;
  #endif
      return false;
    }

    sShare q = sk_share + sShare::constant(el, P.my_num(), alpha_i);

    sShare r = getRandom(proc);

    protocol.init_mul(&proc);
    protocol.prepare_mul(r, q);
    protocol.exchange();
    auto sigma = proc.MC.open(protocol.finalize_mul(), P);
    sigma.invert();
    q = sigma * r;

    pShare acc_share = mul(acc, q);
    acc = MCp.open(acc_share, P);

    proc.MC.Check(P);
    MCp.Check(P);

  #ifdef DEBUG_MPCACC
    std::cout << "Delete took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "acc: " << acc << std::endl;
    (P.comm_stats - stats).print(true);
  #endif
    return true;
  }

  template<template<class U> class T>
  bool Acc<T>::WitUpdate(Witness<T>& wit, Auxillary<T>& aux, Element& el)
  {
  #ifdef DEBUG_MPCACC
    Timer timer;
    timer.start();
    auto stats = P.comm_stats;
  #endif
    auto& protocol = proc.protocol;
    auto alpha_i = proc.MC.get_alphai();

    Acc::Element e = wit.getElement();
    if (!aux.inSet(e) || el == e)
    {
  #ifdef DEBUG_MPCACC
      std::cout << "WitUpdate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element not found in auxillary data." << std::endl;
  #endif
      return false;
    }

    typename Auxillary<T>::UPDATE op = aux.getUpdate();
    Acc::Wit w = wit.getWitness();
    sShare q, r;
    q = sk_share + sShare::constant(el, P.my_num(), alpha_i);
    pShare wit_share;
    REllipticCurve::Scalar sigma;

    switch(op)
    {
      case Auxillary<T>::UPDATE::Add:
        wit_share = mul(w, q);
        w = MCp.open(wit_share, P);
        wit.update(w);

        MCp.Check(P);
        break;
      case Auxillary<T>::UPDATE::Delete:
        r = getRandom(proc);

        protocol.init_mul(&proc);
        protocol.prepare_mul(r, q);
        protocol.exchange();
        sigma = proc.MC.open(protocol.finalize_mul(), P);
        sigma.invert();
        q = sigma * r;

        wit_share = mul(w, q);
        w = MCp.open(wit_share, P);
        wit.update(w);

        proc.MC.Check(P);
        MCp.Check(P);
        break;
      default:
  #ifdef DEBUG_MPCACC
        std::cout << "WitUpdate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
        std::cout << "Action: None." << std::endl;
  #endif
        return false;
    }

    aux.clearUpdate();

  #ifdef DEBUG_MPCACC
    std::cout << "WitUpdate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "Action: " << (op == Auxillary<T>::UPDATE::Add ? "Add." : "Delete.") << std::endl;
    std::cout << "Witness: " << wit.getWitness() << std::endl;
    (P.comm_stats - stats).print(true);
  #endif
    return true;
  }

}
