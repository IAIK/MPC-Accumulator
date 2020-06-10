package com.author.mpc_acc;

import dk.alexandra.fresco.framework.builder.numeric.*;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.framework.sce.SecureComputationEngine;
import dk.alexandra.fresco.suite.spdz.SpdzResourcePool;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import com.author.utils.NetworkManager;

/**
 * MPCParams
 * The build computation function adds the MPC functionality to the protocol builder
 *
 * @author ******
 *
 */
public class MPCParams {

    static Logger log = LoggerFactory.getLogger(MPCParams.class);
    int maxBitLength;
    int numParties;
    int myID;
    boolean logging;
    SpdzResourcePool myPool;
    Network myNetwork;
    NetworkManager myNetworkManager;
    SecureComputationEngine<SpdzResourcePool, ProtocolBuilderNumeric> mySce;

    public NetworkManager getMyNetworkManager() {
        return myNetworkManager;
    }

    public boolean isLogging() {
        return logging;
    }

    public Network getMyNetwork() {
        return myNetwork;
    }

    public SpdzResourcePool getMyPool() {
        return myPool;
    }

    public SecureComputationEngine<SpdzResourcePool, ProtocolBuilderNumeric> getMySce() {
        return mySce;
    }

    public void closeNetwork(){
        if(this.myNetworkManager != null){
            myNetworkManager.close();
        }
    }

    public void shutdownSce(){
        this.mySce.shutdownSCE();
    }

    public void shutdown(){
        shutdownSce();
        closeNetwork();
    }

    MPCParams(){}


    public void log(String string){
        if(logging){
            log.info(string);
        }
    }
}
