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

import java.util.*;


/**
 * Utility class for storing details on a bunch of connections for the XML export. 
 * @author fredericzubler
 *
 */


public class ProjectionHolder {

	String name;
	String source;
	String target;
	String synapse_type;
	String prop_delay = "5.0e-3";
	double weight = 1.0;
	
	Vector<ConnectionHolder> connections = new Vector<ConnectionHolder>(); 
	
	ProjectionHolder(){}

	public ProjectionHolder(String name, String source, String target, String synapse_type) {
		super();
		this.name = name;
		this.source = source;
		this.target = target;
		this.synapse_type = synapse_type;
	}
	
	public void addConnectionHolder(ConnectionHolder ch){
		connections.add(ch);
	}
	
	public StringBuilder toXML(String ident) {
		
		StringBuilder sb = new StringBuilder();
		// <projection name="jgj" source="tztr" target="fds">
		sb.append(ident).append("<projection name=\"").append(name).append("\" ");
		sb.append("source=\"").append(source).append("\" ");
		sb.append("target=\"").append(target).append("\">\n");
		// 		<synapse_props synapse_type="jgj" prop_delay="tztr" weight="fds">
		// 		</synapse_props
		sb.append(ident+"   ").append("<synapse_props synapse_type=\"").append(synapse_type).append("\" ");
		sb.append("prop_delay=\"").append(prop_delay).append("\" ");
		sb.append("weight=\"").append(weight).append("\">\n");
		sb.append(ident+"   ").append("</synapse_props>\n");
		// 		<connections>.........................
		sb.append(ident+"   ").append("<connections>\n");
		// all connections: 
		for (ConnectionHolder ch : connections) {
			sb.append(ch.toXML(ident+"      "));
			sb.append("\n");
		}
		// 		</connections>.........................
		sb.append(ident).append(ident).append("</connections>\n");
		// </projections>
		sb.append(ident).append("</projection>\n");
		return sb;
	}	
	
	public static void main(String[] args) {
		ProjectionHolder ph = new ProjectionHolder("Excite_to_Excit", "inhibitory_cells", "Excitatory_cells", "Inh_Syn");
		ph.connections.add(new ConnectionHolder(14,60));
		ph.connections.add(new ConnectionHolder(32,4));
		System.out.println(ph.toXML("  "));
	}
	
	// Roman Bauer: Implement getters and setters to calculate connectivity pattern
	public Vector<ConnectionHolder> getConnections() {
		return this.connections;
	}
	
	public String getName() {
		return this.name;
	}
	
	public void setConnections(Vector<ConnectionHolder> c) {
		this.connections = c;
	}
	
	public void setName(String s) {
		this.name = s;
	}
	
	public int getNrOfConnections() {
		return connections.size();
	}
	// Roman Bauer

}