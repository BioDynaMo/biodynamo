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
import static ini.cx3d.utilities.Matrix.normalize;
import static ini.cx3d.utilities.Matrix.printlnLine;
import static ini.cx3d.utilities.Matrix.scalarMult;
import static ini.cx3d.utilities.Matrix.subtract;
import ini.cx3d.Param;
import ini.cx3d.physics.IntracellularSubstance;
import ini.cx3d.physics.PhysicalBond;
import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.physics.PhysicalSphere;
import ini.cx3d.physics.Substance;
import ini.cx3d.simulations.ECM;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
import ini.cx3d.synapses.Excrescence;
import ini.cx3d.utilities.StringUtilities;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.GradientPaint;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Stroke;
import java.awt.geom.Ellipse2D;
import java.awt.geom.Line2D;
import java.awt.geom.Rectangle2D;
import java.util.Collections;
import java.util.Hashtable;
import java.util.Vector;

import javax.swing.JComponent;

public class View extends JComponent {

	// **************************************************************************
	// Defining some parameters
	// **************************************************************************
	ECM ecm = ECM.getInstance();

	// 1. some temporary variables
	Color colorOfThePhysicalSpaces = new Color(100, 100, 100, 100);
	double[] newCoord;
	double[] coord1;
	double[] coord2;
	Stroke oneStroke, twoStroke, threeStroke, fourStroke, fiveStroke,
			sixStroke, sevenStroke, eightStroke, nineStroke, tenStroke,
			twelveStroke, fifteenStroke, twentyStroke, thirtyStroke,
			fourtyStroke, fiftyStroke;

	// 2. definition of some values
	public final static int PROJECTION_TYPE = 0;
	public final static int OM_SLICE_TYPE = 1;
	public final static int EM_SLICE_TYPE = 2;

	// 3. modified by the GUI
	boolean drawit = true;
	private double magnification = 1; // .4
	double diameter = 10 * getMagnification();
	public double displacementx = 0;
	public double displacementy = 0;
	public int representationType = PROJECTION_TYPE;
	public boolean perspective = false;
	public boolean drawDelaunayNeighbors = false;
	public boolean drawDelaunayVertices = false;
	public boolean drawPointMass = false;
	public boolean drawScaleBar = true;
	public boolean drawPhysicalBonds = false;
	public boolean drawSpines = false;
	public boolean drawPaleCells = true;
	public boolean drawForces = false;
	// should be modified by GUI :
	public boolean drawEffectiveSphereRadius = false;
	public double addedToTheRadius = 1.5;
	public boolean Shownumbers = false;

	// The smaller sub-window that is saved in the dumpImage() method of ECM
	// public Rectangle smallWindowRectangle = new Rectangle(100,100,240,320);
	public Rectangle smallWindowRectangle = new Rectangle(10000, 10000, 1, 1);
	// public Rectangle smallWindowRectangle = new Rectangle(100,100,480,640);
	public boolean drawSmallWindowRectangle = true;

	// for projection type
	double alphaAroundZ = 0.0;
	double alphaAroundX = 0.0;
	double rotationSpeed = 0.005;
	double a = 100; // distance eye-projection plane
	double b = 900; // distance projection plane - plane y=0
	double delta = a / (a + b); // the correction FORCE_FACTOR for an object at
	// y=0
	double[][] V;
	double[][] VrotZ;
	double[][] VrotX;
	// for slice type
	public static double OM_SLICE_THICKNESS = 10;
	public static double EM_SLICE_THICKNESS = 0.5;
	public double sliceThickness = OM_SLICE_THICKNESS;
	public double sliceYPosition = 0.5;
	public boolean drawChemical;

	// for chemicals
	// public String chemicalType = Param.VENTRICULAR_ZONE;
	public boolean drawChemicals = false;
	public static int chemicalDrawFactor = 10;
	// for the Artificial chemicals
	public boolean drawA = false;
	public boolean drawB = false;
	public boolean drawC = false;
	public boolean drawD = false;
	public boolean drawE = false;
	public boolean drawF = false;
	public boolean drawTOP = false;

	private double maxXdrawn = -Double.MAX_VALUE;
	private double maxZdrawn = -Double.MAX_VALUE;
	private double minXdrawn = Double.MAX_VALUE;
	private double minZdrawn = Double.MAX_VALUE;

	private Vector<Substance> tobeDrawn = new Vector<Substance>();

	// Check if object dispay have to be sorted for a more realistic 3D view
	private boolean sortDraw = false;

	// **************************************************************************
	// Constructor
	// **************************************************************************
	public View() {
		setPreferredSize(new Dimension(2000, 2000));
		// Some initialization for the projection representation
		V = new double[3][3];
		VrotX = new double[3][3];
		VrotZ = new double[3][3];
		for (int i = 0; i < 3; i++) {
			V[i][i] = 1;
			VrotX[i][i] = 1;
			VrotZ[i][i] = 1;
		}
	}

	// **************************************************************************
	// paint() & friends
	// **************************************************************************

	public void paint(Graphics g) {
		if (!drawit)
			return;
		Graphics2D g2D = (Graphics2D) g;
		g2D.setRenderingHint(RenderingHints.KEY_ANTIALIASING,
				RenderingHints.VALUE_ANTIALIAS_ON);
		// thickness of the line ...........................................
		oneStroke = g2D.getStroke();
		twoStroke = new BasicStroke(2);
		threeStroke = new BasicStroke(3);
		fourStroke = new BasicStroke(4);
		fiveStroke = new BasicStroke(5);
		sixStroke = new BasicStroke(6);
		sevenStroke = new BasicStroke(7);
		eightStroke = new BasicStroke(8);
		nineStroke = new BasicStroke(9);
		tenStroke = new BasicStroke(10);
		twelveStroke = new BasicStroke(12);
		fifteenStroke = new BasicStroke(15);
		twentyStroke = new BasicStroke(20);
		thirtyStroke = new BasicStroke(30);
		fourtyStroke = new BasicStroke(40);
		fiftyStroke = new BasicStroke(50);

		g.setColor(Color.WHITE);
		g.fillRect(0, 0, getWidth(), getHeight());

		// draw the artificial chemical
		for (Substance s : tobeDrawn) {
			drawSubstance(g2D, s);
		}

		// draw the scale bar
		if (drawScaleBar) {
			double temp;
			if (getMagnification() >= 1) {
				temp = Math.floor(getMagnification());
			} else {
				temp = 0.5;
				if (getMagnification() < 0.5)
					temp = 0.25;
				if (getMagnification() < 0.25)
					temp = 0.125;
			}
			double lengthName = 100.0 / temp;
			double length = lengthName * getMagnification();

			g2D.setStroke(oneStroke);
			g2D.setPaint(Color.black);
			Line2D.Double l = new Line2D.Double(smallWindowRectangle.x + 10,
					smallWindowRectangle.y + 100, smallWindowRectangle.x + 10
							+ length, smallWindowRectangle.y + 100);
			g2D.drawString(StringUtilities.doubleToString(lengthName, 2)
					+ " microns", smallWindowRectangle.x + 10,
					smallWindowRectangle.y + 70);
			g2D.draw(l);
		}

		// Loop through all Spheres
		paintPhysicalSpheres(g2D);

		// Loop through all Cylinders
		paintPhysicalCylinders(g2D);

		// Loop through all PhysicalNodes
		paintPhysicalNodes(g2D);

		// draw the rectangle used for saving snapshots
		g2D.setColor(Color.black);
		if (drawSmallWindowRectangle) {
			g2D.draw(new Rectangle2D.Double(smallWindowRectangle.x - 1,
					smallWindowRectangle.y - 1, smallWindowRectangle.width + 2,
					smallWindowRectangle.height + 2));
		}
		g2D.setColor(Color.black);
	}

