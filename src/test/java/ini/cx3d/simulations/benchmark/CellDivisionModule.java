package ini.cx3d.simulations.benchmark;

import java.awt.Color;

import ini.cx3d.BaseSimulationTest;
import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.cells.CellModule;
import ini.cx3d.localBiology.SomaElement;
import ini.cx3d.physics.PhysicalSphere;
import ini.cx3d.simulations.ECM;
import ini.cx3d.simulations.Scheduler;

import static ini.cx3d.utilities.Matrix.*;

class GrowDivide implements CellModule {

	Cell cell;

	public Cell getCell() {
		return cell;
	}

	public void setCell(Cell cell) {
		this.cell = cell;
	}

	public void run() {
		PhysicalSphere sphere = cell.getSomaElement().getPhysicalSphere();
		if(sphere.getDiameter() >= 40){
			cell.divide();
		}else{
			sphere.changeVolume(300);
		}
	}

	public GrowDivide getCopy(){
		return new GrowDivide();
	}

	public boolean isCopiedWhenCellDivides() {
		return true;
	}

	@Override
	public StringBuilder simStateToJson(StringBuilder sb) {
		sb.append("{}");
		return sb;
	}
}

public class CellDivisionModule extends BenchmarkTest {

	public CellDivisionModule() {
		super(CellDivisionModule.class);
	}

	@Override
	public void simulation() {
		ECM.setRandomSeed(1L);

    // extra PhysicalNodes :
    ECM ecm = ECM.getInstance();
		for (int i = 0; i < 12; i++) {
			double[] loc = concat(randomNoise(1000,2), randomNoise(100,1));
			ecm.getPhysicalNodeInstance(loc);
		}

    int cells_per_dim = 10;
    double space = 20;

    for (int x = 0; x < cells_per_dim; x++) {
      double x_pos = x * space;
      for (int y = 0; y < cells_per_dim; y++) {
        double y_pos = y * space;
        for (int z = 0; z < cells_per_dim; z++) {
          double[] cellOrigin = {x_pos, y_pos, z * space};
          Cell cell = CellFactory.getCellInstance(cellOrigin);
          SomaElement soma = cell.getSomaElement();
      		PhysicalSphere sphere = soma.getPhysicalSphere();
          sphere.setDiameter(41);
          sphere.setAdherence(0.4);
          sphere.setMass(1.0);
          cell.addCellModule(new GrowDivide());
        }
      }
    }

    long start = System.currentTimeMillis();
    for (int i = 0; i < 10; i++) {
      Scheduler.simulateOneStep();
    }
    long stop = System.currentTimeMillis();

    System.out.println("simulation runtime: "+getClass().getSimpleName().replace("_", "") + " " + (stop - start));
    System.out.println("Fished simulation");
	}
}
