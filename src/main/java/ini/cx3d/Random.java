package ini.cx3d;

/**
 * Implementation of simple random number generator to get same sequence in Java and C++
 * -> important for reproducibility of simulation results
 */
public class Random {
    private static java.util.Random ecmRandom = new java.util.Random();
    private static java.util.Random matrixRandom = new java.util.Random();

    public static void setSeed(long seed){
        ecmRandom = new java.util.Random(seed);
        matrixRandom = new java.util.Random(seed);
    }

    public static double nextDoubleECM() {
        return ecmRandom.nextDouble();
    }

    public static double nextGaussianDoubleECM(double mean , double standardDeviation) {
        return mean + standardDeviation*ecmRandom.nextGaussian();
    }

    public static double nextDoubleMatrix() {
        return matrixRandom.nextDouble();
    }

}
