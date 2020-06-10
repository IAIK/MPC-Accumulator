package com.author.mpc_acc;

import java.math.BigInteger;
import iaik.security.ec.math.curve.ECPoint;

/**
 * A simple class containing the AUX data
 *
 * @author ******
 */
public class Witness {

  private ECPoint witness;
  private BigInteger element;

  public Witness(ECPoint witness, BigInteger element) {
    this.witness = witness;
    this.element = element;
  }

  public ECPoint getWitness() {
    return witness;
  }

  public BigInteger getElement() {
    return element;
  }
}
