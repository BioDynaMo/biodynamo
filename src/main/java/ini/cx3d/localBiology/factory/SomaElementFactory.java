package ini.cx3d.localBiology.factory;


import ini.cx3d.localBiology.interfaces.SomaElement;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.swig.simulation.simulation;

public class SomaElementFactory {
    private static final boolean NATIVE = simulation.useNativeSomaElement;
    public static final boolean DEBUG = false;

    static {
        ini.cx3d.swig.simulation.CellElement.setECM(ECMFacade.getInstance());
    }

    public static SomaElement create() {
        if(NATIVE){
            SomaElement soma = ini.cx3d.swig.simulation.SomaElement.create();
            ini.cx3d.swig.simulation.CellElement.registerJavaObject(soma);
            return soma;
        } else if(!DEBUG){
            return new ini.cx3d.localBiology.SomaElement();
        } else {
            throw new UnsupportedOperationException("");
        }
    }

}
