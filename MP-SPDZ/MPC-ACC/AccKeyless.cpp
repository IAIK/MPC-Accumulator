#include "AccKeyless.h"
#include "Tools/time-func.h"

namespace KEYLESS
{
  Auxillary::Auxillary() : set(), op(None)
  {

  }

  void Auxillary::init(Acc::Set& set, Acc::Element& r)
  {
    this->r = r;
    this->set.clear();
    this->set.reserve(set.size());
    this->clearUpdate();
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

  Acc::Set& Auxillary::getSet()
  {
    return set;
  }

  Acc::Accumulator& Auxillary::getAcc()
  {
    return acc;
  }

  void Auxillary::setAcc(Acc::Accumulator& acc)
  {
    this->acc = acc;
  }

  Acc::Element& Auxillary::getR()
  {
    return r;
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

  bool Auxillary::inSet(Acc::Element& el, Acc::Set& new_set)
  {
    new_set.reserve(set.size() - 1);
    bool found = false;
    for (auto& e : set)
    {
      if (e == el)
        found = true;
      else
        new_set.push_back(e);
    }
    return found;
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

  Acc::Acc(REllipticCurve& curve, uint32_t t) : curve(curve), pk(), pk2(), t(t)
  {
  }

  void Acc::Gen()
  {
  #ifdef DEBUG_ACC_KEYLESS
    Timer timer;
    timer.start();
  #endif

    REccNumber sk;
    sk.randomize();
    REccNumber s = sk;

    REccPoint g = curve.getGenerator();
    pk.clear();
    pk.reserve(t + 1);
    pk.push_back(g);
    pk.push_back(g * s);
    for (uint32_t i = 2; i <= t; i++)
    {
      s *= sk;
      pk.push_back(g * s);
    }
    pk2 = curve.getPairingGenerator();
    pk2 *= sk;
  #ifdef DEBUG_ACC_KEYLESS
    std::cout << "Gen took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "sk: " << sk << std::endl;
    std::cout << "pk: " << pk2 << std::endl;
  #endif
  }

  void Acc::Eval(Set& elements, Accumulator& acc, Element& r)
  {
    auto pol = Polynomial::expand(elements);
    acc = pk[0] * (*pol)[0];
    for (int i = 1; i <= pol->degree(); i++)
      acc += pk[i] * (*pol)[i];
    acc *= r;
  }

  void Acc::Eval(Set& elements, Accumulator & acc, Auxillary &aux)
  {
  #ifdef DEBUG_ACC_KEYLESS
    Timer timer;
    timer.start();
  #endif

    REccNumber r;
    r.randomize();
    Eval(elements, acc, r);
    aux.init(elements, r);

  #ifdef DEBUG_ACC_KEYLESS
    std::cout << "Eval took " << timer.elapsed() * 1e3 << " ms for " << elements.size() << " Elements" << std::endl;
    std::cout << "acc: " << acc << std::endl;
  #endif
  }

  void Acc::genRandomSet(Set& set, int num)
  {
  #ifdef DEBUG_ACC_KEYLESS
    Timer timer;
    timer.start();
  #endif
    set.resize(num);

    for (int i = 0; i < num; i++)
    {
      set[i].randomize();
    }
  #ifdef DEBUG_ACC_KEYLESS
    std::cout << "Generated a set of " << set.size() << " Elements in " << timer.elapsed() * 1e3 << " ms for " << std::endl;
  #endif
  }

  bool Acc::WitCreate(Auxillary& aux, Element& el, Witness& wit)
  {
  #ifdef DEBUG_ACC_KEYLESS
    Timer timer;
    timer.start();
  #endif

    Acc::Set new_set;
    bool found = aux.inSet(el, new_set);
    if (!found)
    {
  #ifdef DEBUG_ACC_KEYLESS
      std::cout << "WitCreate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element not found in auxillary data." << std::endl;
  #endif
      return false;
    }

    Acc::Accumulator w;
    Eval(new_set, w, aux.getR());
    wit.init(w, el);

  #ifdef DEBUG_ACC_KEYLESS
    std::cout << "WitCreate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "Witness: " << wit.getWitness() << std::endl;
  #endif
    return true;
  }

  bool Acc::Verify(Accumulator& acc, Element& el, Witness& wit)
  {
  #ifdef DEBUG_ACC_KEYLESS
    Timer timer;
    timer.start();
  #endif

    REccPairingPoint g2 = curve.getPairingGenerator();
    REccPairingResult rp1 = curve.pair(acc, g2);
    g2 *= el;
    g2 += pk2;

    Acc::Wit w = wit.getWitness();
    REccPairingResult rp2 = curve.pair(w, g2);

    // bool result = (wit.getElement == el) && (rp1 == rp2);
    bool result = (rp1 == rp2);
  #ifdef DEBUG_ACC_KEYLESS
    std::cout << "Verify took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "result: " << result << std::endl;
  #endif
    return result;
  }

  bool Acc::Add(Accumulator& acc, Auxillary& aux, Element& el)
  {
  #ifdef DEBUG_ACC_KEYLESS
    Timer timer;
    timer.start();
  #endif

    bool res = aux.add(el);
    if (!res)
    {
  #ifdef DEBUG_ACC_KEYLESS
      std::cout << "Add took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element already found in auxillary data." << std::endl;
  #endif
      return false;
    }

    aux.setAcc(acc);
    Eval(aux.getSet(), acc, aux.getR());

  #ifdef DEBUG_ACC_KEYLESS
    std::cout << "Add took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "acc: " << acc << std::endl;
  #endif
    return true;
  }

  bool Acc::Delete(Accumulator& acc, Auxillary& aux, Element& el)
  {
  #ifdef DEBUG_ACC_KEYLESS
    Timer timer;
    timer.start();
  #endif

    bool res = aux.del(el);
    if (!res)
    {
  #ifdef DEBUG_ACC_KEYLESS
      std::cout << "Delete took " << timer.elapsed() * 1e3 << " ms" << std::endl;
      std::cout << "Element not found in auxillary data." << std::endl;
  #endif
      return false;
    }

    Eval(aux.getSet(), acc, aux.getR());
    aux.setAcc(acc);

  #ifdef DEBUG_ACC_KEYLESS
    std::cout << "Delete took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "acc: " << acc << std::endl;
  #endif
    return true;
  }

  bool Acc::WitUpdate(Witness& wit, Auxillary& aux, Element& el)
  {
  #ifdef DEBUG_ACC_KEYLESS
    Timer timer;
    timer.start();
  #endif

    Acc::Element e = wit.getElement();
    if (!aux.inSet(e) || el == e)
    {
  #ifdef DEBUG_ACC_KEYLESS
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
        w = wit.getWitness() * (el - e) + aux.getAcc();
        wit.update(w);
        break;
      case Auxillary::UPDATE::Delete:
        w = ((wit.getWitness() - aux.getAcc()) * (el - e).inverse());
        wit.update(w);
        break;
      default:
  #ifdef DEBUG_ACC_KEYLESS
        std::cout << "WitUpdate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
        std::cout << "Action: None." << std::endl;
  #endif
        return false;
    }

    aux.clearUpdate();

  #ifdef DEBUG_ACC_KEYLESS
    std::cout << "WitUpdate took " << timer.elapsed() * 1e3 << " ms" << std::endl;
    std::cout << "Action: " << (op == Auxillary::UPDATE::Add ? "Add." : "Delete.") << std::endl;
    std::cout << "Witness: " << wit.getWitness() << std::endl;
  #endif
    return true;
  }

  Polynomial::Polynomial(int degree) : coeffs(degree + 1, 0), deg(degree)
  {
  }


  Acc::Element& Polynomial::operator[](int i)
  {
    return coeffs[i];
  }

  Acc::Element& Polynomial::getCoefficient(int i)
  {
    return coeffs[i];
  }

  void Polynomial::setCoefficient(int i, Acc::Element& el)
  {
    coeffs[i] = el;
  }

  int Polynomial::degree() const
  {
    return deg;
  }

  std::unique_ptr<Polynomial> Polynomial::expand(Acc::Set& roots)
  {
    int degree = roots.size();
    if (degree == 0)
    {
      auto p = make_unique<Polynomial>(0);
      (*p)[0] = 1;
      return p;
    }

    int currentDeg = 1;
    auto r = make_unique<Polynomial>(degree);
    (*r)[0] = roots[0];
    (*r)[1] = 1;

    for (int i = 1; i < degree; i++)
    {
      currentDeg = r->powX(currentDeg);
      for (int j = 0; j < currentDeg; j++)
      {
        (*r)[j] += (*r)[j + 1] * roots[i];
      }
    }

    return r;
  }

  // expands the polynomial by 1
  int Polynomial::powX(int currentDeg)
  {
    if (currentDeg >= deg)
      return currentDeg;

    for (int i = currentDeg; i >= 0; i--)
    {
      coeffs[i + 1] = coeffs[i];
    }
    coeffs[0] = 0;
    return currentDeg + 1;
  }
}
