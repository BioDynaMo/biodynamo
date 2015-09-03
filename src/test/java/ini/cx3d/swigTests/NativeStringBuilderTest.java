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

import org.junit.Test;

import static org.junit.Assert.assertEquals;

public class NativeStringBuilderTest {

    @Test
    public void testStringBuilder() {
        ini.cx3d.swig.NativeStringBuilder sb = new ini.cx3d.swig.NativeStringBuilder();
        sb.append("Hello ").append("World?");
        assertEquals("Hello World?", sb.str());
        sb.overwriteLastCharOnNextAppend();
        assertEquals("Hello World?", sb.str());
        sb.append("!");
        assertEquals("Hello World!", sb.str());
    }
}
