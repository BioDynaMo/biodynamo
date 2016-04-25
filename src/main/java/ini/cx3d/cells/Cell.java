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

package ini.cx3d.cells;

import static ini.cx3d.SimStateSerializationUtil.keyValue;
import static ini.cx3d.SimStateSerializationUtil.removeLastChar;
import static ini.cx3d.SimStateSerializationUtil.unorderedCollection;
import static ini.cx3d.utilities.Matrix.add;

import ini.cx3d.localBiology.interfaces.NeuriteElement;
import ini.cx3d.localBiology.interfaces.SomaElement;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.swig.biology.ListT_CellModule;
import ini.cx3d.swig.biology.ListT_NeuriteElement;

import java.awt.Color;
import java.util.AbstractSequentialList;
import java.util.Vector;

/**
 * Class <code>Cell</code> implements the cell at biological level. Every cell is characterized
 * by a unique cellId, cellType (cell state), <code>LyonCellCycle</code> (cell cycle) and are eventually
 * organized in a cell lineage tree (<code>CellLinNode</code>).
 * This class contains the genome (for now a list of <code>Gene</code>), a list of <code>GeneSubstance</code>
 * (seen as the product of the genes in the Gene vector), and is characterized by a cell type (defined by the
 * relative concentrations of the GeneSubstances.

 * @author sabina & RJD & fredericzubler
 *
 */
public class Cell extends ini.cx3d.swig.biology.Cell implements ini.cx3d.cells.interfaces.Cell {

	/* Unique identification for this Cell instance. */
	private int ID = 0;
	
	/* Counter to uniquely identify every cell. */
	private static int idCounter = 0;

	/* Reference to the ECM. */
	private static ini.cx3d.simulations.interfaces.ECM ecm = ECMFacade.getInstance();
			
	/* List of all cell modules that are run at each time step*/
	private AbstractSequentialList<CellModule> cellModules = new ListT_CellModule();

	/* List of the SomaElements belonging to the cell */
	private SomaElement somaElement = null;

	/* List of the first Neurite of all Nurites belonging to the cell */
	private Vector<ini.cx3d.localBiology.interfaces.NeuriteElement> neuriteRootList = new Vector<ini.cx3d.localBiology.interfaces.NeuriteElement>(); // TODO: not working yet

	/* The electrophsiology type of this cell */
	private ini.cx3d.swig.biology.Cell.NeuroMLType neuroMLType = NeuroMLType.kExcitatatory;
	
	/* Some convenient way to store properties of  for cells. 
	 * Should not be confused with neuroMLType. */
	private String type = "";

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{");

		keyValue(sb, "id", ID);
		keyValue(sb, "idCounter", idCounter);
		unorderedCollection(sb, "cellModules", cellModules);
		keyValue(sb, "somaElement", somaElement);
		unorderedCollection(sb, "neuriteRootList", neuriteRootList);
		String neuro_type = "";
		if (neuroMLType == NeuroMLType.kExcitatatory) neuro_type = "Excitatory_cells";
		else if (neuroMLType == NeuroMLType.kInhibitory) neuro_type = "Inhibitory_cells";
		keyValue(sb, "neuroMlType", neuro_type, true);
		keyValue(sb, "type", type, true);

		removeLastChar(sb);
		sb.append("}");
		return sb;
	}
	
	/**
	 * Generate <code>Cell</code>. and registers the <code>Cell</code> to <code>ECM<</code>. 
	 * Every cell is identified by a unique cellID number.
	 */
	public Cell() {
		registerJavaObject(this);
		idCounter++;
		ID = idCounter;
		ecm.addCell(this);
	}

	public String toString(){
		return "Cell";
	}

	/**
	 * Run Cell: run <code>Gene</code>, run <code>LyonCellCycle</code>, run Conditions, run EnergyProduction.
	 * We move one step further in the simulation by running the <code>Gene</code>, <code>GeneSubstances</code>,
	 * the <code>LyonCellCycle</code>, EnergyProduction and than we test conditions with ConditionTester. 
	 */
	@Override
	public void run() {
		
		// Run all the CellModules
		// Important : the vector might be modified during the loop (for instance if a module deletes itself)
		for (int j = 0; j < cellModules.size(); j++) {
			CellModule module = cellModules.get(j);
			module.run();
		}
		
	}

	// *************************************************************************************
	// *      METHODS FOR DIVISION                                                         *
	// *************************************************************************************

	/**
	 * Divide the cell. Of the two daughter cells, one is this one (but smaller, with half GeneSubstances etc.),
	 * and the other one is instantiated de novo and is returned. Both cells have more or less the same volume, 
	 * the axis of division is random.
	 * @return the other daughter cell.
	 */
	@Override
	public ini.cx3d.cells.interfaces.Cell divide() {
		// find a volume ration close to 1;
		return divide(0.9 + 0.2*ECMFacade.getRandomDouble());
	}
	
	/**
	 * Divide the cell. Of the two daughter cells, one is this one (but smaller, with half GeneSubstances etc.),
	 * and the other one is instantiated de novo and is returned. The axis of division is random.
	 * @param volumeRatio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0 gives equal cells.
	 * @return the second daughter cell.
	 */
	@Override
	public ini.cx3d.cells.interfaces.Cell divide(double volumeRatio){
			// find random point on sphere (based on : http://mathworld.wolfram.com/SpherePointPicking.html)
			double theta = 6.28318531* ECMFacade.getRandomDouble();
			double phi = Math.acos(2*ECMFacade.getRandomDouble()-1);
			return divide(volumeRatio, phi, theta);
	}
	
