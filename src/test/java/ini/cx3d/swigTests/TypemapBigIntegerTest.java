package ini.cx3d.swigTests;

import ini.cx3d.swig.BigIntegerConsumer;
import org.junit.Test;

import java.math.BigInteger;

import static junit.framework.TestCase.assertEquals;

/**
 * Test correctness of java.math.BigInteger to mpz_class (gmplib) conversion
 */
public class TypemapBigIntegerTest {

    private final String digits = "847496354513740338901262819252506596677203608618987774785298183069392403389107017859317" +
            "36857089202352580113742252818586689755604914";

    @Test
    public void testIn() {
        String digitsCPP = BigIntegerConsumer.getString(new BigInteger(digits));
        assertEquals(digits, digitsCPP);
    }

    @Test
    public void testOut(){
        BigInteger expected = new BigInteger(digits);
        BigInteger result = BigIntegerConsumer.createBigInt(digits);
        assertEquals(expected, result);
    }
}
