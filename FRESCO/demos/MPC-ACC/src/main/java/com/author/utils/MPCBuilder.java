package com.author.utils;

import dk.alexandra.fresco.framework.DRes;
import dk.alexandra.fresco.framework.Party;
import dk.alexandra.fresco.framework.builder.numeric.DefaultPreprocessedValues;
import dk.alexandra.fresco.framework.builder.numeric.ProtocolBuilderNumeric;
import dk.alexandra.fresco.framework.builder.numeric.field.BigIntegerFieldDefinition;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.configuration.NetworkConfiguration;
import dk.alexandra.fresco.framework.configuration.NetworkConfigurationImpl;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.framework.sce.evaluator.BatchEvaluationStrategy;
import dk.alexandra.fresco.framework.sce.evaluator.BatchedProtocolEvaluator;
import dk.alexandra.fresco.framework.sce.evaluator.BatchedStrategy;
import dk.alexandra.fresco.framework.sce.evaluator.EvaluationStrategy;
import dk.alexandra.fresco.framework.sce.resources.storage.FilebasedStreamedStorageImpl;
import dk.alexandra.fresco.framework.sce.resources.storage.InMemoryStorage;
import dk.alexandra.fresco.framework.util.*;
import dk.alexandra.fresco.framework.value.SInt;
import dk.alexandra.fresco.logging.BatchEvaluationLoggingDecorator;
import dk.alexandra.fresco.suite.ProtocolSuite;
import dk.alexandra.fresco.suite.spdz.SpdzProtocolSuite;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePool;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePoolImpl;
import dk.alexandra.fresco.suite.spdz.configuration.PreprocessingStrategy;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzSInt;
import dk.alexandra.fresco.suite.spdz.storage.*;
import dk.alexandra.fresco.tools.ot.base.NaorPinkasOt;
import dk.alexandra.fresco.tools.ot.base.Ot;
import dk.alexandra.fresco.tools.ot.otextension.RotList;
import dk.alexandra.fresco.suite.spdz.ECCExtension.SpdzOpenedValueECCStoreImpl;
import dk.alexandra.fresco.tools.ot.base.DhParameters;

import org.apache.commons.cli.ParseException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.math.BigInteger;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.function.Function;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

/**
 * The abstract builder class to instantiate the application objects. This builder takes care of the boiler plate code
 * which is required to setup the framework. It also handles the use case specific input, these two parts could
 * be separated at some point..
 * @param <ObjectT> The Object type the Builder extension wants to implement (either client or host)
 *
 * @author ******
 */
public abstract class MPCBuilder<ObjectT> {
    protected int numberOfParties;
    protected int myID;
    protected SpdzResourcePool myPool;
    protected ProtocolSuite<SpdzResourcePool, ProtocolBuilderNumeric> mySuite;
    protected NetworkConfiguration myNetworkConfiguration;
    protected BatchEvaluationStrategy<SpdzResourcePool> batchEvalStrat;
    protected Network myNetwork;
    protected int maxBitLength;
    protected NetworkManager myNetworkManager;
    protected Logger log = LoggerFactory.getLogger(MPCBuilder.class);
    protected boolean logging;

    /**
     * Creating the builder following the builder pattern.
     * @param logging whether logging is activated during the computation
     */
    public MPCBuilder(boolean logging) {
        this.logging = logging;
    }

    /**
     * Setting the id of the application object
     * @param id the id to be set
     * @return this
     */
    public MPCBuilder<ObjectT> withID(int id){
        this.myID = id;
        return this;
    }

