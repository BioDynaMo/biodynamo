package ini.cx3d.physics.factory;

import ini.cx3d.physics.interfaces.Substance;
import ini.cx3d.swig.simulation.simulation;

import java.awt.Color;

/**
 * Factory that generates Substance objects
 */
public class SubstanceFactory {

    private static final boolean NATIVE = simulation.useNativeSubstance;
    public static final boolean DEBUG = simulation.debugSubstance;

    public static Substance create() {
        if (NATIVE) {
            return ini.cx3d.swig.simulation.Substance.create();
        } else if(!DEBUG) {
            return new ini.cx3d.physics.Substance();
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static Substance create(String id, double diffusionConstant, double degradationConstant) {
        if (NATIVE) {
            return ini.cx3d.swig.simulation.Substance.create(id, diffusionConstant, degradationConstant);
        } else if(!DEBUG) {
            return new ini.cx3d.physics.Substance(id, diffusionConstant, degradationConstant);
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static Substance create(String id, Color color) {
        if (NATIVE) {
            return ini.cx3d.swig.simulation.Substance.create(id, color);
        } else if(!DEBUG) {
            return new ini.cx3d.physics.Substance(id, color);
        } else {
            throw new UnsupportedOperationException();
        }
    }

    public static Substance create(Substance templateSubstance) {
        if (NATIVE) {
            return ini.cx3d.swig.simulation.Substance.create(templateSubstance);
        } else if(!DEBUG) {
            return new ini.cx3d.physics.Substance((ini.cx3d.physics.Substance) templateSubstance);
        } else {
            throw new UnsupportedOperationException();
        }
    }
}