	private void paintPhysicalCylinders(Graphics2D g2D) {

		// modified by sabina: uncheck this code to sort the physicalSpheres
		// according to the viewer position.
		if (this.sortDraw) {
			Collections.sort(ecm.physicalCylinderList,
					new SortPhysicalObjects());
		}

		for (int i = 0; i < ecm.physicalCylinderList.size(); i++) {
			PhysicalCylinder aCylinder = ecm.physicalCylinderList.get(i);
			double[] myNeuriteDistalEnd = aCylinder.distalEnd(); // =
			// massLocation
			double[] myNeuriteProximalEnd = aCylinder.proximalEnd();
			myNeuriteDistalEnd = mult(V, myNeuriteDistalEnd); // rotation
			myNeuriteProximalEnd = mult(V, myNeuriteProximalEnd); // rotation
			double x0, y0, x1, y1;
			double[] newCoord;
			newCoord = getDisplayCoord(myNeuriteDistalEnd);
			// find the min max cords for exporting a picture!
			minXdrawn = Math.min(minXdrawn, newCoord[0]);
			maxXdrawn = Math.max(maxXdrawn, newCoord[0]);
			minZdrawn = Math.min(minZdrawn, newCoord[1]);
			maxZdrawn = Math.max(maxZdrawn, newCoord[1]);
			x0 = newCoord[0];
			y0 = newCoord[1];

			// ***chagned by haurian: get the drawing color form the internally
			// secreted stuff
			// if it has been selected to be drawn otherwise use the standard of
			// the object
			Color drawcolor = aCylinder.getColor();
			for (Substance sub : tobeDrawn) {
				if (sub instanceof IntracellularSubstance) {
					if (aCylinder.getIntracellularSubstances().containsKey(
							sub.getId())) {
						drawcolor = aCylinder.getIntracellularSubstances().get(
								sub.getId()).getConcentrationDependentColor();
						if (aCylinder.getNeuriteElement()
								.getLocalBiologyModulesList().size() > 0) {

							Color col = g2D.getColor();
							g2D.setColor(Color.black);
							paintrealSubstance(g2D, aCylinder
									.getIntracellularSubstances().get(
											sub.getId()), newCoord);
							g2D.setColor(col);
						}

					}
					paintrealSubstanceShowNumberDependant(g2D, aCylinder
							.getIntracellularSubstances().get(sub.getId()),
							getDisplayCoord(myNeuriteDistalEnd));
				}
			}

			// end haurian changes

			// in case of slice representation type, we skip this cylinder if
			// the 2 ends are before or after the slice
			if ((representationType != PROJECTION_TYPE)
					&& ((myNeuriteDistalEnd[1] > sliceYPosition
							+ sliceThickness / 2.0 && myNeuriteProximalEnd[1] > sliceYPosition
							+ sliceThickness / 2.0) || (myNeuriteDistalEnd[1] < sliceYPosition
							- sliceThickness / 2.0 && myNeuriteProximalEnd[1] < sliceYPosition
							- sliceThickness / 2.0))) {
				continue;
			}

			// a few temporary local variables
			Ellipse2D.Double E1;
			Line2D.Double L;

			// links to the neighboring vertices in the
			// triangulation..........................
			// (only if drawDelaunayNeighbors== true)
			if (drawDelaunayNeighbors) {
				paintDelaunayNeighbors(g2D, aCylinder.getColor(), aCylinder
						.getSoNode());
			}

			// The cylinder point mass
			// ......................................................
			g2D.setPaint(drawcolor);

			if (drawPointMass == true) {
				double massDiameter = aCylinder.getDiameter()
						* getMagnification()
						* getScaleFactor(myNeuriteProximalEnd);
				massDiameter = Math.min(massDiameter, 10);
				E1 = new Ellipse2D.Double(x0 - massDiameter, y0 - massDiameter,
						2 * massDiameter, 2 * massDiameter);
				g2D.fill(E1);
			}

			// The cylinder itself
			// ..........................................................
			if (representationType != PROJECTION_TYPE) {
				double[][] twoEndsOfTheNeurite = getCylinderApparentEndPoints(
						myNeuriteDistalEnd, myNeuriteProximalEnd);
				newCoord = twoEndsOfTheNeurite[0];
				newCoord = getDisplayCoord(newCoord);
				x0 = newCoord[0];
				y0 = newCoord[1];
				newCoord = twoEndsOfTheNeurite[1];
				newCoord = getDisplayCoord(newCoord);
				x1 = newCoord[0];
				y1 = newCoord[1];
			} else {
				newCoord = getDisplayCoord(myNeuriteProximalEnd);
				x1 = newCoord[0];
				y1 = newCoord[1];
			}

			double thickness = aCylinder.getDiameter() * getMagnification()
					* getScaleFactor(myNeuriteProximalEnd);
			chooseCorrectLineThickness(thickness, g2D);
			L = new Line2D.Double(x0, y0, x1, y1);
			g2D.setPaint(drawcolor);
			g2D.draw(L);

			// if excrescences, we also draw them .............................
			if (drawSpines) {
				for (Excrescence ex : aCylinder.getExcrescences()) {
					double[] proximalExEnd = mult(V, ex.getProximalEnd()); // rotation
					double[] distalExEnd = mult(V, ex.getDistalEnd());
					if (representationType != PROJECTION_TYPE) {
						double[][] twoEndsOfTheNeurite = getCylinderApparentEndPoints(
								proximalExEnd, distalExEnd);
						newCoord = twoEndsOfTheNeurite[0];
						newCoord = getDisplayCoord(newCoord);
						x0 = newCoord[0];
						y0 = newCoord[1];
						newCoord = twoEndsOfTheNeurite[1];
						newCoord = getDisplayCoord(newCoord);
						x1 = newCoord[0];
						y1 = newCoord[1];
					} else {
						newCoord = getDisplayCoord(proximalExEnd);
						x0 = newCoord[0];
						y0 = newCoord[1];
						newCoord = getDisplayCoord(distalExEnd);
						x1 = newCoord[0];
						y1 = newCoord[1];
					}
					thickness = 1.0 * getMagnification()* getScaleFactor(myNeuriteProximalEnd);
					chooseCorrectLineThickness(thickness, g2D);
					L = new Line2D.Double(x0, y0, x1, y1);
					g2D.draw(L);
				}

			}
			// if PhysicalBonds, we also draw them .............................
			if (drawPhysicalBonds) {
				for (PhysicalBond pb : aCylinder.getPhysicalBonds()) {
					double[] firstEnd = mult(V, pb.getFirstEndLocation()); // rotation
					double[] secondEnd = mult(V, pb.getSecondEndLocation());
					if (representationType != PROJECTION_TYPE) {
						double[][] twoEndsOfTheNeurite = getCylinderApparentEndPoints(
								firstEnd, secondEnd);
						newCoord = twoEndsOfTheNeurite[0];
						newCoord = getDisplayCoord(newCoord);
						x0 = newCoord[0];
						y0 = newCoord[1];
						newCoord = twoEndsOfTheNeurite[1];
						newCoord = getDisplayCoord(newCoord);
						x1 = newCoord[0];
						y1 = newCoord[1];
					} else {
						newCoord = getDisplayCoord(firstEnd);
						x0 = newCoord[0];
						y0 = newCoord[1];
						newCoord = getDisplayCoord(secondEnd);
						x1 = newCoord[0];
						y1 = newCoord[1];
					}
					L = new Line2D.Double(x0, y0, x1, y1);
					g2D.setColor(Param.X_SOLID_GRAY);
					g2D.setColor(Color.black);
					g2D.draw(L);
				}
			}

			// the Force acting on the somaElement ......................
			double[] forceOnACylinder = aCylinder.getTotalForceLastTimeStep();
			if (drawForces && forceOnACylinder[3] > 0) {
				newCoord = getDisplayCoord(myNeuriteDistalEnd);
				x0 = newCoord[0];
				y0 = newCoord[1];
				double[] myForceEnd = ini.cx3d.utilities.Matrix.add(aCylinder
						.getMassLocation(), forceOnACylinder);
				myForceEnd = mult(V, myForceEnd);
				newCoord = getDisplayCoord(myForceEnd);
				x1 = newCoord[0];
				y1 = newCoord[1];
				L = new Line2D.Double(x0, y0, x1, y1);
				g2D.setColor(Color.black);
				chooseCorrectLineThickness(0.5 * getMagnification(), g2D);
				g2D.draw(L);

				// arrow head :
				double theta = 0.5;
				double[] arrow = { x0 - x1, y0 - y1 };
				arrow = scalarMult(3 * getMagnification(), normalize(arrow));
				double[] arrowSide1 = {
						arrow[0] * Math.cos(theta) - arrow[1] * Math.sin(theta),
						arrow[0] * Math.sin(theta) + arrow[1] * Math.cos(theta) };
				double[] arrowSide2 = {
						arrow[0] * Math.cos(-theta) - arrow[1]
								* Math.sin(-theta),
						arrow[0] * Math.sin(-theta) + arrow[1]
								* Math.cos(-theta) };

				L = new Line2D.Double(x1, y1, x1 + arrowSide1[0], y1
						+ arrowSide1[1]);
				g2D.draw(L);
				L = new Line2D.Double(x1, y1, x1 + arrowSide2[0], y1
						+ arrowSide2[1]);
				g2D.draw(L);
			}
			g2D.setStroke(oneStroke);
		}

	}

