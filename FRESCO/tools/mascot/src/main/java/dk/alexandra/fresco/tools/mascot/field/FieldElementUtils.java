package dk.alexandra.fresco.tools.mascot.field;

import dk.alexandra.fresco.framework.builder.numeric.Addable;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldDefinition;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;
import java.util.stream.IntStream;
import java.util.stream.Stream;

public final class FieldElementUtils {

  private final FieldDefinition definition;
  private final List<FieldElement> generators;

  /**
   * Creates new {@link FieldElementUtils}.
   *
   * @param definition field definition for underlying field element operations
   */
  public FieldElementUtils(FieldDefinition definition) {
    this.definition = definition;
    this.generators = precomputeGenerators();
  }

  private List<FieldElement> precomputeGenerators() {
    List<FieldElement> generators = new ArrayList<>(definition.getBitLength());
    BigInteger current = BigInteger.ONE;
    for (int i = 0; i < definition.getBitLength(); i++) {
      generators.add(definition.createElement(current));
      current = current.shiftLeft(1);
    }
    return generators;
  }

  /**
   * Multiplies two lists of field elements, pair-wise.
   *
   * @param leftFactors left factors
   * @param rightFactors right factors
   * @return list of products
   */
  public List<FieldElement> pairWiseMultiply(List<FieldElement> leftFactors,
      List<FieldElement> rightFactors) {
    if (leftFactors.size() != rightFactors.size()) {
      throw new IllegalArgumentException("Lists must be same size");
    }
    return pairWiseMultiplyStream(leftFactors, rightFactors).collect(Collectors.toList());
  }

  /**
   * Multiplies two lists of field elements, pair-wise.
   *
   * @param leftFactors left factors
   * @param rightFactors right factors
   * @return stream of products
   */
  private Stream<FieldElement> pairWiseMultiplyStream(List<FieldElement> leftFactors,
      List<FieldElement> rightFactors) {
    return IntStream.range(0, leftFactors.size()).mapToObj(idx -> {
      FieldElement l = leftFactors.get(idx);
      FieldElement r = rightFactors.get(idx);
      return l.multiply(r);
    });
  }

  /**
   * Computes inner product of two lists of field elements.
   *
   * @param left left factors
   * @param right right factors
   * @return inner product
   */
  public FieldElement innerProduct(List<FieldElement> left, List<FieldElement> right) {
    if (left.size() != right.size()) {
      throw new IllegalArgumentException("Lists must have same size");
    }
    return Addable.sum(pairWiseMultiplyStream(left, right));
  }

  /**
   * Multiplies each value in list by scalar.
   *
   * @param values list of factors
   * @param scalar scalar factor
   * @return list of products
   */
  public List<FieldElement> scalarMultiply(List<FieldElement> values, FieldElement scalar) {
    return values.stream().map(scalar::multiply).collect(Collectors.toList());
  }

  /**
   * Computes inner product of elements and powers of twos.<br> e0 * 2**0 + e1 * 2**1 + ... + e(n -
   * 1) * 2**(n - 1) Elements must have same modulus, otherwise we get undefined behaviour.
   *
   * @param elements elements to recombine
   * @return recombined elements
   */
  public FieldElement recombine(List<FieldElement> elements) {
    if (elements.size() > definition.getBitLength()) {
      throw new IllegalArgumentException("Number of elements cannot exceed bit-length");
    }
    return innerProduct(elements, generators.subList(0, elements.size()));
  }

  /**
   * Duplicates each element stretchBy times. <br> For instance, stretching [e0, e1, e2] by 2
   * results in [e0, e0, e1, e1, e3, e3].
   *
   * @param elements elements to be stretched
   * @param stretchBy number of duplications per element
   * @return stretched list
   */
  public List<FieldElement> stretch(List<FieldElement> elements, int stretchBy) {
    List<FieldElement> stretched = new ArrayList<>(elements.size() * stretchBy);
    for (FieldElement element : elements) {
      stretched.addAll(Collections.nCopies(stretchBy, element));
    }
    return stretched;
  }

  /**
   * Appends padding elements to end of list numPads times.
   *
   * @param elements elements to pad
   * @param padElement element to pad with
   * @param numPads number of times to pad
   * @return padded list
   */
  public List<FieldElement> padWith(List<FieldElement> elements, FieldElement padElement,
      int numPads) {
    List<FieldElement> copy = new ArrayList<>(elements);
    copy.addAll(Collections.nCopies(numPads, padElement));
    return copy;
  }
}
