package dk.alexandra.fresco.suite.spdz.ECCExtension;

import dk.alexandra.fresco.commitment.HashBasedCommitmentSerializer;
import dk.alexandra.fresco.framework.DRes;
import dk.alexandra.fresco.framework.MaliciousException;
import dk.alexandra.fresco.framework.builder.Computation;
import dk.alexandra.fresco.framework.builder.numeric.ProtocolBuilderNumeric;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldDefinition;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.util.AesCtrDrbg;
import dk.alexandra.fresco.framework.util.Drbg;
import dk.alexandra.fresco.framework.util.Pair;
import dk.alexandra.fresco.lib.generic.CoinTossingComputation;
import dk.alexandra.fresco.lib.generic.CommitmentComputation;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzSInt;
import iaik.security.ec.math.curve.ECPoint;
import iaik.security.ec.math.curve.EllipticCurve;

import java.math.BigInteger;
import java.util.List;
import java.util.function.Function;
import dk.alexandra.fresco.suite.spdz.ECCExtension.SpdzECPoint;

/**
 * Protocol which handles the MAC check internal to SPDZ. If this protocol reaches the end, no
 * malicious activity was detected and the storage is reset.
 */
public class SpdzECCMacCheckProtocol implements Computation<Void, ProtocolBuilderNumeric> {

  private final BigInteger modulus;
  private final List<SpdzSInt> closedValues;
  private final List<FieldElement> openedValues;
  private final List<SpdzECPoint> closedValuesECC;
  private final List<ECPoint> openedValuesECC;
  private final FieldElement alpha;
  private final Function<byte[], Drbg> jointDrbgSupplier;
  private final int drbgByteLength;

  /**
   * Protocol which handles the MAC check internal to SPDZ. If this protocol reaches the end, no
   * malicious activity was detected and the storage is reset.
   *
   * @param toCheck opened values and corresponding macs to check
   * @param modulus the global modulus used
   * @param jointDrbgSupplier supplier of DRBG to be used for joint randomness
   * @param alpha this party's key share
   * @param drbgSeedBitLength seed length for local DRBG
   */
  public SpdzECCMacCheckProtocol(
      final Pair<List<SpdzSInt>, List<FieldElement>> toCheck,
      final Pair<List<SpdzECPoint>, List<ECPoint>> toCheckECC,
      final BigInteger modulus,
      final Function<byte[], Drbg> jointDrbgSupplier,
      final FieldElement alpha,
      final int drbgSeedBitLength) {
    this.closedValues = toCheck.getFirst();
    this.openedValues = toCheck.getSecond();
    this.closedValuesECC = toCheckECC.getFirst();
    this.openedValuesECC = toCheckECC.getSecond();
    this.modulus = modulus;
    this.jointDrbgSupplier = jointDrbgSupplier;
    this.alpha = alpha;
    this.drbgByteLength = drbgSeedBitLength / 8;
  }

