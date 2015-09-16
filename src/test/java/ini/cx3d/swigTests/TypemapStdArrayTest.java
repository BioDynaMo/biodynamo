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

import ini.cx3d.swig.ArrayUtil;
import org.junit.Test;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;

/**
 * Test correctness of java array/list to std::array conversion
 */
public class TypemapStdArrayTest {
    @Test
    public void testArrayInputArgumentConst() {
        double l1Norm = new ArrayUtil().l1Norm(new double[]{-1.0, 4.1, -8.2});
        assertEquals(13.3, l1Norm, 0.0001);
    }

    @Test
    public void testArrayInputArgumentNonConst() {
        double[] array = {-1.0, 4.1, -8.2};
        new ArrayUtil().scalarAddition(array, 2.3);
        assertArrayEquals(new double[]{1.3, 6.4, -5.9}, array, 0.0001);
    }

    @Test
    public void testArrayReturnValue() {
        double[] array = new ArrayUtil().getContent();
        Double[] expectedResult = new Double[]{98.0, 97.0};
        assertArrayEquals(new double[]{98.0, 97.0, 96.0}, array, 0.0001);
    }
}
