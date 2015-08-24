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

package ini.cx3d.simulations;

import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.randomNoise;
import ini.cx3d.cells.Cell;
import ini.cx3d.graphics.ECM_GUI_Creator;
import ini.cx3d.graphics.View;
import ini.cx3d.localBiology.NeuriteElement;
import ini.cx3d.localBiology.SomaElement;
import ini.cx3d.physics.ECMChemicalReaction;
import ini.cx3d.physics.IntracellularSubstance;
import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.physics.PhysicalNodeMovementListener;
import ini.cx3d.physics.PhysicalSphere;
import ini.cx3d.physics.Substance;
import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.SpaceNode;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
import ini.cx3d.utilities.Matrix;

import java.awt.Color;
import java.awt.Graphics;
import java.awt.image.BufferedImage;
import java.io.File;
import java.util.Hashtable;
import java.util.Random;
import java.util.Vector;
import java.util.concurrent.Semaphore;

import javax.imageio.ImageIO;
import javax.swing.JFileChooser;
import javax.swing.JFrame;

/**
 * Contains some lists with all the elements of the simulation, and methods to add 
 * or remove elements. Contains lists of "real" and "artificial" Substances.
 * Possibility to define a cubic region of space where elements are confined. 
 * @author fredericzubler
 *
 */
public class ECM {

	// List of all the CX3DRunbable objects in the simulation ............................

	/** List of all the PhysicalNode instances. */
	public Vector<PhysicalNode> physicalNodeList = new Vector<PhysicalNode>();
	/** List of all the PhysicalSphere instances. */
	public Vector<PhysicalSphere> physicalSphereList = new Vector<PhysicalSphere>();
	/** List of all the PhysicalCylinder instances. */
	public Vector<PhysicalCylinder> physicalCylinderList = new Vector<PhysicalCylinder>();
	/** List of all the SomaElement instances. */
	public Vector<SomaElement> somaElementList  = new Vector<SomaElement>();
	/** List of all the NeuriteElement instances. */
	public Vector<NeuriteElement> neuriteElementList = new Vector<NeuriteElement>();
	/** List of all the Cell instances. */
	public Vector<Cell> cellList  = new Vector<Cell>();
	/** List of all the Chemical reactions instances. */
	public Vector<ECMChemicalReaction> ecmChemicalReactionList = new Vector<ECMChemicalReaction>(); 
	
	
	/** needed for run and pause.*/ 
	public Semaphore canRun = new Semaphore(0);

	/* Reference time throughout the simulation (in hours) */
	private double ECMtime = 0; 

	/* An SON used to get new SON instances from*/
	private SpatialOrganizationNode<PhysicalNode> initialNode;

	/* In here we keep a template for each (extra-cellular) Substance in the simulation that have
	 * non-standard value for diffusion and degradation constant.*/
	private Hashtable<String, Substance> substancesLibrary = new Hashtable<String, Substance>();
	
	/* In here we keep a template for each (intra-cellular) Substance in the simulation that have
	 * non-standard value for diffusion and degradation constant.*/
	private Hashtable<String, IntracellularSubstance> intracellularSubstancesLibrary = new Hashtable<String, IntracellularSubstance>();

	/* In here we store a color attributed to specific cell types.*/
	private Hashtable<String, Color> cellTypeColorLibrary = new Hashtable<String, Color>();

	// GUI ..................................................................................

	public View view;
	public ECM_GUI_Creator  myGuiCreator;
	volatile private boolean simulationOnPause = true;
	public boolean continuouslyRotating = false;
	// saving snapshots
	private int pictureNumber = 0;
	private String picturesName = null;
	public boolean takingSnapshotAtEachTimeStep = false;
	public boolean takingSnapshotEach100TimeSteps = false;

	


	// Artificial walls ...................................................................

	// if true, PhysicalSphere instances (and only those) are forced to stay in a defined volume.
	private boolean artificialWallsForSpheres = false; 
	
	// if true, PhysicalCylinders instances (and only those) are forced to stay in a defined volume.
	private boolean artificialWallsForCylinders = false; 

	
	/* Boundaries of the simulation area, within which the PhysicalSpheres 
	 * have to stay (but PhysicalCylinders and Substances don't...) */
	private double Xmin = -100;
	private double Xmax = 100;
	private double Ymin = -100;
	private double Ymax = 100;
	private double Zmin = -100;
	private double Zmax = 300;

	// Artificial gradients ...............................................................

	// whether we use real artificial gradient:
	private boolean anyArtificialGradientDefined = false;
	// (in the next: all hash tables are public for View.paint)
	/* List of all the chemicals with a gaussian distribution along the Z-axis
	 * max value, mean (z-coord of the max value), sigma2 (thickness). */
	public Hashtable<Substance,double[]> gaussianArtificialConcentrationZ = new Hashtable<Substance,double[]>();

	/* List of all the chemicals with a linear distribution along the Z-axis
	 * max value, TOP (z-coord of the max value), DOWN (z-coord of the 0 value). */ 
	public Hashtable<Substance,double[]> linearArtificialConcentrationZ = new Hashtable<Substance,double[]>();

	/* List of all the chemicals with a gaussian distribution along the X-axis
	 * max value, mean (x-coord of the max value), sigma2 (thickness). */ 
	public Hashtable<Substance,double[]> gaussianArtificialConcentrationX = new Hashtable<Substance,double[]>();

