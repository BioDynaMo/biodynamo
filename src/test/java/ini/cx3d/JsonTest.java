package ini.cx3d;

import com.google.gson.JsonElement;
import com.google.gson.JsonParser;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotEquals;

/**
 * tests some properties related to json equality
 * this is related to persisting the simulation state: <code>SimStateSerializable</code>
 */
public class JsonTest {

    @Test
    public void test(){
        JsonParser parser = new JsonParser();
        JsonElement o1 = parser.parse("{  \"cellList\": {    \"1000002\": {          \"ID\": 2        },    \"1000001\": {        \"ID\": 1      }    }}");
        JsonElement o2 = parser.parse("{  \"cellList\": {    \"1000001\": {          \"ID\": 1        },    \"1000002\": {        \"ID\": 2      }    }}");
        assertEquals(o1, o2);
    }

    @Test
    public void testArrayInvariance(){
        JsonParser parser = new JsonParser();
        JsonElement o1 = parser.parse("{  \"cellList\": [0, 1, 2] }");
        JsonElement o2 = parser.parse("{  \"cellList\": [1, 2, 3] }");
        assertNotEquals(o1, o2);
    }

    @Test
    public void testObjectNonUniqueKey(){
        JsonParser parser = new JsonParser();
        JsonElement o1 = parser.parse("{  \"cellList\": { \"a\": 0, \"a\":1, \"a\":2} }");
        JsonElement o2 = parser.parse("{  \"cellList\": { \"a\": 2, \"a\":1, \"a\":0} }");
        assertNotEquals(o1, o2);
    }

    @Test
    public void testFloatsEqual() {
        String a = "{key: 2.3012345678899999e+02}";
        String b = "{key: 230.123456789}";

        JsonParser parser = new JsonParser();
        JsonElement o1 = parser.parse(a);
        JsonElement o2 = parser.parse(b);
        assertEquals(o1, o2);
    }

    @Test public void testFloatsNotEqual() {
        String a = "{key: 230.878191}";
        String b = "{key: 230.8781907032909}";

        JsonParser parser = new JsonParser();
        JsonElement o1 = parser.parse(a);
        JsonElement o2 = parser.parse(b);
        assertNotEquals(o1, o2);
    }
}
