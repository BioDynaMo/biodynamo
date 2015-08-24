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
 * Abstract class implementing the <code>LocalBiologyModule</code> interface. This class can be extended
 * to design new local modules. By default, each copy method returns <code>false</code>.
 * @author fredericzubler
 *
 */
public abstract class AbstractLocalBiologyModule implements LocalBiologyModule{

	protected CellElement cellElement; // "protected" so subclasses can access it
	
	public CellElement getCellElement() {return cellElement;}

	public void setCellElement(CellElement cellElement) {
		this.cellElement = cellElement;
	}
	
	public boolean isCopiedWhenNeuriteBranches() {return false;}
	
	public boolean isCopiedWhenNeuriteElongates() {return false;}
	
	public boolean isCopiedWhenNeuriteExtendsFromSoma() {return false;}
	
	public boolean isCopiedWhenSomaDivides() {return false;}
	
	public boolean isDeletedAfterNeuriteHasBifurcated() {return false;}
	
	public abstract AbstractLocalBiologyModule getCopy();
	
	public abstract void run();
}
