package ini.cx3d;

//import ini.cx3d.swig.StringBuilder;


/**
 * Classes that implement that interface serialize their simulation state to json with as little implementation details
 * as possible (e.g. state of locks or which collection implementation has been used)
 *
 * Special note on Collections that contain elements where the order is irrelevant: e.g. Set of Cells
 * don't use json array notation, but object notation with the same key repeated
 * This important for the comparison if two simulation states are the same
 * e.g. two worker threads adding cell elements to an ArrayList -> position irrelevant
 *
 */
public interface SimStateSerializable {
    ini.cx3d.swig.StringBuilder simStateToJson(ini.cx3d.swig.StringBuilder sb);
}

