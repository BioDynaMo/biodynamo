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

/**
 * 
 * Classes implementing this interface can be added in the CellElements, and be run.
 * They represent the biological model that CX3D is simulating.
 * Each instance of a localBiologyModule "lives" inside a particular CellElement. 
 * At SomaElement division or NeuriteElement branching, a cloned version is inserted into the new CellElement. 
 * If the clone() method is overwritten to return null, than the new CellElement doesn't contain a copy of the module.
 * 
 * @author fredericzubler, haurian 
 *
 */
public interface LocalBiologyModule {
	
	/** Perform your specific action*/
	abstract public void run();

	/** @return the cellElement this module leaves in*/
	public CellElement getCellElement();

	/**@param cellElement the cellElement this module lives in*/
	public void setCellElement(CellElement cellElement);
	
	/** */
	public LocalBiologyModule getCopy();
	
	
	/** Specifies if instances of LocalBiologicalModules are are copied into new branches.*/
	public boolean isCopiedWhenNeuriteBranches();
	
	/** Specifies if instances of LocalBiologicalModules are copied when the soma divides.*/
	public boolean isCopiedWhenSomaDivides();
	
	/** Specifies if instances of LocalBiologicalModules are copied when the neurite elongates 
	 * (not in new branches!).*/
	public boolean isCopiedWhenNeuriteElongates();
	
	/** Specifies if instances of LocalBiologicalModules are copied into NeuriteElements in case of 
	 * extension of a new neurte from a soma.*/
	public boolean isCopiedWhenNeuriteExtendsFromSoma();
	
	/** Specifies if instances of LocalBiologicalModules are deleted in a NeuriteElement that
	 * has just bifurcated (and is thus no longer a terminal neurite element).
	 */
	public boolean isDeletedAfterNeuriteHasBifurcated();
	
}