	@Override
	public ini.cx3d.cells.interfaces.Cell divide(double[] axisOfDivision) {
		ini.cx3d.physics.interfaces.PhysicalSphere sphere = somaElement.getPhysicalSphere();
		double[] polarcoord = sphere.transformCoordinatesGlobalToPolar(
				add(axisOfDivision, sphere.getMassLocation()));
		return divide(0.9 + 0.2*ECMFacade.getRandomDouble(), polarcoord[1], polarcoord[2]);
	}
	/**
	 * Divide the cell. Of the two daughter cells, one is this one (but smaller, with half GeneSubstances etc.),
	 * and the other one is instantiated de novo and is returned. The axis of division is random.
	 * @param volumeRatio the ratio (Volume daughter 1)/(Volume daughter 2). 1.0 gives equal cells.
	 * @param axisOfDivision direction of 
	 * @return the second daughter cell
	 */
	@Override
	public ini.cx3d.cells.interfaces.Cell divide(double volumeRatio, double[] axisOfDivision) {
		ini.cx3d.physics.interfaces.PhysicalSphere sphere = somaElement.getPhysicalSphere();
		double[] polarcoord = sphere.transformCoordinatesGlobalToPolar(
				add(axisOfDivision, sphere.getMassLocation()));
		return divide(volumeRatio, polarcoord[1], polarcoord[2]);
	}
	
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
	@Override
	public ini.cx3d.cells.interfaces.Cell divide(double volumeRatio, double phi, double theta) {
		
		// 1) Create a new daughter cell. The mother cell and the 1st daughter cell are the same java object instance!
		ini.cx3d.cells.interfaces.Cell newCell = new Cell();
		idCounter = idCounter + 1;
		this.ID = idCounter;
		
		// 2) Copy the CellModules that have to be copied
		for (CellModule module : cellModules) {
			if(module.isCopiedWhenCellDivides()){
				newCell.addCellModule(module.getCopy());
			}
		}
		
		// 3) Also divide the LocalBiologyLayer 
		newCell.setSomaElement(this.somaElement.divide(volumeRatio, phi, theta));				

		return newCell;
	}


	// *************************************************************************************
	// *      METHODS FOR CELL MODULES                                                     *
	// *************************************************************************************

	/**
	 * Adds a <code>CellModule</code> that will be run at each time step.
	 * @param m
	 */
	@Override
	public void addCellModule(CellModule m){
		cellModules.add(m);
		m.setCell(this);
	}
	/**
	 * Removes a particular <code>CellModule</code> from this <code>Cell</code>.
	 * It will therefore not be run anymore.
	 * @param m
	 */
	@Override
	public void removeCellModule(CellModule m){
		cellModules.remove(m);
	}

	  /** Removes all the <code>CellModule</code> in this <code>Cell</code>.*/
	@Override
	public void cleanAllCellModules() {
		cellModules.clear();
	}


	// *************************************************************************************
	// *      GETTERS & SETTERS                                                            *
	// *************************************************************************************

	/** Currently, there are two types of cells : Inhibitory_cells and Excitatory_cells.*/
	@Override
	public void setNeuroMLType(ini.cx3d.swig.biology.Cell.NeuroMLType neuroMLType) {
		this.neuroMLType = neuroMLType;
	}
	
	/** Currently, there are two types of cells :  <code>Inhibitory_cells</code> and  <code>Excitatory_cells</code>.*/
	@Override
	public ini.cx3d.swig.biology.Cell.NeuroMLType getNeuroMLType() {
		return neuroMLType;
	}
	
	/** Returns the cell type. This is just a convenient way to store some property for the cell. 
	 * Should not be confused with NeuroMLType. 
	 */
	@Override
	public String getType() {
		return type;
	}

	/** Sets the cell type. This is just a convenient way to store some property for the cell. 
	 * Should not be confused with NeuroMLType. 
	 */
	@Override
	public void setType(String type) {
		this.type = type;
	}

	
	@Override
	public SomaElement getSomaElement() {
		return somaElement;
	}

	@Override
	public void setSomaElement(SomaElement somaElement) {
		this.somaElement = somaElement;
		somaElement.setCell(this);
	}

	@Override
	public int getID(){
		return this.ID;
	}


	/**
	 * Sets the color for all the <code>PhysicalObjects</code> associated with the 
	 * <code>CellElements</code> of this Cell..
	 * @param color
	 */
	@Override
	public void setColorForAllPhysicalObjects(Color color) {
			somaElement.getPhysical().setColor(color);
			for (ini.cx3d.localBiology.interfaces.NeuriteElement ne : getNeuriteElements()) {
				ne.getPhysical().setColor(color);
			}
	}

	/** Returns the list of all the CellModules.*/
//	@Override
//	public AbstractSequentialList<CellModule> getCellModules() {
//		return cellModules;
//	}

	/**
	 * @return a <code>Vector</code> containing all the <code>NeuriteElement</code>s of this cell.
	 */
	@Override
	public AbstractSequentialList<ini.cx3d.localBiology.interfaces.NeuriteElement> getNeuriteElements() {
		AbstractSequentialList<NeuriteElement> allTheNeuriteElements = new ListT_NeuriteElement();
		for (ini.cx3d.localBiology.interfaces.NeuriteElement ne : somaElement.getNeuriteList()) {
			ne.addYourselfAndDistalNeuriteElements(allTheNeuriteElements);
		}
		return allTheNeuriteElements;
	}

}