	private void paintDelaunayNeighbors(Graphics2D g2D, Color col,
			SpatialOrganizationNode<PhysicalNode> son) {
		double[] nCoord = son.getPosition();
		nCoord = mult(V, nCoord); // rotation
		newCoord = getDisplayCoord(nCoord);
		double x0 = newCoord[0];
		double y0 = newCoord[1];
		g2D.setPaint(Param.X_SOLID_GRAY); // the links are in this color
		for (PhysicalNode n : son.getPermanentListOfNeighbors()) {
			double[] mCoord = n.getSoNode().getPosition();
			mCoord = mult(V, mCoord); // rotation
			newCoord = getDisplayCoord(mCoord);
			double x1 = newCoord[0];
			double y1 = newCoord[1];
			chooseCorrectLineThickness(0.5 * getMagnification(), g2D);
			Line2D L = new Line2D.Double(x0, y0, x1, y1);
			g2D.draw(L);
		}
	}

	private void paintPhysicalSpheres(Graphics2D g2D) {

		// modified by sabina: uncheck this code to sort the physicalSpheres
		// according to the viewer position.
		if (this.sortDraw) {
			Collections.sort(ecm.physicalSphereList, new SortPhysicalObjects());
		}

		for (int i = 0; i < ecm.physicalSphereList.size(); i++) {
			PhysicalSphere aSphere = ecm.physicalSphereList.get(i);

			double sphereRadius = 0.5 * aSphere.getDiameter();
			double[] mySomaMassLocation = aSphere.getMassLocation();

			double x0, y0, x1, y1;
			double[] newCoord;

			mySomaMassLocation = mult(V, mySomaMassLocation);
			newCoord = getDisplayCoord(mySomaMassLocation);

			// find the min max cords for exporting a picture!
			minXdrawn = Math.min(minXdrawn, newCoord[0]);
			maxXdrawn = Math.max(maxXdrawn, newCoord[0]);
			minZdrawn = Math.min(minZdrawn, newCoord[1]);
			maxZdrawn = Math.max(maxZdrawn, newCoord[1]);

			x0 = newCoord[0];
			y0 = newCoord[1];
			// ***changed by haurian: get the drawing color form the internally
			// secreted stuff
			// if it has been selected to be drawn otherwise use the standard of
			// the object
			Color sphereColor = aSphere.getColor();
			for (Substance sub : tobeDrawn) {
				if (sub instanceof IntracellularSubstance) {
					if (aSphere.getIntracellularSubstances().containsKey(
							sub.getId())) {
						sphereColor = aSphere.getIntracellularSubstances().get(
								sub.getId()).getConcentrationDependentColor();
						if (aSphere.getSomaElement()
								.getLocalBiologyModulesList().size() > 0) {
							paintrealSubstance(g2D, aSphere
									.getIntracellularSubstances().get(
											sub.getId()), newCoord);
						}
						// System.out.println("concentration sphere
						// "+aSphere.getIntracellularSubstances().get(sub.getId()).getConcentration());
					}
				}
			}
			// ***
			// if a "pale cell", we might not consider it
			// if(drawPaleCells==false && sphereColor.getAlpha()<100){
			// continue;
			// }

			// if outside the slice (in case of slice type of representation
			// type), we skip this somaElement:
			if (representationType == OM_SLICE_TYPE
					&& !isInsideTheSlice(mySomaMassLocation, OM_SLICE_THICKNESS)) {
				continue;
			} else if (representationType == EM_SLICE_TYPE
					&& Math.abs(mySomaMassLocation[1] - sliceYPosition) > sphereRadius
							+ addedToTheRadius) {
				continue;
			}
			// a few temporay local variables

			Ellipse2D.Double E1;
			Line2D.Double L;

			// links to the neighboring vertices in the
			// triangulation.................
			if (drawDelaunayNeighbors) {
				paintDelaunayNeighbors(g2D, aSphere.getColor(), aSphere
						.getSoNode());
			}

			// The somaElement itself
			// ........................................................
			g2D.setPaint(sphereColor);

			double radius = getSphereApparentRadius(mySomaMassLocation,
					sphereRadius);
			E1 = new Ellipse2D.Double(x0 - radius, y0 - radius, radius * 2,
					radius * 2);
			boolean fillSoma = true; // if false, only border and center are
			// painted
			if (fillSoma) {
				g2D.fill(E1);
			} else {
				// the border
				chooseCorrectLineThickness(getMagnification(), g2D);
				g2D.draw(E1);
				// the center
				double massDiameter = 2 * getMagnification()
						* getScaleFactor(aSphere.getMassLocation());
				massDiameter = Math.min(massDiameter, 10);
				E1 = new Ellipse2D.Double(x0 - massDiameter, y0 - massDiameter,
						2 * massDiameter, 2 * massDiameter);
				// g2D.fill(E1);
			}

			if (drawEffectiveSphereRadius) {
				radius = getSphereApparentRadius(mySomaMassLocation,
						sphereRadius + addedToTheRadius);
				E1 = new Ellipse2D.Double(x0 - radius, y0 - radius, radius * 2,
						radius * 2);
				g2D.setColor(Color.black);
				chooseCorrectLineThickness(0.5 * getMagnification(), g2D);
				g2D.draw(E1);
			}

			// inserted by roman
			// if excrescences on soma, we also draw them
			// .............................
			if (drawSpines) {

				for (Excrescence ex : aSphere.getExcrescences()) {
					double[] proximalExEnd = mult(V, ex.getProximalEnd()); 
					double[] distalExEnd = mult(V, ex.getDistalEnd());
					if (representationType != PROJECTION_TYPE) {
						double[][] twoEndsOfTheNeurite = getCylinderApparentEndPoints(
								proximalExEnd, distalExEnd);
						newCoord = twoEndsOfTheNeurite[0];
						newCoord = getDisplayCoord(newCoord);
						
						x0 = newCoord[0];
						y0 = newCoord[1];
						newCoord = twoEndsOfTheNeurite[1];
						newCoord = getDisplayCoord(newCoord);
						
						x1 = newCoord[0];
						y1 = newCoord[1];
					} else {
						newCoord = getDisplayCoord(proximalExEnd);
						x0 = newCoord[0];
						y0 = newCoord[1];
						newCoord = getDisplayCoord(distalExEnd);
						x1 = newCoord[0];
						y1 = newCoord[1];
					}
					L = new Line2D.Double(x0, y0, x1, y1);
					g2D.draw(L);
				}

			}

			// if PhysicalBonds, we also draw them .............................
			if (drawPhysicalBonds) {
				for (PhysicalBond pb : aSphere.getPhysicalBonds()) {
					double[] firstEnd = mult(V, pb.getFirstEndLocation()); // rotation
					double[] secondEnd = mult(V, pb.getSecondEndLocation());
					if (false && representationType != PROJECTION_TYPE) {
						double[][] twoEndsOfTheNeurite = getCylinderApparentEndPoints(
								firstEnd, secondEnd);
						newCoord = twoEndsOfTheNeurite[0];
						newCoord = getDisplayCoord(newCoord);
						x0 = newCoord[0];
						y0 = newCoord[1];
						newCoord = twoEndsOfTheNeurite[1];
						newCoord = getDisplayCoord(newCoord);
						x1 = newCoord[0];
						y1 = newCoord[1];
					} else {
						newCoord = getDisplayCoord(firstEnd);
						x0 = newCoord[0];
						y0 = newCoord[1];
						newCoord = getDisplayCoord(secondEnd);
						x1 = newCoord[0];
						y1 = newCoord[1];
					}
					L = new Line2D.Double(x0, y0, x1, y1);
					g2D.setColor(Param.X_SOLID_GRAY);
					g2D.setColor(Color.black);
					g2D.draw(L);
				}
			}

			// until here: inserted by roman

			// the Force acting on the somaElement
			double[] forceOnASphere = aSphere.getTotalForceLastTimeStep();
			if (drawForces && forceOnASphere[3] > 0) {
				double[] myForceEnd = ini.cx3d.utilities.Matrix.add(aSphere
						.getMassLocation(), forceOnASphere);
				myForceEnd = mult(V, myForceEnd);
				newCoord = getDisplayCoord(myForceEnd);
				x1 = newCoord[0];
				y1 = newCoord[1];
				L = new Line2D.Double(x0, y0, x1, y1);
				g2D.setColor(Color.black);
				chooseCorrectLineThickness(0.5 * getMagnification(), g2D);
				g2D.draw(L);

				// arrow head :
				double theta = 0.5;
				double[] arrow = { x0 - x1, y0 - y1 };
				arrow = scalarMult(3 * getMagnification(), normalize(arrow));
				double[] arrowSide1 = {
						arrow[0] * Math.cos(theta) - arrow[1] * Math.sin(theta),
						arrow[0] * Math.sin(theta) + arrow[1] * Math.cos(theta) };
				double[] arrowSide2 = {
						arrow[0] * Math.cos(-theta) - arrow[1]
								* Math.sin(-theta),
						arrow[0] * Math.sin(-theta) + arrow[1]
								* Math.cos(-theta) };

				L = new Line2D.Double(x1, y1, x1 + arrowSide1[0], y1
						+ arrowSide1[1]);
				g2D.draw(L);
				L = new Line2D.Double(x1, y1, x1 + arrowSide2[0], y1
						+ arrowSide2[1]);
				g2D.draw(L);
			}

		}

	}

