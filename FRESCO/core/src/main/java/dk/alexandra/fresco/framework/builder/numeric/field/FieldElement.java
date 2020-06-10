package dk.alexandra.fresco.framework.builder.numeric.field;

import dk.alexandra.fresco.framework.builder.numeric.Addable;
import java.io.Serializable;

/**
 * An element of a finite field defined
 * by {@link FieldDefinition}.
 */
public interface FieldElement extends Serializable, Addable<FieldElement> {

  /**
   * Computes this element subtracted with the operand.
   *
   * @param other operand
   * @return <code>this - other</code>
   */
  FieldElement subtract(FieldElement other);

  /**
   * Computes the additive inverse of this element.
   *
   * @return <code>-this</code>
   */
  FieldElement negate();

  /**
   * Computes the product between an element and  an element from this element.
   *
   * @param other operand
   * @return <code>this * other</code>
   */
  FieldElement multiply(FieldElement other);

  /**
   * Computes the multiplicative inverse of this element.
   *
   * @return x, where <code>x*x = this</code>
   */
  FieldElement sqrt();

  /**
   * Computes the modular inverse of this element.
   *
   * @return <code>this<sup>-1</sup> mod modulus</code>
   */
  FieldElement modInverse();
}
