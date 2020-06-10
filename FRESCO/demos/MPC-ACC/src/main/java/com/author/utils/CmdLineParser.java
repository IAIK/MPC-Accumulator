package com.author.utils;

import dk.alexandra.fresco.framework.Party;
import dk.alexandra.fresco.framework.sce.evaluator.EvaluationStrategy;
import dk.alexandra.fresco.suite.spdz.configuration.PreprocessingStrategy;
import org.apache.commons.cli.*;
// import org.slf4j.Logger;
// import org.slf4j.LoggerFactory;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.*;

/**
 * Utility class to gather all the builder parameters necessary for the applications from the command line.
 *
 * @author ******
 */
public class CmdLineParser {
    public static final String IDMSG          = "The id of this player. Must be a unique positive integer.";
    public static final String SETMSG         = "The number of elements in the generator.";
    public static final String PARTYMSG       = "Connection data for a party. Use -p multiple times to specify many players. You must always at least include yourself. Must be on the form [id]:[hostname]:[port]. id is a unique positive integer for the player, host and port is where to find the player";
    public static final String PRESTRATMSG    = "Used to set the preprocessing Strategy of SPDZ";
    public static final String LOGGINGMSG     = "Informs FRESCO that performance logging should be triggered";
    public static final String IDERRMSG       = "ID must be positive";
    public static final String PARTYERRMSG    = "Party ids must be unique";
    public static final String SETERRMSG       = "number of elements must be > 1";
    // private static Logger log = LoggerFactory.getLogger(CmdLineParser.class);
    public static int newID = 0;


    /**
     * Checks on a basic level if the party input after -p is in the correct form
     * @param partyOption The option provided after this -p argument
     * @return Returns the party parsed from the partyOption
     * @throws ParseException If the partyOption is not compliant with the expected format
     */
    public static Party checkParty(String partyOption) throws ParseException {
        String[] p = partyOption.split(":");
        if(p.length != 3){
            throw new ParseException("Could not parse '" + partyOption + "' as [id]:[host]:[port]");
        }
        try {
            int id = Integer.parseInt(p[0]);
            InetAddress.getByName(p[1]); // Check that hostname is valid.
            return new Party(id, p[1], Integer.parseInt(p[2]));
        } catch (NumberFormatException | UnknownHostException e) {
            throw new ParseException("Could not parse '" + partyOption + "': " + e.getMessage());
        }
    }


    /**
     * Main functionality to read the input arguments and parse the Builder Parameters. Every option is required but the multithreaded
     * and logging flag. the correct format of each option can be read below or int the examples given in the makefile
     * @param args the command line arguments provided when starting up the program
     * @return Returns a BuilderParams object, providing everything the builders need.
     * @throws ParseException throws this exception if some value is not as expected
     */
    public static BuilderParams GetCmdLineParams(String[] args) throws ParseException {

        //Define possible CommandLine options
        Options options = new Options();
        options.addOption(Option.builder("i").desc(IDMSG).longOpt("id").required(true).hasArg().build());
        options.addOption(Option.builder("p").desc(PARTYMSG).longOpt("party").required(true).hasArgs().build());
        options.addOption(Option.builder("D").desc(PRESTRATMSG).required(true).hasArg().numberOfArgs(2).valueSeparator().build());
        options.addOption(Option.builder("l").desc(LOGGINGMSG).required(false).hasArg(false).build());
        options.addOption(Option.builder("n").desc(SETMSG).required(false).hasArg().build());

        // Parse Command line input into CommandLine format
        CommandLineParser parser = new DefaultParser();
        CommandLine cmd = parser.parse(options, args);

        // Extract input values from cmd
        int myid = Integer.parseInt(cmd.getOptionValue("i"));
        int set = 10;
        if (cmd.hasOption("n"))
            set = Integer.parseInt(cmd.getOptionValue("n"));
        boolean logging = cmd.hasOption("l");
        boolean multiThreaded = cmd.hasOption("m");
        final Map<Integer, Party> parties = new HashMap<>();
        List<Map<Integer, Party>> partyList = new ArrayList<>();
        Party myParty = null;
        Properties spdzProperties = cmd.getOptionProperties("D");
        PreprocessingStrategy strategy = PreprocessingStrategy.valueOf(spdzProperties.getProperty("spdz.preprocessingStrategy", PreprocessingStrategy.DUMMY.toString()));

        // Sanitize input values -- only on a basic level
        if (myid < 1)
            throw new ParseException(IDERRMSG);

        if (set < 2)
            throw new ParseException(SETERRMSG);


        // check if all parties are entered correctly
        for (String partyOptions : cmd.getOptionValues("p")) {
            Party party = checkParty(partyOptions);
            if(parties.containsKey(party.getPartyId())){
                throw new ParseException(PARTYERRMSG);
            }
            parties.put(party.getPartyId(), party);
        }
        myParty = parties.get(myid);
        if(myParty == null){
            throw new ParseException("This party is given the id " + myid
                    + " but this id is not present in the list of parties: ");
        }
        if(multiThreaded){
            partyList = createPartyMap(parties, myid);
            myParty = new Party(newID, myParty.getHostname(), myParty.getPort());
        } else {
            partyList.add(parties);
        }

        // Build the IFX.MPC.IFX.Client.IFXMPC using the parsed values
        BuilderParams params = new BuilderParams(logging, multiThreaded);
        params.setEl(set);
        params.setId(myid);
        params.setParties(partyList, myParty);
        params.setPreprocessingStrategy(strategy);
        params.setEvaluationStrategy(EvaluationStrategy.SEQUENTIAL);
        return params;

    }


