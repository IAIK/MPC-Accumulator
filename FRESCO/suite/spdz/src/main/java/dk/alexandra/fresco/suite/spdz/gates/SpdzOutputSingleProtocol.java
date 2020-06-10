package dk.alexandra.fresco.suite.spdz.gates;

import dk.alexandra.fresco.framework.DRes;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldDefinition;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.framework.value.SInt;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePool;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzInputMask;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzSInt;
import dk.alexandra.fresco.suite.spdz.storage.SpdzDataSupplier;
import java.math.BigInteger;
import java.util.List;

public class SpdzOutputSingleProtocol extends SpdzNativeProtocol<BigInteger>
    implements SpdzOutputProtocol {

  private DRes<SInt> in;
  private BigInteger out;
  private int targetPlayer;
  private SpdzInputMask mask;
  private SpdzSInt inMinusMask;

  public SpdzOutputSingleProtocol(DRes<SInt> in, int targetPlayer) {
    this.in = in;
    this.targetPlayer = targetPlayer;
  }

  @Override
  public BigInteger out() {
    return out;
  }

  @Override
  public EvaluationStatus evaluate(int round, SpdzResourcePool spdzResourcePool,
      Network network) {
    int myId = spdzResourcePool.getMyId();
    SpdzDataSupplier dataSupplier = spdzResourcePool.getDataSupplier();
    FieldDefinition definition = spdzResourcePool.getFieldDefinition();
    if (round == 0) {
      this.mask = dataSupplier.getNextInputMask(targetPlayer);
      SpdzSInt closedValue = (SpdzSInt) this.in.out();
      inMinusMask = closedValue.subtract(this.mask.getMask());
      network.sendToAll(inMinusMask.serializeShare(definition));
      return EvaluationStatus.HAS_MORE_ROUNDS;
    } else {
      List<byte[]> shares = network.receiveFromAll();
      FieldElement openedVal = definition.deserialize(shares.get(0));
      for (int i = 1; i < shares.size(); i++) {
        byte[] buffer = shares.get(i);
        openedVal = openedVal.add(definition.deserialize(buffer));
      }
      spdzResourcePool.getOpenedValueStore().pushOpenedValue(inMinusMask, openedVal);
      if (targetPlayer == myId) {
        openedVal = openedVal.add(this.mask.getRealValue());
        this.out = definition.convertToUnsigned(openedVal);
      }
      return EvaluationStatus.IS_DONE;
    }
  }
}
