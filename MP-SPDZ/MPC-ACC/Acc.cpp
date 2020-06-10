#include "Acc.h"
#include "Tools/time-func.h"

namespace WITH_KEY
{
  Auxillary::Auxillary() : set(), op(None)
  {

  }

  void Auxillary::init(Acc::Set& set)
  {
    this->set.clear();
    this->set.reserve(set.size());
    for (auto& el : set)
      this->set.push_back(el);
  }

  bool Auxillary::add(Acc::Element& el)
  {
    if (inSet(el))
      return false;

    set.push_back(el);
    op = Add;
    return true;
  }

  bool Auxillary::del(Acc::Element& el)
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

  Auxillary::UPDATE Auxillary::getUpdate() const
  {
    return op;
  }

  void Auxillary::clearUpdate()
  {
    op = UPDATE::None;
  }

  bool Auxillary::inSet(Acc::Element& el)
  {
    for (auto& e : set)
    {
      if (e == el)
        return true;
    }
    return false;
  }

  Acc::Wit Witness::getWitness() const
  {
    return wit;
  }
  Acc::Element Witness::getElement() const
  {
    return el;
  }

  void Witness::init(Acc::Wit& wit, Acc::Element& el)
  {
    this->wit = wit;
    this->el = el;
  }

  void Witness::update(Acc::Wit& wit)
  {
    this->wit = wit;
  }

  Acc::Acc(REllipticCurve& curve) : curve(curve), sk(), pk()
  {

  }

  void Acc::Gen()
  {
    Timer timer;
    timer.start();

    sk.randomize();
    pk = curve.getPairingGenerator();
    pk *= sk;
  #ifdef DEBUG_ACC
    std::cout << "Gen took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "sk: " << sk << std::endl;
    std::cout << "pk: " << pk << std::endl;
  #endif
  }

  void Acc::Eval(Set& elements, Accumulator & acc, Auxillary &aux)
  {
  #ifdef DEBUG_ACC
    Timer timer;
    timer.start();
  #endif

    REccNumber q;
    q.randomize();

    aux.init(elements);

    for (auto& el : elements) {
      q *= (el + sk);
    }

    acc = curve.getGenerator();
    acc *= q;

  #ifdef DEBUG_ACC
    std::cout << "Eval took " << timer.elapsed() * 1e3 << " ms for " << elements.size() << " Elements" << std::endl;
    std::cout << "acc: " << acc << std::endl;
  #endif
  }

  void Acc::genRandomSet(Set& set, int num)
  {
  #ifdef DEBUG_ACC
    Timer timer;
    timer.start();
  #endif
    set.resize(num);

    for (int i = 0; i < num; i++)
    {
      set[i].randomize();
    }
  #ifdef DEBUG_ACC
    std::cout << "Generated a set of " << set.size() << " Elements in " << timer.elapsed() * 1e3 << " ms for " << std::endl;
  #endif
  }

  bool Acc::WitCreate(Accumulator& acc, Auxillary& aux, Element& el, Witness& wit)
  {
  #ifdef DEBUG_ACC
    Timer timer;
    timer.start();
  #endif

    bool found = aux.inSet(el);
    if (!found)
    {
  #ifdef DEBUG_ACC
      std::cout << "WitCreate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element not found in auxillary data." << std::endl;
  #endif
      return false;
    }

    Acc::Wit w = acc * (el + sk).inverse();
    wit.init(w, el);

  #ifdef DEBUG_ACC
    std::cout << "WitCreate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "Witness: " << wit.getWitness() << std::endl;
  #endif
    return true;
  }

  bool Acc::Verify(Accumulator& acc, Element& el, Witness& wit)
  {
  #ifdef DEBUG_ACC
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
  #ifdef DEBUG_ACC
    std::cout << "Verify took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "result: " << result << std::endl;
  #endif
    return result;
  }

  bool Acc::Add(Accumulator& acc, Auxillary& aux, Element& el)
  {
  #ifdef DEBUG_ACC
    Timer timer;
    timer.start();
  #endif

    bool res = aux.add(el);
    if (!res)
    {
  #ifdef DEBUG_ACC
      std::cout << "Add took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element already found in auxillary data." << std::endl;
  #endif
      return false;
    }

    acc *= (el + sk);

  #ifdef DEBUG_ACC
    std::cout << "Add took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "acc: " << acc << std::endl;
  #endif
    return true;
  }

  bool Acc::Delete(Accumulator& acc, Auxillary& aux, Element& el)
  {
  #ifdef DEBUG_ACC
    Timer timer;
    timer.start();
  #endif

    bool res = aux.del(el);
    if (!res)
    {
  #ifdef DEBUG_ACC
      std::cout << "Delete took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element not found in auxillary data." << std::endl;
  #endif
      return false;
    }

    acc *= (el + sk).inverse();

  #ifdef DEBUG_ACC
    std::cout << "Delete took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "acc: " << acc << std::endl;
  #endif
    return true;
  }

  bool Acc::WitUpdate(Witness& wit, Auxillary& aux, Element& el)
  {
  #ifdef DEBUG_ACC
    Timer timer;
    timer.start();
  #endif
    Acc::Element e = wit.getElement();
    if (!aux.inSet(e) || el == e)
    {
  #ifdef DEBUG_ACC
      std::cout << "WitUpdate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element not found in auxillary data." << std::endl;
  #endif
      return false;
    }

    Auxillary::UPDATE op = aux.getUpdate();
    Acc::Wit w;

    switch(op)
    {
      case Auxillary::UPDATE::Add:
        w = wit.getWitness() * (el + sk);
        wit.update(w);
        break;
      case Auxillary::UPDATE::Delete:
        w = wit.getWitness() * (el + sk).inverse();
        wit.update(w);
        break;
      default:
  #ifdef DEBUG_ACC
        std::cout << "WitUpdate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
        std::cout << "Action: None." << std::endl;
  #endif
        return false;
    }

    aux.clearUpdate();

  #ifdef DEBUG_ACC
    std::cout << "WitUpdate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "Action: " << (op == Auxillary::UPDATE::Add ? "Add." : "Delete.") << std::endl;
    std::cout << "Witness: " << wit.getWitness() << std::endl;
  #endif
    return true;
  }
}
