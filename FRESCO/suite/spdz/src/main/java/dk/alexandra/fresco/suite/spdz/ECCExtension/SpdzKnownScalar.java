package dk.alexandra.fresco.suite.spdz.ECCExtension;

import dk.alexandra.fresco.suite.spdz.gates.SpdzNativeProtocol;
import java.math.BigInteger;

import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePool;
import dk.alexandra.fresco.framework.value.SInt;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzSInt;
import dk.alexandra.fresco.suite.spdz.storage.SpdzDataSupplier;
import dk.alexandra.fresco.suite.spdz.storage.SpdzMascotDataSupplier;

/**
 * A Spdz protocol
 *
 * @author ******
 */
public class SpdzKnownScalar extends SpdzNativeProtocol<SInt> {

  private BigInteger k;
  private SpdzSInt out;

  public SpdzKnownScalar(BigInteger k) {
    this.k = k;
  }

  @Override
  public EvaluationStatus evaluate(int round, SpdzResourcePool spdzResourcePool,
      Network network) {

    FieldElement value = spdzResourcePool.getFieldDefinition().createElement(k);

    SpdzDataSupplier dataSupplier = spdzResourcePool.getDataSupplier();
    if (!(dataSupplier instanceof SpdzMascotDataSupplier)) {
      FieldElement globalKeyShare = spdzResourcePool.getDataSupplier().getSecretSharedKey();
      FieldElement mac = value.multiply(globalKeyShare);
      out = new SpdzSInt(value, mac);
      return EvaluationStatus.IS_DONE;
    }

    out = ((SpdzMascotDataSupplier)dataSupplier).shareKnownElement(value);

    return EvaluationStatus.IS_DONE;
  }

  @Override
  public SInt out() {
    return out;
  }
}
