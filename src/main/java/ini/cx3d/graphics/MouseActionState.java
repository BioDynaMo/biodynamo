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
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;
import java.util.Vector;

import javax.swing.JComponent;

public abstract class MouseActionState implements MouseListener, MouseMotionListener, MouseWheelListener {
	public  static MouseActionState mouseModeNav = new NavMouseState();
	public  static SmallerWindowState mouseSmallerWindow = new SmallerWindowState();
	public  static SelectionState selectionState = new SelectionState();
	
	
	private static Vector<MouseActionState> existingStates = new Vector<MouseActionState>();
	protected View view;
	
	public void EnableState(View view)
	{
		this.view = view;
		existingStates.add(this);
		for (MouseActionState m : existingStates) {
			view.removeMouseListener(m);
			view.removeMouseMotionListener(m);
			view.removeMouseWheelListener(m);
		}
		view.addMouseListener(this);
		view.addMouseMotionListener(this);
		view.addMouseWheelListener(this);
		setMouseSymbol();
	}
	
	protected void setMouseSymbol()
	{
	
		
	}
	
	public void mouseClicked(MouseEvent arg0)
	{
		if((arg0.getButton() == MouseEvent.BUTTON1 || arg0.getModifiers() == 16) && arg0.getClickCount() ==1)
		{
			click(arg0);
		}
		else if ((arg0.getButton() == MouseEvent.BUTTON1 || arg0.getModifiers() == 16) && arg0.getClickCount() ==2)
		{
			doubleClick(arg0);
		}
		else if((arg0.getButton() == MouseEvent.BUTTON3 || arg0.getModifiers() == 4))
		{
			rightClick(arg0);
		}
	}	

	public  void doubleClick(MouseEvent arg0){}
	public  void click(MouseEvent arg0){}
	public  void rightClick(MouseEvent arg0){}
	
	public void mouseEntered(MouseEvent arg0){}

	public void mouseExited(MouseEvent arg0){}

	public void mousePressed(MouseEvent arg0)
	{
		if(arg0.getButton() == MouseEvent.BUTTON1 || arg0.getModifiers() == 16)
		{
			leftPressed(arg0);
		}
		else if(arg0.getButton() == MouseEvent.BUTTON3 || arg0.getModifiers() == 4)
		{
			rightPressed(arg0);
			
		}
	}
	public void leftPressed(MouseEvent arg0){}
	public void rightPressed(MouseEvent arg0){}

	
	public void mouseReleased(MouseEvent arg0)
	{
		if(arg0.getButton() == MouseEvent.BUTTON1 || arg0.getModifiers() == 16)
		{
			leftReleased(arg0);
		}
		else if(arg0.getButton() == MouseEvent.BUTTON3 || arg0.getModifiers() == 4)
		{
			rightReleased(arg0);
			
		}
	}
	public void leftReleased(MouseEvent arg0){};
	public void rightReleased(MouseEvent arg0){};

	public  void mouseDragged(MouseEvent arg0)
	{
	
		if(arg0.getButton() == MouseEvent.BUTTON1 || arg0.getModifiers() == 16)
		{
			leftDragged(arg0);
		}
		else if(arg0.getButton() == MouseEvent.BUTTON3 || arg0.getModifiers() == 4)
		{
			rightDragged(arg0);
			
		}
	}
	public void leftDragged(MouseEvent arg0){};
	public void rightDragged(MouseEvent arg0){};
	
	public void mouseMoved(MouseEvent arg0){}

	public void mouseWheelMoved(MouseWheelEvent arg0){}

}
