package ini.cx3d.simulations;

import ini.cx3d.JavaUtil;
import ini.cx3d.JavaUtil2;
import ini.cx3d.physics.factory.PhysicalObjectFactory;
import ini.cx3d.simulations.interfaces.*;
import ini.cx3d.swig.simulation.SpaceNodeT_PhysicalNode;
import ini.cx3d.swig.simulation.simulation;

public class ECMFacade {

    static final JavaUtil2 java = new JavaUtil2();
    static {
        ini.cx3d.swig.simulation.ECM.setJavaUtil(java);


    }

    public static ini.cx3d.simulations.interfaces.ECM getInstance() {
        if(simulation.useNativeECM) {
            return ini.cx3d.swig.simulation.ECM.getInstance();
        } else {
            return ECM.getInstance();
        }
    }
}
