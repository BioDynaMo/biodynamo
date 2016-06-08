package ini.cx3d;

/**
 * Implementation of simple random number generator to get same sequence in Java and C++
 * -> important for reproducibility of simulation results
 */
public class Random {

    private static java.util.Random random = new java.util.Random();

    public static void setSeed(long seed){
        random = new java.util.Random(seed);
    }

    public static double nextDoubleECM() {
        return random.nextDouble();
    }

    public static double nextGaussianDoubleECM(double mean , double standardDeviation) {
        return mean + standardDeviation* random.nextGaussian();
    }

    public static double nextDoubleMatrix() {
        return random.nextDouble();
    }
}
