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

import ini.cx3d.swig.NotPortedCppType;
import ini.cx3d.swig.NotPortedTemplatedT_intCppType;
import ini.cx3d.swig.Ported;
import ini.cx3d.swig.PortedTemplatedT_int;
import org.junit.Test;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

/**
 * Tests correctness of the java defined class macros
 * They are used for classes that are needed on the C++ side, but still implemented in Java
 */
public class JavaDefinedClassTest {
    public static class NotPortedJavaImpl extends NotPortedCppType{
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
    public void testReturnValue() {
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
    public void testExecutionOfJavaImplementation(){
        NotPortedJavaImpl notPorted = new NotPortedJavaImpl();
        Ported ported = new Ported(notPorted);
        assertTrue(notPorted == ported.getNotPorted());
    }


    public static class NotPortedTemplatedJavaImpl<T> extends NotPortedTemplatedT_intCppType{
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
    public void testReturnValueTemplated() {
        NotPortedTemplatedJavaImpl notPorted = new NotPortedTemplatedJavaImpl();
        PortedTemplatedT_int ported = new PortedTemplatedT_int(notPorted);
        assertEquals(512, ported.multBy4(128));
    }

    @Test
    public void testExecutionOfJavaImplementationTemplate(){
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
}
