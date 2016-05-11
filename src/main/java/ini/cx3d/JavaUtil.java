package ini.cx3d;

import ini.cx3d.spatialOrganization.NewDelaunayTest;
import ini.cx3d.spatialOrganization.interfaces.OpenTriangleOrganizer;
import ini.cx3d.spatialOrganization.factory.OpenTriangleOrganizerFactory;
import ini.cx3d.swig.simulation.JavaUtilT_PhysicalNode;
import ini.cx3d.swig.simulation.simulation;
import ini.cx3d.utilities.Matrix;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * provides functionality that has not been implemented yet in C++
 * especially static methods as they can't be handled by SWIG directors
 */
public class JavaUtil extends JavaUtilT_PhysicalNode {

//    @Override
//    public OpenTriangleOrganizer oto_createSimpleOpenTriangleOrganizer() {
//        return OpenTriangleOrganizerFactory.createSimpleOpenTriangleOrganizer();
//    }

    static List<Integer> list = Arrays.asList(new Integer[]{
            0, 1, 2, 3});

    @Override
    public int[] generateTriangleOrder() {
        Collections.shuffle(list, NewDelaunayTest.rand);
        int[] ret = new int[4];
        for (int i = 0; i < list.size(); i++) {
            ret[i] = list.get(i);
        }
        return ret;
    }

    public double[] matrixRandomNoise3(double k){
        return Matrix.randomNoise(k, 3);
    }
}
