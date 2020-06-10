package com.author.mpc_acc;

import java.math.BigInteger;
import java.util.List;

import iaik.security.ec.math.curve.ECPoint;

/**
 * A simple class containing the Auxillery data
 *
 * @author ******
 */
public class Auxillery {

  enum UPDATE {
    NONE,
    ADD,
    DELETE
  }

  private List<BigInteger> list;
  private BigInteger r;
  private UPDATE update;
  private ECPoint acc;
  private ECPoint acc_prime;

  public Auxillery(List<BigInteger> list, UPDATE update) {
    this.list = list;
    this.update = update;
    this.r = null;
    this.acc = null;
    this.acc_prime = null;
  }

  public Auxillery(List<BigInteger> list, UPDATE update, ECPoint acc, ECPoint acc_prime) {
    this.list = list;
    this.update = update;
    this.r = null;
    this.acc = acc;
    this.acc_prime = acc_prime;
  }

  public Auxillery(List<BigInteger> list, BigInteger r) {
    this.list = list;
    this.update = UPDATE.NONE;
    this.r = r;
    this.acc = null;
    this.acc_prime = null;
  }

  public List<BigInteger> getList() {
    return list;
  }

  public UPDATE getUpdate() {
    return update;
  }

  public void clearUpdate() {
    update = UPDATE.NONE;
  }

  public BigInteger getR() {
    return r;
  }

  public ECPoint getAcc() {
    return acc;
  }

  public ECPoint getAccPrime() {
    return acc_prime;
  }
}
