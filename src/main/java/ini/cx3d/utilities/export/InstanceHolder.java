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

package ini.cx3d.utilities.export;

import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;

/**
 * Utility class for storing information on a Cell for the XML export
 * @author fredericzubler
 *
 */
public class InstanceHolder {
	
	private Cell c;
	private int id = 0;
	
	public InstanceHolder() {}
	
	public InstanceHolder(Cell c, int id) {
		super();
		this.c = c;
		this.id = id;
	}

	public void setCell(Cell cell) {
		this.c = cell;
	}
	
	public int getId() {
		return id;
	}

	public void setId(int id) {
		this.id = id;
	}

	public static void main(String[] args) {
		Cell c = CellFactory.getCellInstance(new double[] {.2, -236.474745, Math.PI});
		InstanceHolder ch = new InstanceHolder();
		ch.c = c;
		System.out.println(ch.toXML("    "));
	}
	
	
	public StringBuilder toXML(String ident) {
		double[] location = c.getSomaElement().getLocation();
		StringBuilder sb = new StringBuilder();
		sb.append(ident).append("<instance id=\"").append(id).append("\">\n");    // <instance id = "234">
		sb.append(ident).append(ident).append("<location x=\"").append(location[0]).append("\" ");
		sb.append("y=\"").append(location[1]).append("\" ");
		sb.append("z=\"").append(location[2]).append("\"/>\n");
		sb.append(ident).append("</instance>");    
		return sb;
	}	
	
}
