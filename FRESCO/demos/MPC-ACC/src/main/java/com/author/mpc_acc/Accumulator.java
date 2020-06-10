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
 * A simple class computing the non-MPC, keyless accumulator.
 *
 * @author ******
 */
 public class Accumulator {

  private final Pairing PAIRING;
  private final EllipticCurve CURVE1;
  private final EllipticCurve CURVE2;
  private final BigInteger Q;
  private final ECPoint G1;
  private final ECPoint G2;
  private int size;
  private int t;
  private List<ECPoint> pk;
  // private ECPoint pk;
  private ECPoint pk2;
  private SecureRandom random;

  public Accumulator(int size, int t) {
    this.PAIRING = AtePairingOverBarretoNaehrigCurveFactory
        .getPairing(PairingTypes.TYPE_3, size);
    this.size = size;
    this.t = t;
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
    BigInteger sk = getRandomScalar();
    BigInteger s_ = sk;
    pk = new ArrayList<>();
    pk.add(G1.clone());
    pk.add(G1.clone().multiplyPoint(s_));
    for (int i = 2; i <= t; i++) {
      s_ = s_.multiply(sk).mod(Q);
      pk.add(G1.clone().multiplyPoint(s_));
    }
    pk2 = G2.clone().multiplyPoint(sk);
  }

  private EvalResult Eval(List<BigInteger> X, BigInteger r) {
    Polynomial pol = Polynomial.expand(X.toArray(new BigInteger[X.size()]), Q);
    BigInteger[] coeff = pol.getCoefficient();

    ECPoint acc = pk.get(0).clone().multiplyPoint(coeff[0]);
    for (int i = 1; i <= pol.getDegree(); i++) {
      acc.addPoint(pk.get(i).clone().multiplyPoint(coeff[i]));
    }

    List<BigInteger> list = new ArrayList<BigInteger>(X);
    return new EvalResult(acc.multiplyPoint(r), list, r);
  }

  private EvalResult Eval(List<BigInteger> X) {
    return Eval(X, getRandomScalar());
  }

  private Witness WitCreate(ECPoint acc, Auxillery aux, BigInteger x) {
    List<BigInteger> list = aux.getList();
    if (!list.contains(x))
      return null;

    List<BigInteger> X_prime = new ArrayList<BigInteger>(list);
    X_prime.remove(x);
    EvalResult res = Eval(X_prime, aux.getR());
    return new Witness(res.getAcc(), x);
  }

  private Boolean Verify(ECPoint acc, Witness wit, BigInteger x) {
    GenericFieldElement p1 = PAIRING.pair(acc.clone(), G2);
    ECPoint p2_EP = G2.clone().multiplyPoint(x);
    p2_EP.addPoint(pk2);
    GenericFieldElement p2 = PAIRING.pair(wit.getWitness().clone(), p2_EP);
    return p1.equals(p2);
  }

  private EvalResult Add(ECPoint acc, Auxillery aux, BigInteger x) {
    List<BigInteger> list = aux.getList();
    if (list.contains(x))
      return null;

    List<BigInteger> list_prime = new ArrayList<BigInteger>(list);
    list_prime.add(x);

    EvalResult res = Eval(list_prime, aux.getR());
    return new EvalResult(res.getAcc(), list_prime, Auxillery.UPDATE.ADD, acc);
  }

  private EvalResult Delete(ECPoint acc, Auxillery aux, BigInteger x) {
    List<BigInteger> list = aux.getList();
    if (!list.contains(x))
      return null;

    List<BigInteger> list_prime = new ArrayList<BigInteger>(list);
    list_prime.remove(x);

    EvalResult res = Eval(list_prime, aux.getR());
    return new EvalResult(res.getAcc(), list_prime, Auxillery.UPDATE.DELETE, acc);

  }

  private Witness WitUpdate(Witness wit, Auxillery aux, BigInteger x) {
    Witness wit_prime = null;
    BigInteger el = wit.getElement();
    if (x.equals(el))
      return null;

    switch (aux.getUpdate()) {
      case ADD:
        ECPoint wit_new_add = wit.getWitness().clone().multiplyPoint(x.subtract(el).mod(Q)).addPoint(aux.getAcc());
        wit_prime = new Witness(wit_new_add, el);
        break;
      case DELETE:
        ECPoint wit_new_del = wit.getWitness().clone().addPoint(aux.getAccPrime().clone().negatePoint()).multiplyPoint(x.subtract(el).mod(Q).modInverse(Q));
      wit_prime = new Witness(wit_new_del, el);
        break;
      default:
        return null;
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

    long time = System.currentTimeMillis();
    EvalResult ev = Eval(X);
    time = System.currentTimeMillis() - time;
    System.out.println("Eval time: " + time + "ms");

    BigInteger x = X.get(0);

    ECPoint acc = ev.getAcc();
    Auxillery aux = ev.getAuxillery();

    time = System.currentTimeMillis();
    Witness wit = WitCreate(acc, aux, x);
    time = System.currentTimeMillis() - time;
    System.out.println("WitCreate time: " + time + "ms");
    if (wit == null)
      return;

    System.out.println("Correct verify: " + Verify(acc, wit, x));
    System.out.println("Incorrect verify: " + Verify(acc, wit, X.get(1)));

    System.out.println("Verify add:");
    BigInteger new_el = getRandomScalar();
    time = System.currentTimeMillis();
    EvalResult add_res = Add(acc, aux, new_el);
    time = System.currentTimeMillis() - time;
    System.out.println("Add time: " + time + "ms");
    if (add_res == null)
      return;

    time = System.currentTimeMillis();
    Witness new_wit = WitUpdate(wit, add_res.getAuxillery(), new_el);
    time = System.currentTimeMillis() - time;
    System.out.println("WitUpdateAdd time: " + time + "ms");
    System.out.println("Correct verify: " + Verify(add_res.getAcc(), new_wit, x));
    System.out.println("Incorrect verify: " + Verify(add_res.getAcc(), wit, x));

    System.out.println("Verify delete:");
    time = System.currentTimeMillis();
    EvalResult del_res = Delete(acc, aux, X.get(2));
    time = System.currentTimeMillis() - time;
    System.out.println("Delete time: " + time + "ms");
    if (del_res == null)
      return;

    time = System.currentTimeMillis();
    Witness new_wit2 = WitUpdate(wit, del_res.getAuxillery(), X.get(2));
    time = System.currentTimeMillis() - time;
    System.out.println("WitUpdateDel time: " + time + "ms");
    System.out.println("Correct verify: " + Verify(del_res.getAcc(), new_wit2, x));
    System.out.println("Incorrect verify: " + Verify(del_res.getAcc(), wit, x));

    // System.out.println("Verify delete element of wit:");
    // EvalResult del_res2 = Delete(acc, aux, X.get(0));
    // if (del_res2 == null)
    //   return;

    // Witness new_wit3 = WitUpdate(wit, del_res2.getAuxillery(), X.get(0));
    // if (new_wit3 != null)
    //   System.out.println("Incorrect verify: " + Verify(del_res2.getAcc(), new_wit3, x));

  }

  public BigInteger getQ() {
    return Q;
  }

 }
