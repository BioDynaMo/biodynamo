package ini.cx3d;

import java.awt.*;
import java.util.Collection;
import java.util.Map;
import ini.cx3d.swig.StringBuilder;

/**
 * This class contains helper methods to serialize the simulation state to json
 */
public class SimStateSerializationUtil{

    public static StringBuilder key(StringBuilder sb, String key){
        sb.append("\"").append(key).append("\":");
        return sb;
    }

    public static StringBuilder keyValue(StringBuilder sb, String key, String value) {
        return keyValue(sb, key, value, false);
    }

    public static StringBuilder keyValue(StringBuilder sb, String key, double value) {
        return keyValue(sb, key, Double.toString(value), false);
    }

    public static StringBuilder keyValue(StringBuilder sb, String key, int value) {
        return keyValue(sb, key, Integer.toString(value), false);
    }

    public static StringBuilder keyValue(StringBuilder sb, String key, boolean value) {
        return keyValue(sb, key, Boolean.toString(value), false);
    }

    public static StringBuilder keyValue(StringBuilder sb, String key, String value, boolean wrapWithQuotes) {
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

    public static StringBuilder keyValue(StringBuilder sb, String key, SimStateSerializable value) {
        key(sb, key);
        if (value != null) {
            value.simStateToJson(sb);
        } else {
            sb.append("null");
        }
        sb.append(",");
        return sb;
    }

    public static StringBuilder keyValue(StringBuilder sb, String key, double[] vector) {
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

    public static StringBuilder keyValue(StringBuilder sb, String key, Double[] vector) {
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

    public static StringBuilder keyValue(StringBuilder sb, String key, Collection<? extends SimStateSerializable> elements) {
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

    public static StringBuilder map(StringBuilder sb, String key, Map<? extends Object, ? extends SimStateSerializable> map) {
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
    public static StringBuilder mapOfObjects(StringBuilder sb, String key, Map<? extends Object, ? extends Object> map) {
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

    public static StringBuilder mapOfDoubleArray(StringBuilder sb, String key, Map<? extends Object, double[]> map) {
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

    public static StringBuilder unorderedCollection(StringBuilder sb, String key, Collection<? extends SimStateSerializable> elements) {
        //simple implementation of unorderedCollection did not work
        //for now forward call to ordered collection (position of an element matters in equality comparisons)
        //if true position invariance in a collection is needed implement a more sophisticated solution
        //e.g. use object instead of array with key = hash(value) -> json string has to be traversed a second time after
        //build
        return orderedCollection(sb, key, elements);
    }

    public static StringBuilder orderedCollection(StringBuilder sb, String key, Collection<? extends SimStateSerializable> elements) {
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

    public static StringBuilder removeLastChar(StringBuilder sb) {
        sb.overwriteLastCharOnNextAppend();
        return sb;
    }

    public static String colorToHex(Color c) {
        return String.format("#%02x%02x%02x", c.getRed(), c.getGreen(), c.getBlue());
    }

}
