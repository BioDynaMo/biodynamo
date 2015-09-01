package ini.cx3d.swig;

import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class StringBuilderTest {

    @Test
    public void testStringBuilder() {
        ini.cx3d.swig.StringBuilder sb = new ini.cx3d.swig.StringBuilder();
        sb.append("Hello ").append("World?");
        assertEquals("Hello World?", sb.str());
        sb.overwriteLastCharOnNextAppend();
        assertEquals("Hello World?", sb.str());
        sb.append("!");
        assertEquals("Hello World!", sb.str());
    }
}