    /**
     * Initializing the resourcePool Object required by the framework. MASCOT has to be used as a preprocessingStrategy
     * to achieve active security
     * @param strategy the PreprocessingStrategy used
     * @param modBitLength the bitLength of the modulus (128 bit is recommended)
     * @return this
     * @throws ParseException thrown when preprocessingStrategy is unknown
     */
    public MPCBuilder<ObjectT> withResourcePool(PreprocessingStrategy strategy, BigInteger modulus) throws ParseException {
        SpdzDataSupplier supplier;
        int PRGSeedLength =  256;
        switch (strategy){

            case DUMMY:
                supplier = new SpdzDummyDataSupplier(this.myID, numberOfParties, new BigIntegerFieldDefinition(modulus), modulus);
                break;

            case MASCOT:
                List<Integer> partyIds = IntStream.range(1, numberOfParties + 1).boxed().collect(Collectors.toList());
                Drbg drbg = getDrbg(myID, PRGSeedLength);
                final BigIntegerFieldDefinition definition = new BigIntegerFieldDefinition(modulus);
                long time = System.currentTimeMillis();
                Map<Integer, RotList> seedOts = getSeedOts(myID, partyIds, PRGSeedLength, drbg, myNetworkManager.createExtraNetwork());
                time = System.currentTimeMillis() - time;
                log.debug("NaorPinkas OT took " + time + "ms");
                int modBitLength = modulus.bitLength();
                FieldElement ssk = SpdzMascotDataSupplier.createRandomSsk(definition, PRGSeedLength);
                supplier = SpdzMascotDataSupplier.createSimpleSupplier(myID, numberOfParties,
                    () -> myNetworkManager.createExtraNetwork(), modBitLength,
                    definition, new Function<Integer, SpdzSInt[]>() {

                        private SpdzMascotDataSupplier tripleSupplier;
                        private Network pipeNetwork;

                        @Override
                        public SpdzSInt[] apply(Integer pipeLength) {
                            if (pipeNetwork == null) {
                                pipeNetwork = myNetworkManager.createExtraNetwork();
                                tripleSupplier = SpdzMascotDataSupplier.createSimpleSupplier(myID, numberOfParties,
                                    () -> pipeNetwork, modBitLength, definition, null,
                                    seedOts, drbg, ssk);
                                if(logging){
                                    log.info("created pipeNetwork and triple supplier");
                                }

                            }
                            if(logging){
                                log.info("creating pipe");
                            }
                            DRes<List<DRes<SInt>>> pipe = createPipe(myID, numberOfParties, pipeLength, pipeNetwork, tripleSupplier, maxBitLength);
                            if(logging){
                                log.info(" created pipe");
                            }
                            return computeSInts(pipe);
                        }
                    }, seedOts, drbg, ssk);
                break;

            case STATIC:
                int noOfThreadsUsed = 1;
                String storageName = SpdzStorageDataSupplier.STORAGE_NAME_PREFIX + noOfThreadsUsed + "_" + this.myID + "_" + 0 + "_";
                supplier = new SpdzStorageDataSupplier(new FilebasedStreamedStorageImpl(new InMemoryStorage()), storageName, numberOfParties);
                break;

            default:
                throw new ParseException("Prepossessing Strategy unknown");
        }
        this.myPool = new SpdzResourcePoolImpl(this.myID, numberOfParties, new SpdzOpenedValueStoreImpl(), new SpdzOpenedValueECCStoreImpl(), supplier, AesCtrDrbg::new);
        return this;
    }

    /**
     * Instantiating the SpdzProtocolSuite required for the computation
     * @param maxBitLength the maximum number of bits for each shared variable
     * @return this.
     */
    public MPCBuilder<ObjectT> withSpdzLength(int maxBitLength){
        this.maxBitLength = maxBitLength;
        this.mySuite = new SpdzProtocolSuite(maxBitLength);
        return this;
    }

    /**
     * Set the BatchEvaluationStrategy accordingly, get the loggingDecorator if logging is activated
     * @param strat the evaluation strategy to be used
     * @return this
     */
    public MPCBuilder<ObjectT> withBatchEvalStrat(EvaluationStrategy strat){
        batchEvalStrat = strat.getStrategy();
        if(logging){
            batchEvalStrat = new BatchEvaluationLoggingDecorator<>(batchEvalStrat);
        }
        return this;
    }

    /**
     * Setting up a network manager and creating the first network to communicate with the other parties
     * @param parties the parties to connect with
     * @param myParty this party
     * @return this
     * @throws ParseException is thrown, if myParty is not contained in the map of parties
     */
    public MPCBuilder<ObjectT> withNetwork(Map<Integer, Party> parties, Party myParty) throws ParseException{
        myID = myParty.getPartyId();
        if (!parties.containsKey(myID)) {
            throw new ParseException("This party is given the id " + myID
                    + " but this id is not present in the list of parties: " + parties.keySet());
        }
        if(logging){
            log.info("These are my parties. " + parties.values());
        }
        numberOfParties = parties.size();
        myNetworkConfiguration = new NetworkConfigurationImpl(myID, parties);
        myNetworkManager = new NetworkManager(myNetworkConfiguration, logging, parties);
        myNetwork = myNetworkManager.createExtraNetwork();
        return this;
    }

    /**
     * This function has to be implemented in each child
     * @return The Object of the class which is built here
     */
    public abstract ObjectT build();

