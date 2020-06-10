package com.author.mpc_acc;

import dk.alexandra.fresco.framework.builder.numeric.ProtocolBuilderNumeric;
import dk.alexandra.fresco.framework.sce.SecureComputationEngine;
import dk.alexandra.fresco.framework.sce.SecureComputationEngineImpl;
import dk.alexandra.fresco.framework.sce.evaluator.BatchedProtocolEvaluator;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePool;
import com.author.utils.MPCBuilder;

/**
 * MPCParamsBuilder, extends the generic Builder and facilitates creating an instance of the client application: MPCParams
 *
 *  @author ******
 */
public class MPCParamsBuilder extends MPCBuilder<MPCParams> {


    public MPCParamsBuilder(boolean logging) {
        super(logging);
        //        private Supplier<Network> myNetworkSupplier;
    }

    /**
     * The ifxmpc object is created and its members are set according to the members set in the parent builder.
     * @return the fully initiated ifxmpc object.
     */
    public MPCParams build(){
        BatchedProtocolEvaluator<SpdzResourcePool> evaluator = new BatchedProtocolEvaluator<>(batchEvalStrat, mySuite, 4096);
        SecureComputationEngine<SpdzResourcePool, ProtocolBuilderNumeric> mySce = new SecureComputationEngineImpl<>(mySuite, evaluator);
        MPCParams param        = new MPCParams();
        param.myID             = myID;
        param.maxBitLength     = maxBitLength;
        param.numParties       = numberOfParties;
        param.myNetwork        = myNetwork;
        param.mySce            = mySce;
        param.myPool           = myPool;
        param.logging          = logging;
        param.myNetworkManager = myNetworkManager;
        return param;
    }
}
