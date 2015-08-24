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



import java.util.Vector;


/**
 *  Utility class for passing details of a population to XML.
 * @author fredericzubler
 *
 */

       
public class PopulationHolder{
	
	private String name;
	private String cellType;
	private Vector <InstanceHolder> instances = new Vector<InstanceHolder>();
	

	public  PopulationHolder(String name, String cellType){
		 this.name = name;
		 this.cellType = cellType;
	 }
	
	 public  PopulationHolder(String name, String cellType, Vector <InstanceHolder> instances){
		 this.name = name;
		 this.cellType = cellType;
		 this.instances = instances;
	 }

	public Vector <InstanceHolder> getInstances() {
		return instances;
	}
	
	public void setCells(Vector <InstanceHolder> instances) {
		this.instances = instances;
	}

	public String getCellType() {
		return cellType;
	}

	public String getName() {
		return name;
	}

	public void addInstanceHolder(InstanceHolder ih){
		instances.add(ih);
	}
	
public StringBuilder toXML(String ident) {
		StringBuilder sb = new StringBuilder();
		// <population name="jgj" cell_type="tztr">
		sb.append(ident).append("<population name=\"").append(name).append("\" ");
		sb.append("cell_type=\"").append(cellType).append("\">\n");
		// 		<instances size ="14">.........................................
		sb.append(ident+"   ").append("<instances size=\"").append(instances.size()).append("\">\n");
		
		// all connections: 
		for (InstanceHolder ih : instances) {
			sb.append(ih.toXML(ident+"      "));
			sb.append("\n");
		}
		// 		</instances>.........................
		sb.append(ident+"   ").append("</instances>\n");
		// </population>
		sb.append(ident).append("</population>\n");
		return sb;
	}	

	
}