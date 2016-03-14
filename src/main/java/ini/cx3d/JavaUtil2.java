package ini.cx3d;

import ini.cx3d.spatialOrganization.NewDelaunayTest;
import ini.cx3d.spatialOrganization.factory.OpenTriangleOrganizerFactory;
import ini.cx3d.spatialOrganization.interfaces.OpenTriangleOrganizer;
import ini.cx3d.swig.spatialOrganization.JavaUtilT_PhysicalNode;
import ini.cx3d.utilities.Matrix;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * provides functionality that has not been implemented yet in C++
 * especially static methods as they can't be handled by SWIG directors
 */
public class JavaUtil2 extends ini.cx3d.swig.physics.JavaUtil2 {

   public double[] matrixRandomNoise3(double k){
        return Matrix.randomNoise(k, 3);
    }
}