	/* List of all the chemicals with a linear distribution along the X-axis
	 * max value, TOP (x-coord of the max value), DOWN (x-coord of the 0 value). */  
	public Hashtable<Substance,double[]> linearArtificialConcentrationX = new Hashtable<Substance,double[]>();

	/* to link the one instance of Substance we have used in the definition of the gradient, with the name of
	 * the chemical that can be given as argument in the methods to know the concentration/grad.. */
	public Hashtable<String, Substance> allArtificialSubstances = new Hashtable<String, Substance>();



	// **************************************************************************
	// Singleton pattern
	// **************************************************************************

	private static ECM instance = null;

	private ECM() {
	}

	/** 
	 * Gets a reference to the (unique) ECM
	 * @return the ECM
	 */
	public static ECM getInstance() {
		if (instance == null) {
			instance = new ECM();
		}
		return instance;
	}

	/**Classic singleton precaution: a singleton cannot be cloned */
	public Object clone() throws CloneNotSupportedException {
		throw new CloneNotSupportedException(" Hey, what do you think you're doing !?! Nobody can clone the ECM !!!!");
	}

	

	// **************************************************************************
	// Random Number
	// **************************************************************************
	static Random random = new Random();
	
	/**
	 * @return a random number between, from uniform probability 0 and 1;
	 */
	public static double getRandomDouble(){
		return random.nextDouble();
	}
	
	/**
	 * returns a random number from gaussian distribution
	 * @param mean
	 * @param standardDeviation
	 * @return
	 */
	public static double getGaussianDouble(double mean, double standardDeviation){
		return mean + standardDeviation*random.nextGaussian();
	}
	
	
	
	/**
	 * Initialises the random number generator. 
	 * @param seed
	 */
	public static void setRandomSeed(long seed){
		random = new Random(seed);
		Matrix.setRandomSeedTo(seed);
	}
	
	// **************************************************************************
	// Artificial Wall
	// **************************************************************************
	/**
	 * Set the boundaries of a pseudo wall, that maintains the PhysicalObjects in a closed volume.
	 * Automatically turns on this mechanism for spheres.
	 * @param Xmin
	 * @param Xmax
	 * @param Ymin
	 * @param Ymax
	 * @param Zmin
	 * @param Zmax
	 */
	public void setBoundaries(double Xmin, double Xmax, double Ymin, double Ymax, double Zmin, double Zmax){
		this.Xmin = Xmin;
		this.Xmax = Xmax;
		this.Ymin = Ymin;
		this.Ymax = Ymax;
		this.Zmin = Zmin;
		this.Zmax = Zmax;
		setArtificialWallsForSpheres(true);
	}
	/** If set to true, the PhysicalSpheres tend to stay inside a box, 
	 * who's boundaries are set with setBoundaries().
	 * @param artificialWalls
	 */
	public void setArtificialWallsForSpheres(boolean artificialWallsForSpheres){
		this.artificialWallsForSpheres = artificialWallsForSpheres;
	}

	/** If true, the PhysicalSpheres tend to stay inside a box, who's boundaries are set with
	 * setBoundaries().
	 * @param artificialWalls
	 */
	public boolean getArtificialWallForSpheres(){
		return artificialWallsForSpheres;
	}
	/** If set to true, the PhysicalCyliners tend to stay inside a box, 
	 * who's boundaries are set with setBoundaries().
	 * @param artificialWalls
	 */
	public void setArtificialWallsForCylinders(boolean artificialWallsForCylinders){
		this.artificialWallsForCylinders = artificialWallsForCylinders;
	}

	/** If true, the PhysicalCyliners tend to stay inside a box, who's boundaries are set with
	 * setBoundaries().
	 * @param artificialWalls
	 */
	public boolean getArtificialWallForCylinders(){
		return artificialWallsForCylinders;
	}
	
	/**
	 * Returns a force that would be applied to a PhysicalSphere that left the boundaries
	 * of the artificial wall. 
	 * @param location the center of the PhysicalSphere
	 * @param radius the radius of the PhysicalSphere
	 * @return [Fx,Fy,Fz] the force applied to the cell
	 */
	public double[] forceFromArtificialWall(double[] location, double radius){
		// TODO : take the radius into account
		double[] force = new double[3]; 
		double springThatPullsCellsBackIntoBoundaries = 2.0;
		if(location[0] < Xmin) {
			force[0] += springThatPullsCellsBackIntoBoundaries*(Xmin-location[0]);
		}else if(location[0] > Xmax) {
			force[0] += springThatPullsCellsBackIntoBoundaries*(Xmax-location[0]);
		}

		if(location[1] < Ymin) {
			force[1] += springThatPullsCellsBackIntoBoundaries*(Ymin-location[1]);
		}else if(location[1] > Ymax) {
			force[1] += springThatPullsCellsBackIntoBoundaries*(Ymax-location[1]);
		}

		if(location[2] < Zmin) {
			force[2] += springThatPullsCellsBackIntoBoundaries*(Zmin-location[2]);
		}else if(location[2] > Zmax) {
			force[2] += springThatPullsCellsBackIntoBoundaries*(Zmax-location[2]);
		}
		return force;
	}

	// **************************************************************************
	// SOM and Interaction with PO & CellElements (add, remove, ..)
	// **************************************************************************


