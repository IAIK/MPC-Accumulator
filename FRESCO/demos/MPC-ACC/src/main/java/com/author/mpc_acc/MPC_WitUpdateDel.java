package com.author.mpc_acc;

import dk.alexandra.fresco.framework.Application;
import dk.alexandra.fresco.framework.DRes;
import dk.alexandra.fresco.framework.builder.numeric.Numeric;
// import org.slf4j.LoggerFactory;
import dk.alexandra.fresco.framework.builder.numeric.ProtocolBuilderNumeric;

import dk.alexandra.fresco.suite.spdz.ECCExtension.SECPoint;
import dk.alexandra.fresco.suite.spdz.ECCExtension.SpdzECCOps;
import java.math.BigInteger;
import iaik.security.ec.math.curve.ECPoint;
import dk.alexandra.fresco.framework.value.SInt;

// import org.slf4j.Logger;

/**
 * Updates the witness for deletion
 *
 * @author ******
 */
public class MPC_WitUpdateDel implements Application<ECPoint, ProtocolBuilderNumeric> {

  // private static Logger log = LoggerFactory.getLogger(MPC_Gen.class);

  private BigInteger sk_share;
  private BigInteger x;
  private ECPoint wit;
  private DRes<SInt> ro;

  /**
   * Construct a new DistanceDemo.
   * @param id The party id
   * @param x The x coordinate
   * @param y The y coordinate
   */
  public MPC_WitUpdateDel(BigInteger sk_share, BigInteger x, ECPoint wit) {
    this.sk_share = sk_share;
    this.x = x;
    this.wit = wit;
  }

  @Override
  public DRes<ECPoint> buildComputation(ProtocolBuilderNumeric producer) {
    return producer.seq(seq -> {
      SpdzECCOps ops = new SpdzECCOps(seq);
      DRes<SInt> share = ops.knownScalar(sk_share);
      Numeric numericIo = seq.numeric();
      DRes<SInt> q = numericIo.add(x, share);
      // invert q: z = q^-1
      ro = numericIo.randomElement();
      DRes<SInt> sigma = numericIo.mult(ro, q);
      return numericIo.open(sigma);
    }).seq((seq, sigma) -> {
      BigInteger sigma_inv = sigma.modInverse(wit.getCurve().getOrder());
      Numeric numericIo = seq.numeric();
      DRes<SInt> z = numericIo.mult(sigma_inv, ro);
      SpdzECCOps ops = new SpdzECCOps(seq);
      DRes<SECPoint> wit_new =  ops.multiply(wit, z);
      return ops.open(wit_new);
    });
  }
}
