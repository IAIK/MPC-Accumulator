package dk.alexandra.fresco.suite.dummy.arithmetic;

import dk.alexandra.fresco.framework.DRes;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.framework.value.SInt;

/**
 * Implements addition for the Dummy Arithmetic protocol suite, where all operations are done in the
 * clear.
 */
public class DummyArithmeticAddProtocol extends DummyArithmeticNativeProtocol<SInt> {

  private DRes<SInt> left;
  private DRes<SInt> right;
  private SInt out;

  /**
   * Constructs a protocol to sum the result of two computations.
   *
   * @param left the left operand
   * @param right the right operand
   */
  public DummyArithmeticAddProtocol(DRes<SInt> left, DRes<SInt> right) {
    this.left = left;
    this.right = right;
  }

  @Override
  public EvaluationStatus evaluate(int round, DummyArithmeticResourcePool resourcePool,
      Network network) {
    FieldElement l = ((DummyArithmeticSInt) left.out()).getValue();
    FieldElement r = ((DummyArithmeticSInt) right.out()).getValue();
    FieldElement sum = r.add(l);
    out = new DummyArithmeticSInt(sum);
    return EvaluationStatus.IS_DONE;
  }

  @Override
  public SInt out() {
    return out;
  }
}
