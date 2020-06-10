package com.author.mpc_acc;

import com.author.utils.CmdLineParser;
import com.author.utils.NetworkManager;
import dk.alexandra.fresco.logging.NetworkLoggingDecorator;
import dk.alexandra.fresco.suite.spdz.storage.SpdzMascotDataSupplier;

import java.math.BigInteger;
import java.security.SecureRandom;
import org.apache.commons.cli.*;

import iaik.security.ec.common.SecurityStrength;


import iaik.security.ec.math.curve.Pairing;
import iaik.security.ec.math.curve.PairingTypes;
import iaik.security.ec.math.field.GenericFieldElement;
import iaik.security.ec.math.curve.AtePairingOverBarretoNaehrigCurveFactory;
import iaik.security.ec.math.curve.ECPoint;
import iaik.security.ec.math.curve.EllipticCurve;
import java.util.List;
import java.util.ArrayList;


/**
 * A simple demo computing the MPC accumulator.
 *
 * @author ******
 */
public class MPC_Acc {

  private final Pairing PAIRING;
  private final EllipticCurve CURVE1;
  private final EllipticCurve CURVE2;
  private final BigInteger Q;
  private final ECPoint G1;
  private final ECPoint G2;
  private int size;
  private ECPoint pk2;

  private MPCParams mpc_params;


  private BigInteger sk_share;
  private SecureRandom random;



  public BigInteger getRandomScalar() {
    return new BigInteger(size - 1, random);
  }

  /**
   * Main method for mpc_acc.
   */
  public MPC_Acc(int size, CmdLineParser.BuilderParams params)throws ParseException {

    // init ECC
    this.PAIRING = AtePairingOverBarretoNaehrigCurveFactory
    .getPairing(PairingTypes.TYPE_3, size);
    this.size = size;
    this.CURVE1 = PAIRING.getGroup1();
    this.CURVE2 = PAIRING.getGroup2();
    this.G1 = CURVE1.getGenerator();
    this.G2 = CURVE2.getGenerator();
    this.Q = CURVE1.getOrder();
    this.random = SecurityStrength.getSecureRandom(SecurityStrength
    .getSecurityStrength(CURVE1.getField().getFieldSize()));
    // init fresco
    params.setMaxBitLength(Q.bitLength());

    this.mpc_params = new MPCParamsBuilder(params.logging)
                .withBatchEvalStrat(params.evaluationStrategy)
                .withID(params.id)
                .withNetwork(NetworkManager.getPartyMap(params.partyList,params.myParty), params.myParty)
                .withResourcePool(params.preprocessingStrategy, Q)
                .withSpdzLength(params.maxBitLength)
                .build();

  }

  public void gen() {
    // init share
    sk_share = getRandomScalar();

    // if (mpc_params.myID == 1)
    //   sk_share = new BigInteger("17191040908167902938755975263021743320738002366367610617835821881036644918754");
    // else
    //   sk_share = new BigInteger("37282823970065119890333119733369559669903125033399073802841636231429348436679");

    MPC_Gen gen = new MPC_Gen(sk_share, G2);
    pk2 = mpc_params.getMySce().runApplication(gen, mpc_params.getMyPool(), mpc_params.getMyNetwork());
  }

  public EvalResult eval(List<BigInteger> X) {
    // init share
    MPC_Eval eval = new MPC_Eval(sk_share, G1, X);
    ECPoint acc = mpc_params.getMySce().runApplication(eval, mpc_params.getMyPool(), mpc_params.getMyNetwork());

    List<BigInteger> list = new ArrayList<BigInteger>(X);

    return new EvalResult(acc, list);
  }

  public Witness witCreate(ECPoint acc, Auxillery aux, BigInteger x) {
    if (!aux.getList().contains(x))
      return null;

    MPC_WitCreate wit = new MPC_WitCreate(sk_share, x, acc);
    ECPoint witness = mpc_params.getMySce().runApplication(wit, mpc_params.getMyPool(), mpc_params.getMyNetwork());
    return new Witness(witness, x);
  }

  public Boolean verify(ECPoint acc, Witness wit, BigInteger x) {
    GenericFieldElement p1 = PAIRING.pair(acc.clone(), G2);
    ECPoint p2_EP = G2.clone().multiplyPoint(x);
    p2_EP.addPoint(pk2);
    GenericFieldElement p2 = PAIRING.pair(wit.getWitness().clone(), p2_EP);
    return p1.equals(p2);
  }

  public EvalResult add(ECPoint acc, Auxillery aux, BigInteger x) {
    List<BigInteger> list = aux.getList();
    if (list.contains(x))
      return null;

    ECPoint r1 = acc.clone().multiplyPoint(x);
    MPC_Add add = new MPC_Add(sk_share, acc);
    ECPoint r2 = mpc_params.getMySce().runApplication(add, mpc_params.getMyPool(), mpc_params.getMyNetwork());
    List<BigInteger> list_new = new ArrayList<BigInteger>(list);
    list_new.add(x);
    return new EvalResult(r1.addPoint(r2), list_new, Auxillery.UPDATE.ADD);
  }

  public EvalResult delete(ECPoint acc, Auxillery aux, BigInteger x) {
    List<BigInteger> list = aux.getList();
    if (!list.contains(x))
      return null;

    MPC_Del del = new MPC_Del(sk_share, x, acc);
    ECPoint new_acc = mpc_params.getMySce().runApplication(del, mpc_params.getMyPool(), mpc_params.getMyNetwork());

    List<BigInteger> list_new = new ArrayList<BigInteger>(list);
    list_new.remove(x);
    return new EvalResult(new_acc, list_new, Auxillery.UPDATE.DELETE);
  }

  public Witness witUpdate(Witness wit, Auxillery aux, BigInteger x) {
    if (!aux.getList().contains(wit.getElement()))
      return null;

    ECPoint witness = wit.getWitness();
    Witness wit_new = null;
    switch (aux.getUpdate()) {
      case ADD:
        MPC_WitUpdateAdd witUpdateAdd = new MPC_WitUpdateAdd(sk_share, witness);
        ECPoint share_add = mpc_params.getMySce().runApplication(witUpdateAdd, mpc_params.getMyPool(), mpc_params.getMyNetwork());
        wit_new = new Witness(share_add.addPoint(witness.clone().multiplyPoint(x)), wit.getElement());
        break;
      case DELETE:
        MPC_WitUpdateDel witUpdateDel = new MPC_WitUpdateDel(sk_share, x, witness);
        ECPoint share_del = mpc_params.getMySce().runApplication(witUpdateDel, mpc_params.getMyPool(), mpc_params.getMyNetwork());
        wit_new = new Witness(share_del, wit.getElement());
        break;
      default:
        return null;
    }

    aux.clearUpdate();
    return wit_new;
  }

  public void close() {
    if(mpc_params.isLogging()){
      Long total = mpc_params.getMyNetworkManager().getLoggedValues().get(NetworkLoggingDecorator.NETWORK_TOTAL_BYTES);
      mpc_params.log(NetworkLoggingDecorator.NETWORK_TOTAL_BYTES + " " + total);
    }
    mpc_params.shutdown();
  }

  public void prepareBatches(int mul, int random) {
    SpdzMascotDataSupplier supplier = (SpdzMascotDataSupplier)mpc_params.getMyPool().getDataSupplier();
    MPC_TripleDummy.produceTriples(supplier, mul);
    MPC_TripleDummy.produceRandomFieldElements(supplier, random);
  }

  public BigInteger getQ() {
    return Q;
  }

}
