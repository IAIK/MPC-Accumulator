package dk.alexandra.fresco.suite.spdz2k;

import dk.alexandra.fresco.suite.spdz2k.datatypes.CompUInt128;
import dk.alexandra.fresco.suite.spdz2k.datatypes.CompUIntConverter128;
import dk.alexandra.fresco.suite.spdz2k.datatypes.UInt64;

/**
 * Protocol suite using {@link CompUInt128} as the underlying plain-value type.
 */
public class Spdz2kProtocolSuiteK64 extends Spdz2kProtocolSuite<UInt64, UInt64, CompUInt128> {

  public Spdz2kProtocolSuiteK64() {
    super(new CompUIntConverter128());
  }

}
