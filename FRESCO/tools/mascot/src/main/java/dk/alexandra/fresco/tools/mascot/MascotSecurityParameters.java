package dk.alexandra.fresco.tools.mascot;

/**
 * The security parameters used throughout the MASCOT protocol MASCOT protocol (<a
 * href="https://eprint.iacr.org/2016/505.pdf">https://eprint.iacr.org/2016/505.pdf</a>).
 */
public class MascotSecurityParameters {

  private final int lambdaSecurityParam;
  private final int prgSeedLength;
  private final int numCandidatesPerTriple;

  /**
   * Creates new {@link MascotSecurityParameters}.
   *
   * @param lambdaSecurityParam OT security parameter num bits (lambda in Mascot paper)
   * @param prgSeedLength bit length of seed used to underlying prg
   * @param numCandidatesPerTriple number of factors that go into Sacrifice step of Protocol 4
   *     (tau in Mascot paper) For each triple we generate, we will generate and
   *     numCandidatesPerTriple - 1 triples for a single right factor and sacrifice these to
   *     authenticate the triple.
   */
  public MascotSecurityParameters(
      int lambdaSecurityParam, int prgSeedLength, int numCandidatesPerTriple) {
    this.lambdaSecurityParam = lambdaSecurityParam;
    this.prgSeedLength = prgSeedLength;
    this.numCandidatesPerTriple = numCandidatesPerTriple;
  }

  /**
   * Creates new {@link MascotSecurityParameters} with realistic parameters (based on paper
   * recommendations).
   */
  public MascotSecurityParameters() {
    this(64, 256, 3);
  }

  /**
   * Gets OT security parameter num bits (lambda in Mascot paper).
   *
   * @return lambda security parameter
   */
  public int getLambdaSecurityParam() {
    return lambdaSecurityParam;
  }

  /**
   * Gets bit length of seed used to underlying prg.
   *
   * @return prg seed bit length
   */
  public int getPrgSeedLength() {
    return prgSeedLength;
  }

  /**
   * Gets number of factors that go into sacrifice step of Protocol 4 (tau in Mascot paper). <p> For
   * each triple we generate, we will generate and numCandidatesPerTriple - 1 triples for a single
   * right factor and sacrifice these to authenticate the triple.</p>
   *
   * @return number of factors
   */
  public int getNumCandidatesPerTriple() {
    return numCandidatesPerTriple;
  }
}
