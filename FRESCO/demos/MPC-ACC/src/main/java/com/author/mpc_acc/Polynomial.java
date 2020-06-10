package com.author.mpc_acc;

import java.math.BigInteger;

/**
 * This class represents a monic, reducible polynomial over Z_p
 *
 * @author david
 * @author chanser
 */
public class Polynomial {

  private final BigInteger[] coeffs_;
  private final int degree_;

  /**
   * c'tor
   *
   * @param degree
   *          the degree of the polynomial
   * @param order
   *          the order to be applied to the coefficients
   */
  public Polynomial(int degree, BigInteger order) {
    degree_ = degree;
    coeffs_ = new BigInteger[degree + 1];

    for (int i = 0; i < degree + 1; i++) {
      coeffs_[i] = BigInteger.ZERO;
    }
  }

  /**
   * Expands a polynomial of the form $\prod_{i=0}^{n} (X + A_i)$
   *
   * @param roots
   *          The list containing the root A_i
   * @param order
   *          modulus
   *
   * @return The expanded polynomial
   */
  public static Polynomial expand(BigInteger[] roots, BigInteger order) {
    final int degree = roots.length;

    if (degree == 0) {
      final Polynomial p = new Polynomial(1, order);
      p.setCoefficient(0, BigInteger.ONE);

      return p;
    }

    int currentDegree = 1;

    final Polynomial result = new Polynomial(degree, order);

    result.setCoefficient(0, roots[0]);
    result.setCoefficient(1, BigInteger.ONE);

    for (int i = 1; i < degree; i++) {
      currentDegree = result.powX(currentDegree);

      for (int j = 0; j < currentDegree; j++) {
        result.setCoefficient(
            j,
            result.getCoefficient(j).add(result.getCoefficient(j + 1).multiply(roots[i]))
                .mod(order));
      }
    }

    return result;
  }

  /**
   * Returns a coefficient of this polynomial
   *
   * @param pos
   *          the index
   * @return the corresponding coefficient
   */
  BigInteger getCoefficient(int pos) {
    return coeffs_[pos];
  }

  BigInteger[] getCoefficient() {
    return coeffs_;
  }

  int getDegree() {
    return degree_;
  }

  /**
   * Sets a coefficition of this polynomial
   *
   * @param pos
   *          the index
   * @param coeff
   *          the new coefficient value
   */
  void setCoefficient(int pos, BigInteger coeff) {
    coeffs_[pos] = coeff;
  }

  /**
   * Increases the degree_ of the polynomial by 1. This method is immutable.
   *
   * @param currentDegree
   *          the current degree_
   *
   * @return the resulting polynomial
   */
  public int powX(int currentDegree) {
    if (currentDegree >= degree_) {
      return currentDegree;
    }

    for (int i = currentDegree; i >= 0; i--) {
      coeffs_[i + 1] = coeffs_[i];
    }
    coeffs_[0] = BigInteger.ZERO;

    return currentDegree + 1;
  }

  @Override
  public String toString() {
    String ret = "f(X) = ";

    for (int i = degree_; i >= 0; i--) {
      ret += coeffs_[i].toString() + " * X^" + i + " + ";
    }

    ret = ret.substring(0, ret.length() - 3);

    return ret;
  }
}
