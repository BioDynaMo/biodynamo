package ini.cx3d.swigTests;

import ini.cx3d.swig.ListT_Integer;
import org.junit.Test;

import java.util.AbstractSequentialList;
import java.util.List;

import static junit.framework.Assert.assertFalse;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * Test correctness of java AbstractSequentialList to std::list conversion
 */
public class TypemapStdListTest {
    @Test
    public void testList() {
        AbstractSequentialList list = new ListT_Integer();
        assertEquals(0, list.size());
        assertTrue(list.isEmpty());

        list.add(1);
        assertList(new int[]{1}, list);
        assertFalse(list.isEmpty());
        list.add(4);
        list.add(5);
        assertList(new int[]{1, 4, 5}, list);

        list.remove(1);
        assertList(new int[]{1, 5}, list);
        list.add(3);
        assertList(new int[]{1, 5, 3}, list);

        list.set(1, 9);
        assertList(new int[]{1, 9, 3}, list);
        list.remove((Integer) 3);
        list.remove(0);
        assertList(new int[]{9}, list);
    }

    private static void assertList(int[] expected, List<Integer> list) {
        assertEquals(expected.length, list.size());
        for (int i = 0; i < list.size(); i++) {
            assertEquals(expected[i], (int) list.get(i));
        }
    }
}
