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

import java.awt.Cursor;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;


public class NavMouseState extends MouseActionState {
		int lastX = 0, lastY = 0;
	
		protected void setMouseSymbol()
		{
			view.setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
		}
		
		public void leftPressed(MouseEvent arg0){
			lastX = arg0.getX();
			lastY = arg0.getY();
		}
		
		public void rightPressed(MouseEvent arg0){
			lastX = arg0.getX();
			lastY = arg0.getY();
		}
		
		public void leftDragged(MouseEvent arg0){
			double xd = arg0.getX() - lastX;
			double yd = arg0.getY() - lastY;
			view.displacementy -= yd/view.getMagnification();
			view.displacementx += xd/view.getMagnification();
			view.repaint();
			lastX = arg0.getX();
			lastY = arg0.getY();
			
//			System.out.println("view.displacementy = "+view.displacementy+";  view.displacementx = "+view.displacementx);
			
		}
		public void rightDragged(MouseEvent arg0){
			double xd = arg0.getX() - lastX;
			double yd = arg0.getY() - lastY;
			view.rotateAroundZ(xd * 0.005);
			view.rotateAroundX(yd * 0.005);
			view.repaint();
			lastX = arg0.getX();
			lastY = arg0.getY();
//			System.out.println("view.displacementy = "+view.displacementy+";  view.displacementx = "+view.displacementx);
			
		}
	

		public void mouseWheelMoved(MouseWheelEvent e) {
			int clicks =e.getWheelRotation();
			if (clicks < 0)
				for (int i = 0; i < Math.abs(clicks); i++)
				{
					
					view.increaseScalingFactor();
					
				}	
			else {
				for (int i = 0; i < Math.abs(clicks); i++)
				{	
					view.decreaseScalingFactor();
				
				}
				
			}
			view.repaint();
		}

		

}

	
	
