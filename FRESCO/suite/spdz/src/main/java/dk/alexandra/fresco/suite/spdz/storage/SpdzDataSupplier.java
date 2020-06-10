package dk.alexandra.fresco.suite.spdz.storage;

import dk.alexandra.fresco.framework.builder.numeric.field.FieldDefinition;
import dk.alexandra.fresco.framework.builder.numeric.field.FieldElement;
import dk.alexandra.fresco.lib.compare.zerotest.ZeroTestBruteforce;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzInputMask;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzSInt;
import dk.alexandra.fresco.suite.spdz.datatypes.SpdzTriple;

public interface SpdzDataSupplier {

  /**
   * Supplies the next triple.
   *
   * @return the next new triple
   */
  SpdzTriple getNextTriple();

  /**
   * Supplies the next exponentiation pipe. <p>An exponentiation pipe is a list of numbers in the
   * following format: r^{-1}, r, r^{2}, r^{3}, ..., r^{l}, where r is a random element, l is the
   * length of exponentiation pipe, and all exponentiations are mod the prime we are working with.
   * </p><p>Exponetiation pipes are used, for instance, in {@link ZeroTestBruteforce}.</p>
   *
   * @return the next new exp pipe
   */
  SpdzSInt[] getNextExpPipe();

  /**
   * Supplies the next inputmask for a given input player.
   *
   * @param towardPlayerId the id of the input player
   * @return the appropriate input mask
   */
  SpdzInputMask getNextInputMask(int towardPlayerId);

  /**
   * Supplies the next bit (i.e. a SpdzSInt representing a value in {0, 1}).
   *
   * @return the next new bit
   */
  SpdzSInt getNextBit();

  /**
   * The field definition used for this instance of SPDZ.
   *
   * @return a field definition
   */
  FieldDefinition getFieldDefinition();

  /**
   * Returns the Players share of the Shared Secret Key (alpha). This is never to be send to anyone
   * else!
   *
   * @return a share of the key
   */
  FieldElement getSecretSharedKey();

  /**
   * Returns the next random field element.
   *
   * @return A SpdzSInt representing a random secret shared field element.
   */
  SpdzSInt getNextRandomFieldElement();
}