	/** 
	 * Returns an instance of a class implementing SpatialOrganizationNode. 
	 * If it is the first node of the simulation, it will fix the Class type of the SpatialOrganizationNode.
	 * CAUTION : NEVER call this method if there exist already SpatialOrganizationNodes in 
	 * the simulation, and initialNode in ECM has not been instatialized : there will then be
	 * two different unconnected Delaunay
	 * @param position
	 * @param userObject
	 * @return
	 */
	public SpatialOrganizationNode<PhysicalNode> getSpatialOrganizationNodeInstance(double[] position, PhysicalNode userObject){
		if(initialNode == null){
			SpaceNode<PhysicalNode> sn1 = new SpaceNode<PhysicalNode>(position,userObject);
			PhysicalNodeMovementListener listener = new PhysicalNodeMovementListener();
//			XX_oldMoveListener listener = new XX_oldMoveListener();
			sn1.addSpatialOrganizationNodeMovementListener(listener);
			initialNode = sn1;
			return sn1;
		}
		try {
			return initialNode.getNewInstance(position, userObject);
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
			return null;
		}
	}

	/** 
	 * Returns an instance of a class implementing SpatialOrganizationNode. 
	 * If it is the first node of the simulation, it will fix the Class type of the SpatialOrganizationNode.
	 * CAUTION : NEVER call this method if there exist already SpatialOrganizationNodes in 
	 * the simulation, and initialNode in ECM has not been instantiated : there will then be
	 * two different unconnected Delaunay
	 * @param n an already existing SpaceNode close to the place where the new one should be
	 * @param position
	 * @param userObject
	 * @return
	 */
	public SpatialOrganizationNode<PhysicalNode> getSpatialOrganizationNodeInstance(
			SpatialOrganizationNode<PhysicalNode> n, double[] position, PhysicalNode userObject){
		if(initialNode == null){
			SpaceNode<PhysicalNode> sn1 = new SpaceNode<PhysicalNode>(position,userObject);
			PhysicalNodeMovementListener listener = new PhysicalNodeMovementListener();
//			XX_oldMoveListener listener = new XX_oldMoveListener();
			sn1.addSpatialOrganizationNodeMovementListener(listener);
			initialNode = sn1;
			return sn1;

		}
		try {
			return n.getNewInstance(position, userObject);
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
			return null;
		}
	}


	/**
	 * Adding some "dummy nodes", i.e. some  PhysicalSplace.
	 * @param x1 lower boundary on the X axis
	 * @param x2 upper boundary on the X axis
	 * @param y1 upper boundary on the Y axis
	 * @param y2 lower boundary on the Y axis
	 * @param z1 upper boundary on the Z axis
	 * @param z2 lower boundary on the Z axis
	 * @param d inter-node distance
	 */
	public void addGridOfPhysicalNodes(double x1, double x2, double y1, double y2, double z1, double z2, double d) {
		// distance outside the boundary where you put your first nodes
		double borderLength = 20;

		// finding the number of nodes in each coordinate (total length / internode dist.)
		int xLim = (int)((x2-x1 + 2*borderLength) / d); 
		int yLim = (int)((y2-y1 + 2*borderLength) / d); 
		int zLim = (int)((z2-z1 + 2*borderLength) / d); 

		// the neighbor Node (close to which we will create the new one
		SpatialOrganizationNode<PhysicalNode> oldSon = null;
		// loop to put the nodes in 3D space
		for (int kx = 0; kx < xLim+1; kx++) {
			for (int ky = 0; ky < yLim+1; ky++) {
				for (int kz = 0; kz < zLim+1; kz++) {
					// finding exact position
					double[] coord = new double[] { 	(x1-borderLength) + d * kx,
							(y1-borderLength) + d * ky,
							(z1-borderLength) + d * kz };
					// add small jitter
					coord = add(coord, randomNoise(d*0.01, 3));
					// create the node
					PhysicalNode pn = new PhysicalNode();
					// request a delaunay vertex
					SpatialOrganizationNode<PhysicalNode> newSon; 
					if(oldSon!=null){
						newSon = getSpatialOrganizationNodeInstance(oldSon, coord, pn);
					}else{
						newSon = getSpatialOrganizationNodeInstance(coord, pn);
					}
					// setting call-back
					pn.setSoNode(newSon);
					// register this node as in ECM
					addPhysicalNode(pn);
					// becomes the neighbor of the next node
					oldSon = newSon;
				}
			}
		}
	}

	/** Adds a "simple" PhysicalNode (with its SON) at a desired location.*/ 
	public PhysicalNode getPhysicalNodeInstance(double[] nodeLocation){
		PhysicalNode pn = new PhysicalNode();
		SpatialOrganizationNode<PhysicalNode> son = getSpatialOrganizationNodeInstance(nodeLocation, pn);
		pn.setSoNode(son);
		addPhysicalNode(pn);
		return pn;
	}

	// Physical Objects-------------------------------------------
	// PhysicalCylinder and PhysicalSphere are also instances of PhysicalNode.
	// PhysicalNode contains a SpatialOrganizerNode.
	// So: add/remove PhysicalCylinder/PhysicalSphere makes a call to
	// add/remove-PhysicalNode.
	// the later also calls the remove() method of the associated SpatialOrganizationNode.

	public void addPhysicalCylinder(PhysicalCylinder newCylinder) {
		physicalCylinderList.add(newCylinder);
		addPhysicalNode(newCylinder);
	}

	public void removePhysicalCylinder(PhysicalCylinder oldCylinder) {
		physicalCylinderList.remove(oldCylinder);
		removePhysicalNode(oldCylinder);
	}

