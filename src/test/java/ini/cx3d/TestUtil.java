package ini.cx3d;

import java.io.*;
import java.net.URI;
import java.util.*;

/**
 * This class contains static functions that are used during unit testing
 */
public class TestUtil {

    public static String executeCmd(String cmd) throws IOException {
        Process process = Runtime.getRuntime().exec(cmd);
        BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));

        StringBuilder sb = new StringBuilder();
        String line;
        while ((line = reader.readLine()) != null) {
            sb.append(line);
        }
        return sb.toString();
    }

    public static LinkedList<Map.Entry<String, Double>> parseExecutionTimesCsv(String filename) throws IOException {
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new FileReader(filename));
            LinkedList<Map.Entry<String, Double>> entries = new LinkedList<Map.Entry<String, Double>>();
            String line;
            while ((line = reader.readLine()) != null) {
                String[] fields = line.split(";");
                entries.add(new AbstractMap.SimpleEntry<String, Double>(fields[0], Double.parseDouble(fields[1])));
            }
            return entries;
        } finally {
            if (reader != null) {
                reader.close();
            }
        }
    }

    public static void persistExecutionTimesCsv(String filePath, LinkedList<Map.Entry<String, Double>> executionTimes) throws IOException {
        PrintWriter out = null;
        try{
            out = new PrintWriter(new BufferedWriter(new FileWriter(filePath, false)));
            for(Map.Entry<String, Double> entry : executionTimes) {
                out.println(entry.getKey() + ";" + entry.getValue());
            }
        } finally {
            if (out != null) {
                out.close();
            }
        }
    }

    public static void persistJson(String filePath, String json) throws IOException {
        PrintWriter out = null;
        try{
            out = new PrintWriter(new BufferedWriter(new FileWriter(filePath, false)));
            out.println(json);
        } finally {
            if (out != null) {
                out.close();
            }
        }
    }

    /**
     * Reads the relative path to the resource directory from the <code>testResourcePath</code> file located in
     * <code>src/test/resources</code>
     * http://stackoverflow.com/questions/21567497/how-to-output-text-to-a-file-in-resource-folder-maven
     * @return the relative path to the <code>resources</code> in the file system, or
     *         <code>null</code> if there was an error
     */
    public static String getResourcePath() {
        try {
            String resourcePathFile = System.class.getResource("/testResourcePath").getFile();
            String resourcePath = new BufferedReader(new FileReader(resourcePathFile)).readLine();
            URI rootURI = new File("").toURI();
            URI resourceURI = new File(resourcePath).toURI();
            URI relativeResourceURI = rootURI.relativize(resourceURI);
            return relativeResourceURI.getPath();
        } catch (Exception e) {
            return null;
        }
    }
}
