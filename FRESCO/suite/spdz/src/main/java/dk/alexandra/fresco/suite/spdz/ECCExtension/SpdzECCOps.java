package dk.alexandra.fresco.suite.spdz.ECCExtension;

import dk.alexandra.fresco.suite.spdz.ECCExtension.SECPoint;
import java.math.BigInteger;

import dk.alexandra.fresco.framework.DRes;
import dk.alexandra.fresco.framework.builder.numeric.ProtocolBuilderNumeric;
import iaik.security.ec.math.curve.ECPoint;
import dk.alexandra.fresco.framework.value.SInt;

public class SpdzECCOps {
  private final ProtocolBuilderNumeric protocolBuilder;

  public SpdzECCOps(ProtocolBuilderNumeric protocolBuilder) {
    this.protocolBuilder = protocolBuilder;
  }

  public DRes<SInt> knownScalar(BigInteger k) {
    SpdzKnownScalar spdzKnownScalara = new SpdzKnownScalar(k);
    return protocolBuilder.append(spdzKnownScalara);
  }

  public DRes<SECPoint> knownMultiply(ECPoint p, BigInteger k) {
    SpdzKnownMultECCProtocol spdzKnownMultECCProtocol = new SpdzKnownMultECCProtocol(p, k);
    return protocolBuilder.append(spdzKnownMultECCProtocol);
  }

  public DRes<SECPoint> multiply(ECPoint p, DRes<SInt> k) {
    SpdzMultECCProtocol spdzMultECCProtocol = new SpdzMultECCProtocol(p, k);
    return protocolBuilder.append(spdzMultECCProtocol);
  }

  public DRes<ECPoint> open(DRes<SECPoint> secretshare) {
    SpdzOutputPointProtocol spdzOutputPointProtocol = new SpdzOutputPointProtocol(secretshare);
    return protocolBuilder.append(spdzOutputPointProtocol);
  }

}