	public void addPhysicalSphere(PhysicalSphere newSphere) {
		physicalSphereList.add(newSphere);
		addPhysicalNode(newSphere);
	}

	public void removePhysicalSphere(PhysicalSphere oldSphere) {
		physicalSphereList.remove(oldSphere);
		removePhysicalNode(oldSphere);
	}

	public void addPhysicalNode(PhysicalNode newPhysicalNode) {
		physicalNodeList.add(newPhysicalNode);
	}

	public void removePhysicalNode(PhysicalNode oldPhysicalNode) {
		physicalNodeList.remove(oldPhysicalNode);
		oldPhysicalNode.getSoNode().remove();
	}

	public void addECMChemicalReaction(ECMChemicalReaction chemicalReaction){
		ecmChemicalReactionList.add(chemicalReaction);
	}
	
	public void removeECMChemicalReaction(ECMChemicalReaction chemicalReaction){
		ecmChemicalReactionList.remove(chemicalReaction);
	}
	
//	Cells

	public void addCell(Cell newCell) {
		cellList.add(newCell);
	}

	public void removeCell(Cell oldCell) {
		cellList.remove(oldCell);
		
	}
	
	// Cell Elements--------------------------------------------------
	public void addSomaElement(SomaElement newSoma) {
		somaElementList.add(newSoma);
	}

	public void removeSomaElement(SomaElement oldSoma) {
		somaElementList.remove(oldSoma);
	}

	public void addNeuriteElement(NeuriteElement newNE) {
		neuriteElementList.add(newNE);
	}

	public void removeNeuriteElement(NeuriteElement oldNE) {
		neuriteElementList.remove(oldNE);
	}

	public void resetTime()
	{
		this.ECMtime = 0.0;
	}

	/**
	 * Removes all the objects in the simulation, including SpaceNodes and the triangulation.
	 */
	public void clearAll(){
		// Layer 1 : Cells
		cellList  = new Vector<Cell>();
		// Layer 2 : local biology
		somaElementList  = new Vector<SomaElement>();
		neuriteElementList = new Vector<NeuriteElement>();
		
		// Layer 3 : physics 
		physicalNodeList = new Vector<PhysicalNode>();
		physicalSphereList = new Vector<PhysicalSphere>();
		physicalCylinderList = new Vector<PhysicalCylinder>();
		allArtificialSubstances.clear();
		gaussianArtificialConcentrationX.clear();
		gaussianArtificialConcentrationZ.clear();
		linearArtificialConcentrationX.clear();
		linearArtificialConcentrationZ.clear();
		this.intracellularSubstancesLibrary.clear();
		this.substancesLibrary.clear();
		// Layer 4 : triangulation
		initialNode = null;
		if(myGuiCreator!=null) myGuiCreator.removeAllChemicalSubstances();
		// Clear GUI:
		if(view!=null){
			view.repaint();
		}
	}

	
	// *********************************************************************
	// *** Substances (real chemicals)
	// *********************************************************************
	
	/** Define a template for each (extra-cellular) <code>Substance</code> in the simulation that has
	 * non-standard values for diffusion and degradation constant.*/
	public void addNewSubstanceTemplate(Substance s){
		substancesLibrary.put(s.getId(), s);
		if(myGuiCreator!=null) myGuiCreator.addNewChemical(s);
		
	}
	
	/** Define a template for each <code>IntracellularSubstance</code> in the simulation that has
	 * non-standard values for diffusion and degradation constant, and outside visibility and volume dependency.*/
	public void addNewIntracellularSubstanceTemplate(IntracellularSubstance s){
		intracellularSubstancesLibrary.put(s.getId(), s);
		if(myGuiCreator!=null)myGuiCreator.addNewChemical(s);
	}

	/** Returns an instance of <code>Substance</code>. If a similar substance (with the same id)
	 * has already been declared as a template Substance, a copy of it is made (with
	 * same id, degradation and diffusion constant, but concentration and quantity 0).
	 * If it is the first time that this id is requested, a new template Substance is made
	 * (with by-default values) and stored, and a copy will be returned.
	 * @param id
	 * @return new Substance instance
	 */
	public Substance substanceInstance(String id){
		Substance s = substancesLibrary.get(id);
		if(s==null){
			s = new Substance();
			s.setId(id);
			// s will have the default color blue, diff const 1000 and degrad const 0.01
			substancesLibrary.put(id, s);
		}
		return new Substance(s);
	}

	/** Returns an instance of <code>IntracellularSubstance</code>. If a similar 
	 * IntracellularSubstance (with the same id) has already been declared as a template 
	 * IntracellularSubstance, a copy of it is made (with same id, degradation constant, 
	 * diffusion constant, outside visibility and volume dependency, but concentration 
	 * and quantity 0).
	 * If it is the first time that this id is requested, a new template IntracellularSubstance
	 *  is made (with by-default values) and stored, and a copy will be returned.
	 * @param id
	 * @return new IntracellularSubstance instance
	 */
	public IntracellularSubstance intracellularSubstanceInstance(String id){
		IntracellularSubstance s = intracellularSubstancesLibrary.get(id);
		if(s==null){
			s = new IntracellularSubstance();
			s.setId(id);
			// s will have the default color blue, diff const 1000, degrad const 0.01,
			// visibleFromOuside false and volumeDep false.
			this.addNewIntracellularSubstanceTemplate(s);
			intracellularSubstancesLibrary.put(id, s);
		}
		return new IntracellularSubstance(s);
	}
	// *********************************************************************
	// *** Pre-defined cellType colors
	// *********************************************************************
	public void addNewCellTypeColor(String cellType, Color color){
		cellTypeColorLibrary.put(cellType, color);
	}

