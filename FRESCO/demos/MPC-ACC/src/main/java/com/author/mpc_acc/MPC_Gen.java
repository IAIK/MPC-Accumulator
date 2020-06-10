package com.author.mpc_acc;

import dk.alexandra.fresco.framework.Application;
import dk.alexandra.fresco.framework.DRes;
// import org.slf4j.LoggerFactory;
import dk.alexandra.fresco.framework.builder.numeric.ProtocolBuilderNumeric;

import dk.alexandra.fresco.suite.spdz.ECCExtension.SECPoint;
import dk.alexandra.fresco.suite.spdz.ECCExtension.SpdzECCOps;
import java.math.BigInteger;
import iaik.security.ec.math.curve.ECPoint;

// import org.slf4j.Logger;

/**
 * Generates the secret key shares
 *
 * @author ******
 */
public class MPC_Gen implements Application<ECPoint, ProtocolBuilderNumeric> {

  // private static Logger log = LoggerFactory.getLogger(MPC_Gen.class);

  private BigInteger sk_share;
  private ECPoint g2;

  /**
   * Construct a new DistanceDemo.
   * @param id The party id
   * @param x The x coordinate
   * @param y The y coordinate
   */
  public MPC_Gen(BigInteger sk_share, ECPoint g2) {
    this.sk_share = sk_share;
    this.g2 = g2;
  }

  @Override
  public DRes<ECPoint> buildComputation(ProtocolBuilderNumeric producer) {
    return producer.seq(seq -> {
      SpdzECCOps ops = new SpdzECCOps(seq);
      DRes<SECPoint> share =  ops.knownMultiply(g2, sk_share);
      DRes<ECPoint> pk = ops.open(share);
      return pk;
    });
  }
}
