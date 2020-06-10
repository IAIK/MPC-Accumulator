package dk.alexandra.fresco.suite.spdz.ECCExtension;

import dk.alexandra.fresco.suite.spdz.gates.SpdzNativeProtocol;
import java.math.BigInteger;

import dk.alexandra.fresco.framework.builder.numeric.field.FieldDefinition;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePool;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzSInt;
import iaik.security.ec.math.curve.ECPoint;
import dk.alexandra.fresco.suite.spdz.storage.SpdzDataSupplier;
import dk.alexandra.fresco.suite.spdz.storage.SpdzMascotDataSupplier;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;

/**
 * A Spdz protocol
 *
 * @author ******
 */
public class SpdzKnownMultECCProtocol extends SpdzNativeProtocol<SECPoint> {

  private ECPoint left;
  private BigInteger right;
  private SpdzECPoint out;

  public SpdzKnownMultECCProtocol(ECPoint p, BigInteger k) {
    this.left = p;
    this.right = k;
  }

  @Override
  public EvaluationStatus evaluate(int round, SpdzResourcePool spdzResourcePool,
      Network network) {

    BigInteger share = null;
    BigInteger mac = null;
    FieldDefinition fd = spdzResourcePool.getFieldDefinition();


    SpdzDataSupplier dataSupplier = spdzResourcePool.getDataSupplier();
    if (!(dataSupplier instanceof SpdzMascotDataSupplier)) {
      BigInteger mod = fd.getModulus();
      BigInteger globalKeyShare = fd.convertToUnsigned(spdzResourcePool.getDataSupplier().getSecretSharedKey());
      share = right;
      mac = right.multiply(globalKeyShare).mod(mod);
    }
    else {
      FieldElement value = fd.createElement(right);
      SpdzSInt tmp = ((SpdzMascotDataSupplier)dataSupplier).shareKnownElement(value);
      share = fd.convertToUnsigned(tmp.getShare());
      mac = fd.convertToUnsigned(tmp.getMac());
    }
    out = SpdzECPoint.multiplyPoint(left, share, mac);

    return EvaluationStatus.IS_DONE;
  }

  @Override
  public SpdzECPoint out() {
    return out;
  }
}
