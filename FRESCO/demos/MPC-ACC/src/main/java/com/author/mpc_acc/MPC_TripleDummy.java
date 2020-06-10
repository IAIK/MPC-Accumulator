package com.author.mpc_acc;

import org.slf4j.LoggerFactory;
import dk.alexandra.fresco.suite.spdz.storage.SpdzMascotDataSupplier;

import org.slf4j.Logger;

/**
 * Dummy to create the batches of triples and random shares in the "offline" phase
 *
 * @author ******
 */
public final class MPC_TripleDummy {

  private static final int LIMIT = 1024;

  private static Logger logger = LoggerFactory.getLogger(MPC_TripleDummy.class);

  public static void produceTriples(SpdzMascotDataSupplier supplier, int elements) {
    logger.debug("Getting another " + elements + " triple");
    int to_produce = elements;
    while (to_produce > 0) {
      int produce = (to_produce > LIMIT) ? LIMIT : to_produce;
      supplier.produceTriples(produce);
      to_produce = to_produce - produce;
    }
    logger.debug("Got another " + elements + " triple");
  }

  public static void produceRandomFieldElements(SpdzMascotDataSupplier supplier, int elements) {
    logger.debug("Getting another " + elements + " random element");
    int to_produce = elements;
    while (to_produce > 0) {
      int produce = (to_produce > LIMIT) ? LIMIT : to_produce;
      supplier.produceRandomFieldElements(produce);
      to_produce = to_produce - produce;
    }
    logger.debug("Got another " + elements + " random element");
  }
}
