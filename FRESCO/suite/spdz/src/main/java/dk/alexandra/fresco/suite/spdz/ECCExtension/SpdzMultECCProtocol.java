package dk.alexandra.fresco.suite.spdz.ECCExtension;
import dk.alexandra.fresco.suite.spdz.gates.SpdzNativeProtocol;
import java.math.BigInteger;

import dk.alexandra.fresco.framework.builder.numeric.field.FieldDefinition;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePool;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzSInt;
import iaik.security.ec.math.curve.ECPoint;
import dk.alexandra.fresco.framework.value.SInt;
import dk.alexandra.fresco.framework.DRes;

/**
 * A Spdz protocol
 *
 * @author ******
 */
public class SpdzMultECCProtocol extends SpdzNativeProtocol<SECPoint> {

  private ECPoint left;
  private DRes<SInt> right;
  private SpdzECPoint out;

  public SpdzMultECCProtocol(ECPoint p, DRes<SInt> k) {
    this.left = p;
    this.right = k;
  }

  @Override
  public EvaluationStatus evaluate(int round, SpdzResourcePool spdzResourcePool,
      Network network) {

    FieldDefinition fd = spdzResourcePool.getFieldDefinition();
    SpdzSInt right = (SpdzSInt) this.right.out();
    BigInteger share = fd.convertToUnsigned(right.getShare());
    BigInteger mac = fd.convertToUnsigned(right.getMac());

    out = SpdzECPoint.multiplyPoint(left, share, mac);

    return EvaluationStatus.IS_DONE;
  }

  @Override
  public SpdzECPoint out() {
    return out;
  }
}
