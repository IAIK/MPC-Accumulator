package dk.alexandra.fresco.suite.spdz.gates;

import dk.alexandra.fresco.framework.MaliciousException;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.framework.network.serializers.ByteSerializer;
import dk.alexandra.fresco.framework.value.SInt;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePool;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzInputMask;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzSInt;
import dk.alexandra.fresco.suite.spdz.storage.SpdzDataSupplier;
import java.math.BigInteger;

public class SpdzInputProtocol extends SpdzNativeProtocol<SInt> {

  private SpdzInputMask inputMask; // is opened by this gate.
  protected BigInteger input;
  private FieldElement valueMasked;
  protected SpdzSInt out;
  private int inputter;
  private byte[] digest;

  public SpdzInputProtocol(BigInteger input, int inputter) {
    this.input = input;
    this.inputter = inputter;
  }

  @Override
  public EvaluationStatus evaluate(int round, SpdzResourcePool spdzResourcePool,
      Network network) {
    int myId = spdzResourcePool.getMyId();
    SpdzDataSupplier dataSupplier = spdzResourcePool.getDataSupplier();
    ByteSerializer<FieldElement> serializer = spdzResourcePool.getFieldDefinition();
    if (round == 0) {
      this.inputMask = dataSupplier.getNextInputMask(this.inputter);
      if (myId == this.inputter) {
        FieldElement inputElement = spdzResourcePool.getFieldDefinition().createElement(this.input);
        FieldElement bcValue = inputElement.subtract(this.inputMask.getRealValue());
        network.sendToAll(serializer.serialize(bcValue));
      }
      return EvaluationStatus.HAS_MORE_ROUNDS;
    } else if (round == 1) {
      byte[] receivedBytes = network.receive(inputter);
      this.valueMasked = serializer.deserialize(receivedBytes);
      this.digest =
          sendBroadcastValidation(spdzResourcePool.getMessageDigest(), network, receivedBytes);
      return EvaluationStatus.HAS_MORE_ROUNDS;
    } else {
      boolean validated = receiveBroadcastValidation(network, digest);
      if (!validated) {
        throw new MaliciousException("Broadcast digests did not match");
      }
      FieldElement maskedValue = dataSupplier.getSecretSharedKey().multiply(valueMasked);
      SpdzSInt valueMaskedElement = new SpdzSInt(valueMasked, maskedValue);
      this.out = this.inputMask.getMask().add(valueMaskedElement, myId);
      return EvaluationStatus.IS_DONE;
    }
  }

  @Override
  public SpdzSInt out() {
    return out;
  }
}
