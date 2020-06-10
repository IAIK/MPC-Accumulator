package dk.alexandra.fresco.tools.mascot.maccheck;

import dk.alexandra.fresco.framework.MaliciousException;
import dk.alexandra.fresco.framework.builder.numeric.Addable;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.framework.network.Network;
import dk.alexandra.fresco.tools.mascot.MascotResourcePool;
import dk.alexandra.fresco.tools.mascot.commit.CommitmentBasedInput;
import java.math.BigInteger;
import java.util.List;

/**
 * Actively-secure protocol for performing a MAC check on a public field element e. <br>
 * Each party p_i holds a share of the MAC m_i and a share of the MAC key alpha_i. <br>
 * This protocol validates that e * (alpha_1 + ... + alpha_n) = m_1 + ... + m_n.
 */
public class MacCheck extends CommitmentBasedInput<FieldElement> {

  /**
   * Constructs new mac checker.
   */
  public MacCheck(MascotResourcePool resourcePool, Network network) {
    super(resourcePool, network, resourcePool.getFieldDefinition());
  }

  /**
   * Runs mac-check on open value. <br>
   * Conceptually, checks (macShare0 + ... + macShareN) = (open) * (keyShare0 + ... + keyShareN)
   * 
   * @param opened the opened element to validate
   * @param macKeyShare this party's share of the mac key
   * @param macShare this party's share of the mac
   * @throws MaliciousException if mac-check fails
   */
  public void check(
      FieldElement opened, FieldElement macKeyShare, FieldElement macShare) {
    // we will check that all sigmas together add up to 0
    FieldElement sigma = macShare.subtract(opened.multiply(macKeyShare));

    // commit to own value
    List<FieldElement> sigmas = allCommit(sigma);
    // add up all sigmas
    FieldElement sigmaSum = Addable.sum(sigmas);

    BigInteger outputSum = getResourcePool().getFieldDefinition().convertToUnsigned(sigmaSum);
    // sum of sigmas must be 0
    if (outputSum.signum() != 0) {
      throw new MaliciousException("Malicious mac forging detected");
    }
  }

}