	private void paintPhysicalNodes(Graphics2D g2D) {
		for (int i = 0; i < ecm.physicalNodeList.size(); i++) {
			PhysicalNode n = ecm.physicalNodeList.get(i);

			double[] nodeCoord = n.getSoNode().getPosition();
			nodeCoord = mult(V, nodeCoord); // rotation

			// if outside the slice (in case of slice type of representation
			// type), we skip this somaElement:
			// if(representationType!=PROJECTION_TYPE &&
			// !isInsideTheSlice(nodeCoord, OM_SLICE_THICKNESS)){
			// continue;
			// }
			if (representationType == OM_SLICE_TYPE
					&& !isInsideTheSlice(nodeCoord, OM_SLICE_THICKNESS)) {
				continue;
			} else if (representationType == EM_SLICE_TYPE
					&& Math.abs(nodeCoord[1] - sliceYPosition) > 10) {
				continue;
			}

			nodeCoord = getDisplayCoord(nodeCoord);

			for (Substance s : tobeDrawn) {

				Substance realS = n.getExtracellularSubstances().get(s.getId());
				if (realS != null) {

					paintrealSubstanceJustColor(g2D, realS, nodeCoord);
					paintrealSubstanceShowNumberDependant(g2D, realS, nodeCoord);
				}
			}

		}
	}

