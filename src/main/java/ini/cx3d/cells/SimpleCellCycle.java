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

import ini.cx3d.physics.PhysicalSphere;

public class SimpleCellCycle implements CellModule{

	/* the cell it belongs to.*/
	private Cell cell;
	/* turned on or off */
	private boolean enable = true;
	
	/* the speed of metabolic update.*/
	private double dVdt = 150.0;
	/* the minimum size to obtain before being allowed to divide.*/
	private double minimumDiameter = 20.0;
	
	
	public Cell getCell() {
		return cell;
	}

	public boolean isEnabled() {
		return enable;
	}

	// the cell cycle model lies in this method:
	public void run() {
		if(!enable)
			return;
		PhysicalSphere ps = cell.getSomaElement().getPhysicalSphere(); 
		// is diameter smaller than min
		if(ps.getDiameter() < minimumDiameter){
			ps.changeVolume(dVdt);
		}else{
			// otherwise divide
			cell.divide();
		}
	}

	public void setCell(Cell cell) {
		this.cell = cell;
	}

	public void reset(){
		cell.divide();
	}
	
	public void setEnabled(boolean enabled) {
		this.enable = enabled;
	}
	
	public CellModule getCopy(){
		SimpleCellCycle cc = new SimpleCellCycle();
		cc.enable = this.enable;
		return cc;
	}

	public boolean isCopiedWhenCellDivides() {
		return true;
	}
}
