package com.author.mpc_acc;

import java.math.BigInteger;
import java.util.List;
import iaik.security.ec.math.curve.ECPoint;
import com.author.mpc_acc.Auxillery;

/**
 * A simple class containing the eval result
 *
 * @author ******
 */
public class EvalResult {
  private ECPoint acc;
  private Auxillery aux;

  public EvalResult(ECPoint acc, List<BigInteger> list) {
    this.acc = acc;
    this.aux = new Auxillery(list, Auxillery.UPDATE.NONE);
  }

  public EvalResult(ECPoint acc, List<BigInteger> list, Auxillery.UPDATE update) {
    this.acc = acc;
    this.aux = new Auxillery(list, update);
  }

  public EvalResult(ECPoint acc, List<BigInteger> list, Auxillery.UPDATE update, ECPoint acc_old) {
    this.acc = acc;
    this.aux = new Auxillery(list, update, acc_old, acc);
  }

  public EvalResult(ECPoint acc, List<BigInteger> list, BigInteger r) {
    this.acc = acc;
    this.aux = new Auxillery(list, r);
  }

  public ECPoint getAcc() {
    return acc;
  }

  public Auxillery getAuxillery() {
    return aux;
  }

}
