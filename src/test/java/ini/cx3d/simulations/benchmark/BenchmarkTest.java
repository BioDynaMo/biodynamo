package ini.cx3d.simulations.benchmark;

import ini.cx3d.simulations.ECM;
import org.junit.AssumptionViolatedException;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.Stopwatch;
import org.junit.runner.Description;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.net.URL;
import java.util.AbstractMap;
import java.util.LinkedList;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;

import static org.junit.Assert.*;

/**
 * Base class for benchmark tests
 */
public abstract class BenchmarkTest {
    private static final Logger logger = Logger.getLogger("");

    private static void logInfo(Description description, String status, long nanos) {
        String testName = description.getMethodName();
        logger.info(String.format("Test %s %s, spent %d microseconds",
                testName, status, TimeUnit.NANOSECONDS.toMicros(nanos)));
    }


    public BenchmarkTest(Class clazz) {}

    public abstract void simulation() throws Exception;

    @Test
    public void runTest() {
        try {
            configure();

            long start = System.currentTimeMillis();
            simulation();
            long stop = System.currentTimeMillis();
            System.out.println("TOTAL RUNTIME "+getClass().getSimpleName().replace("_", "") + " " + (stop - start));

        } catch (Exception e) {
            e.printStackTrace();
            fail(e.getMessage());
        }
    }

    private void configure() {
        ECM.headlessGui = true;// TestUtil.isRunningOnTravis();
        // run simulation (don't start in pause mode)
        ECM.getInstance().canRun.release();
        ECM.setRandomSeed(1L);
    }
}
