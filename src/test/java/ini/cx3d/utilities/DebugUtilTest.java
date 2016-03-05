package ini.cx3d.utilities;

import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.lang.reflect.Proxy;
import java.util.AbstractSequentialList;

import java.util.LinkedList;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * Test correctness of DebugUtil class
 */
public class DebugUtilTest {
    interface TestInterface{

        String test1(String s);
        int test2(int s);
        char test3(char c);
        boolean test4(boolean b);
        double test5(double d);
        double[] test6(double d[]);
        double[][] test7(double d[][]);
        SomeOtherClass[] test8(SomeOtherClass arg[]);
        AbstractSequentialList<SomeOtherClass> test9(AbstractSequentialList<SomeOtherClass> arg);

        void callTest1Inside();
    }

    static class TestImpl implements TestInterface{

        public boolean equals(Object other) {
            if (other instanceof Proxy) {
                return other.equals(this);
            } else if (!(other instanceof TestImpl)) {
                return false;
            }
            return other == this;
        }

        @Override
        public String toString() {
            return "toString of TestImpl";
        }

        @Override
        public String test1(String s) {
            return s;
        }

        @Override
        public int test2(int s) {
            return s;
        }

        @Override
        public char test3(char c) {
            return c;
        }

        @Override
        public boolean test4(boolean b) {
            return b;
        }

        @Override
        public double test5(double d) {
            return d;
        }

        @Override
        public double[] test6(double[] d) {
            return d;
        }

        @Override
        public double[][] test7(double[][] d) {
            return d;
        }

        @Override
        public SomeOtherClass[] test8(SomeOtherClass[] arg) {
            return arg;
        }

        @Override
        public AbstractSequentialList<SomeOtherClass> test9(AbstractSequentialList<SomeOtherClass> arg) {
            return arg;
        }

        @Override
        public void callTest1Inside() {
            test1("cern");
        }
    }

    static class SomeOtherClass {
        private int id;

        public SomeOtherClass(int id) {
            this.id = id;
        }

        public String toString() {
            return "SomeOtherClass" + id;
        }
    }

    @Test
    public void test() {
        final ByteArrayOutputStream myOut = new ByteArrayOutputStream();
        System.setOut(new PrintStream(myOut));

        TestInterface test = DebugUtil.createDebugLoggingProxy(new TestImpl(), new Class[]{TestInterface.class});
        test.test1("Hello World");
        test.test2(42);
        test.test3('L');
        test.test4(true);
        test.test5(6.9);
        test.test6(new double[]{1.2, 8.6});
        test.test7(new double[][]{{1.2, 8.6}, {5.7, 11.3}});
        test.test8(new SomeOtherClass[]{new SomeOtherClass(8), new SomeOtherClass(4)});
        AbstractSequentialList<SomeOtherClass> list = new LinkedList<SomeOtherClass>();
        list.add(new SomeOtherClass(2));
        list.add(new SomeOtherClass(5));
        test.test9(list);
        test.callTest1Inside();

        final String result = myOut.toString();
        String expected = "DBG test1 args: {Hello World, } innerState: toString of TestImpl\n" +
                "DBG test1 return Hello World innerState: toString of TestImpl\n" +
                "DBG test2 args: {42, } innerState: toString of TestImpl\n" +
                "DBG test2 return 42 innerState: toString of TestImpl\n" +
                "DBG test3 args: {L, } innerState: toString of TestImpl\n" +
                "DBG test3 return L innerState: toString of TestImpl\n" +
                "DBG test4 args: {true, } innerState: toString of TestImpl\n" +
                "DBG test4 return true innerState: toString of TestImpl\n" +
                "DBG test5 args: {401b99999999999a, } innerState: toString of TestImpl\n" +
                "DBG test5 return 401b99999999999a innerState: toString of TestImpl\n" +
                "DBG test6 args: {{3ff3333333333333, 4021333333333333, }, } innerState: toString of TestImpl\n" +
                "DBG test6 return {3ff3333333333333, 4021333333333333, } innerState: toString of TestImpl\n" +
                "DBG test7 args: {{{3ff3333333333333, 4021333333333333, }, {4016cccccccccccd, 402699999999999a, }, }, } innerState: toString of TestImpl\n" +
                "DBG test7 return {{3ff3333333333333, 4021333333333333, }, {4016cccccccccccd, 402699999999999a, }, } innerState: toString of TestImpl\n" +
                "DBG test8 args: {{SomeOtherClass8, SomeOtherClass4, }, } innerState: toString of TestImpl\n" +
                "DBG test8 return {SomeOtherClass8, SomeOtherClass4, } innerState: toString of TestImpl\n" +
                "DBG test9 args: {{SomeOtherClass2, SomeOtherClass5, }, } innerState: toString of TestImpl\n" +
                "DBG test9 return {SomeOtherClass2, SomeOtherClass5, } innerState: toString of TestImpl\n" +
                "DBG callTest1Inside args:  innerState: toString of TestImpl\n" +
                "DBG callTest1Inside return null innerState: toString of TestImpl\n";
        assertEquals(expected, result);
    }

    @Test
    public void testEquals(){
        TestImpl test = new TestImpl();
        TestInterface o1 = DebugUtil.createDebugLoggingProxy(test, new Class[]{TestInterface.class});
        TestInterface o2 = DebugUtil.createDebugLoggingProxy(test, new Class[]{TestInterface.class});
        assertTrue(o1.equals(o2));
        assertTrue(o1.equals(test));
        assertTrue(test.equals(o1));
    }
}