	private void paintrealSubstance(Graphics2D g2D, Substance s, double[] coord) {
		Color temp = g2D.getColor();
		g2D.setColor(new Color(0, 0, 0, temp.getAlpha()));
		g2D.setFont(new Font("sansserif", Font.BOLD, 10));
		g2D.drawString(StringUtilities.doubleToString(s.getConcentration(), 5),
				(float) coord[0], (float) coord[1]);
		g2D.setColor(temp);

	}

	private void paintrealSubstanceShowNumberDependant(Graphics2D g2D,
			Substance s, double[] coord) {
		if (!Shownumbers)
			return;
		paintrealSubstance(g2D, s, coord);

	}

	private void paintrealSubstanceJustColor(Graphics2D g2D, Substance s,
			double[] coord) {
		int intensity = (int) (chemicalDrawFactor * s.getConcentration());
		if (intensity > 255) {
			intensity = 255;
		} else if (intensity < 0) {
			intensity = 0;
		}
		Color temp = g2D.getColor();
		// int proxyDiameter = (int)diameter;
		// Ellipse2D.Double E1 = new
		// Ellipse2D.Double(coord[0]-proxyDiameter/2,coord[1]-proxyDiameter/2,proxyDiameter,proxyDiameter);
		Color col = s.getColor();
		Color transparentColor = new Color(col.getRed(), col.getGreen(), col
				.getBlue(), intensity);
		// g2D.setPaint(transparentColor);
		// g2D.fill(E1);
		// g2D.setPaint(Color.black);
		g2D.setColor(transparentColor);
		g2D.fillOval((int) coord[0], (int) coord[1], 10, 10);
		// g2D.drawString(StringUtilities.doubleToString(s.getConcentration(),
		// 5),(float)coord[0], (float)coord[1]);
		g2D.setColor(temp);

	}

	// **************************************************************************
	// Artificial Gradient
	// **************************************************************************

	private void drawSubstance(Graphics2D g2D, Substance sub) {
		if (ecm.gaussianArtificialConcentrationX.containsKey(sub)) {
			double[] v = ecm.gaussianArtificialConcentrationX.get(sub);
			drawGaussianGradientX(g2D, v, sub);
		}
		if (ecm.gaussianArtificialConcentrationZ.containsKey(sub)) {
			double[] v = ecm.gaussianArtificialConcentrationZ.get(sub);
			drawGaussianGradientZ(g2D, v, sub);
		}
		if (ecm.linearArtificialConcentrationX.containsKey(sub)) {
			double[] v = ecm.linearArtificialConcentrationX.get(sub);
			drawLinearGradientX(g2D, v, sub);
		}
		if (ecm.linearArtificialConcentrationZ.containsKey(sub)) {
			double[] v = ecm.linearArtificialConcentrationZ.get(sub);
			drawLinearGradientZ(g2D, v, sub);
		}
	}

	private void drawGradientRectangleZ(Graphics2D g2d, double[] cMax,
			Color maxColor, double[] cMin, Color minColor) {
		GradientPaint gp = new GradientPaint((int) cMax[0], (int) cMax[1],
				maxColor, (int) cMin[0], (int) cMin[1], minColor);
		g2d.setPaint(gp);
		int y1, y2;
		if (cMax[1] < cMin[1]) {
			y1 = (int) cMax[1];
			y2 = (int) (cMin[1] - cMax[1]);
		} else {
			y1 = (int) cMin[1];
			y2 = (int) (cMax[1] - cMin[1]);
		}
		g2d.fillRect(0, y1 - 1, 10000, y2 + 2); // -1 & +2 to avoid a white
		// strip
	}

	private void drawGradientRectangleX(Graphics2D g2d, double[] cMax,
			Color maxColor, double[] cMin, Color minColor) {
		GradientPaint gp = new GradientPaint((int) cMax[0], (int) cMax[1],
				maxColor, (int) cMin[0], (int) cMin[1], minColor);
		g2d.setPaint(gp);
		int x1, x2;
		if (cMax[0] < cMin[0]) {
			x1 = (int) cMax[0];
			x2 = (int) (cMin[0] - cMax[0]);
		} else {
			x1 = (int) cMin[0];
			x2 = (int) (cMax[0] - cMin[0]);
		}

		g2d.fillRect(x1 - 1, 0, x2 + 2, 10000); // -1 & +2 to avoid a white
		// strip
	}

	private void drawLinearGradientX(Graphics2D g2d, double[] v, Substance s) {
		// double[] v = {8,114,177}; // the values in the hashtable :
		// MaxValue-MaxPt-MinPt
		int maxIntensity = (int) v[0] * chemicalDrawFactor;
		if (maxIntensity > 255) {
			maxIntensity = 255;
		} else if (maxIntensity < 0) {
			maxIntensity = 0;
		}
		Color substancecolor = s.getColor();
		Color maxColor = new Color(substancecolor.getRed(), substancecolor
				.getGreen(), substancecolor.getBlue(), maxIntensity);
		Color minColor = new Color(0, 0, 0, 0);
		double[] maxPt = { v[1], 0, 0 };
		double[] minPt = { v[2], 0, 0 };
		maxPt = mult(V, maxPt);
		minPt = mult(V, minPt);

		double[] cMax = getDisplayCoord(maxPt);
		double[] cMin = getDisplayCoord(minPt);
		drawGradientRectangleX(g2d, cMax, maxColor, cMin, minColor);
	}

