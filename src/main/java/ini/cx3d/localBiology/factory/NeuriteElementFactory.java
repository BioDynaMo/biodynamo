package ini.cx3d.localBiology.factory;


import ini.cx3d.localBiology.interfaces.NeuriteElement;
import ini.cx3d.localBiology.interfaces.SomaElement;
import ini.cx3d.simulations.ECM;
import ini.cx3d.swig.biology.biology;

public class NeuriteElementFactory {
    private static final boolean NATIVE = biology.useNativeNeuriteElement;
    public static final boolean DEBUG = false;

    static {
        ini.cx3d.swig.biology.CellElement.setECM(ECM.getInstance());
    }

    public static NeuriteElement create() {
        if(NATIVE){
            NeuriteElement neurite = ini.cx3d.swig.biology.NeuriteElement.create();
            ini.cx3d.swig.biology.CellElement.registerJavaObject(neurite);
            ini.cx3d.swig.biology.NeuriteElement.registerJavaObject(neurite);
            return neurite;
        } else if(!DEBUG){
            return new ini.cx3d.localBiology.NeuriteElement();
        } else {
            throw new UnsupportedOperationException("");
        }
    }

}
