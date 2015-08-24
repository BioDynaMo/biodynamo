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

/**
 * 
 * Classes implementing this interface can be added to a <code>Cell</code>, and be run.
 * They represent the biological model that CX3D is simulating.
 * 
 * @author fredericzubler
 *
 */
public interface CellModule {
	
	/** Run the simulation*/
	public void run();

	/** @return the <code>Cell</code> this module leaves in*/
	public Cell getCell();

	/**@param cell the <code>Cell</code> this module lives in*/
	public void setCell(Cell cell);
	
	/** Get a copy */
	public CellModule getCopy();
	
	/** If returns <code>true</code>, this module is copied during cell division.*/
	public boolean isCopiedWhenCellDivides();
}
