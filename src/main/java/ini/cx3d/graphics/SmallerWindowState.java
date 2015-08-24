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

import static ini.cx3d.utilities.Matrix.matRotAroundX;
import static ini.cx3d.utilities.Matrix.matRotAroundZ;
import static ini.cx3d.utilities.Matrix.mult;
import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.physics.PhysicalSphere;
import ini.cx3d.simulations.ECM;

import java.awt.Cursor;
import java.awt.Rectangle;
import java.awt.event.MouseEvent;
import java.awt.event.MouseWheelEvent;

public class SmallerWindowState extends MouseActionState {
	
	int x0, x1, x2, z0, z1, z2;
	
	protected void setMouseSymbol()
	{
		view.setCursor(new Cursor(Cursor.CROSSHAIR_CURSOR));
	}
	
	
	public void leftPressed(MouseEvent e) {
		x0 = e.getX();
		z0 = e.getY();
	} 

	public void mouseReleased(MouseEvent e) {
		
			x1 = e.getX();
			z1 = e.getY();
			
			Rectangle smallWindowRectangle = new Rectangle();
			if(x0<x1){
				smallWindowRectangle.x = x0;
				smallWindowRectangle.width = x1-x0;
			}else{
				smallWindowRectangle.x = x1;
				smallWindowRectangle.width = x0-x1;
			}
			if(z0<z1){
				smallWindowRectangle.y = z0;
				smallWindowRectangle.height = z1-z0;
			}else{
				smallWindowRectangle.y = z1;
				smallWindowRectangle.height = z0-z1;
			}
			view.smallWindowRectangle = smallWindowRectangle;
			view.repaint();
			MouseActionState.mouseModeNav.EnableState(view);
			System.out.println("SmallerWindowState.mouseReleased() "+view.smallWindowRectangle);
	}

	

	
}
