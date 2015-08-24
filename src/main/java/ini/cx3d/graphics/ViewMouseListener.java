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

import java.awt.Rectangle;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.MouseWheelEvent;
import java.awt.event.MouseWheelListener;




public class ViewMouseListener implements MouseListener, MouseMotionListener, MouseWheelListener {

	ECM ecm = ECM.getInstance();
	PhysicalNode physicalSpace;
	View view;
	boolean aNodeIsSelected;
	int lastX = 0, lastY = 0;
	boolean rightMouseButtonPressed = false;
	boolean leftMouseButtonPressed = false;
	int x0, x1, x2, z0, z1, z2, y0;
	
	final static int SELECT_SMALLER_WINDOW = 0;
	final static int SELECT_A_PHYSICAL_NODE = 1;
	final static int SELECT_A_PHYSICAL_SPHERE = 2;
	final static int THREE_D_NAVIGATION = 3;
	
	int mouseOperation = THREE_D_NAVIGATION;
	
	
	public ViewMouseListener(View view){
		this.view = view;
	}
	
	public void mouseClicked(MouseEvent e) {
		// get the screen position :
		x0 = e.getX();
		y0 = e.getY();
		// get the real x-1-z coord // x-y coord
		double[] realCoord = findActualCoordinates(x0,y0);
		// A) if we want to select a node
		if(mouseOperation == SELECT_A_PHYSICAL_NODE){
			PhysicalNode node = findPhysicalNode(realCoord);
			if(node!= null){
				//Roman			
				System.out.println("concentration (A): " + node.getExtracellularConcentration("A") + " volume (A): " + node.getSoNode().getVolume());
				System.out.println("concentration (B): " + node.getExtracellularConcentration("B"));
				System.out.println("x-y-coordinates: " + realCoord[0] + ", " + realCoord[1]);
			}
			else System.out.println("No node");

			
		}
		// B) if we want to select a PhysicalSphere
		if(mouseOperation == SELECT_A_PHYSICAL_SPHERE){
			PhysicalSphere physicalSphere = findPhysicalSphere(realCoord);
			if(physicalSphere!= null){
				//Roman			
				System.out.println("concentration (A): " + physicalSphere.getExtracellularConcentration("A") + " volume (A): " + physicalSphere.getSoNode().getVolume());
				System.out.println("concentration (B): " + physicalSphere.getExtracellularConcentration("B"));
				System.out.println("x-y-coordinates: " + realCoord[0] + ", " + realCoord[1]);
			}
			else System.out.println("No node");

				
				
				

//				Color randomColor = new Color((float) Math.random(), (float) Math.random(),(float) Math.random());
//				physicalSphere.setColor(randomColor);
//				
//				SomaElement soma = physicalSphere.getSomaElement();
//				// if RIGHT_BUTTON CLICK : 
//				if(e.getButton()==e.BUTTON1){
//		//			new ProbePanel(somaElement.getCell(somaElement.getCell())); 
//					try {
//						ProbeUtilities.probe(soma.getCell());
//					} catch (Exception ef) {
//						ef.printStackTrace();
//					}
//				}
//				view.repaint();
//				// if RIGHT_BUTTON CLICK test to see what cells are in contact 
//				if(true && e.getButton()==e.BUTTON3){
//					TestDiffusion.ps = physicalSphere;
//					if(TestDiffusion.type_of_chemical==TestDiffusion.MEINHARDT_GIERER){
//						System.out.println("-----------------------------------------");
//						System.out.println("Volume : "+physicalSphere.getSoNode().getVolume());
//						System.out.println("A = "+physicalSphere.getExtracellularConcentration("A")+"   (quantity: "+physicalSphere.getExtracellularSubstances().get("A").getQuantity());
//						System.out.println("H = "+physicalSphere.getExtracellularConcentration("B")+"   (quantity: "+physicalSphere.getExtracellularSubstances().get("B").getQuantity());
//						
//					}else if(TestDiffusion.type_of_chemical==TestDiffusion.DELTA_NOTCH){
//						System.out.println("Delta = "+physicalSphere.getExtracellularConcentration("B"));
//						System.out.println("Notch = "+physicalSphere.getExtracellularConcentration("A"));
//						for (PhysicalNode neigh : physicalSphere.getSoNode().getNeighbors()) {
//							if(neigh instanceof PhysicalSphere){
//								if(physicalSphere.isInContact((PhysicalSphere) neigh)){
//									((PhysicalSphere) neigh).setColor(physicalSphere.getColor());
//								}
//							}
//						}
//					}

			
//				SomaElement soma = physicalSphere.getSomaElement();
//				// if RIGHT_BUTTON CLICK : 
//				if(e.getButton()==e.BUTTON1){
//		//			new ProbePanel(somaElement.getCell(somaElement.getCell())); 
//					try {
//						ProbeUtilities.probe(soma.getCell());
//					} catch (Exception ef) {
//						ef.printStackTrace();
//					}
//				}
//				view.repaint();
//				// if RIGHT_BUTTON CLICK test to see what cells are in contact 
//				if(true && e.getButton()==e.BUTTON3){
//					TestDiffusion.ps = physicalSphere;
//					if(TestDiffusion.type_of_chemical==TestDiffusion.MEINHARDT_GIERER){
//						System.out.println("-----------------------------------------");
//						System.out.println("Volume : "+physicalSphere.getSoNode().getVolume());
//						System.out.println("A = "+physicalSphere.getExtracellularConcentration("A")+"   (quantity: "+physicalSphere.getExtracellularSubstances().get("A").getQuantity());
//						System.out.println("H = "+physicalSphere.getExtracellularConcentration("B")+"   (quantity: "+physicalSphere.getExtracellularSubstances().get("B").getQuantity());
//						
//					}else if(TestDiffusion.type_of_chemical==TestDiffusion.DELTA_NOTCH){
//						System.out.println("Delta = "+physicalSphere.getExtracellularConcentration("B"));
//						System.out.println("Notch = "+physicalSphere.getExtracellularConcentration("A"));
//						for (PhysicalNode neigh : physicalSphere.getSoNode().getNeighbors()) {
//							if(neigh instanceof PhysicalSphere){
//								if(physicalSphere.isInContactWithSphere((PhysicalSphere) neigh)){
//									((PhysicalSphere) neigh).setColor(physicalSphere.getColor());
//								}
//							}
//						}
//					}
//				}
			}

	}

	
	