	public Color cellTypeColor(String cellType){
		//Select color from a list of cellsTypes
		Color c;
		if (cellType == null){						// if cell type is null				
			cellType = "null";
		}
		c = cellTypeColorLibrary.get(cellType);
		if(c==null){
			c = new Color((float) ECM.getRandomDouble(),(float) ECM.getRandomDouble(),(float) ECM.getRandomDouble(),0.7f);
			cellTypeColorLibrary.put(cellType, c);
		}
		return c;
	}

	// *********************************************************************
	// *** Artificial concentration of chemicals
	// *********************************************************************

	/** Returns true if some artificial gradient (of any sorts) have been defined.*/
	public boolean thereAreArtificialGradients(){
		return anyArtificialGradientDefined;
	}

	/* If as Substance is not already registered, we register it for you. No charges! Order now!*/
	private Substance getRegisteredArtificialSubstance(Substance substance){
		Substance registeredSubstance = allArtificialSubstances.get(substance.getId());
		if(registeredSubstance!=null){
			return registeredSubstance;
		}else{
			registeredSubstance = substance.getCopy();
			allArtificialSubstances.put(substance.getId(), registeredSubstance);
			anyArtificialGradientDefined = true;
			if(myGuiCreator!=null) myGuiCreator.addNewChemical(substance);
			return registeredSubstance;
		}
	}
	/* If as Substance is not already registered, we register it for you. No charges! Order now!*/
	private Substance getRegisteredArtificialSubstance(String substanceId){
		Substance registeredSubstance = allArtificialSubstances.get(substanceId);
		if(registeredSubstance!=null){
			return registeredSubstance;
		}else{
			registeredSubstance = new Substance(substanceId,Color.blue);;
			allArtificialSubstances.put(substanceId, registeredSubstance);
			anyArtificialGradientDefined = true;
			if(myGuiCreator!=null) myGuiCreator.addNewChemical(registeredSubstance);
			return registeredSubstance;
		}
	}
	
	
	/**
	 * Defines a bell-shaped artificial concentration in ECM, along the Z axis (ie uniform along X,Y axis). 
	 * It is a continuous value, and not instances of the class Substance!.
	 *   
	 * @param substance 
	 * @param maxConcentration the value of the concentration at its peak
	 * @param zCoord the location of the peak
	 * @param sigma the thickness of the layer (= the variance)
	 */
	public void addArtificialGaussianConcentrationZ(Substance substance, double maxConcentration, double zCoord, double sigma){
		substance = getRegisteredArtificialSubstance(substance);
		// define distribution values for the chemical, and store them together
		double[] value = new double[] {maxConcentration, zCoord, sigma}; 
		gaussianArtificialConcentrationZ.put(substance, value);
	}
	
	/**
	 * Defines a bell-shaped artificial concentration in ECM, along the Z axis (ie uniform along X,Y axis). 
	 * It is a continuous value, and not instances of the class Substance!.
	 *   
	 * @param substanceName 
	 * @param maxConcentration the value of the concentration at its peak
	 * @param zCoord the location of the peak
	 * @param sigma the thickness of the layer (= the variance)
	 */
	public void addArtificialGaussianConcentrationZ(String substanceName, double maxConcentration, double zCoord, double sigma){
		Substance substance = getRegisteredArtificialSubstance(substanceName);
		// define distribution values for the chemical, and store them together
		double[] value = new double[] {maxConcentration, zCoord, sigma}; 
		gaussianArtificialConcentrationZ.put(substance, value);
	}

	/**
	 * Defines a linear artificial concentration in ECM, between two points along the Z axis. Outside this interval
	 * the value will be 0. Between the interval the value is the linear interpolation between 
	 * the maximum value and 0.
	 * 
	 * It is a continuous value, and not instances of the class Substance!
	 * 
	 * @param Substance
	 * @param maxConcentration the value of the concentration at its peak
	 * @param zCoordMax the location of the peak 
	 * @param zCoordMin the location where the concentration reaches the value 0
	 */
	public void addArtificialLinearConcentrationZ(Substance substance, double maxConcentration, double zCoordMax, double zCoordMin){
		substance = getRegisteredArtificialSubstance(substance);
		// define distribution values for the chemical, and store them together
		double[] value = new double[] {maxConcentration, zCoordMax, zCoordMin}; 
		linearArtificialConcentrationZ.put(substance, value);
	}
	
	/**
	 * Defines a linear artificial concentration in ECM, between two points along the Z axis. Outside this interval
	 * the value will be 0. Between the interval the value is the linear interpolation between 
	 * the maximum value and 0.
	 * 
	 * It is a continuous value, and not instances of the class Substance!
	 * 
	 * @param nameOfTheChemical
	 * @param maxConcentration the value of the concentration at its peak
	 * @param zCoordMax the location of the peak 
	 * @param zCoordMin the location where the concentration reaches the value 0
	 */
	public void addArtificialLinearConcentrationZ(String substanceName, double maxConcentration, double zCoordMax, double zCoordMin){
		Substance substance = getRegisteredArtificialSubstance(substanceName);
		// define distribution values for the chemical, and store them together
		double[] value = new double[] {maxConcentration, zCoordMax, zCoordMin}; 
		linearArtificialConcentrationZ.put(substance, value);
	}
	
