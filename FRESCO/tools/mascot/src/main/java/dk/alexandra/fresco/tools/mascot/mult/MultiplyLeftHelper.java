package dk.alexandra.fresco.tools.mascot.mult;

import dk.alexandra.fresco.framework.builder.numeric.field.FieldDefinition;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.framework.util.StrictBitVector;
import dk.alexandra.fresco.tools.mascot.MascotResourcePool;
import dk.alexandra.fresco.tools.mascot.field.FieldElementUtils;
import dk.alexandra.fresco.tools.ot.base.RotBatch;
import java.math.BigInteger;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * A helper class for multiplication-based protocols {@link dk.alexandra.fresco.tools.mascot.cope.CopeSigner}
 * and the multiplication sub-protocol used by {@link dk.alexandra.fresco.tools.mascot.triple.TripleGeneration}.
 * These two classes share a lot functionality. This functionality is implemented here.
 */
public class MultiplyLeftHelper {

  private final RotBatch rot;
  private final MascotResourcePool resourcePool;
  private final FieldElementUtils fieldElementUtils;

  public MultiplyLeftHelper(MascotResourcePool resourcePool, Network network, int otherId) {
    this.resourcePool = resourcePool;
    this.fieldElementUtils = new FieldElementUtils(resourcePool.getFieldDefinition());
    this.rot = resourcePool.createRot(otherId, network);
  }

  /**
   * Uses left factors as choice bits to receive seeds to prgs.
   *
   * @param leftFactors the left side of the multiplication
   * @param seedLength the length of the seeds that the ROT produces
   * @return list of seeds to prgs
   */
  public List<StrictBitVector> generateSeeds(List<FieldElement> leftFactors, int seedLength) {
    FieldDefinition fieldDefinition = resourcePool.getFieldDefinition();
    StrictBitVector packedFactors =
        new StrictBitVector(fieldDefinition.serialize(reversed(leftFactors)));
    // use rot to get choice seeds
    List<StrictBitVector> seeds = rot.receive(packedFactors, seedLength);
    Collections.reverse(seeds);
    return seeds;
  }

  private List<FieldElement> reversed(List<FieldElement> leftFactors) {
    List<FieldElement> elements = leftFactors;
    elements = new ArrayList<>(elements);
    Collections.reverse(elements);
    return elements;
  }

  public List<StrictBitVector> generateSeeds(FieldElement leftFactor, int seedLength) {
    return generateSeeds(Collections.singletonList(leftFactor), seedLength);
  }

  /**
   * Computes this party's shares of the products. <br> There is a product share per left factor.
   *
   * @param leftFactors this party's multiplication factors
   * @param feSeeds seeds as field elements
   * @param diffs the diffs received from other party
   * @return product shares
   */
  public List<FieldElement> computeProductShares(List<FieldElement> leftFactors,
      List<FieldElement> feSeeds, List<FieldElement> diffs) {
    final FieldElement zeroElement =
        resourcePool.getFieldDefinition().createElement(BigInteger.ZERO);
    List<FieldElement> result = new ArrayList<>(leftFactors.size());
    int diffIdx = 0;
    for (FieldElement leftFactor : leftFactors) {
      StrictBitVector currentBits = resourcePool.getFieldDefinition()
          .convertToBitVector(leftFactor);
      List<FieldElement> summands = new ArrayList<>(resourcePool.getModBitLength());
      for (int b = 0; b < resourcePool.getModBitLength(); b++) {
        FieldElement feSeed = feSeeds.get(diffIdx);
        FieldElement diff = diffs.get(diffIdx);
        boolean bit = currentBits.getBit(b, true);
        FieldElement select = bit ? diff : zeroElement;
        FieldElement summand = select.add(feSeed);
        summands.add(summand);
        diffIdx++;
      }
      FieldElement productShare = fieldElementUtils.recombine(summands);
      result.add(productShare);
    }
    return result;
  }
}
