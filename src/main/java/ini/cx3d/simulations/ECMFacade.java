package ini.cx3d.simulations;

import ini.cx3d.simulations.interfaces.*;
import ini.cx3d.swig.biology.biology;

public class ECMFacade {
    public static ini.cx3d.simulations.interfaces.ECM getInstance() {
        if(biology.useNativeECM) {
            throw new UnsupportedOperationException("ECM doesn't have native implementaiton yet!");
//            return ini.cx3d.swig.biology.ECM.getInstance();
        } else {
            return ECM.getInstance();
        }
    }

    public static double getRandomDouble() {
        if(biology.useNativeECM) {
            throw new UnsupportedOperationException("ECM doesn't have native implementaiton yet!");
//            return ini.cx3d.swig.biology.ECM.getInstance();
        } else {
            return ECM.getRandomDouble();
        }
    }
}