	/**
	 * Defines a bell-shaped artificial concentration in ECM, along the X axis (ie uniform along Y,Z axis). 
	 * It is a continuous value, and not instances of the class Substance!.
	 *   
	 * @param nameOfTheChemical 
	 * @param maxConcentration the value of the concentration at its peak
	 * @param xCoord the location of the peak
	 * @param sigma the thickness of the layer (= the variance)
	 */
	public void addArtificialGaussianConcentrationX(Substance substance, double maxConcentration, double xCoord, double sigma){
		// look if we already have a substance with the same id
		substance = getRegisteredArtificialSubstance(substance);
		// define distribution values for the chemical, and store them together
		double[] value = new double[] {maxConcentration, xCoord, sigma}; 
		gaussianArtificialConcentrationX.put(substance, value);
	}
	
	/**
	 * Defines a bell-shaped artificial concentration in ECM, along the X axis (ie uniform along Y,Z axis). 
	 * It is a continuous value, and not instances of the class Substance!.
	 *   
	 * @param nameOfTheChemical 
	 * @param maxConcentration the value of the concentration at its peak
	 * @param xCoord the location of the peak
	 * @param sigma the thickness of the layer (= the variance)
	 */
	public void addArtificialGaussianConcentrationX(String substanceName, double maxConcentration, double xCoord, double sigma){
		// look if we already have a substance with the same id
		Substance substance = getRegisteredArtificialSubstance(substanceName);
		// define distribution values for the chemical, and store them together
		double[] value = new double[] {maxConcentration, xCoord, sigma}; 
		gaussianArtificialConcentrationX.put(substance, value);
	}

	/**
	 * Defines a linear artificial concentration in ECM, between two points along the X axis. Outside this interval
	 * the value will be 0. Between the interval the value is the linear interpolation between 
	 * the maximum value and 0.
	 * 
	 * It is a continuous value, and not instances of the class Substance!
	 * 
	 * @param nameOfTheChemical
	 * @param maxConcentration the value of the concentration at its peak
	 * @param xCoordMax the location of the peak 
	 * @param xCoordMin the location where the concentration reaches the value 0
	 */
	public void addArtificialLinearConcentrationX(Substance substance, double maxConcentration, double xCoordMax, double xCoordMin){
		// look if we already have a substance with the same id
		substance = getRegisteredArtificialSubstance(substance);
		// define distribution values for the chemical, and store them together
		double[] value = new double[] {maxConcentration, xCoordMax, xCoordMin}; 
		linearArtificialConcentrationX.put(substance, value);
	}
	
	/**
	 * Defines a linear artificial concentration in ECM, between two points along the X axis. Outside this interval
	 * the value will be 0. Between the interval the value is the linear interpolation between 
	 * the maximum value and 0.
	 * 
	 * It is a continuous value, and not instances of the class Substance!
	 * 
	 * @param nameOfTheChemical
	 * @param maxConcentration the value of the concentration at its peak
	 * @param xCoordMax the location of the peak 
	 * @param xCoordMin the location where the concentration reaches the value 0
	 */
	public void addArtificialLinearConcentrationX(String substanceId, double maxConcentration, double xCoordMax, double xCoordMin){
		// look if we already have a substance with the same id
		Substance substance = getRegisteredArtificialSubstance(substanceId);
		// define distribution values for the chemical, and store them together
		double[] value = new double[] {maxConcentration, xCoordMax, xCoordMin}; 
		linearArtificialConcentrationX.put(substance, value);
	}

	/**
	 * Gets the value of a chemical, at a specific position in space
	 * @param nameOfTheChemical
	 * @param position the location [x,y,z]
	 * @return
	 */
	public double getValueArtificialConcentration(String nameOfTheChemical, double[] position){
		double x = position[0];
		double z = position[2];
		// does the substance exist at all ?
		Substance sub = null;
		if(allArtificialSubstances.containsKey(nameOfTheChemical)){
			sub = allArtificialSubstances.get(nameOfTheChemical);
		}else{
			return 0;
		}
		// if yes, we look for every type of gradient it might be implicated in,
		// and we add them up
		double concentration = 0;
		// X Gaussian
		if(gaussianArtificialConcentrationX.containsKey(sub)){
			double[] val = gaussianArtificialConcentrationX.get(sub);
			double exposant = (x-val[1])/val[2];
			exposant = exposant*exposant*0.5;
			concentration += val[0]*Math.exp(-exposant);
		}
		// Z Gaussian
		if(gaussianArtificialConcentrationZ.containsKey(sub)){
			double[] val = gaussianArtificialConcentrationZ.get(sub);
			double exposant = (z-val[1])/val[2];
			exposant = exposant*exposant*0.5;
			concentration += val[0]*Math.exp(-exposant);
		}
		// X linear
		if(linearArtificialConcentrationX.containsKey(sub)){
			double[] val = linearArtificialConcentrationX.get(sub);
			// only if between max and min
			if( (x<val[1] && x>val[2]) || (x>val[1] && x<val[2]) ){
				double slope = val[0]/(val[1]-val[2]);
				double result = (x-val[2])*slope;
				concentration += result;
			}
		}
		// Z linear
		if(linearArtificialConcentrationZ.containsKey(sub)){
			double[] val = linearArtificialConcentrationZ.get(sub);
			// only if between max and min
			if( (z<val[1] && z>val[2]) || (z>val[1] && z<val[2]) ){
				double slope = val[0]/(val[1]-val[2]);
				double result = (z-val[2])*slope;
				concentration += result;
			}
		}
		return concentration;
	}


