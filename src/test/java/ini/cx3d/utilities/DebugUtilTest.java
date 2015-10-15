package ini.cx3d.utilities;

import org.junit.Test;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
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
        String expected = "DBG L#1 test1 args: {Hello World, } innerState: toString of TestImpl\n" +
                "DBG L#2 test1 return Hello World innerState: toString of TestImpl\n" +
                "DBG L#3 test2 args: {42, } innerState: toString of TestImpl\n" +
                "DBG L#4 test2 return 42 innerState: toString of TestImpl\n" +
                "DBG L#5 test3 args: {L, } innerState: toString of TestImpl\n" +
                "DBG L#6 test3 return L innerState: toString of TestImpl\n" +
                "DBG L#7 test4 args: {true, } innerState: toString of TestImpl\n" +
                "DBG L#8 test4 return true innerState: toString of TestImpl\n" +
                "DBG L#9 test5 args: {6.90000, } innerState: toString of TestImpl\n" +
                "DBG L#10 test5 return 6.90000 innerState: toString of TestImpl\n" +
                "DBG L#11 test6 args: {{1.20000, 8.60000, }, } innerState: toString of TestImpl\n" +
                "DBG L#12 test6 return {1.20000, 8.60000, } innerState: toString of TestImpl\n" +
                "DBG L#13 test7 args: {{{1.20000, 8.60000, }, {5.70000, 11.30000, }, }, } innerState: toString of TestImpl\n" +
                "DBG L#14 test7 return {{1.20000, 8.60000, }, {5.70000, 11.30000, }, } innerState: toString of TestImpl\n" +
                "DBG L#15 test8 args: {{SomeOtherClass8, SomeOtherClass4, }, } innerState: toString of TestImpl\n" +
                "DBG L#16 test8 return {SomeOtherClass8, SomeOtherClass4, } innerState: toString of TestImpl\n" +
                "DBG L#17 test9 args: {{SomeOtherClass2, SomeOtherClass5, }, } innerState: toString of TestImpl\n" +
                "DBG L#18 test9 return {SomeOtherClass2, SomeOtherClass5, } innerState: toString of TestImpl\n" +
                "DBG L#19 callTest1Inside args:  innerState: toString of TestImpl\n" +
                "DBG L#20 callTest1Inside return  innerState: toString of TestImpl\n";
        assertEquals(expected, result);
    }

    @Test
    public void testEquals(){
        TestImpl test = new TestImpl();
        TestInterface o1 = DebugUtil.createDebugLoggingProxy(test, new Class[]{TestInterface.class});
        TestInterface o2 = DebugUtil.createDebugLoggingProxy(test, new Class[]{TestInterface.class});
        assertTrue(o1.equals(o2));
    }
}
