package com.author.mpc_acc;

import dk.alexandra.fresco.framework.Application;
import dk.alexandra.fresco.framework.DRes;
// import org.slf4j.LoggerFactory;

import dk.alexandra.fresco.framework.builder.numeric.Numeric;
import dk.alexandra.fresco.framework.builder.numeric.ProtocolBuilderNumeric;
import dk.alexandra.fresco.suite.spdz.ECCExtension.SECPoint;
import dk.alexandra.fresco.suite.spdz.ECCExtension.SpdzECCOps;
import java.math.BigInteger;
import iaik.security.ec.math.curve.ECPoint;
import java.util.List;
import dk.alexandra.fresco.framework.value.SInt;

// import org.slf4j.Logger;

/**
 * Creates the accumulator
 *
 * @author ******
 */
public class MPC_Eval implements Application<ECPoint, ProtocolBuilderNumeric> {

  // private static Logger log = LoggerFactory.getLogger(MPC_Eval.class);

  private BigInteger sk_share;
  private ECPoint g1;
  private List<BigInteger> X;

  /**
   * Construct a new DistanceDemo.
   * @param id The party id
   * @param x The x coordinate
   * @param y The y coordinate
   */
  public MPC_Eval(BigInteger sk_share, ECPoint g1, List<BigInteger> X) {
    this.sk_share = sk_share;
    this.g1 = g1;
    this.X = X;
  }

  @Override
  public DRes<ECPoint> buildComputation(ProtocolBuilderNumeric producer) {
    return producer.seq(seq -> {
      SpdzECCOps ops = new SpdzECCOps(seq);
      DRes<SInt> share_s = ops.knownScalar(sk_share);
      Numeric numericIo = seq.numeric();
      DRes<SInt> q = numericIo.add(X.get(0), share_s);
      for (int i = 1; i < X.size(); i++) {
        q = numericIo.mult(q, numericIo.add(X.get(i), share_s));
      }
      DRes<SInt> share_r = numericIo.randomElement();
      DRes<SECPoint> o_share =  ops.multiply(g1, numericIo.mult(q, share_r));
      return ops.open(o_share);
    });
  }
}