	/**
	 * Gets the value of a chemical, at a specific position in space
	 * @param nameOfTheChemical
	 * @param position the location [x,y,z]
	 * @return
	 */
	public double getValueArtificialConcentration(Substance substance, double[] position){
		return getValueArtificialConcentration(substance.getId(), position);
	}
	///////// GRADIENT
	/**
	 * Gets the gradient of a chemical, at a specific altitude
	 * @param nameOfTheChemical
	 * @param position the location [x,y,z]
	 * @return the gradient [dc/dx , dc/dy , dc/dz]
	 */
	public double[] getGradientArtificialConcentration(String nameOfTheChemical, double[] position){
		// Do we have the substance in stock?
		Substance sub = null;
		if(allArtificialSubstances.containsKey(nameOfTheChemical)){
			sub = allArtificialSubstances.get(nameOfTheChemical);
		}else{
			return new double[] {0,0,0};
		}
		// if yes, we look for every type of gradient it might be implicated in
		double[] gradient = {0,0,0};
		double x = position[0];
		double z = position[2];
		// Gaussian X
		if(gaussianArtificialConcentrationX.containsKey(sub)){
			double[] val = gaussianArtificialConcentrationX.get(sub);
			double exposant = (x-val[1])/val[2];
			exposant = exposant*exposant*0.5;
			double xValOfGradient = -((x-val[1])/(val[2]*val[2])) *val[0]*Math.exp(-exposant);
			gradient[0] += xValOfGradient;
		}
		// Gaussian Z
		if(gaussianArtificialConcentrationZ.containsKey(sub)){
			double[] val = gaussianArtificialConcentrationZ.get(sub);
			double exposant = (z-val[1])/val[2];
			exposant = exposant*exposant*0.5;
			double zValOfGradient = -((z-val[1])/(val[2]*val[2])) *val[0]*Math.exp(-exposant);
			gradient[2] += zValOfGradient;
		}
		// Linear X
		if(linearArtificialConcentrationX.containsKey(sub)){
			double[] val = linearArtificialConcentrationX.get(sub);
			// only if x between max and min
			if (val[1]>x && x>val[2]){	// if up is higher, the gradient points up
				double slope = val[0]/(val[1]-val[2]);
				gradient[0] += slope;
			}
			if (val[1]<x && x<val[2]){
				double slope = val[0]/(val[1]-val[2]); // otherwise the gradient points down
				gradient[0] += slope;
			}
		}
		// Linear Z
		if(linearArtificialConcentrationZ.containsKey(sub)){
			double[] val = linearArtificialConcentrationZ.get(sub);
			// only if x between max and min
			if (val[1]>z && z>val[2]){	// if up is higher, the gradient points up
				double slope = val[0]/(val[1]-val[2]);
				gradient[2] += slope;
			}
			if (val[1]<z && z<val[2]){
				double slope = val[0]/(val[1]-val[2]); // otherwise the gradient points down
				gradient[2] += slope;
			}
		}
		return gradient;
	}


	public double getGradientArtificialConcentration(Substance s, double[] position){
		return getValueArtificialConcentration(s.getId(), position);
	}


	// **************************************************************************
	// Simulation Time
	// **************************************************************************
	public  String nicelyWrittenECMtime(){
		double hours = Math.floor(ECMtime);
		double minutes = ECMtime-hours;

		minutes*=60.0;

		minutes *=100;							// to avoid decimals
		minutes = Math.floor(minutes);
		minutes/= 100;

		if(hours<24){
			return ""+hours+" h "+minutes+" min.";
		}
		double hdivided = hours/24.0; 
		double days = Math.floor(hdivided); 
		hours = hours-days*24;
		return ""+days+" days "+hours+" hours "+minutes+" min.";
	}

	// **************************************************************************
	// GUI & pause
	// **************************************************************************
	public View createGUI(){
		myGuiCreator= new ECM_GUI_Creator();
		view = myGuiCreator.createPrincipalGUIWindow();
		for (Substance s : allArtificialSubstances.values()) {
			myGuiCreator.addNewChemical(s);
		}
		for (Substance s : this.intracellularSubstancesLibrary.values()) {
			myGuiCreator.addNewChemical(s);
		}
		for (Substance s : this.substancesLibrary.values()) {
			myGuiCreator.addNewChemical(s);
		}
		return view;
	}

	public View createGUI(int x, int y, int width, int height){
		myGuiCreator= new ECM_GUI_Creator();
		view = myGuiCreator.createPrincipalGUIWindow(x,y,width,height);
		for (Substance s : allArtificialSubstances.values()) {
			myGuiCreator.addNewChemical(s);
		}
		for (Substance s : this.intracellularSubstancesLibrary.values()) {
			myGuiCreator.addNewChemical(s);
		}
		for (Substance s : this.substancesLibrary.values()) {
			myGuiCreator.addNewChemical(s);
		}
		return view;
	}

	public static void pause(int time){
		try{
			Thread.sleep(time);
		}catch(Exception e){

		}
	}

