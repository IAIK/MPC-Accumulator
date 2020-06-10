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
 * Updates the witness for addition
 *
 * @author ******
 */
public class MPC_WitUpdateAdd implements Application<ECPoint, ProtocolBuilderNumeric> {

  // private static Logger log = LoggerFactory.getLogger(MPC_Gen.class);

  private BigInteger sk_share;
  private ECPoint wit;

  /**
   * Construct a new DistanceDemo.
   * @param id The party id
   * @param x The x coordinate
   * @param y The y coordinate
   */
  public MPC_WitUpdateAdd(BigInteger sk_share, ECPoint wit) {
    this.sk_share = sk_share;
    this.wit = wit;
  }

  @Override
  public DRes<ECPoint> buildComputation(ProtocolBuilderNumeric producer) {
    return producer.seq(seq -> {
      SpdzECCOps ops = new SpdzECCOps(seq);
      DRes<SECPoint> share =  ops.knownMultiply(wit, sk_share);
      DRes<ECPoint> o_share = ops.open(share);
      return o_share;
    });
  }
}
