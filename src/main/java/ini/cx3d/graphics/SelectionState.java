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

import static ini.cx3d.utilities.Matrix.mult;
import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.SomaElement;
import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.physics.PhysicalObject;
import ini.cx3d.physics.PhysicalSphere;
import ini.cx3d.simulations.ECM;
import ini.cx3d.utilities.Matrix;

import java.awt.Color;
import java.awt.Cursor;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;


public class SelectionState extends MouseActionState{
		private Color c;
	
		protected void setMouseSymbol()
		{
			view.setCursor(new Cursor(Cursor.HAND_CURSOR));
		}
		private PhysicalObject p;
		public PhysicalNode getSelectedPhysicalObject()
		{
			return p;
		}
		public boolean isAphysicalObjectSelected()
		{
			return p!=null;
		}
		
		public void rightPressed(MouseEvent arg0)
		{
			if(!isAphysicalObjectSelected()) return;
			ShowInfo s = new ShowInfo();
			s.gatherInfo(p, 3);
			s.setVisible(true);
		}
		
		public void leftPressed(MouseEvent arg0){
			
			PhysicalObject newnode = findPhysicalNode(new double []{arg0.getX(),arg0.getY()});
			System.out.println(newnode);
			if(newnode == p) return;
			if(isAphysicalObjectSelected())
			{
				p.setColor(c);
			}
			p = newnode;
			if(!isAphysicalObjectSelected()) return;
			c = p.getColor();
			p.setColor(c.darker().darker());
			view.repaint();
		}
	
		private PhysicalObject findPhysicalNode(double[] actualCoord){
			
		
			PhysicalObject node= null;
			double distance = Double.MAX_VALUE;
			for (PhysicalNode pn : ECM.getInstance().physicalNodeList) {
				if(!pn.isAPhysicalObject()) continue;
				double[] coordOfPn = pn.getSoNode().getPosition();
				double ypn = coordOfPn[1];
				// if it is a slice view, we can exclude everyone outside of the slice
				if(	view.representationType == View.OM_SLICE_TYPE ||
					view.representationType == View.EM_SLICE_TYPE	
				){
					if( ypn>(view.sliceYPosition+15) || ypn<(view.sliceYPosition-15) ){
						continue;
					}
				}
				
				
				double [] mySomaMassLocation = Matrix.mult(view.V, coordOfPn);
				double [] newCoord = view.getDisplayCoord(mySomaMassLocation);
				
				double distancepn = Matrix.distance(newCoord, actualCoord);
				if(distancepn < distance){
					distance = distancepn;
					node = (PhysicalObject)pn;
				}
			}
			return node;
		}
		

}

	
	
