package com.author.mpc_acc;

import com.author.mpc_acc.Auxillery;
import com.author.mpc_acc.EvalResult;
import java.math.BigInteger;
import java.security.SecureRandom;
import iaik.security.ec.common.SecurityStrength;

import java.util.List;
import java.util.ArrayList;

import iaik.security.ec.math.curve.Pairing;
import iaik.security.ec.math.curve.PairingTypes;
import iaik.security.ec.math.field.GenericFieldElement;
import iaik.security.ec.math.curve.AtePairingOverBarretoNaehrigCurveFactory;
import iaik.security.ec.math.curve.ECPoint;
import iaik.security.ec.math.curve.EllipticCurve;

/**
 * A simple class computing the non-MPC accumulator using the secret key.
 *
 * @author ******
 */
 public class AccumulatorDemo {

  private final Pairing PAIRING;
  private final EllipticCurve CURVE1;
  private final EllipticCurve CURVE2;
  private final BigInteger Q;
  private final ECPoint G1;
  private final ECPoint G2;
  private int size;
  // private ECPoint pk;
  private ECPoint pk2;
  private BigInteger sk;
  private SecureRandom random;

  public AccumulatorDemo(int size) {
    this.PAIRING = AtePairingOverBarretoNaehrigCurveFactory
        .getPairing(PairingTypes.TYPE_3, size);
    this.size = size;
    this.CURVE1 = PAIRING.getGroup1();
    this.CURVE2 = PAIRING.getGroup2();
    this.G1 = CURVE1.getGenerator();
    this.G2 = CURVE2.getGenerator();
    this.Q = CURVE1.getOrder();
    random = SecurityStrength.getSecureRandom(SecurityStrength
        .getSecurityStrength(CURVE1.getField().getFieldSize()));
  }

  private BigInteger getRandomScalar() {
    return new BigInteger(size - 1, random);
  }

  private void Gen() {
    sk = getRandomScalar();
    // pk = G1.clone().multiplyPoint(sk);
    pk2 = G2.clone().multiplyPoint(sk);
  }

  private EvalResult Eval(List<BigInteger> X) {
    BigInteger r = getRandomScalar();
    BigInteger q = BigInteger.ONE;
    for (BigInteger x : X) {
      q = q.multiply(x.add(sk).mod(Q)).mod(Q);
    }

    List<BigInteger> list = new ArrayList<BigInteger>(X);

    return new EvalResult(G1.clone().multiplyPoint(q.multiply(r).mod(Q)), list);
  }

  private ECPoint WitCreate(ECPoint acc, Auxillery aux, BigInteger x) {
    if (!aux.getList().contains(x))
      return null;

    return acc.clone().multiplyPoint(x.add(sk).mod(Q).modInverse(Q));
  }

  private Boolean Verify(ECPoint acc, ECPoint wit, BigInteger x) {
    GenericFieldElement p1 = PAIRING.pair(acc.clone(), G2);
    ECPoint p2_EP = G2.clone().multiplyPoint(x);
    p2_EP.addPoint(pk2);
    GenericFieldElement p2 = PAIRING.pair(wit.clone(), p2_EP);
    return p1.equals(p2);
  }

  private EvalResult Add(ECPoint acc, Auxillery aux, BigInteger x) {
    List<BigInteger> list = aux.getList();
    if (list.contains(x))
      return null;

    ECPoint r1 = acc.clone().multiplyPoint(x);
    ECPoint r2 = acc.clone().multiplyPoint(sk);
    List<BigInteger> list_prime = new ArrayList<BigInteger>(list);
    list_prime.add(x);
    return new EvalResult(r1.addPoint(r2), list_prime, Auxillery.UPDATE.ADD);
  }

  private EvalResult Delete(ECPoint acc, Auxillery aux, BigInteger x) {
    List<BigInteger> list = aux.getList();
    if (!list.contains(x))
      return null;

    BigInteger y = x.add(sk).mod(Q).modInverse(Q);

    List<BigInteger> list_prime = new ArrayList<BigInteger>(list);
    list_prime.remove(x);
    return new EvalResult(acc.clone().multiplyPoint(y), list_prime, Auxillery.UPDATE.DELETE);
  }

  private ECPoint WitUpdate(ECPoint wit, Auxillery aux, BigInteger x) {
    ECPoint wit_prime = null;
    switch (aux.getUpdate()) {
      case ADD:
        ECPoint r1 = wit.clone().multiplyPoint(x);
        ECPoint r2 = wit.clone().multiplyPoint(sk);
        wit_prime = r1.addPoint(r2);
        break;
      case DELETE:
        wit_prime = wit.clone().multiplyPoint(x.add(sk).mod(Q).modInverse(Q));
        break;
      default:
        return wit;
    }

    aux.clearUpdate();
    return wit_prime;
  }

  public void test(List<BigInteger> X) {
    System.out.println("Verify standard:");
    Gen();
    // List<BigInteger> X = new ArrayList<>();
    // for (int i = 0; i < 10; i++)
    //   X.add(getRandomScalar());
    EvalResult ev = Eval(X);

    BigInteger x = X.get(0);

    ECPoint acc = ev.getAcc();
    Auxillery aux = ev.getAuxillery();
    ECPoint wit = WitCreate(acc, aux, x);
    if (wit == null)
      return;

    System.out.println("Correct verify: " + Verify(acc, wit, x));
    System.out.println("Incorrect verify: " + Verify(acc, wit, X.get(1)));

    System.out.println("Verify add:");
    BigInteger new_el = getRandomScalar();
    EvalResult add_res = Add(acc, aux, new_el);
    if (add_res == null)
      return;

    ECPoint new_wit = WitUpdate(wit, add_res.getAuxillery(), new_el);
    System.out.println("Correct verify: " + Verify(add_res.getAcc(), new_wit, x));
    System.out.println("Incorrect verify: " + Verify(add_res.getAcc(), wit, x));

    System.out.println("Verify delete:");
    EvalResult del_res = Delete(acc, aux, X.get(2));
    if (del_res == null)
      return;

    ECPoint new_wit2 = WitUpdate(wit, del_res.getAuxillery(), X.get(2));
    System.out.println("Correct verify: " + Verify(del_res.getAcc(), new_wit2, x));
    System.out.println("Incorrect verify: " + Verify(del_res.getAcc(), wit, x));

    // System.out.println("Verify delete element of wit:");
    // EvalResult del_res2 = Delete(acc, aux, X.get(0));
    // if (del_res2 == null)
    //   return;

    // ECPoint new_wit3 = WitUpdate(wit, del_res2.getAuxillery(), X.get(0));
    // System.out.println("Incorrect verify: " + Verify(del_res2.getAcc(), new_wit3, x));

  }

 }
