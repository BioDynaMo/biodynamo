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



/** 
 * Utility class for passing details of a Synapse to the XML export
 * @author Toby Weston & fredericzubler
 */

import ini.cx3d.cells.Cell;

public class ConnectionHolder{

		private static long nextSynapseID = 0;
		private  long synapseID = 0;
		private int preSynapticID;
		private int postSynapticID;
		
		private String preSynapticType;
		private String postSynapticType;

		 public  ConnectionHolder( int preSynapticID, int postSynapticID){
			 this.synapseID = nextSynapseID++;
			 this.preSynapticID = preSynapticID;
			 this.postSynapticID = postSynapticID;
			 this.preSynapticType = Cell.ExcitatoryCell;  // by default, it's excitatory
			 this.postSynapticType = Cell.ExcitatoryCell;
		 }

		 public  ConnectionHolder( int preSynapticID, int postSynapticID, String preSynapticType, String postSynapticType ){
			 this.synapseID = nextSynapseID++;
			 this.preSynapticID = preSynapticID;
			 this.postSynapticID = postSynapticID;
			 this.preSynapticType = preSynapticType;
			 this.postSynapticType = postSynapticType;
		 }

		public int getPostSynapticID() {
			return postSynapticID;
		}

		public void setPostSynapticID(int postSynapticID) {
			this.postSynapticID = postSynapticID;
		}

		public int getPreSynapticID() {
			return preSynapticID;
		}

		public void setPreSynapticID(int preSynapticID) {
			this.preSynapticID = preSynapticID;
		}

		public long getSynapseID() {
			return synapseID;
		}

		public String getPostSynapticType() {
			return postSynapticType;
		}

		public String getPreSynapticType() {
			return preSynapticType;
		}

		public StringBuilder toXML(String ident) {
			
			StringBuilder sb = new StringBuilder();
			sb.append(ident).append("<connection id=\"").append(synapseID).append("\">\n");    
			sb.append(ident+"   ").append("<pre cell_id=\"").append(preSynapticID).append("\"/>\n");  
			sb.append(ident+"   ").append("<post cell_id=\"").append(postSynapticID).append("\"/>\n");  
			sb.append(ident).append("</connection>");    
			return sb;
		}	

		public static void main(String[] args) {
			ConnectionHolder sh = new ConnectionHolder(14,60);
			System.out.println(sh.toXML("    "));
		}


	}
