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

import ini.cx3d.SimStateSerializable;
import ini.cx3d.swig.*;
import ini.cx3d.swig.NativeStringBuilder;
import org.junit.Test;

import static org.junit.Assert.assertEquals;

/**
 * Tests two way communication between Java and CPP on the example of SimStateSerializable
 */
public class SimStateSerializableTest {


    @Test
    public void testJavaCallsCppDefinedFunction() {
        //create native SimulationObject which implements SimStateSerializable
        SimulationObject simulationObject = new SimulationObject();
        //create native NativeStringBuilder
        NativeStringBuilder sb = new NativeStringBuilder();
        simulationObject.simStateToJson(sb);
        assertEquals("Hello World from CPP defined SimulationObject!", sb.str());
    }

    @Test
    public void testCppCallsJavaDefinedFunction() {
        //native object
        SimStateSerializerConsumer consumer = new SimStateSerializerConsumer();
        //consumer.persistSimState expects an object instance that implements SimStateSerializable
        //this time we pass a new Java object to this cpp function
        SimStateSerializable serializable = new JavaSimulationObject();
        NativeStringBuilder sb = consumer.persistSimState(serializable);
        assertEquals("Hello World from Java defined JavaSimulationObject!", sb.str());
    }

    static class JavaSimulationObject implements SimStateSerializable{

        @Override
        public NativeStringBuilder simStateToJson(NativeStringBuilder sb) {
            return sb.append("Hello World from Java defined JavaSimulationObject!");
        }
    }
}