    /**
     * Auxiliary function when initiating the MASCOT protocol in the resource pool function
     * @param myId my id
     * @param prgSeedLength the seed length for the deterministic random bit generator
     * @return a new Drbg instance
     */
    protected static Drbg getDrbg(int myId, int prgSeedLength) {
        byte[] seed = new byte[prgSeedLength / 8];
        new Random(myId).nextBytes(seed);
        return AesCtrDrbgFactory.fromDerivedSeed(seed);
    }

    /**
     * Converting the List of SInts that the are the pipe into an array of SpdzSInts
     * @param pipe the pipe to be converted
     * @return the array of SpdzSInt
     */
    protected static SpdzSInt[] computeSInts(DRes<List<DRes<SInt>>> pipe) {
        List<DRes<SInt>> out = pipe.out();
        SpdzSInt[] result = new SpdzSInt[out.size()];
        for (int i = 0; i < out.size(); i++) {
            DRes<SInt> sIntResult = out.get(i);
            result[i] = (SpdzSInt) sIntResult.out();
        }
        return result;
    }

    /**
     * Creates a protocol for the exponentiation pipe.
     * @param myId my id
     * @param noOfPlayers the number of players in the network
     * @param pipeLength the required length of the new pipe
     * @param pipeNetwork the network to be used to create the network in an MPC environment
     * @param tripleSupplier A simple triple supplier used to do the pipe creation using the MASCOT protocol
     * @param maxBitLength the maximum bit length of variables in this application
     * @return The newly created pipe
     */
    protected static DRes<List<DRes<SInt>>> createPipe(int myId, int noOfPlayers, int pipeLength, Network pipeNetwork, SpdzMascotDataSupplier tripleSupplier, int maxBitLength) {
        SpdzResourcePoolImpl tripleResourcePool = new SpdzResourcePoolImpl(myId, noOfPlayers, new OpenedValueStoreImpl<>(), tripleSupplier, AesCtrDrbg::new);
        SpdzProtocolSuite spdzProtocolSuite = new SpdzProtocolSuite(maxBitLength);
        ProtocolBuilderNumeric sequential = spdzProtocolSuite.init(tripleResourcePool).createSequential();
        DRes<List<DRes<SInt>>> exponentiationPipe = new DefaultPreprocessedValues(sequential).getExponentiationPipe(pipeLength);
        evaluate(sequential, tripleResourcePool, pipeNetwork, spdzProtocolSuite);
        return exponentiationPipe;
    }

    /**
     * Evaluating the pipe creation protocol, so that the defered pipe can be used
     * @param spdzBuilder the protocolbuilder used to do MPC operations
     * @param tripleResourcePool The resource pool used to execute this MPC protocol
     * @param network The network used for communication
     * @param spdzProtocolSuite the protocolSuite instance used
     */
    protected static void evaluate(ProtocolBuilderNumeric spdzBuilder, SpdzResourcePool tripleResourcePool, Network network, SpdzProtocolSuite spdzProtocolSuite) {
        BatchedStrategy<SpdzResourcePool> batchedStrategy = new BatchedStrategy<>();
        BatchedProtocolEvaluator<SpdzResourcePool> batchedProtocolEvaluator = new BatchedProtocolEvaluator<>(batchedStrategy, spdzProtocolSuite);
        batchedProtocolEvaluator.eval(spdzBuilder.build(), tripleResourcePool, network);
    }

    /**
     * An auxiliary function used for the oblivious transfer necessary to initiate the MASCOT protocol.
     * @param myId my id
     * @param partyIds a list of all ids in the network
     * @param prgSeedLength the length of the random seed
     * @param drbg the actual random bit generator instance
     * @param network the network used to do the oblivious transfer
     * @return The map of SeedOts
     */
    protected static Map<Integer, RotList> getSeedOts(int myId, List<Integer> partyIds, int prgSeedLength,
                                                   Drbg drbg, Network network) {
        Map<Integer, RotList> seedOts = new HashMap<>();
        for (Integer otherId : partyIds) {
            if (myId != otherId) {
                Ot ot = new NaorPinkasOt(otherId, drbg, network, DhParameters.getStaticDhParams());
                RotList currentSeedOts = new RotList(drbg, prgSeedLength);
                if (myId < otherId) {
                    currentSeedOts.send(ot);
                    currentSeedOts.receive(ot);
                } else {
                    currentSeedOts.receive(ot);
                    currentSeedOts.send(ot);
                }
                seedOts.put(otherId, currentSeedOts);
            }
        }
        return seedOts;
    }
}
