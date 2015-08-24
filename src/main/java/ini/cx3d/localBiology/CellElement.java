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



package ini.cx3d.localBiology;

import ini.cx3d.cells.Cell;
import ini.cx3d.physics.PhysicalObject;
import ini.cx3d.simulations.ECM;

import java.util.Vector;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * Super class for the local biological discrete elements (SomaElement & NeuriteElement).
 * Contains a <code>Vector</code> of <code>LocalBiologyModule</code>.
 * @author fredericzubler
 *
 */
public abstract class CellElement {

	/* Unique identification for this CellElement instance.*/
	int ID = 0;
	static AtomicInteger idCounter = new AtomicInteger(0);

	/* Reference to the ECM. */
	protected static ECM ecm = ECM.getInstance();

	/* The cells.Cell this CellElement belongs to.*/
	protected Cell cell;

	/* List of all the SubElements : small objects performing some biological operations.*/
	protected Vector<LocalBiologyModule> localBiologyModulesList = new Vector<LocalBiologyModule>();


	/** Simple constructor.*/
	public CellElement() {
		this.ID =  idCounter.incrementAndGet();
	}

	// *************************************************************************************
	// *      METHODS FOR LOCAL BIOLOGY MODULES                                            *
	// *************************************************************************************

	/* Calls the run() method in all the <code>SubElements</code>. 
	 * Is done automatically during the simulation, and thus doesn't have to be called by the user*/ 
	protected void runLocalBiologyModules(){
		//This type of loop because the removal of a SubElements from the subElementsList
		// could cause a ConcurrentModificationException.
		for (int i = 0; i < localBiologyModulesList.size(); i++) {
			localBiologyModulesList.get(i).run();
		}
	}

	/** Adds the argument to the <code>LocalBiologyModule</code> list, and registers this as it's 
	 * <code>CellElements</code>.*/
	public void addLocalBiologyModule(LocalBiologyModule m){
		localBiologyModulesList.add(m);
		m.setCellElement(this); // set the callback
	}

	/** Removes the argument from the <code>LocalBiologyModule</code> list.*/
	public void removeLocalBiologyModule(LocalBiologyModule m){
		localBiologyModulesList.remove(m);
	}
    
    /** Removes all the <code>LocalBiologyModule</code> in this <code>CellElements</code>.*/
	public void cleanAllLocalBiologyModules() {
		localBiologyModulesList.clear();
	}

	/** Returns the localBiologyModule List (not a copy).*/
	public Vector<LocalBiologyModule> getLocalBiologyModulesList() {
		return localBiologyModulesList;
	}
	
	/** Sets the localBiologyModule List.*/
	public void setLocalBiologyModulesList(Vector<LocalBiologyModule> localBiologyModulesList) {
		this.localBiologyModulesList = localBiologyModulesList;
	}


	// *************************************************************************************
	// *      METHODS FOR SETTING CELL                                                     *
	// *************************************************************************************

	/**
	 * Sets the <code>Cell</code> this <code>CellElement</code> is part of. 
	 * @param cell
	 */
	public void setCell(Cell cell) {
		this.cell = cell;
	}
	
	/**
	 * 
	 * @return the <code>Cell</code> this <code>CellElement</code> is part of.
	 */
	public Cell getCell() {
		return cell;
	}
	
	// *************************************************************************************
	// *      METHODS FOR DEFINING TYPE (neurite element vs soma element)                                                  *
	// *************************************************************************************

	/** Returns <code>true</code> if is a <code>NeuriteElement</code>.*/
	public abstract boolean isANeuriteElement();
	/** Returns <code>true</code> if is a <code>SomaElement</code>.*/
	public abstract boolean isASomaElement();

	// *************************************************************************************
	// *      METHODS FOR CALLS TO PHYSICS (POSITION, ETC)                                 *
	// *************************************************************************************

	/** Returns the location of the point mass of the <code>PhysicalObject</code> 
	 * associated with this <code>CellElement</code>.
	 */
	public double[] getLocation() {
		return getPhysical().getMassLocation();
	}

	/** The <code>PhysicalSphere or <code>PhysicalCylinder</code> linked with this <code>CellElement</code>.*/
	public abstract PhysicalObject getPhysical();

	/** The <code>PhysicalSphere or <code>PhysicalCylinder</code> linked with this <code>CellElement</code>.*/
	public abstract void setPhysical(PhysicalObject p);

	/**
	 * Displaces the point mass of the <code>PhysicalObject</code> associated with
	 * this <code>CellElement</code>.
	 * @param speed in microns/hours
	 * @param direction (norm not taken into account)
	 */
	public void move(double speed, double[] direction){
		getPhysical().movePointMass(speed, direction);
	}
}
