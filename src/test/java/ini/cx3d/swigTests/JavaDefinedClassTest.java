/*
 Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
 Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
 Sabina Pfister, Adrian M. Whatley & Lukas Breitwieser.

 This file is part of CX3D.

 CX3D is free software: you can redistribute it and/or modify
 it under the terms of the GNU General virtual License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 CX3D is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General virtual License for more details.

 You should have received a copy of the GNU General virtual License
 along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
*/

package ini.cx3d.swigTests;

import ini.cx3d.swig.*;
import org.junit.Test;

import java.util.AbstractSequentialList;
import java.util.LinkedList;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * Tests correctness of the java defined class macros
 * They are used for classes that are needed on the C++ side, but still implemented in Java
 */
public class JavaDefinedClassTest {
    public static class NotPortedJavaImpl extends NotPorted{

        public NotPortedJavaImpl() {
            registerJavaObject(this);
        }

        public int multBy2(int value){
            return  value << 1;
        }

        //this method definition illustrates that not all method signatures must be
        //defined on the C++ side. Only those that are needed.
        public int divBy2(int value) {
            return value >> 1;
        }
    }

    @Test
    public void testExecutionOfJavaImplementation() {
        NotPortedJavaImpl notPorted = new NotPortedJavaImpl();
        Ported ported = new Ported(notPorted);
        assertEquals(512, ported.multBy4(128));
    }

    @Test
    public void testReturnArray(){
        NotPortedJavaImpl notPorted1 = new NotPortedJavaImpl();
        NotPortedJavaImpl notPorted2 = new NotPortedJavaImpl();
        NotPortedJavaImpl[] notPortedArr = {notPorted1, notPorted2};
        NotPortedJavaImpl[] result = new Ported(notPorted1).getNotPortedArray(notPortedArr);
        assertTrue(notPortedArr[0] == result[0]);
        assertTrue(notPortedArr[1] == result[1]);
    }

    @Test
    public void testReturnValue(){
        NotPortedJavaImpl notPorted = new NotPortedJavaImpl();
        Ported ported = new Ported(notPorted);
        assertTrue(notPorted == ported.getNotPorted());
    }

    @Test
    public void testReturnList(){
        NotPortedJavaImpl notPorted1 = new NotPortedJavaImpl();
        NotPortedJavaImpl notPorted2 = new NotPortedJavaImpl();
        AbstractSequentialList<NotPortedJavaImpl> list = new LinkedList<>();
        list.add(notPorted1);
        list.add(notPorted2);
        AbstractSequentialList<NotPortedJavaImpl> result = new Ported(notPorted1).getNotPortedList(list);
        assertTrue(list.get(0) == result.get(0));
        assertTrue(list.get(1) == result.get(1));
    }

    @Test
    public void testPerformance(){

        int numOfCalls = 10000000;

        NotPortedJavaImpl notPorted = new NotPortedJavaImpl();
        Ported ported = new Ported(notPorted);

        long begin = System.currentTimeMillis();
        for (int i = 0; i< numOfCalls; i++) {
            ported.multBy2Cpp(i);
        }
        long end = System.currentTimeMillis();
        System.out.println(numOfCalls + " calls of an C++ function from Java took " + (end - begin) + " ms");

        begin = System.currentTimeMillis();
        ported.callJdcMultby2(numOfCalls);
        end = System.currentTimeMillis();

        System.out.println(numOfCalls + " calls of an Java function from C++ took " + (end - begin)  + " ms");
    }


    public static class NotPortedTemplatedJavaImpl<T> extends NotPortedTemplatedT_int{

        public NotPortedTemplatedJavaImpl() {
            registerJavaObject(this);
        }

        public int multBy2(int value){
            return  value << 1;
        }

        //this method definition illustrates that not all method signatures must be
        //defined on the C++ side. Only those that are needed.
        public int divBy2(int value) {
            return value >> 1;
        }
    }

    @Test
    public void testExecutionOfJavaImplementationTemplate() {
        NotPortedTemplatedJavaImpl notPorted = new NotPortedTemplatedJavaImpl();
        PortedTemplatedT_int ported = new PortedTemplatedT_int(notPorted);
        assertEquals(512, ported.multBy4(128));
    }

    @Test
    public void testReturnValueTemplated(){
        NotPortedTemplatedJavaImpl notPorted = new NotPortedTemplatedJavaImpl();
        PortedTemplatedT_int ported = new PortedTemplatedT_int(notPorted);
        assertTrue(notPorted == ported.getNotPortedTemplated());
    }

    @Test
    public void testReturnArrayTemplated(){
        NotPortedTemplatedJavaImpl notPorted1 = new NotPortedTemplatedJavaImpl();
        NotPortedTemplatedJavaImpl notPorted2 = new NotPortedTemplatedJavaImpl();
        NotPortedTemplatedJavaImpl[] notPortedArr = {notPorted1, notPorted2};
        NotPortedTemplatedJavaImpl[] result = new PortedTemplatedT_int(notPorted1).getNotPortedTemplatedArray(notPortedArr);
        assertTrue(notPortedArr[0] == result[0]);
        assertTrue(notPortedArr[1] == result[1]);
    }

    @Test
    public void testReturnListTemplated(){
        NotPortedTemplatedJavaImpl notPorted1 = new NotPortedTemplatedJavaImpl();
        NotPortedTemplatedJavaImpl notPorted2 = new NotPortedTemplatedJavaImpl();
        AbstractSequentialList<NotPortedTemplatedJavaImpl> list = new LinkedList<>();
        list.add(notPorted1);
        list.add(notPorted2);
        AbstractSequentialList<NotPortedTemplatedJavaImpl> result = new PortedTemplatedT_int(notPorted1).getNotPortedemplatedList(list);
        assertTrue(list.get(0) == result.get(0));
        assertTrue(list.get(1) == result.get(1));
    }
}
