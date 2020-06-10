package dk.alexandra.fresco.tools.mascot.bit;

import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.util.StrictBitVector;
import dk.alexandra.fresco.tools.mascot.CustomAsserts;
import dk.alexandra.fresco.tools.mascot.MascotTestContext;
import dk.alexandra.fresco.tools.mascot.MascotTestUtils;
import dk.alexandra.fresco.tools.mascot.NetworkedTest;
import dk.alexandra.fresco.tools.mascot.elgen.ElementGeneration;
import dk.alexandra.fresco.tools.mascot.field.AuthenticatedElement;
import dk.alexandra.fresco.tools.mascot.online.OnlinePhase;
import dk.alexandra.fresco.tools.mascot.prg.FieldElementPrg;
import dk.alexandra.fresco.tools.mascot.prg.FieldElementPrgImpl;
import dk.alexandra.fresco.tools.mascot.triple.TripleGeneration;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Callable;
import org.junit.Test;

public class TestBitConverter extends NetworkedTest {

  private FieldElementPrg getJointPrg(int prgSeedLength) {
    return new FieldElementPrgImpl(new StrictBitVector(prgSeedLength), getFieldDefinition());
  }

  private List<FieldElement> runConvertToBits(MascotTestContext ctx, FieldElement macKeyShare,
      List<FieldElement> randomValues) {
    FieldElementPrg prg = getJointPrg(ctx.getPrgSeedLength());
    ElementGeneration elementGeneration = new ElementGeneration(ctx.getResourcePool(),
        ctx.getNetwork(), macKeyShare, prg);
    OnlinePhase onlinePhase = new OnlinePhase(ctx.getResourcePool(),
        new TripleGeneration(ctx.getResourcePool(), ctx.getNetwork(),
            elementGeneration, prg),
        elementGeneration, macKeyShare);
    BitConverter bitConverter = new BitConverter(ctx.getResourcePool(), onlinePhase, macKeyShare);
    List<AuthenticatedElement> closed = (ctx.getMyId() == 1) ? elementGeneration.input(randomValues)
        : elementGeneration.input(1, randomValues.size());
    List<AuthenticatedElement> bits = bitConverter.convertToBits(closed);
    List<FieldElement> opened = elementGeneration.open(bits);
    elementGeneration.check(bits, opened);
    return opened;
  }

  @Test
  public void testTwoPartiesBatchedConvertToBits() {
    initContexts(2);

    // left party mac key share
    FieldElement macKeyShareOne = getFieldDefinition().createElement("11231");

    // right party mac key share
    FieldElement macKeyShareTwo = getFieldDefinition().createElement("7719");

    // party one inputs
    List<FieldElement> randomValues =
        MascotTestUtils.generateSingleRow(new int[]{12, 11, 1, 2}, getFieldDefinition());

    // define task each party will run
    Callable<List<FieldElement>> partyOneTask =
        () -> runConvertToBits(contexts.get(1), macKeyShareOne, randomValues);
    Callable<List<FieldElement>> partyTwoTask =
        () -> runConvertToBits(contexts.get(2), macKeyShareTwo, randomValues);

    List<List<FieldElement>> results =
        testRuntime.runPerPartyTasks(Arrays.asList(partyOneTask, partyTwoTask));
    List<FieldElement> partyOneOutput = results.get(0);
    List<FieldElement> partyTwoOutput = results.get(1);

    // outputs should be same
    CustomAsserts.assertEquals(getFieldDefinition(), partyOneOutput, partyTwoOutput);

    // outputs should be bits
    for (FieldElement actualBit : partyOneOutput) {
      CustomAsserts.assertFieldElementIsBit(getFieldDefinition(), actualBit);
    }
  }

}
