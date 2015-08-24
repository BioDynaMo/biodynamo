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

package ini.cx3d.graphics;


import ini.cx3d.localBiology.LocalBiologyModule;
import ini.cx3d.localBiology.NeuriteElement;
import ini.cx3d.localBiology.SomaElement;
import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.PhysicalSphere;

public class InfoVisiter {

	public ComplexDisplayNode VisitALL(Object o)
	{
		if(o instanceof PhysicalSphere)
		{
			return visit((PhysicalSphere)o);
		}
		else if(o instanceof PhysicalCylinder)
		{
			return visit((PhysicalCylinder)o);
		}
		else if (o instanceof SomaElement)
		{
			return visit((SomaElement)o);
		}
		else if(o instanceof NeuriteElement)
		{
			return visit((NeuriteElement)o);
		}
		return new ComplexDisplayNode();
	}
	
	private ComplexDisplayNode visit(PhysicalSphere o)
	{
		ComplexDisplayNode n = new ComplexDisplayNode();
		n.addSimpleDisplayNode("Adherence", o.getAdherence()+"");
		double  [] tempvec =o.getAxis();
		n.addSimpleDisplayNode("Axis", "["+tempvec[0]+","+tempvec[1]+","+tempvec[2]+"]");
		n.addSimpleDisplayNode( "Diameter", o.getDiameter()+"");
		n.addSimpleDisplayNode("Color", o.getColor().toString());
		n.addSimpleDisplayNode("Id", o.getID()+"");
		n.addCompelxDisplayNode("Soma:", visit(o.getSomaElement()));
		return n;
	}
	
	private ComplexDisplayNode visit(PhysicalCylinder o)
	{
		ComplexDisplayNode n = new ComplexDisplayNode();
		n.addSimpleDisplayNode("Adherence", o.getAdherence()+"");
		double  [] tempvec =o.getAxis();
		n.addSimpleDisplayNode("Axis", "["+tempvec[0]+","+tempvec[1]+","+tempvec[2]+"]");
		n.addSimpleDisplayNode( "Diameter", o.getDiameter()+"");
		n.addSimpleDisplayNode("Color", o.getColor().toString());
		n.addSimpleDisplayNode("Id", o.getID()+"");
		n.addCompelxDisplayNode("Soma:", visit(o.getNeuriteElement()));
		return n;
		
	}
	private ComplexDisplayNode visit(SomaElement o)
	{
//		ComplexDisplayNode n = new ComplexDisplayNode();
//		n.addSimpleDisplayNode("ref", o.hashCode()+"");
//		for (LocalBiologyModule i : o.getLocalBiologyModulesList()) {
//			if(i instanceof Machine)
//				n.addSimpleDisplayNode("Machine",((Machine)i).getName() );
//			else
//				n.addSimpleDisplayNode("moduleid",i.hashCode()+"");
//			
//		}
		
		return null;
		
	}
	private ComplexDisplayNode visit(NeuriteElement o)
	{
//		ComplexDisplayNode n = new ComplexDisplayNode();
//		n.addSimpleDisplayNode("ref", o.hashCode()+"");
//		for (LocalBiologyModule i : o.getLocalBiologyModulesList()) {
//			if(i instanceof Machine)
//				n.addSimpleDisplayNode("Machine",((Machine)i).getName() );
//			else
//				n.addSimpleDisplayNode("moduleid",i.hashCode()+"");
//			
//		}
		
		return null;
	}
	
}
