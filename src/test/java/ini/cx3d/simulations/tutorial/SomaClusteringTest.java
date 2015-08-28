/*
Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
Sabina Pfister & Adrian M. Whatley.

This file is part of CX3D.

CX3D is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CX3D is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
*/

package ini.cx3d.simulations.tutorial;

import static ini.cx3d.SimStateSerializationUtil.keyValue;
import static ini.cx3d.SimStateSerializationUtil.removeLastChar;
import static ini.cx3d.utilities.Matrix.normalize;
import static ini.cx3d.utilities.Matrix.randomNoise;

import java.awt.Rectangle;

import ini.cx3d.BaseSimulationTest;
import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.AbstractLocalBiologyModule;
import ini.cx3d.physics.PhysicalObject;
import ini.cx3d.physics.Substance;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

public class SomaClusteringTest extends BaseSimulationTest {
    public SomaClusteringTest() {
        super(SomaClusteringTest.class);
    }

    @Override
    public void simulate() throws Exception {
        ini.cx3d.utilities.SystemUtilities.tic();
        ECM ecm = ECM.getInstance();
        ECM.setRandomSeed(1L);


//		// set the rectangle for ROI
//		Rectangle smallWindowRectangle = new Rectangle();
//		smallWindowRectangle.x = 100;
//		smallWindowRectangle.y = 100;
//		smallWindowRectangle.width = 320;
//		smallWindowRectangle.height = 320;

//		Scheduler.simulateOneStep();
//		ecm.view.smallWindowRectangle = smallWindowRectangle;
        Substance yellowSubstance = new Substance("Yellow", 1000, 0.01);
        Substance violetSubstance = new Substance("Violet", 1000, 0.01);
        ecm.addNewSubstanceTemplate(yellowSubstance);
        ecm.addNewSubstanceTemplate(violetSubstance);
        for (int i = 0; i < 400; i++) {
            ecm.getPhysicalNodeInstance(randomNoise(700, 3));
        }
        for (int i = 0; i < 60; i++) {
            Cell c = CellFactory.getCellInstance(randomNoise(50, 3));
            c.getSomaElement().addLocalBiologyModule(new SomaClustering("Yellow"));
            c.setColorForAllPhysicalObjects(Param.X_SOLID_YELLOW);
        }
        for (int i = 0; i < 60; i++) {
            Cell c = CellFactory.getCellInstance(randomNoise(50, 3));
            c.getSomaElement().addLocalBiologyModule(new SomaClustering("Violet"));
            c.setColorForAllPhysicalObjects(Param.X_SOLID_VIOLET);
        }
        Scheduler.setPrintCurrentECMTime(false);

        ini.cx3d.utilities.SystemUtilities.tacAndTic();
        for (int i = 0; i < 1000; i++) {
            Scheduler.simulateOneStep();
            if (i % 100 == 0) {
                System.out.print("time step " + i + ", time = ");
                ini.cx3d.utilities.SystemUtilities.tacAndTic();
            }
        }
    }
}

class SomaClustering extends AbstractLocalBiologyModule {

    private String substanceID;

    public SomaClustering(String substanceID) {
        this.substanceID = substanceID;
    }

    public AbstractLocalBiologyModule getCopy() {
        return new SomaClustering(substanceID);
    }

    public void run() {
        PhysicalObject physical = super.cellElement.getPhysical();
        // move
        double speed = 100;
        double[] grad = physical.getExtracellularGradient(substanceID);
        physical.movePointMass(speed, normalize(grad));
        // secrete
        physical.modifyExtracellularQuantity(substanceID, 1000);
    }

    @Override
    public StringBuilder simStateToJson(StringBuilder sb) {
        super.simStateToJson(sb);

        keyValue(sb, "substanceID", substanceID);

        removeLastChar(sb);
        sb.append("}");
        return sb;
    }
}
