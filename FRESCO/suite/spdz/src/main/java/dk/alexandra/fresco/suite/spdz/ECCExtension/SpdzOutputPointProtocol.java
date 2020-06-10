package dk.alexandra.fresco.suite.spdz.ECCExtension;

import dk.alexandra.fresco.suite.spdz.gates.SpdzNativeProtocol;

import dk.alexandra.fresco.framework.DRes;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePool;
import iaik.security.ec.math.curve.ECPoint;
import iaik.security.ec.math.curve.EllipticCurve;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePoolImpl;
import dk.alexandra.fresco.framework.MaliciousException;

import java.util.List;

/**
 * A Spdz protocol
 *
 * @author ******
 */
public class SpdzOutputPointProtocol extends SpdzNativeProtocol<ECPoint> {

  private DRes<SECPoint> in;
  private ECPoint out;

  public SpdzOutputPointProtocol(DRes<SECPoint> in) {
    this.in = in;
  }

  @Override
  public EvaluationStatus evaluate(int round, SpdzResourcePool spdzResourcePool,
      Network network) {
    if (round == 0) {
      SpdzECPoint share = (SpdzECPoint) in.out();
      network.sendToAll(share.serializeShare());
      return EvaluationStatus.HAS_MORE_ROUNDS;
    } else {
      EllipticCurve curve = ((SpdzECPoint)in.out()).getCurve();
      List<byte[]> shares = network.receiveFromAll();
      ECPoint openedVal = null;
      try {
        openedVal = curve.decodePoint(shares.get(0));
        for (int i = 1; i < shares.size(); i++) {
          byte[] buffer = shares.get(i);
          openedVal = openedVal.addPoint(curve.decodePoint(buffer));
        }
      }
      catch (Exception e) {
        throw new MaliciousException("Decoding error! Aborting!");
      }
      ((SpdzResourcePoolImpl)spdzResourcePool).getOpenedECCValueStore().pushOpenedValue(((SpdzECPoint) in.out()), openedVal);
      this.out = openedVal;
      return EvaluationStatus.IS_DONE;
    }
  }

  @Override
  public ECPoint out() {
    return out;
  }
}
