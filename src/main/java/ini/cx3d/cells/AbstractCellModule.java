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

public abstract class AbstractCellModule implements CellModule {

	protected ini.cx3d.cells.interfaces.Cell cell; // and not "private", so that subclass can access it
	
	public ini.cx3d.cells.interfaces.Cell getCell() {
		return cell;
	}

	public void setCell(ini.cx3d.cells.interfaces.Cell cell) {
		this.cell = cell;
	}
	
	public abstract CellModule getCopy();
	
	/** By default returns true.*/
	public boolean isCopiedWhenCellDivides() {
		return true;
	}

	public abstract void run();


}
