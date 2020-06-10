#include "test.h"

bool testNonMPCAcc(osuCrypto::REllipticCurve curve)
{
    using WITH_KEY::Acc;
    using WITH_KEY::Auxillary;
    using WITH_KEY::Witness;

    // non-mpc accumulator with access to private key
    std::cout << "Testing Non-MPC acc with access to the secret key" << std::endl;
    Acc acc_class(curve);
    acc_class.Gen();
    Acc::Accumulator acc;
    Auxillary aux;
    Acc::Set el;
    acc_class.genRandomSet(el, SET_LENGTH);
    acc_class.Eval(el, acc, aux);
    Witness wit;
    if(!acc_class.WitCreate(acc, aux, el[0], wit))
        return false;

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[1], wit))
        return false;

    Acc::Element new_el;
    new_el.randomize();

    if (!acc_class.Add(acc, aux, new_el))
      return false;

    Witness old_wit;
    Acc::Wit tmp1 = wit.getWitness();
    Acc::Element tmp2 = wit.getElement();
    old_wit.init(tmp1, tmp2);
    if (!acc_class.WitUpdate(wit, aux, new_el))
        return false;

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[0], old_wit))
        return false;

    if (!acc_class.Delete(acc, aux, el[2]))
      return false;

    tmp1 = wit.getWitness();
    tmp2 = wit.getElement();
    old_wit.init(tmp1, tmp2);
    if (!acc_class.WitUpdate(wit, aux, el[2]))
        return false;

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[0], old_wit))
        return false;

    if (!acc_class.Delete(acc, aux, el[0]))
      return false;

    if (acc_class.WitUpdate(wit, aux, el[0]))
        return false;

    if (acc_class.Verify(acc, el[0], wit))
        return false;

    return true;
}

bool testNonMPCAccKeyless(osuCrypto::REllipticCurve curve)
{
    using KEYLESS::Acc;
    using KEYLESS::Auxillary;
    using KEYLESS::Witness;

    // non-mpc accumulator without access to private key
    std::cout << "Testing Non-MPC acc without access to the secret key" << std::endl;
    Acc acc_class(curve, SET_LENGTH + 1);
    acc_class.Gen();
    Acc::Accumulator acc;
    Auxillary aux;
    Acc::Set el;
    acc_class.genRandomSet(el, SET_LENGTH);
    acc_class.Eval(el, acc, aux);
    Witness wit;
    if(!acc_class.WitCreate(aux, el[0], wit))
        return false;

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[1], wit))
        return false;

    Acc::Element new_el;
    new_el.randomize();

    if (!acc_class.Add(acc, aux, new_el))
      return false;

    Witness old_wit;
    Acc::Wit tmp1 = wit.getWitness();
    Acc::Element tmp2 = wit.getElement();
    old_wit.init(tmp1, tmp2);
    if (!acc_class.WitUpdate(wit, aux, new_el))
        return false;

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[0], old_wit))
        return false;

    if (!acc_class.Delete(acc, aux, el[2]))
      return false;

    tmp1 = wit.getWitness();
    tmp2 = wit.getElement();
    old_wit.init(tmp1, tmp2);
    if (!acc_class.WitUpdate(wit, aux, el[2]))
        return false;

    if (!acc_class.Verify(acc, el[0], wit))
        return false;
    if (acc_class.Verify(acc, el[0], old_wit))
        return false;

    if (!acc_class.Delete(acc, aux, el[0]))
      return false;

    if (acc_class.WitUpdate(wit, aux, el[0]))
        return false;

    if (acc_class.Verify(acc, el[0], wit))
        return false;

    return true;
}
