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


import java.io.BufferedWriter;
import java.io.FileWriter;

import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.PhysicalSphere;
import ini.cx3d.simulations.ECM;

public class ExportToVisualizer {

	public static void Export(String filename)
	{
		try{
		    // Create file 
		FileWriter fstream = new FileWriter(filename);
		BufferedWriter out = new BufferedWriter(fstream);
		out.write("<Vexport>\n");
		for (PhysicalSphere s : ECM.getInstance().getPhysicalSphereList()) {
			out.write(toXML(s,"   ").toString());
		}
		out.write("</Vexport>");
		//Close the output stream
		out.close();
	}catch (Exception e){//Catch exception if any
	  System.err.println("Error: " + e.getMessage());
	}
		
		
		
	}

	public static StringBuilder toXML(PhysicalSphere s,String ident) {
		
		StringBuilder temp = new StringBuilder();
		temp.append(ident).append("<sphere id=\"").append(s.getID()).append("\" ");
		double [] k=s.getMassLocation();
		temp.append("x=\"").append(k[0]).append("\" ");
		temp.append("y=\"").append(k[1]).append("\" ");
		temp.append("z=\"").append(k[2]).append("\" ");
		temp.append("d=\"").append(s.getDiameter()).append("\" >\n");
		for (PhysicalCylinder c : s.getDaughters()) {
			temp.append(toXML(c,ident+"  "));
		}
		temp.append(ident).append("</sphere>\n");
		return temp;
	}	
	
	public static StringBuilder toXML(PhysicalCylinder c,String ident) {
		
		StringBuilder temp = new StringBuilder();
		temp.append(ident).append("<cylinder id=\"").append(c.getID()).append("\" ");
		double [] prox=c.proximalEnd();
		double [] distal=c.distalEnd();
		temp.append("x1=\"").append(prox[0]).append("\" ");
		temp.append("y1=\"").append(prox[1]).append("\" ");
		temp.append("z1=\"").append(prox[2]).append("\" ");
		temp.append("x2=\"").append(distal[0]).append("\" ");
		temp.append("y2=\"").append(distal[1]).append("\" ");
		temp.append("z2=\"").append(distal[2]).append("\" ");
		temp.append("d=\"").append(c.getDiameter()).append("\" >\n");
		if(c.getDaughterLeft()!=null)
		{
			temp.append(toXML(c.getDaughterLeft(), ident+"  "));
		}
		if(c.getDaughterRight()!=null)
		{
			temp.append(toXML(c.getDaughterRight(), ident+"  "));
		}
		temp.append(ident).append("</cylinder>\n");
		return temp;
	}	
	
	
}
