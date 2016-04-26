package ini.cx3d.localBiology.factory;


import ini.cx3d.localBiology.interfaces.NeuriteElement;
import ini.cx3d.localBiology.interfaces.SomaElement;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.swig.simulation.simulation;

public class NeuriteElementFactory {
    private static final boolean NATIVE = simulation.useNativeNeuriteElement;
    public static final boolean DEBUG = false;

    static {
        ini.cx3d.swig.simulation.CellElement.setECM(ECMFacade.getInstance());
    }

    public static NeuriteElement create() {
        if(NATIVE){
            NeuriteElement neurite = ini.cx3d.swig.simulation.NeuriteElement.create();
            ini.cx3d.swig.simulation.CellElement.registerJavaObject(neurite);
            ini.cx3d.swig.simulation.NeuriteElement.registerJavaObject(neurite);
            return neurite;
        } else if(!DEBUG){
            return new ini.cx3d.localBiology.NeuriteElement();
        } else {
            throw new UnsupportedOperationException("");
        }
    }

}
