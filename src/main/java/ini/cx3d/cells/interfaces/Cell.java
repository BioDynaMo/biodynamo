package ini.cx3d.cells.interfaces;

import ini.cx3d.SimStateSerializable;
import ini.cx3d.cells.CellModule;
import ini.cx3d.localBiology.interfaces.SomaElement;

import java.awt.*;
import java.util.AbstractSequentialList;

/**
 * Created by lukas on 20.04.16.
 */
public interface Cell extends SimStateSerializable {
	/** Represents inhibitory type of cell in the NeuroML export*/
	String InhibitoryCell = "Inhibitory_cells";
	/** Represents excitatory type of cell in the NeuroML export*/
	String ExcitatoryCell = "Excitatory_cells";

	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	/**
	 * Run Cell: run <code>Gene</code>, run <code>LyonCellCycle</code>, run Conditions, run EnergyProduction.
	 * We move one step further in the simulation by running the <code>Gene</code>, <code>GeneSubstances</code>,
	 * the <code>LyonCellCycle</code>, EnergyProduction and than we test conditions with ConditionTester.
	 */
	void run();

	/**
	 * Divide the cell. Of the two daughter cells, one is this one (but smaller, with half GeneSubstances etc.),
	 * and the other one is instantiated de novo and is returned. Both cells have more or less the same volume,
	 * the axis of division is random.
	 * @return the other daughter cell.
	 */
	Cell divide();

	/**
	 * Divide the cell. Of the two daughter cells, one is this one (but smaller, with half GeneSubstances etc.),
	 * and the other one is instantiated de novo and is returned. The axis of division is random.
	 * @param volumeRatio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0 gives equal cells.
	 * @return the second daughter cell.
	 */
	Cell divide(double volumeRatio);

	Cell divide(double[] axisOfDivision);

	/**
	 * Divide the cell. Of the two daughter cells, one is this one (but smaller, with half GeneSubstances etc.),
	 * and the other one is instantiated de novo and is returned. The axis of division is random.
	 * @param volumeRatio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0 gives equal cells.
	 * @param axisOfDivision direction of
	 * @return the second daughter cell
	 */
	Cell divide(double volumeRatio, double[] axisOfDivision);

	/**
	 * Divide mother cell in two daughter cells by coping <code>Cell</code>, <code>SomaElement</code>,
	 * <code>PhysicalSpehre</code>, list of <code>CellModules</code>.
	 * <code>CellSubstances</code> are dispatched in the two cells.
	 * The <code>CellClock</code>  and cell lineage, if present, are also copied..
	 * When mother cell divides, by definition:
	 * 1) the mother cell becomes the 1st daughter cell
	 * 2) the new cell becomes the 2nd daughter cell and inherits a equal or bigger volume than the 1st
	 *    daughter cell, which means that this cell will eventually inherit more differentiating factors
	 *    and will be recorded in the left side of the lineage tree.
	 *
	 * @return the second daughter cell
	 */
	Cell divide(double volumeRatio, double phi, double theta);

	/**
	 * Adds a <code>CellModule</code> that will be run at each time step.
	 * @param m
	 */
	void addCellModule(CellModule m);

	/**
	 * Removes a particular <code>CellModule</code> from this <code>Cell</code>.
	 * It will therefore not be run anymore.
	 * @param m
	 */
	void removeCellModule(CellModule m);

	/** Removes all the <code>CellModule</code> in this <code>Cell</code>.*/
	void cleanAllCellModules();

	/** Currently, there are two types of cells : Inhibitory_cells and Excitatory_cells.*/
	void setNeuroMLType(ini.cx3d.swig.simulation.Cell.NeuroMLType neuroMLType);

	/** Currently, there are two types of cells :  <code>Inhibitory_cells</code> and  <code>Excitatory_cells</code>.*/
	ini.cx3d.swig.simulation.Cell.NeuroMLType getNeuroMLType();

	/** Returns the cell type. This is just a convenient way to store some property for the cell.
	 * Should not be confused with NeuroMLType.
	 */
	String getType();

	/** Sets the cell type. This is just a convenient way to store some property for the cell.
	 * Should not be confused with NeuroMLType.
	 */
	void setType(String type);

	SomaElement getSomaElement();

	void setSomaElement(SomaElement somaElement);

	int getID();

	/**
	 * Sets the color for all the <code>PhysicalObjects</code> associated with the
	 * <code>CellElements</code> of this Cell..
	 * @param color
	 */
	void setColorForAllPhysicalObjects(Color color);

	/**
	 * @return a <code>Vector</code> containing all the <code>NeuriteElement</code>s of this cell.
	 */
	AbstractSequentialList<ini.cx3d.localBiology.interfaces.NeuriteElement> getNeuriteElements();
}