    /**
     * In case the application is executed using multi threading, this function separates the parties into different party maps.
     * in these different maps, the ids are newly set, since in the framework the ids have to contain 1 and have to ascend from one on.
     * If this is to be changed, one has to replace the socket network of the framework with an independent implementation.
     * @param parties the current party map which is to be separated
     * @param myID the id this user currently possesses
     * @return a list of new party maps, one for each subgroup of clients
     *
     * NOTE: This function has to be exchanged/modified if one wants to change the logic of separation (e.g. separation by date)
     * Now separation is by id: (1,2,3), (1,4,5) ... 1 is infineon and has to be present in every map
     */
    private static List<Map<Integer, Party>> createPartyMap(Map<Integer, Party> parties, int myID){
        List<Map<Integer, Party>> partiesList = new ArrayList<>();
        Map<Integer, Party> parties_ = new HashMap<>();
        int hostID = 1;
        int cnt = parties.size();
        int id_cnt = 2;
        for(Map.Entry<Integer, Party> entry : parties.entrySet()){
            cnt--;
            if(entry.getKey() != hostID) {
                parties_.put(id_cnt, new Party(id_cnt, entry.getValue().getHostname(), entry.getValue().getPort()));
                if(entry.getKey() == myID){
                    newID = id_cnt;
                }
                id_cnt++;
            } if (parties_.size() == 2 && cnt > hostID){
                parties_.put(hostID, parties.get(hostID));
                partiesList.add(parties_);
                //log.info("map: " + parties_.keySet());
                //log.info("map: " + parties_.values());
                id_cnt = 2;
                parties_ = new HashMap<>();
            }
        }
        parties_.put(hostID, parties.get(hostID));
        //log.info("map: " + parties_.keySet());
        //log.info("map: " + parties_.values());
        partiesList.add(parties_);

        return partiesList;
    }


    /**
     * A class providing storage for all the parameters the applications need to be properly initialized.
     * This includes parameters used in the framework as well as parameters which are use case specific.
     *
     */
    public static class BuilderParams{
        public boolean logging;
        public int id;
        public Party myParty;
        public Map<Integer, Party> parties;
        public List<Map<Integer, Party>> partyList;
        public int maxBitLength;
        public PreprocessingStrategy preprocessingStrategy;
        public EvaluationStrategy evaluationStrategy;
        public boolean multiThreaded;
        public int el;

        public BuilderParams(boolean logging, boolean multiThreaded){ this.logging = logging; this.multiThreaded = multiThreaded;}
        public void setId(int id){ this.id = id; }
        public void setEl(int el){ this.el = el; }
        public void setParties(List<Map<Integer, Party>> partyList, Party party){
            this.myParty = party;
            this.partyList = partyList;
            if(!multiThreaded){
                this.parties = partyList.get(0);
            }
        }
        public void setMaxBitLength(int maxBitLength){ this.maxBitLength = maxBitLength; }
        public void setPreprocessingStrategy(PreprocessingStrategy strategy){ this.preprocessingStrategy = strategy; }
        public void setEvaluationStrategy(EvaluationStrategy strategy){ this.evaluationStrategy = strategy; }
    }

}
