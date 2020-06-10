package dk.alexandra.fresco.suite.dummy.arithmetic;

import dk.alexandra.fresco.framework.DRes;
import dk.alexandra.fresco.framework.builder.numeric.BuilderFactoryNumeric;
import dk.alexandra.fresco.framework.builder.numeric.Numeric;
import dk.alexandra.fresco.framework.builder.numeric.ProtocolBuilderNumeric;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.framework.value.SInt;
import dk.alexandra.fresco.lib.compare.MiscBigIntegerGenerators;
import dk.alexandra.fresco.lib.field.integer.BasicNumericContext;
import dk.alexandra.fresco.lib.real.RealNumericContext;
import java.math.BigInteger;
import java.util.Random;

/**
 * A {@link BuilderFactoryNumeric} implementation for the Dummy Arithmetic suite. This class has
 * built-in support for logging the amount of different operations (i.e. protocols) that the
 * application asks for.
 */
public class DummyArithmeticBuilderFactory implements BuilderFactoryNumeric {

  private BasicNumericContext basicNumericContext;
  private RealNumericContext realNumericContext;
  private MiscBigIntegerGenerators mog;
  private Random rand;

  /**
   * Creates a dummy arithmetic builder factory which creates basic numeric operations.
   *
   * @param basicNumericContext The numeric context we work within.
   * @param realNumericContext The real numeric context we work within.
   */
  public DummyArithmeticBuilderFactory(BasicNumericContext basicNumericContext,
      RealNumericContext realNumericContext) {
    super();
    this.basicNumericContext = basicNumericContext;
    this.realNumericContext = realNumericContext;
    this.rand = new Random(0);
  }

  @Override
  public BasicNumericContext getBasicNumericContext() {
    return basicNumericContext;
  }

  @Override
  public RealNumericContext getRealNumericContext() {
    return realNumericContext;
  }

  @Override
  public Numeric createNumeric(ProtocolBuilderNumeric builder) {
    return new Numeric() {

      private DummyArithmeticSInt createSIntFromConstant(BigInteger b) {
        return new DummyArithmeticSInt(basicNumericContext.getFieldDefinition().createElement(b));
      }

      @Override
      public DRes<SInt> sub(DRes<SInt> a, BigInteger b) {
        DummyArithmeticNativeProtocol<SInt> c =
            new DummyArithmeticSubtractProtocol(a, () -> createSIntFromConstant(b));
        return builder.append(c);
      }

      @Override
      public DRes<SInt> sub(BigInteger a, DRes<SInt> b) {
        DummyArithmeticSubtractProtocol c =
            new DummyArithmeticSubtractProtocol(() -> createSIntFromConstant(a), b);
        return builder.append(c);
      }

      @Override
      public DRes<SInt> sub(DRes<SInt> a, DRes<SInt> b) {
        DummyArithmeticSubtractProtocol c = new DummyArithmeticSubtractProtocol(a, b);
        return builder.append(c);
      }

      @Override
      public DRes<SInt> randomElement() {
        DummyArithmeticNativeProtocol<SInt> c = new DummyArithmeticNativeProtocol<SInt>() {

          DummyArithmeticSInt elm;

          @Override
          public EvaluationStatus evaluate(int round, DummyArithmeticResourcePool resourcePool,
              Network network) {
            BigInteger r;
            BigInteger modulus = basicNumericContext.getModulus();
            do {
              r = new BigInteger(modulus.bitLength(), rand);
            } while (r.compareTo(modulus) >= 0);
            elm = createSIntFromConstant(r);
            return EvaluationStatus.IS_DONE;
          }

          @Override
          public SInt out() {
            return elm;
          }
        };
        return builder.append(c);
      }

      @Override
      public DRes<SInt> randomBit() {
        DummyArithmeticNativeProtocol<SInt> c = new DummyArithmeticNativeProtocol<SInt>() {

          DummyArithmeticSInt bit;

          @Override
          public EvaluationStatus evaluate(int round, DummyArithmeticResourcePool resourcePool,
              Network network) {
            bit = createSIntFromConstant(BigInteger.valueOf(rand.nextInt(2)));
            return EvaluationStatus.IS_DONE;
          }

          @Override
          public SInt out() {
            return bit;
          }
        };
        return builder.append(c);
      }

      @Override
      public DRes<BigInteger> open(DRes<SInt> secretShare) {
        DummyArithmeticOpenToAllProtocol c = new DummyArithmeticOpenToAllProtocol(secretShare);
        return builder.append(c);
      }

      @Override
      public DRes<BigInteger> open(DRes<SInt> secretShare, int outputParty) {
        DummyArithmeticOpenProtocol c = new DummyArithmeticOpenProtocol(secretShare, outputParty);
        return builder.append(c);
      }

      @Override
      public DRes<SInt> mult(BigInteger a, DRes<SInt> b) {
        DummyArithmeticMultProtocol c =
            new DummyArithmeticMultProtocol(() -> createSIntFromConstant(a), b);
        return builder.append(c);
      }

      @Override
      public DRes<SInt> mult(DRes<SInt> a, DRes<SInt> b) {
        DummyArithmeticMultProtocol c = new DummyArithmeticMultProtocol(a, b);
        return builder.append(c);
      }

      @Override
      public DRes<SInt> known(BigInteger value) {
        DummyArithmeticNativeProtocol<SInt> c = new DummyArithmeticNativeProtocol<SInt>() {

          DummyArithmeticSInt val;

          @Override
          public EvaluationStatus evaluate(int round, DummyArithmeticResourcePool resourcePool,
              Network network) {
            val = createSIntFromConstant(value);
            return EvaluationStatus.IS_DONE;
          }

          @Override
          public SInt out() {
            return val;
          }
        };
        return builder.append(c);
      }

      @Override
      public DRes<SInt> input(BigInteger value, int inputParty) {
        FieldElement open =
            value != null ? basicNumericContext.getFieldDefinition().createElement(value) : null;
        DummyArithmeticCloseProtocol c = new DummyArithmeticCloseProtocol(open, inputParty);
        return builder.append(c);
      }

      @Override
      public DRes<SInt> add(BigInteger a, DRes<SInt> b) {
        DummyArithmeticAddProtocol c =
            new DummyArithmeticAddProtocol(() -> createSIntFromConstant(a), b);
        return builder.append(c);
      }

      @Override
      public DRes<SInt> add(DRes<SInt> a, DRes<SInt> b) {
        DummyArithmeticAddProtocol c = new DummyArithmeticAddProtocol(a, b);
        return builder.append(c);
      }
    };
  }

  @Override
  public MiscBigIntegerGenerators getBigIntegerHelper() {
    if (mog == null) {
      mog = new MiscBigIntegerGenerators(basicNumericContext.getModulus());
    }
    return mog;
  }
}