	private void drawLinearGradientZ(Graphics2D g2d, double[] v, Substance s) {
		// double[] v = {8,114,177}; // the values in the hashtable :
		// MaxValue-MaxPt-MinPt
		int maxIntensity = (int) v[0] * chemicalDrawFactor;
		if (maxIntensity > 255) {
			maxIntensity = 255;
		} else if (maxIntensity < 0) {
			maxIntensity = 0;
		}
		Color substancecolor = s.getColor();
		Color maxColor = new Color(substancecolor.getRed(), substancecolor
				.getGreen(), substancecolor.getBlue(), maxIntensity);
		Color minColor = new Color(0, 0, 0);
		double[] maxPt = { 0, 0, v[1] };
		double[] minPt = { 0, 0, v[2] };
		maxPt = mult(V, maxPt);
		minPt = mult(V, minPt);

		double[] cMax = getDisplayCoord(maxPt);
		double[] cMin = getDisplayCoord(minPt);
		drawGradientRectangleZ(g2d, cMax, maxColor, cMin, minColor);
	}

	private void drawGaussianGradientZ(Graphics2D g2d, double[] v, Substance s) {

		Color substancecolor = s.getColor();
		int R = substancecolor.getRed();
		int G = substancecolor.getGreen();
		int B = substancecolor.getBlue();

		int dR = 255 - R;
		int dG = 255 - G;
		int dB = 255 - B; // difference to white
		double coeff1 = 1 - (.4289 / .70711); // relative difference between
		// max and 1 sigma
		double coeff1_5 = 1 - (.2296 / .70711);
		double coeff2 = 1 - (.0957 / .70711);

		int sR = (int) (R + coeff1 * dR);
		int sG = (int) (G + coeff1 * dG);
		int sB = (int) (B + coeff1 * dB);
		int s15R = (int) (R + coeff1_5 * dR);
		int s15G = (int) (G + coeff1_5 * dG);
		int s15B = (int) (B + coeff1_5 * dB);
		int s2R = (int) (R + coeff2 * dR);
		int s2G = (int) (G + coeff2 * dG);
		int s2B = (int) (B + coeff2 * dB);

		Color maxColor = substancecolor;
		Color sigmaColor = new Color(sR, sG, sB);
		Color sigma15Color = new Color(s15R, s15G, s15B);
		Color sigma2Color = new Color(s2R, s2G, s2B);
		Color zeroColor = Color.white;

		double[] maxPt = { 0, 0, v[1] };
		maxPt = mult(V, maxPt);
		double[] cMax = getDisplayCoord(maxPt);

		double[] sigmaPlusPt = { 0, 0, v[1] + v[2] };
		sigmaPlusPt = mult(V, sigmaPlusPt);
		double[] cSPlus = getDisplayCoord(sigmaPlusPt);

		double[] sigma15PlusPt = { 0, 0, v[1] + 1.5 * v[2] };
		sigma15PlusPt = mult(V, sigma15PlusPt);
		double[] cS15Plus = getDisplayCoord(sigma15PlusPt);

		double[] sigma2PlusPt = { 0, 0, v[1] + 2 * v[2] };
		sigma2PlusPt = mult(V, sigma2PlusPt);
		double[] cS2Plus = getDisplayCoord(sigma2PlusPt);

		double[] sigma3PlusPt = { 0, 0, v[1] + 3 * v[2] };
		sigma3PlusPt = mult(V, sigma3PlusPt);
		double[] cS3Plus = getDisplayCoord(sigma3PlusPt);

		double[] sigmaMinusPt = { 0, 0, v[1] - v[2] };
		sigmaMinusPt = mult(V, sigmaMinusPt);
		double[] cSMinus = getDisplayCoord(sigmaMinusPt);

		double[] sigma15MinusPt = { 0, 0, v[1] - 1.5 * v[2] };
		sigma15MinusPt = mult(V, sigma15MinusPt);
		double[] cS15Minus = getDisplayCoord(sigma15MinusPt);

		double[] sigma2MinusPt = { 0, 0, v[1] - 2 * v[2] };
		sigma2MinusPt = mult(V, sigma2MinusPt);
		double[] cS2Minus = getDisplayCoord(sigma2MinusPt);

		double[] sigma3MinusPt = { 0, 0, v[1] - 3 * v[2] };
		sigma3MinusPt = mult(V, sigma3MinusPt);
		double[] cS3Minus = getDisplayCoord(sigma3MinusPt);

		drawGradientRectangleZ(g2d, cMax, maxColor, cSPlus, sigmaColor);
		drawGradientRectangleZ(g2d, cSPlus, sigmaColor, cS15Plus, sigma15Color);
		drawGradientRectangleZ(g2d, cS15Plus, sigma15Color, cS2Plus,
				sigma2Color);
		drawGradientRectangleZ(g2d, cS2Plus, sigma2Color, cS3Plus, zeroColor);

		drawGradientRectangleZ(g2d, cMax, maxColor, cSMinus, sigmaColor);
		drawGradientRectangleZ(g2d, cSMinus, sigmaColor, cS15Minus,
				sigma15Color);
		drawGradientRectangleZ(g2d, cS15Minus, sigma15Color, cS2Minus,
				sigma2Color);
		drawGradientRectangleZ(g2d, cS2Minus, sigma2Color, cS3Minus, zeroColor);
	}