	public void dumpImage() {
		// name of the file
		if(getPicturesName()==null){
			JFileChooser fc = new JFileChooser();
			int returnVal = fc.showSaveDialog(null);
			if (returnVal == JFileChooser.APPROVE_OPTION) {
				File file = fc.getSelectedFile();
				setPicturesName(file.getAbsolutePath());
				System.out.println(file);
			} else {
				return;
			}
		}
		dumpImage(getPicturesName() + String.format("%07d", pictureNumber));
		pictureNumber += 1;
	}

	public void dumpImage(String name) {
		int dimX = view.smallWindowRectangle.width+view.smallWindowRectangle.x;
		int dimY = view.smallWindowRectangle.height+view.smallWindowRectangle.y;
		BufferedImage im = new BufferedImage(dimX, dimY,BufferedImage.TYPE_INT_RGB);
		System.out.println("roi"+view.smallWindowRectangle.toString());
		Graphics g = im.getGraphics();
		view.paint(g);
		im = im.getSubimage(view.smallWindowRectangle.x, view.smallWindowRectangle.y,
				view.smallWindowRectangle.width, view.smallWindowRectangle.height);
		System.out.println("1");
		System.out.println(view.smallWindowRectangle.x+" "+ view.smallWindowRectangle.y);
		try {
			System.out.println("2");
			File f = new File(name + ".png");
			ImageIO.write(im, "png", f);
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * @return the physicalNodeList
	 */
	public Vector<PhysicalNode> getPhysicalNodeList() {
		return physicalNodeList;
	}


	/**
	 * @return the cellList
	 */
	public Vector<Cell> getCellList(){
		return cellList;
	}


	public View getView() {
		return view;
	}

	public void setView(View view) {
		this.view = view;
	}

	public boolean isSimulationOnPause() {
		return simulationOnPause;
	}

	public void setSimulationOnPause(boolean simulationOnPause) {
		this.simulationOnPause = simulationOnPause;
		if(myGuiCreator !=null){
			myGuiCreator.setBauseButtonTo(simulationOnPause);
		}
	}

	public boolean isContinuouslyRotating() {
		return continuouslyRotating;
	}

	public void setContinuouslyRotating(boolean continuouslyRotating) {
		this.continuouslyRotating = continuouslyRotating;
	}

	public boolean isTakingSnapshotAtEachTimeStep() {
		return takingSnapshotAtEachTimeStep;
	}

	public void setTakingSnapshotAtEachTimeStep(boolean takingSnapshotAtEachTimeStep) {
		this.takingSnapshotAtEachTimeStep = takingSnapshotAtEachTimeStep;
	}

	public boolean isTakingSnapshotEach100TimeSteps() {
		return takingSnapshotEach100TimeSteps;
	}

	public void setTakingSnapshotEach100TimeSteps(boolean takingSnapshotEach100TimeSteps) {
		this.takingSnapshotEach100TimeSteps = takingSnapshotEach100TimeSteps;
	}

	public Vector<PhysicalSphere> getPhysicalSphereList() {
		return physicalSphereList;
	}

	public Vector<PhysicalCylinder> getPhysicalCylinderList() {
		return physicalCylinderList;
	}

	public Vector<NeuriteElement> getNeuriteElementList() {
		return neuriteElementList;
	}

	public boolean isAnyArtificialGradientDefined() {
		return anyArtificialGradientDefined;
	}

	public Hashtable<Substance, double[]> getGaussianArtificialConcentrationZ() {
		return gaussianArtificialConcentrationZ;
	}

	public Hashtable<Substance, double[]> getLinearArtificialConcentrationZ() {
		return linearArtificialConcentrationZ;
	}

	public Hashtable<Substance, double[]> getGaussianArtificialConcentrationX() {
		return gaussianArtificialConcentrationX;
	}

	public Hashtable<Substance, double[]> getLinearArtificialConcentrationX() {
		return linearArtificialConcentrationX;
	}

	public double getECMtime() {
		return ECMtime;
	}

	public void setECMtime(double ECMtime) {
		this.ECMtime = ECMtime;
	}

	public void increaseECMtime(double deltaT){
		ECMtime += deltaT;
	}

	public void setCellList(Vector<Cell> cellList) {
		this.cellList = cellList;
	}

	/**
	 * Sets weather ECM gui is the last window to close or not. 
	 * if not do not exit the program.
	 * @param b true if its to be exited at the end.
	 */
	public void setIsLast(boolean b) {
		if(b)
		{
			ECM_GUI_Creator.operationToTakeOnClose(JFrame.EXIT_ON_CLOSE);
		}
		else
		{
			ECM_GUI_Creator.operationToTakeOnClose(JFrame.DO_NOTHING_ON_CLOSE);
		}
	}

	public Hashtable<String, IntracellularSubstance> getIntracelularSubstanceTemplates() {
		// TODO Auto-generated method stub
		return this.intracellularSubstancesLibrary;
	}
	
	public Hashtable<String, Substance> getSubstanceTemplates() {
		// TODO Auto-generated method stub
		return this.substancesLibrary;
	}

	public double[] getMinBounds() {
		// TODO Auto-generated method stub
		return new double []{Xmin,Ymin,Zmin};
	}

	public double[] getMaxBounds() {
		// TODO Auto-generated method stub
		return new double []{Xmax,Ymax,Zmax};
	}




	// **************************************************************************
	// Getters & Setters
	// **************************************************************************

	public void saveToFile(String file)
	{
		
	}
	
	public static void loadFromFile(String file)
	{
		
	}

	public void setPicturesName(String picturesName) {
		this.picturesName = picturesName;
	}

	public String getPicturesName() {
		return picturesName;
	}




}