	public void mousePressed(MouseEvent e) {

		if (e.getButton() == MouseEvent.BUTTON3) {
			lastX = e.getX();
			lastY = e.getY();
			rightMouseButtonPressed = true;
		}
		if (e.getButton() == MouseEvent.BUTTON1) {
			lastX = e.getX();
			lastY = e.getY();
			leftMouseButtonPressed = true;
		}
		if(mouseOperation == SELECT_SMALLER_WINDOW){
			x0 = e.getX();
			z0 = e.getY();
		}
	} 

	public void mouseReleased(MouseEvent e) {
		if (e.getButton() == MouseEvent.BUTTON3) {
			rightMouseButtonPressed = false;
		}
		if (e.getButton() == MouseEvent.BUTTON1) {
			leftMouseButtonPressed = false;
		}
		if(mouseOperation == SELECT_SMALLER_WINDOW){
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
		}
		
		
	}

	/**
	 * Reverts the process in View, going from display coordinates to the 
	 * actual coordinates in the simulation space (ECM). Doesn't work if the 
	 * perspective is turned on.
	 * @param x0
	 * @param z0
	 * @return {real X value, 0, real Z value}
	 */
	private double[] findActualCoordinates(double x0, double z0){
		// 1) Remove the display shift
		x0 -= view.displacementx;
		z0 -= view.displacementy;
		// 2) Divide by the magnification
		x0 /= view.getMagnification();
		z0 /= view.getMagnification();
		// 3) Invert direction of vertical axis
		z0 = -z0;
		// 4) Rotation (reverse order and reverse direction)
		double[][] V = mult( matRotAroundZ(-view.alphaAroundZ), matRotAroundX(-view.alphaAroundX) );
		double[] realCoord = mult(V, new double[] {x0, 0, z0} );
		
		return realCoord;
	}

