package dk.alexandra.fresco.suite.spdz;

import dk.alexandra.fresco.lib.statistics.CreditRaterTest;
import dk.alexandra.fresco.suite.spdz.configuration.PreprocessingStrategy;
import org.junit.Before;
import org.junit.Test;


/**
 * Tests for the CreditRater.
 * 
 */
public class TestSpdzCreditRater2Parties extends AbstractSpdzTest {

  private int[] values;
  private int[][] intervals, scores;

  @Before
  public void setTestValues() {
    values = new int[] {101, 251};
    intervals = new int[2][];
    intervals[0] = new int[] {100, 500};
    intervals[1] = new int[] {250, 500};

    scores = new int[2][];
    scores[0] = new int[] {10, 13, 15};
    scores[1] = new int[] {10, 13, 15};
  }


  @Test
  public void test_CreditRater_alternate_values() {
    values = new int[] {101};
    intervals = new int[1][];
    intervals[0] = new int[] {100, 500};
    scores = new int[1][];
    scores[0] = new int[] {10, 13, 15};
    runTest(new CreditRaterTest.TestCreditRater<>(values, intervals, scores),
        PreprocessingStrategy.DUMMY, 2);

    values = new int[] {10};
    runTest(new CreditRaterTest.TestCreditRater<>(values, intervals, scores),
        PreprocessingStrategy.DUMMY, 2);
    values = new int[] {100};
    runTest(new CreditRaterTest.TestCreditRater<>(values, intervals, scores),
        PreprocessingStrategy.DUMMY, 2);
    values = new int[] {500};
    runTest(new CreditRaterTest.TestCreditRater<>(values, intervals, scores),
        PreprocessingStrategy.DUMMY, 2);
    values = new int[] {1000};
    runTest(new CreditRaterTest.TestCreditRater<>(values, intervals, scores),
        PreprocessingStrategy.DUMMY, 2);

    intervals[0] = new int[] {1000};
    scores[0] = new int[] {10, 20};
    runTest(new CreditRaterTest.TestCreditRater<>(values, intervals, scores),
        PreprocessingStrategy.DUMMY, 2);

    values = new int[] {101, 440, 442};
    intervals = new int[3][];
    intervals[0] = new int[] {100, 500};
    intervals[1] = new int[] {100, 200};
    intervals[2] = new int[] {50, 800};
    scores = new int[3][];
    scores[0] = new int[] {10, 13, 15};
    scores[1] = new int[] {1, 3, 21};
    scores[2] = new int[] {16, 15, 11};
    runTest(new CreditRaterTest.TestCreditRater<>(values, intervals, scores),
        PreprocessingStrategy.DUMMY, 2);

  }


  @Test
  public void test_CreditRater_2() {
    runTest(new CreditRaterTest.TestCreditRater<>(values, intervals, scores),
        PreprocessingStrategy.DUMMY, 2);
  }

}
