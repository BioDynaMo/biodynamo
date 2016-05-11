package ini.cx3d.physics.factory;

import ini.cx3d.JavaUtil;
import ini.cx3d.JavaUtil2;
import ini.cx3d.physics.DefaultForce;
import ini.cx3d.physics.InterObjectForce;
import ini.cx3d.swig.simulation.simulation;

public class DefaultForceFactory {
    private static final boolean NATIVE = simulation.useNativeDefaultForce;
    public static final boolean DEBUG = simulation.debugDefaultForce;

    // JavaUtil needs to be static - otherwise it will be garbage collected and
    // also destroyed on the cpp side
    private static JavaUtil2 java_ = new JavaUtil2();
    static {
        ini.cx3d.swig.simulation.DefaultForce.setJavaUtil(java_);
    }

    public static InterObjectForce create() {
        if(NATIVE){
            InterObjectForce force = ini.cx3d.swig.simulation.DefaultForce.create();
            ini.cx3d.swig.simulation.InterObjectForce.registerJavaObject(force);
            return force;
        } else if(!DEBUG){
            return new DefaultForce();
        } else {
            throw new UnsupportedOperationException("");
        }
    }

}
