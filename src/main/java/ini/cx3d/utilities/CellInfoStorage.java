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

package ini.cx3d.utilities;

import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.SomaElement;
import ini.cx3d.physics.Substance;
import ini.cx3d.simulations.ECM;

import java.awt.Color;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Vector;

public class CellInfoStorage implements Serializable
{
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	double [] cellpos;
	double diameter = 10;
	double mass = 10;
	double ioforce = 10;
	double adherence = 10;
	Color c;
	transient Cell cell; 
	HashMap<String, Substance> quantity = new HashMap<String, Substance>();
	
	
	public static void saveallcells(String name)
	{
		try {
		      FileOutputStream fout = new FileOutputStream(name);
		      ObjectOutputStream oos = new ObjectOutputStream(fout);
		      for (Cell c : ECM.getInstance().getCellList()) {
		    	  CellInfoStorage cont = new CellInfoStorage();
		    	  cont.cellpos = c.getSomaElement().getLocation();
		    	  cont.c = c.getSomaElement().getPhysical().getColor();
		    	  cont.diameter = c.getSomaElement().getPhysical().getDiameter();
		    	  cont.adherence = c.getSomaElement().getPhysical().getAdherence();
		    	  cont.ioforce = c.getSomaElement().getPhysical().getInterObjectForceCoefficient();
		    	  cont.mass = c.getSomaElement().getPhysical().getMass();
		    	  Hashtable<String, Substance> substances = c.getSomaElement().getPhysical().getExtracellularSubstances();
		    	  for (String s: substances.keySet()) {
					cont.quantity.put(s,substances.get(s));
		    	  }
		    	  oos.writeObject(cont);
		      }
		      oos.close();
		    }
		 catch (Exception e) { e.printStackTrace(); }
		
		
	}
	
	public static void loadcells(String name)
	{
		Vector<CellInfoStorage> containers = new Vector<CellInfoStorage>();
		try {
		    FileInputStream fin = new FileInputStream(name);
		    ObjectInputStream ois = new ObjectInputStream(fin);
		    Object o;
		    while((o = ois.readObject())!= null)
		    {
		    	CellInfoStorage cont  = (CellInfoStorage) o;
		    	containers.add(cont);
		    }
		    ois.close();
		   	}
		  catch (Exception e) { e.printStackTrace(); }
			 for (CellInfoStorage container : containers) {
			 Cell cell = CellFactory.getCellInstance(container.cellpos);
			 container.cell =cell;
			 SomaElement soma = cell.getSomaElement();
			 cell.getSomaElement().getPhysical().setColor(container.c);
			 soma.getPhysicalSphere().setMass(container.mass);
			 soma.getPhysicalSphere().setDiameter(container.diameter);
			 soma.getPhysicalSphere().setAdherence(container.adherence); 
			 soma.getPhysicalSphere().setInterObjectForceCoefficient(container.ioforce);
		
		 }

		 for (CellInfoStorage container : containers) {
			 SomaElement soma = container.cell.getSomaElement();
			 for (String s : container.quantity.keySet()) {
				 	Substance sub = container.quantity.get(s);
//				 	sub.setQuantity(9999999910000000000.0);
//				 	sub.setConcentration(1);
//					soma.getPhysical().modifyExtracellularQuantity(s, container.quantity.get(s).doubleValue()/Param.SIMULATION_TIME_STEP);
					soma.getPhysical().getExtracellularSubstances().put(s, sub);
//					System.out.println(soma.getPhysical().getExtracellularConcentration(s));
//					System.out.println(sub +" / "+sub.getQuantity());
			 }
		 }
			
		 
	}
}