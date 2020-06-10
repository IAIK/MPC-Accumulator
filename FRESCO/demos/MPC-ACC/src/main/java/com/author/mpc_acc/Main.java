package com.author.mpc_acc;

import com.author.mpc_acc.MPC_Acc;
import com.author.utils.CmdLineParser;

import org.apache.commons.cli.*;

import iaik.security.ec.provider.ECCelerate;
import iaik.security.provider.IAIK;

import java.util.List;
import java.util.ArrayList;
import java.math.BigInteger;
import iaik.security.ec.math.curve.ECPoint;
/**
 * A simple demo computing the MPC accumulator.
 *
 * @author ******
 */
public class Main{

  private static final int SIZE = 400;

  public static void main(String[] args) throws ParseException {

    IAIK.addAsProvider();
    ECCelerate.addAsProvider();

    System.out.println("MPC Accumulator Demo!");
    System.out.println("ECCelerate addon found: " + ECCelerate.isAddonAvailable());

    CmdLineParser.BuilderParams params = CmdLineParser.GetCmdLineParams(args);

    System.out.println("Verify standard:");
    long time = System.currentTimeMillis();
    MPC_Acc acc = new MPC_Acc(SIZE, params);
    int inverse_ops = 4;
    acc.prepareBatches(params.el + inverse_ops - 1, inverse_ops + 1);
    time = System.currentTimeMillis() - time;
    System.out.println("Preprocessing time (offline): " + time + "ms");
    acc.gen();
    BigInteger Q = acc.getQ();
    List<BigInteger> X = genList(Q, params.el);
    EvalResult res = acc.eval(X);

    BigInteger x = X.get(0);

    ECPoint accu = res.getAcc();
    Auxillery aux = res.getAuxillery();
    Witness wit = acc.witCreate(accu, aux, x);
    if (wit == null)
      return;

    System.out.println("Correct verify: " + acc.verify(accu, wit, x));
    System.out.println("Incorrect verify: " + acc.verify(accu, wit, X.get(1)));

    System.out.println("Verify add:");
    BigInteger new_el = new BigInteger("83702547212275448046482791686331419891000794825496896897047292718659199874066");
    EvalResult add_res = acc.add(accu, aux, new_el);
    if (add_res == null)
      return;

    Witness new_wit = acc.witUpdate(wit, add_res.getAuxillery(), new_el);
    System.out.println("Correct verify: " + acc.verify(add_res.getAcc(), new_wit, x));
    System.out.println("Incorrect verify: " + acc.verify(add_res.getAcc(), wit, x));

    System.out.println("Verify delete:");
    EvalResult del_res = acc.delete(accu, aux, X.get(2));
    if (del_res == null)
      return;

    Witness new_wit2 = acc.witUpdate(wit, del_res.getAuxillery(), X.get(2));
    System.out.println("Correct verify: " +  acc.verify(del_res.getAcc(), new_wit2, x));
    System.out.println("Incorrect verify: " +  acc.verify(del_res.getAcc(), wit, x));

    // System.out.println("Verify delete element of wit:");
    // EvalResult del_res2 =  acc.delete(accu, aux, X.get(0));
    // if (del_res2 == null)
    //   return;

    // Witness new_wit3 =  acc.witUpdate(wit, del_res2.getAuxillery(), X.get(0));
    // if (new_wit3 != null)
    //   System.out.println("Incorrect verify: " +  acc.verify(del_res2.getAcc(), new_wit3, x));



    // BigInteger s = new BigInteger("17191040908167902938755975263021743320738002366367610617835821881036644918754").add(new BigInteger("37282823970065119890333119733369559669903125033399073802841636231429348436679")).mod(Q);

    acc.close();

    System.out.println("-------------------------------------------------");
    System.out.println("Test keyless non-MPC:");

    // AccumulatorDemo acc1 = new AccumulatorDemo(SIZE); // ACC with key
    Accumulator acc1 = new Accumulator(SIZE, params.el + 1); // keyless ACC
    // BigInteger Q = acc1.getQ();
    // List<BigInteger> X = genList(Q, params.el);
    acc1.test(X);

    MerkleTree.test(X);
  }

  private static List<BigInteger> genList(BigInteger mod, int elements) {
    List<BigInteger> X = new ArrayList<>();
    BigInteger k = new BigInteger("59386881280326519533082599654198319504863636781144397045881137201452088394640");
    BigInteger d = new BigInteger("16814356272016849629746703154788324830890002181751649005020090390576047710164");
    for (int i = 0; i < elements; i++)
      X.add(k.multiply(new BigInteger(Integer.toString(i)).add(d).mod(mod)).mod(mod));
    return X;
  }
}
