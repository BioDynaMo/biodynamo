package ini.cx3d;

import ini.cx3d.utilities.Matrix;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * Tests random number generation
 */
public class RandomTest {

    @Test
    public void test(){
        long start = System.currentTimeMillis();
        Random.setSeed(1L);
        assertEquals(0.7308781907032909, Random.nextDoubleECM(), 1e-11);
        assertEquals(0.41008081149220166, Random.nextDoubleMatrix(), 1e-11);
        for(int i = 0; i < 10000000; i++) Random.nextDoubleECM();
        assertEquals(0.06991942722947553, Random.nextDoubleECM(), 1e-11);
        assertEquals(-0.3648863101313806, Random.nextGaussianDoubleECM(0, 1), 1e-11);
        for(int i = 0; i < 100000; i++) Random.nextGaussianDoubleECM(2, 3);
        assertEquals(0.4512373907254288, Random.nextGaussianDoubleECM(3, 4), 1e-11);
        Random.setSeed(99L);
        assertEquals(0.7224575488195071, Random.nextDoubleMatrix(), 1e-11);
        assertEquals(0.9246892004845302, Random.nextGaussianDoubleECM(3, 4), 1e-11);
        double[] noise = Matrix.randomNoise(100, 3);
        assertEquals(73.40813456108145, noise[0], 1e-11);
        assertEquals(53.51089851859078, noise[1], 1e-11);
        assertEquals(-48.27938452667355, noise[2], 1e-11);
        System.out.println(System.currentTimeMillis() - start);
    }
}