	private void drawGaussianGradientX(Graphics2D g2d, double[] v, Substance s) {
		Color substancecolor = s.getColor();
		int R = substancecolor.getRed();
		int G = substancecolor.getGreen();
		int B = substancecolor.getBlue();

		int dR = 255 - R;
		int dG = 255 - G;
		int dB = 255 - B; // difference to white
		double coeff1 = 1 - (.4289 / .70711); // relative difference between
		// max and 1 sigma
		double coeff1_5 = 1 - (.2296 / .70711);
		double coeff2 = 1 - (.0957 / .70711);

		int sR = (int) (R + coeff1 * dR);
		int sG = (int) (G + coeff1 * dG);
		int sB = (int) (B + coeff1 * dB);
		int s15R = (int) (R + coeff1_5 * dR);
		int s15G = (int) (G + coeff1_5 * dG);
		int s15B = (int) (B + coeff1_5 * dB);
		int s2R = (int) (R + coeff2 * dR);
		int s2G = (int) (G + coeff2 * dG);
		int s2B = (int) (B + coeff2 * dB);

		Color maxColor = substancecolor;
		Color sigmaColor = new Color(sR, sG, sB);
		Color sigma15Color = new Color(s15R, s15G, s15B);
		Color sigma2Color = new Color(s2R, s2G, s2B);
		Color zeroColor = Color.white;

		double[] maxPt = { v[1], 0, 0 };
		maxPt = mult(V, maxPt);
		double[] cMax = getDisplayCoord(maxPt);

		double[] sigmaPlusPt = { v[1] + v[2], 0, 0 };
		sigmaPlusPt = mult(V, sigmaPlusPt);
		double[] cSPlus = getDisplayCoord(sigmaPlusPt);

		double[] sigma15PlusPt = { v[1] + 1.5 * v[2], 0, 0 };
		sigma15PlusPt = mult(V, sigma15PlusPt);
		double[] cS15Plus = getDisplayCoord(sigma15PlusPt);

		double[] sigma2PlusPt = { v[1] + 2 * v[2], 0, 0 };
		sigma2PlusPt = mult(V, sigma2PlusPt);
		double[] cS2Plus = getDisplayCoord(sigma2PlusPt);

		double[] sigma3PlusPt = { v[1] + 3 * v[2], 0, 0 };
		sigma3PlusPt = mult(V, sigma3PlusPt);
		double[] cS3Plus = getDisplayCoord(sigma3PlusPt);

		double[] sigmaMinusPt = { v[1] - v[2], 0, 0 };
		sigmaMinusPt = mult(V, sigmaMinusPt);
		double[] cSMinus = getDisplayCoord(sigmaMinusPt);

		double[] sigma2MinusPt = { v[1] - 2 * v[2], 0, 0 };
		sigma2MinusPt = mult(V, sigma2MinusPt);
		double[] cS2Minus = getDisplayCoord(sigma2MinusPt);

		double[] sigma15MinusPt = { v[1] - 1.5 * v[2], 0, 0 };
		sigma15MinusPt = mult(V, sigma15MinusPt);
		double[] cS15Minus = getDisplayCoord(sigma15MinusPt);

		double[] sigma3MinusPt = { v[1] - 3 * v[2], 0, 0 };
		sigma3MinusPt = mult(V, sigma3MinusPt);
		double[] cS3Minus = getDisplayCoord(sigma3MinusPt);

		drawGradientRectangleX(g2d, cMax, maxColor, cSPlus, sigmaColor);
		drawGradientRectangleX(g2d, cSPlus, sigmaColor, cS15Plus, sigma15Color);
		drawGradientRectangleX(g2d, cS15Plus, sigma15Color, cS2Plus,
				sigma2Color);
		drawGradientRectangleX(g2d, cS2Plus, sigma2Color, cS3Plus, zeroColor);

		drawGradientRectangleX(g2d, cMax, maxColor, cSMinus, sigmaColor);
		drawGradientRectangleX(g2d, cSMinus, sigmaColor, cS15Minus,
				sigma15Color);
		drawGradientRectangleX(g2d, cS15Minus, sigma15Color, cS2Minus,
				sigma2Color);
		drawGradientRectangleX(g2d, cS2Minus, sigma2Color, cS3Minus, zeroColor);
	}

	// **************************************************************************
	// communication with the GUI
	// **************************************************************************
	void increaseScalingFactor() {
		setMagnification(getMagnification() * 1.2);
		diameter = 10 * getMagnification();

	}

	void decreaseScalingFactor() {
		setMagnification(getMagnification() / 1.2);
		diameter = 10 * getMagnification();

	}

	// the 4 following methods are called by ECM and therefore are public
	public void rotateAroundZ(double a) {
		alphaAroundZ += a;
		VrotZ[0][0] = Math.cos(alphaAroundZ);
		VrotZ[0][1] = Math.sin(alphaAroundZ);
		VrotZ[0][2] = 0;
		VrotZ[1][0] = -VrotZ[0][1];
		VrotZ[1][1] = VrotZ[0][0];
		VrotZ[1][2] = 0;
		VrotZ[2][0] = 0;
		VrotZ[2][1] = 0;
		VrotZ[2][2] = 1;
		V = mult(VrotX, VrotZ);
		repaint();
	}

	public void rotateAroundX(double a) {
		alphaAroundX += a;
		VrotX[0][0] = 1;
		VrotX[0][1] = 0;
		VrotX[0][2] = 0;
		VrotX[1][0] = 0;
		VrotX[1][1] = Math.cos(alphaAroundX);
		VrotX[1][2] = Math.sin(alphaAroundX);
		VrotX[2][0] = 0;
		VrotX[2][1] = -Math.sin(alphaAroundX);
		VrotX[2][2] = Math.cos(alphaAroundX);
		;
		V = mult(VrotX, VrotZ);
		repaint();
	}

	public void setRotationAroundZ(double a) {
		alphaAroundZ = a;
		VrotZ[0][0] = Math.cos(a);
		VrotZ[0][1] = Math.sin(a);
		VrotZ[0][2] = 0;
		VrotZ[1][0] = -VrotZ[0][1];
		VrotZ[1][1] = VrotZ[0][0];
		VrotZ[1][2] = 0;
		VrotZ[2][0] = 0;
		VrotZ[2][1] = 0;
		VrotZ[2][2] = 1;
		V = mult(VrotX, VrotZ);
		repaint();
	}

	public void setRotationAroundX(double a) {
		alphaAroundX = a;
		VrotX[0][0] = 1;
		VrotX[0][1] = 0;
		VrotX[0][2] = 0;
		VrotX[1][0] = 0;
		VrotX[1][1] = Math.cos(alphaAroundX);
		VrotX[1][2] = Math.sin(alphaAroundX);
		VrotX[2][0] = 0;
		VrotX[2][1] = -Math.sin(alphaAroundX);
		VrotX[2][2] = Math.cos(alphaAroundX);
		;
		V = mult(VrotX, VrotZ);
		repaint();
	}

	public void showXYPlan() {
		setRotationAroundX(-Math.PI * 0.5);
		setRotationAroundZ(0);
	}

	public void showXZPlan() {
		setRotationAroundX(0);
		setRotationAroundZ(0);
	}

	public void showYZPlan() {
		setRotationAroundX(0);
		setRotationAroundZ(Math.PI * 0.5);
	}

	public double[] getRotatedCoord(double[] coord) { // used in Mouse view
		// listener
		return mult(V, coord);
	}

	// **************************************************************************
	// internal methods
	// **************************************************************************

	public double[] getDisplayCoord(double[] coord) {
		double[] newCoord = new double[2];
		double projectionFactor = 1;

		if (representationType == PROJECTION_TYPE && perspective) {
			projectionFactor = (a / (a + b - coord[1])) / delta;
		}
		newCoord[0] = projectionFactor * getMagnification()
				* (coord[0] + displacementx) + this.getWidth() / 2;
		newCoord[1] = -projectionFactor * getMagnification()
				* (coord[2] + displacementy) + this.getHeight() / 2;
		// the Z component is left unchanged

		return newCoord;
	}

	/* Return the distance between a physical sphere and the display. */
	public double getDepth(double[] massLocation) {
		double depth = mult(V, massLocation)[1];
		return depth;
	}

