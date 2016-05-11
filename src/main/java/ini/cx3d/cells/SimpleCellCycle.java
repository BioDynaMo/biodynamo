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

public class SimpleCellCycle implements CellModule{

	/* the cell it belongs to.*/
	private ini.cx3d.cells.interfaces.Cell cell;
	/* turned on or off */
	private boolean enable = true;
	
	/* the speed of metabolic update.*/
	private double dVdt = 150.0;
	/* the minimum size to obtain before being allowed to divide.*/
	private double minimumDiameter = 20.0;

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{");

		keyValue(sb, "cell", cell);
		keyValue(sb, "enable", enable);
		keyValue(sb, "dVdt", dVdt);
		keyValue(sb, "minimumDiameter", minimumDiameter);

		removeLastChar(sb);
		sb.append("}");
		return sb;
	}
	
	public ini.cx3d.cells.interfaces.Cell getCell() {
		return cell;
	}

	public boolean isEnabled() {
		return enable;
	}

	// the cell cycle model lies in this method:
	public void run() {
		if(!enable)
			return;
		ini.cx3d.physics.interfaces.PhysicalSphere ps = cell.getSomaElement().getPhysicalSphere();
		// is diameter smaller than min
		if(ps.getDiameter() < minimumDiameter){
			ps.changeVolume(dVdt);
		}else{
			// otherwise divide
			cell.divide();
		}
	}

	public void setCell(ini.cx3d.cells.interfaces.Cell cell) {
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
