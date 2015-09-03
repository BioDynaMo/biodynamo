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

package ini.cx3d;

import java.awt.*;
import java.util.Collection;
import java.util.Map;
import ini.cx3d.swig.NativeStringBuilder;

/**
 * This class contains helper methods to serialize the simulation state to json
 */
public class SimStateSerializationUtil{

    public static NativeStringBuilder key(NativeStringBuilder sb, String key){
        sb.append("\"").append(key).append("\":");
        return sb;
    }

    public static NativeStringBuilder keyValue(NativeStringBuilder sb, String key, String value) {
        return keyValue(sb, key, value, false);
    }

    public static NativeStringBuilder keyValue(NativeStringBuilder sb, String key, double value) {
        return keyValue(sb, key, Double.toString(value), false);
    }

    public static NativeStringBuilder keyValue(NativeStringBuilder sb, String key, int value) {
        return keyValue(sb, key, Integer.toString(value), false);
    }

    public static NativeStringBuilder keyValue(NativeStringBuilder sb, String key, boolean value) {
        return keyValue(sb, key, Boolean.toString(value), false);
    }

    public static NativeStringBuilder keyValue(NativeStringBuilder sb, String key, String value, boolean wrapWithQuotes) {
        key(sb, key);
        if (wrapWithQuotes) {
            sb.append("\"");
        }
        sb.append(value);
        if (wrapWithQuotes) {
            sb.append("\"");
        }
        sb.append(",");
        return sb;
    }

    public static NativeStringBuilder keyValue(NativeStringBuilder sb, String key, SimStateSerializable value) {
        key(sb, key);
        if (value != null) {
            value.simStateToJson(sb);
        } else {
            sb.append("null");
        }
        sb.append(",");
        return sb;
    }

    public static NativeStringBuilder keyValue(NativeStringBuilder sb, String key, double[] vector) {
        key(sb, key).append("[");
        for (double value : vector) {
            sb.append(Double.toString(value)).append(",");
        }
        if (vector.length != 0) {
            removeLastChar(sb);
        }
        sb.append("],");
        return sb;
    }

    public static NativeStringBuilder keyValue(NativeStringBuilder sb, String key, Double[] vector) {
        key(sb, key).append("[");
        for (double value : vector) {
            sb.append(Double.toString(value)).append(",");
        }
        if (vector.length != 0) {
            removeLastChar(sb);
        }
        sb.append("],");
        return sb;
    }

    public static NativeStringBuilder keyValue(NativeStringBuilder sb, String key, Collection<? extends SimStateSerializable> elements) {
        key(sb, key).append("[");
        for (SimStateSerializable el : elements) {
            el.simStateToJson(sb).append(",");
        }
        if (!elements.isEmpty()) {
            removeLastChar(sb);
        }
        sb.append("],");
        return sb;
    }

    public static NativeStringBuilder map(NativeStringBuilder sb, String key, Map<? extends Object, ? extends SimStateSerializable> map) {
        key(sb, key);
        sb.append("{");
        for (Map.Entry<? extends Object, ? extends SimStateSerializable> entry : map.entrySet()) {
            key(sb, entry.getKey().toString());
            entry.getValue().simStateToJson(sb).append(",");
        }
        if (!map.isEmpty()) {
            removeLastChar(sb);
        }
        sb.append("},");
        return sb;
    }

    /**
     * creates json of maps of objects that don't implement the SimStateSerializable interface (String, Integer, ...)
     * @param sb
     * @param key
     * @param map
     * @return
     */
    public static NativeStringBuilder mapOfObjects(NativeStringBuilder sb, String key, Map<? extends Object, ? extends Object> map) {
        key(sb, key);
        sb.append("{");
        for (Map.Entry<? extends Object, ? extends Object> entry : map.entrySet()) {
            keyValue(sb, entry.getKey().toString(), entry.getValue().toString());
        }
        if (!map.isEmpty()) {
            removeLastChar(sb);
        }
        sb.append("},");
        return sb;
    }

    public static NativeStringBuilder mapOfDoubleArray(NativeStringBuilder sb, String key, Map<? extends Object, double[]> map) {
        key(sb, key);
        sb.append("{");
        for (Map.Entry<? extends Object, double[]> entry : map.entrySet()) {
            keyValue(sb, entry.getKey().toString(), entry.getValue());
        }
        if (!map.isEmpty()) {
            removeLastChar(sb);
        }
        sb.append("},");
        return sb;
    }

    public static NativeStringBuilder unorderedCollection(NativeStringBuilder sb, String key, Collection<? extends SimStateSerializable> elements) {
        //simple implementation of unorderedCollection did not work
        //for now forward call to ordered collection (position of an element matters in equality comparisons)
        //if true position invariance in a collection is needed implement a more sophisticated solution
        //e.g. use object instead of array with key = hash(value) -> json string has to be traversed a second time after
        //build
        return orderedCollection(sb, key, elements);
    }

    public static NativeStringBuilder orderedCollection(NativeStringBuilder sb, String key, Collection<? extends SimStateSerializable> elements) {
        key(sb, key).append("[");
        for (SimStateSerializable el : elements) {
//            sb.append("\"a\": ");
            el.simStateToJson(sb);
            sb.append(",");
        }
        if (!elements.isEmpty()) {
            removeLastChar(sb);
        }
        sb.append("],");
        return sb;
    }

    public static NativeStringBuilder removeLastChar(NativeStringBuilder sb) {
        sb.overwriteLastCharOnNextAppend();
        return sb;
    }

    public static String colorToHex(Color c) {
        return String.format("#%02x%02x%02x", c.getRed(), c.getGreen(), c.getBlue());
    }

}