	private double getScaleFactor(double[] coord) {
		if (representationType == PROJECTION_TYPE && perspective) {
			return (a / (a + b - coord[1])) / delta;
		}
		return 1.0;
	}

	private double getSphereApparentRadius(double[] coord, double radius) {
		if (representationType == PROJECTION_TYPE && perspective) {
			return getMagnification() * radius * (a / (a + b - coord[1]))
					/ delta;
		} else if (representationType == EM_SLICE_TYPE) {
			double distanceCenterToPlane = Math.abs(coord[1] - sliceYPosition);
			return getMagnification()
					* Math.sqrt(radius * radius - distanceCenterToPlane
							* distanceCenterToPlane);
		}
		return getMagnification() * radius;
	}

	private double[][] getCylinderApparentEndPoints2(double[] A, double[] B) {
		if (isInsideTheSlice(A, sliceThickness)
				&& isInsideTheSlice(A, sliceThickness)) {
			return new double[][] { A, B };
		}
		if (isInsideTheSlice(A, sliceThickness)) { // we know B is not inside,
			// otherwise the method
			// would have returned
			// if(B[1]>A[1]) {
			// We parametrize the segment A-B, and go along the parametrisation
			// as long as we are in the slice
			// newB = A + (B-A)*( yPos+halfthickness-A[1]/B[1]-A[1])
			double t = (sliceYPosition + sliceThickness / 2.0 - A[1])
					/ (B[1] - A[1]);
			double[] newB = subtract(B, A);
			newB = scalarMult(t, newB);
			newB = ini.cx3d.utilities.Matrix.add(A, newB);
			return new double[][] { A, newB };
			// } else {
		} else if (isInsideTheSlice(B, sliceThickness)) {
			double t = (sliceYPosition + sliceThickness / 2.0 - B[1])
					/ (A[1] - B[1]);
			double[] newA = subtract(A, B);
			newA = scalarMult(t, newA);
			newA = ini.cx3d.utilities.Matrix.add(B, newA);
			return new double[][] { newA, B };
		}
		double tA = (sliceYPosition + sliceThickness / 2.0 - A[1])
				/ (B[1] - A[1]);
		double tB = (sliceYPosition + sliceThickness / 2.0 - B[1])
				/ (A[1] - B[1]);
		return null;
	}

	private double[][] getCylinderApparentEndPoints(double[] A, double[] B) {
		// see that A has a smaller y value than B
		if (B[1] < A[1]) {
			double[] C = A;
			A = B;
			B = C;
		}
		// We parametrize the segment A-B, and go along the parametrisation as
		// long as we are in the slice
		// The parametrization is sclaled by the difference in the y-coordonates
		// between A and B:
		// A + (B-A)*t/(B[1]-A[1])
		double tA = Math.max(sliceYPosition - sliceThickness / 2.0 - A[1], 0);
		double tB = Math.min(sliceYPosition + sliceThickness / 2.0 - A[1], B[1]
				- A[1]);

		double[] seg = subtract(B, A);
		double[] newA = scalarMult(tA / (B[1] - A[1]), seg);
		newA = ini.cx3d.utilities.Matrix.add(A, newA);
		double[] newB = scalarMult(tB / (B[1] - A[1]), seg);
		newB = ini.cx3d.utilities.Matrix.add(A, newB);

		return new double[][] { newA, newB };
	}

	private boolean isInsideTheSlice(double[] coord, double thickness) {
		if (coord[1] < (sliceYPosition + thickness / 2)
				&& coord[1] > (sliceYPosition - thickness / 2)) {
			return true;
		}
		return false;
	}

	private void chooseCorrectLineThickness(double thicknessToDraw,
			Graphics2D g2D) {
		;
		if (thicknessToDraw < 1.5)
			g2D.setStroke(oneStroke);
		else if (thicknessToDraw < 2.5)
			g2D.setStroke(twoStroke);
		else if (thicknessToDraw < 3.5)
			g2D.setStroke(threeStroke);
		else if (thicknessToDraw < 4.5)
			g2D.setStroke(fourStroke);
		else if (thicknessToDraw < 5.5)
			g2D.setStroke(fiveStroke);
		else if (thicknessToDraw < 6.5)
			g2D.setStroke(sixStroke);
		else if (thicknessToDraw < 7.5)
			g2D.setStroke(sevenStroke);
		else if (thicknessToDraw < 8.5)
			g2D.setStroke(eightStroke);
		else if (thicknessToDraw < 9.5)
			g2D.setStroke(nineStroke);
		else if (thicknessToDraw < 10.5)
			g2D.setStroke(tenStroke);
		else if (thicknessToDraw < 13)
			g2D.setStroke(twelveStroke);
		else if (thicknessToDraw < 16.5)
			g2D.setStroke(fifteenStroke);
		else if (thicknessToDraw < 25)
			g2D.setStroke(twentyStroke);
		else if (thicknessToDraw < 35)
			g2D.setStroke(thirtyStroke);
		else if (thicknessToDraw < 45)
			g2D.setStroke(fourtyStroke);
		else
			g2D.setStroke(fiftyStroke);
	}

	public void addTobeDrawn(Substance tobeDrawn) {
		this.tobeDrawn.add(tobeDrawn);
	}

	public void removeTobeDrawn(Substance s) {
		tobeDrawn.remove(s);
	}

	public boolean containsTobeDrawn(Substance s) {
		return tobeDrawn.contains(s);
	}

	public void removeAllToBeDrawn() {
		tobeDrawn.clear();

	}

	public void setDrawIT(boolean selected) {
		drawit = selected;
	}

	// introduced by haurian better overview with getters and setters

	public void setMagnification(double magnification) {
		this.magnification = magnification;
		diameter = 10 * getMagnification();
	}

	public double getMagnification() {
		return magnification;
	}

	// get roi automaticly by the smalest and biggest coordinates drawn;
	public boolean AutoRoi() {
		this.smallWindowRectangle = new Rectangle((int) (minXdrawn - 15),
				(int) (minZdrawn - 15), (int) (maxXdrawn - minXdrawn + 30),
				(int) (maxZdrawn - minZdrawn + 30));
		return true;
	}

	/**
	 * @return the rotationSpeed
	 */
	public double getRotationSpeed() {
		return rotationSpeed;
	}

	/**
	 * @param rotationSpeed
	 *            the rotationSpeed to set
	 */
	public void setRotationSpeed(double rotationSpeed) {
		this.rotationSpeed = rotationSpeed;
	}

	/**
	 * @param sortDraw
	 *            the sortDraw to set
	 */
	public void setSortDraw(boolean sortDraw) {
		this.sortDraw = sortDraw;
	}

	// **************************************************************************
	// Getters & Setters
	// **************************************************************************

}
