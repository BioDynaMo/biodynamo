package ini.cx3d;

import com.google.gson.JsonElement;
import com.google.gson.JsonParser;
import com.google.gson.stream.JsonReader;
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
 * This is Base class to for a simulation test. Only the simulation code has to be written in the subclass
 * <code>BaseSimulationTest</code> takes care of persisting the simulation state and comparing it to the expected value
 * If the code for the simulation state changes or if new tests are added one wants to update the reference files for
 * all simulation. This can easily be done by setting the maven property <code>updateSimStateReferenceFiles</code>
 * to <code>true</code>
 * pom.xml: project -> properties -> updateSimStateReferenceFiles
 * In this case no assertions are made
 *
 * Furthermore it keeps track of the performance and ensures that the code will run faster on future commits.
 * At the moment it just measures execution time. This means that all tests should run on the same machine with the
 * same load profile. Performance tests can be deactivated using the static variable <code>assertPerformance</code>
 *
 * Each SimulationTest has two associated files in the test resource folder
 *  Classname.json      contains the reference simulation state in json format
 *  Classname.csv       contains the runtimes for each git commit in the follwing format: git_commit_ic;runtime_in_ms
 *
 */
public abstract class BaseSimulationTest {
    private static final Logger logger = Logger.getLogger("");

    private static void logInfo(Description description, String status, long nanos) {
        String testName = description.getMethodName();
        logger.info(String.format("Test %s %s, spent %d microseconds",
                testName, status, TimeUnit.NANOSECONDS.toMicros(nanos)));
    }

    /**
     * do not run performance tests on travis-ci - simple time measuring will not work
     * cannot ensure that tests are run on same machine under same load
     */
    private static boolean assertPerformance = !TestUtil.isRunningOnTravis();

    private String execTimesFileName;
    private LinkedList<Map.Entry<String, Double>> executionTimes;

    /**
     * last git commit id
     */
    private String lastCommit;

    /**
     * value in milliseconds
     */
    private double maxAllowedRuntime = Double.MAX_VALUE;

    @Rule
    public Stopwatch stopwatch = new Stopwatch() {

        @Override
        protected void succeeded(long nanos, Description description) {
            logInfo(description, "succeeded", nanos);
            String execTimesFilePath = TestUtil.getResourcePath() + execTimesFileName;
            logger.info(execTimesFilePath);
            Map.Entry<String, Double> entry = executionTimes.isEmpty() ? null : executionTimes.getLast();
            if (entry != null && entry.getKey().equals(lastCommit)) {
                executionTimes.removeLast();
            }
            executionTimes.add(new AbstractMap.SimpleEntry<String, Double>(lastCommit, nanos / 1000000d));
            try {
                TestUtil.persistExecutionTimesCsv(execTimesFilePath, executionTimes);
            } catch (IOException e) {
                logger.info(e.getMessage());
            }
        }

        @Override
        protected void failed(long nanos, Throwable e, Description description) {
            logInfo(description, "failed", nanos);
        }

        @Override
        protected void skipped(long nanos, AssumptionViolatedException e, Description description) {
            logInfo(description, "skipped", nanos);
        }

        @Override
        protected void finished(long nanos, Description description) {
            logInfo(description, "finished", nanos);
        }
    };

    public BaseSimulationTest(Class clazz) {
        //apply and extract execution time of last commit
        try {
            lastCommit = TestUtil.executeCmd("git rev-parse HEAD");

            execTimesFileName = getClass().getSimpleName() + ".csv";
            URL resource = getClass().getResource("/" + execTimesFileName);
            if (resource == null) {
                executionTimes = new LinkedList<Map.Entry<String, Double>>();
            } else {
                String execTimesFilePath = resource.getFile();
                logger.info(execTimesFilePath);
                executionTimes = TestUtil.parseExecutionTimesCsv(execTimesFilePath);
                getMaxAllowedRuntime(lastCommit);
            }

            logger.info("lastCommit: " + lastCommit);
            logger.info("maxRuntime: " + maxAllowedRuntime);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public abstract void simulation() throws Exception;

    @Test
    public void runTest() {
        try {
            configure();
            simulation();
            assertSimulationState();
        } catch (Exception e) {
            e.printStackTrace();
            fail(e.getMessage());
        }
        if (assertPerformance) {
            assertTrue(stopwatch.runtime(TimeUnit.MILLISECONDS) < maxAllowedRuntime);
        }
    }

    private void configure() {
        ECM.headlessGui = TestUtil.isRunningOnTravis();
        // run simulation (don't start in pause mode)
        ECM.getInstance().canRun.release();
        ECM.setRandomSeed(1L);
    }

    private void assertSimulationState() throws IOException {
        String jsonString = ECM.getInstance().simStateToJson(new StringBuilder()).toString();
        JsonElement jsonTree = new JsonParser().parse(jsonString);

        String refFileName = getClass().getSimpleName() + ".json";
        if (!"true".equals(System.getProperty("updateSimStateReferenceFiles"))) {
            //parse reference
            String refFilePath = getClass().getResource("/" + refFileName).getFile();
            JsonReader reader = new JsonReader(new BufferedReader(new FileReader(refFilePath)));
            JsonElement reference = new JsonParser().parse(reader);

            //pretty print json
//            Gson gson = new GsonBuilder().setPrettyPrinting().create();
//            System.out.println(gson.toJson(jsonTree));
//            System.out.println(gson.toJson(reference));

//            assertEquals(reference.toString(),jsonString);
            assertTrue(jsonTree.equals(reference));
        } else {
            TestUtil.persistJson(TestUtil.getResourcePath()+refFileName, jsonString);
        }
    }

    /**
     * determines the maximum allowed runtime for a given simulation
     * @param lastCommit
     */
    private void getMaxAllowedRuntime(String lastCommit) {
        int listSize = executionTimes.size();
        if (listSize == 0) {
            maxAllowedRuntime = Double.MAX_VALUE;
            return;
        }

        Map.Entry<String, Double> entry = executionTimes.getLast();
        if (!entry.getKey().equals(lastCommit)) {
            entry = executionTimes.getLast();
            maxAllowedRuntime = entry.getValue()+30;
        } else if (listSize > 1) {
            entry = executionTimes.get(listSize - 2);
            maxAllowedRuntime = entry.getValue()+30;
        } else {
            maxAllowedRuntime = Double.MAX_VALUE;
        }
    }
}