	/** 
	 * Returns the PhysicalNode the closest to the value x and z of the arguments
	 * @param actualCoord
	 * @return
	 */
	private PhysicalNode findPhysicalNode(double[] actualCoord){
		double x = actualCoord[0];
		double z = actualCoord[2];
		double distance = 20;
		PhysicalNode node = null;
		for (PhysicalNode pn : ecm.physicalNodeList) {
			double[] coordOfPn = pn.getSoNode().getPosition();
			double xpn = coordOfPn[0];
			double ypn = coordOfPn[1];
			double zpn = coordOfPn[2];
			// if it is a slice view, we can exclude everyone outside of the slice
			if(	view.representationType == View.OM_SLICE_TYPE ||
				view.representationType == View.EM_SLICE_TYPE	
			){
				if( ypn>(view.sliceYPosition+15) || ypn<(view.sliceYPosition-15) ){
					continue;
				}
			}
			// we choose the one that has the closest euclidian distance in the x-z plane
			double distancepn = (x-xpn)*(x-xpn) + (z-zpn)*(z-zpn);
			if(distancepn < distance){
				distance = distancepn;
				node = pn;
			}
		}
		return node;
	}
	

	/** 
	 * Returns the PhysicalSphere the closest to the value x and z of the arguments
	 * @param actualCoord
	 * @return
	 */
	private PhysicalSphere findPhysicalSphere(double[] actualCoord){
		double x = actualCoord[0];
		double z = actualCoord[2];
		double distance = 20;
		PhysicalSphere sphere = null;
		for (PhysicalSphere ps : ecm.physicalSphereList) {
			double[] coordOfPs = ps.getMassLocation();
			double xps = coordOfPs[0];
			double yps = coordOfPs[1];
			double zps = coordOfPs[2];
			// if it is a slice view, we can exclude everyone outside of the slice
			if(	view.representationType == View.OM_SLICE_TYPE ||
				view.representationType == View.EM_SLICE_TYPE	
			){
				double radius = ps.getDiameter()*0.5;
				if( yps>(view.sliceYPosition+15+radius) || yps<(view.sliceYPosition-15-radius) ){
					continue;
				}
			}
			// we choose the one that has the closest euclidian distance in the x-z plane
			double distanceps = (x-xps)*(x-xps) + (z-zps)*(z-zps);
			if(distanceps < distance){
				distance = distanceps;
				sphere = ps;
			}
		}
		return sphere;
	}
	
	public void mouseEntered(MouseEvent arg0) {
		// TODO Auto-generated method stub
	}

	public void mouseExited(MouseEvent arg0) {
		// TODO Auto-generated method stub
	}

	public void mouseDragged(MouseEvent e) {
		double xd = e.getX() - lastX;
		double yd = e.getY() - lastY;
		if (rightMouseButtonPressed && mouseOperation == THREE_D_NAVIGATION) {
			view.rotateAroundZ(xd * 0.005);
			view.rotateAroundX(yd * 0.005);
		}
		if (leftMouseButtonPressed && mouseOperation == THREE_D_NAVIGATION) {
			view.displacementy += yd;
			view.displacementx += xd;
			view.repaint();
		}
		lastX = e.getX();
		lastY = e.getY();
	}

	public void mouseMoved(MouseEvent e) {
		// TODO Auto-generated method stub
	}

    /**
     * @return the mouseOperation
     */
    public int getMouseOperation() {
        return mouseOperation;
    }

    /**
     * @param mouseOperation the mouseOperation to set
     */
    public void setMouseOperation(int mouseOperation) {
        this.mouseOperation = mouseOperation;
    }

	public void mouseWheelMoved(MouseWheelEvent e) {
		int clicks =e.getWheelRotation();
		if (clicks < 0)
			for (int i = 0; i < Math.abs(clicks); i++)
			{
				
				ecm.view.increaseScalingFactor();
				
			}	
		else {
			for (int i = 0; i < Math.abs(clicks); i++)
			{	
				ecm.view.decreaseScalingFactor();
			
			}
			
		}
		ecm.view.repaint();
	}

	

}