  @Override
  public DRes<Void> buildComputation(ProtocolBuilderNumeric builder) {
    final AesCtrDrbg localDrbg = new AesCtrDrbg();
    final HashBasedCommitmentSerializer commitmentSerializer = new HashBasedCommitmentSerializer();
    final FieldDefinition definition = builder
        .getBasicNumericContext()
        .getFieldDefinition();

    return builder
        .seq(new CoinTossingComputation(drbgByteLength, commitmentSerializer, localDrbg))
        .seq((seq, seed) -> {
          Drbg jointDrbg = jointDrbgSupplier.apply(seed);
          FieldElement[] rs = sampleRandomCoefficients(openedValues.size() + openedValuesECC.size(), definition,
              jointDrbg);
          FieldElement a = definition.createElement(0);
          int index = 0;
          for (FieldElement openedValue : openedValues) {
            FieldElement openedValueHidden = openedValue.multiply(rs[index++]);
            a = a.add(openedValueHidden);
          }
          // ECC A
          ECPoint A = null;
          EllipticCurve curve = null;
          if (openedValuesECC.size() != 0)  {
            curve = openedValuesECC.get(0).getCurve();
            A = curve.getNeutralPoint();
            for (ECPoint openedValue : openedValuesECC) {
              ECPoint openedValueHidden = openedValue.clone().multiplyPoint(definition.convertToUnsigned(rs[index++]));
              A.addPoint(openedValueHidden);
            }
          }


          // compute gamma_i as the sum of all MAC's on the opened values times
          // r_j.
          FieldElement gamma = definition.createElement(0);
          index = 0;
          for (SpdzSInt closedValue : closedValues) {
            FieldElement closedValueHidden = rs[index++].multiply(closedValue.getMac());
            gamma = gamma.add(closedValueHidden);
          }
          // ECC GAMMA
          ECPoint GAMMA = null;
          if (openedValuesECC.size() != 0)  {
            GAMMA = curve.getNeutralPoint();
            for (SpdzECPoint closedValue : closedValuesECC) {
              ECPoint closedValueHidden = closedValue.getMac().clone().multiplyPoint(definition.convertToUnsigned(rs[index++]));
              GAMMA.addPoint(closedValueHidden);
            }
          }

          // compute delta_i as: gamma_i - alpha_i*a
          byte[] deltaBytes = null;
          FieldElement delta = gamma.subtract(alpha.multiply(a));
          if (openedValuesECC.size() != 0)  {
            // compute delta_i * G + GAMMA - alpha_i*A
            ECPoint DELTA = curve.getGenerator().clone().multiplyPoint(definition.convertToUnsigned(delta));
            DELTA.addPoint(GAMMA);
            DELTA.subtractPoint(A.clone().multiplyPoint(definition.convertToUnsigned(alpha)));

            deltaBytes = DELTA.encodePoint();
          } else {
            deltaBytes = definition.serialize(delta);
          }

          // Commit to DELTA and open it afterwards
          return seq.seq(new CommitmentComputation(commitmentSerializer, deltaBytes, localDrbg));
        }).seq((seq, commitmentsRaw) -> {
          if (openedValuesECC.size() != 0)  {
            EllipticCurve curve = openedValuesECC.get(0).getCurve();
            ECPoint sum = curve.getNeutralPoint();
            for (byte[] raw : commitmentsRaw) {
              try {
                sum.addPoint(curve.decodePoint(raw));
              }
              catch (Exception e){
                throw new MaliciousException("MAC Decoding error! Aborting!");
              }
            }

            if (!sum.isNeutralPoint()) {
              int elements = openedValues.size() + openedValuesECC.size();
              sum = sum.scalePoint();
              throw new MaliciousException(
                  "The sum of delta's was not 0. Someone was corrupting something amongst "
                      + elements
                      + " macs. Sum was ("
                      + sum.getCoordinate().getX() + ", "
                      + sum.getCoordinate().getY() + ") Aborting!");
            }
          } else {
            List<FieldElement> commitments = definition.deserializeList(commitmentsRaw);
            FieldElement deltaSum =
                commitments
                    .stream()
                    .reduce(definition.createElement(0), FieldElement::add);

            if (!BigInteger.ZERO.equals(definition.convertToUnsigned(deltaSum))) {
              throw new MaliciousException(
                  "The sum of delta's was not 0. Someone was corrupting something amongst "
                      + openedValues.size()
                      + " macs. Sum was " + deltaSum.toString() + " Aborting!");
            }
          }

          // clean up store before returning to evaluating such that we only
          // evaluate the next macs, not those we already checked.
          openedValues.clear();
          closedValues.clear();
          openedValuesECC.clear();
          closedValuesECC.clear();
          return null;
        });
  }

  private FieldElement[] sampleRandomCoefficients(int numCoefficients,
      FieldDefinition fieldDefinition, Drbg jointDrbg) {
    FieldElement[] coefficients = new FieldElement[numCoefficients];
    for (int i = 0; i < numCoefficients; i++) {
      byte[] bytes = new byte[modulus.bitLength() / Byte.SIZE];
      jointDrbg.nextBytes(bytes);
      coefficients[i] = fieldDefinition.createElement(new BigInteger(bytes));
    }
    return coefficients;
  }
}
