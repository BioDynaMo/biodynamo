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

package ini.cx3d.physics;

import static ini.cx3d.utilities.Matrix.dot;
import ini.cx3d.Param;
import ini.cx3d.localBiology.CellElement;
import ini.cx3d.synapses.Excrescence;

import java.awt.Color;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;


/**
 * Superclass of all the physical objects of the simulation (<code>PhysicalSphere</code> and 
 * <code>PhysicalCylinder</code>). It contains methods for different kinds of task: 
 * (1) to organize discrete elements composing the same neuron, in a tree-like structure 
 * (2) to communicate with the local biology module (<code>CellElement</code>) 
 * (3) to run the (inter-object) physics
 * (4) to run intracellular diffusion of <code>IntracellularSubstances</code>. 
 * <p>
 * There are three different coordinates systems :
 * global : the global unique cartesian coordinates ([1,0,0], [0,1,0], [0,0,1])
 * local: the local coord (xAxis, yAxis, zAxis)
 * polar: cylindrical (for PhysicalCylinder) or spherical (for PhysicalSphere)
 * There exist methods to transform the polar (cylindrical/spherical) into a global
 * (cartesian) system, and for transform from global to local..
 * 
 */
public abstract class PhysicalObject extends PhysicalNode {

	
	
	// * The simulation of Force in this simulation.*/
	static InterObjectForce interObjectForce = new DefaultForce();
	
	/*
	 * Tells if a PhysicalObject is still part of the simulation. 
	 * If an object is deleted (either by fusion of two segments, or after retraction),
	 * the value becomes false. Needed because of the copy vector (in a
	 * random order) of the elements vectors in ECM - to avoid that the run
	 * methods are called for elements that were just erased.
	 */
	private boolean stillExisting = true;

	
	/* If true, the PhysicalObject will be run by the Scheduler.
	 * Caution : not the same than onTheSchedulerListForPhysicalNodes.*/
	private boolean onTheSchedulerListForPhysicalObjects = true;


	/* The unique point mass of the object*/
	double[] massLocation = {0.0, 0.0, 0.0};

	/* First axis of the local coordinate system.*/
	double[] xAxis = {1.0, 0.0, 0.0};
	/* Second axis of the local coordinate system.*/
	double[] yAxis = {0.0, 1.0, 0.0};
	/* Third axis of the local coordinate system.*/
	double[] zAxis = {0.0, 0.0, 1.0};

	/* static friction (the minimum force amplitude for triggering a movement). */
	double adherence = 0.1;
	/* kinetic friction (scales the movement amplitude, therefore is called "mass")*/
	double mass = 1; 
	/* diameter of the object (wheter if sphere or cylinder).*/
	double diameter = 1;
	/* volume of this PhysicalObject; updated in updatePhysicalProperties() */
	double volume = 1;

	/* Color used when displaying the object*/
	Color color = Param.VIOLET;

	/* Only for display. Total force on this objects point mass, last time it was computed.
	 * 3 first components give the x,y,z coord, and last one if movement was applied (<0 means no)*/
	double[] totalForceLastTimeStep = {0.0, 0.0, 0.0, -1.0};

	/* All the internal and membrane-bound (diffusible and non-diffusible)
	 *  chemicals that are present inside the PhysicalObject.*/
	protected Hashtable<String, IntracellularSubstance> intracellularSubstances = new Hashtable<String, IntracellularSubstance>();

	/* List of the Physical bonds that this object can do (for cell adhesion where synapse formation occurs)*/
	protected Vector<PhysicalBond> physicalBonds = new Vector<PhysicalBond>();


	/* List of the Physical bonds that this object can do (for cell adhesion, to restore proper configuration)*/
	protected Vector<Excrescence> excrescences = new Vector<Excrescence>();

	/** Poor simple constructor.*/
	public PhysicalObject() {
		super();
	}

	/** Returns true because this object is a PhysicalObject.*/
	public boolean isAPhysicalObject(){
		// This function overwrites the one in PhysicalObject. 
		return true;
	}


	// *************************************************************************************
	// *      METHODS FOR NEURON TREE STRUCTURE                                            *
	// *************************************************************************************


	/**
	 * Returns true if this <code>PhysicalObject</code> and the <code>PhysicalObject</code> given as 
	 * argument have a mother-daughter or sister-sister (e.g. two daughters of a same mother) relation. 
	 */
	public abstract boolean isRelative(PhysicalObject po);


	/**
	 * Returns the absolute coordinates of the location where a <code>PhysicalObject</code> is attached
	 * to this <code>PhysicalObject</code>. Does not necessarily contain a check of the identity of the
	 *  element that makes the request.
	 * 
	 * @param daughterWhoAsks the PhysicalObject attached to us.
	 * @return the coord
	 */
	protected abstract double[] originOf(PhysicalObject daughterWhoAsks);


	/**
	 * Removal of a <code>PhysicalObject</code> from the list of our daughters.
	 * (Mainly in case of complete retraction of the daughter.*/
	protected abstract void removeDaugther(PhysicalObject daughterToRemove);


	/**
	 * Convenient way to change family links in the neuron tree structure 
	 * (mother, daughter branch). This method is useful during elongation and 
	 * retraction for intercalation/removal of elements.
	 * of elements.
	 */
	protected abstract void updateRelative(PhysicalObject oldRelative, PhysicalObject newRelative);


	// *************************************************************************************
	// *      METHODS FOR LINK WITH THE BIOLOGY PART                                       *
	// *************************************************************************************


	/** Returns the <code>CellElement</code>linked to this <code>PhysicalObject</code>.*/
	public abstract CellElement getCellElement();


	/** Adds an <code>Excrescence</code> instance to the Excrescence list of this 
	 * <code>PhysicalObject</code>.*/
	public void addExcrescence(Excrescence ex){
		getRwLock().writeLock().lock();
		excrescences.add(ex);
		getRwLock().writeLock().unlock();
	}


	/** Removes an <code>Excrescence</code> instance to the Excrescence list of this 
	 * <code>PhysicalObject</code>.*/
	public void removeExcrescence(Excrescence ex){
		getRwLock().writeLock().lock();
		excrescences.remove(ex);
		getRwLock().writeLock().unlock();
	}

	/** Active displacement of the point mass of this <code>PhysicalObject</code>. ("active" means
	 * "triggered by a biological process" like in growth or migration). This method MUST NOT be used 
	 * for displacement by purely passive (physical) force.
	 * @param speed in microns/hour
	 * @param direction a vector indicating the direction of movement
	 */
	abstract public void movePointMass(double speed, double[] direction);


	// *************************************************************************************
	// *      METHODS FOR PHYSICS (MECHANICS) COMPUTATION                                              *
	// *************************************************************************************

	/** Compute physical forces, and move accordingly to one simulation time step.*/
	public abstract void runPhysics();

	/**
	 * Returns the force that a daughter branch transmits to a mother's point
	 * mass. It consists of 1) the spring force between the mother and the
	 * daughter point masses and 2) the part of the inter-object mechanical
	 * interactions of the daughter branch (with other objects of the
	 * simulation) that is transmitted to the proximal end of the daughter 
	 * (= point-mass of the mother).
	 * 
	 * @param motherWhoAsks the PhysicalObject attached to the mass.
	 * @return the force in a double[]
	 */
	abstract double[] forceTransmittedFromDaugtherToMother(PhysicalObject motherWhoAsks);

	/**
	 * Resets some computational and physical properties (like the tension, volume) after
	 * a displacement
	 */
	abstract protected void updateDependentPhysicalVariables();

	
	/**
	 * Returns the inter-object force that the <code>PhysicalObject</code> in which the method is called applies 
	 * onto the <code>PhysicalSphere</code> given as argument.
	 * @param s
	 * @return
	 */
	abstract public double[] getForceOn(PhysicalSphere s); 

	/**
	 * Returns the inter-object force that the <code>PhysicalObject</code> in which the method is called applies 
	 * onto the <code>PhysicalCylinder</code> given as argument.
	 * @param c
	 * @return
	 */
	abstract protected double[] getForceOn(PhysicalCylinder c); 

	/**
	 * Returns true if this <code>PhysicalObject</code> and the <code>PhysicalSphere</code> given as 
	 * argument are close enough to be considered as being in contact.
	 * @param s
	 * @return
	 */
	abstract protected boolean isInContactWithSphere(PhysicalSphere s);

	/**
	 * Returns true if this <code>PhysicalObject</code> and the <code>PhysicalSphere</code> given as 
	 * argument are close enough to be considered as being in contact.
	 * @param c
	 * @return
	 */
	abstract protected boolean isInContactWithCylinder(PhysicalCylinder c);
	
	/**
	 * Returns true if this <code>PhysicalObject</code> is in contact, i.e. if it is 
	 * close enough to the <code>PhysicalObject</code> given as argument.
	 * @param o
	 * @return
	 */
	public boolean isInContact(PhysicalObject o){
		try
		{
			getRwLock().readLock().lock();
			if(o instanceof PhysicalSphere){
				return this.isInContactWithSphere((PhysicalSphere) o);
			}else{
				return this.isInContactWithCylinder((PhysicalCylinder) o);
			}
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * Returns all the neighboring objects considered as being in contact with this PhysicalObject.
	 * @return
	 */
	public Vector<PhysicalObject> getPhysicalObjectsInContact(){
		try
		{
			getRwLock().readLock().lock();
			Vector<PhysicalObject> po = new Vector<PhysicalObject>(); 
			for (PhysicalNode n : super.soNode.getNeighbors()) {
				if(n.isAPhysicalObject() && isInContact(((PhysicalObject) n)))
					po.add((PhysicalObject) n);
			}
			return po;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	// Some geometry with local and global coordinates..................................


	abstract public void changeDiameter(double speed);
	
	abstract public void changeVolume(double speed); 
	
	/* Recompute volume after diameter has changed.*/
	abstract protected void updateVolume();

	/* Recompute diameter, after volume has been change.*/
	abstract protected void updateDiameter();



	/**
	 * Returns the position in the global coordinate system (cartesian coordinates)
	 * of a point expressed in polar coordinates (cylindrical or spherical). 
	 * @param positionInPolarCoordinates a point defined in polar coordinate system of a PhysicalObject
	 * @return [x,y,z] the absolute value in space 
	 */
	abstract public double[] transformCoordinatesPolarToGlobal(double[] positionInPolarCoordinates);

	/**
	 * Returns the position in the polar coordinate system (cylindrical or spherical) 
	 * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
	 * @param positionInAbsoluteCoordinates the [x,y,z] cartesian values
	 * @return the position in local coord.
	 */
	abstract public double[] transformCoordinatesGlobalToPolar(double[] positionInAbsoluteCoordinates);

	/**
	 * Returns the position in the local coordinate system (xAxis, yXis, zAxis) 
	 * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
	 * @param positionInGlobalCoord
	 * @return
	 */
	public double[] transformCoordinatesGlobalToLocal(double[] positionInGlobalCoord){
		try{
			getRwLock().readLock().lock();
			return new double[] { 
					dot(positionInGlobalCoord,xAxis), 
					dot(positionInGlobalCoord,yAxis),
					dot(positionInGlobalCoord,zAxis)
					};
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	/**
	 * Returns the position in in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
	 * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
	 * @param positionInLocalCoord
	 * @return
	 */
	public double[] transformCoordinatesLocalToGlobal(double[] positionInLocalCoord){
		try{
			getRwLock().readLock().lock();
			return new double[] { 
					positionInLocalCoord[0]*xAxis[0] + positionInLocalCoord[1]*yAxis[0] + positionInLocalCoord[2]*zAxis[0], 
					positionInLocalCoord[0]*xAxis[1] + positionInLocalCoord[1]*yAxis[1] + positionInLocalCoord[2]*zAxis[1], 
					positionInLocalCoord[0]*xAxis[2] + positionInLocalCoord[1]*yAxis[2] + positionInLocalCoord[2]*zAxis[2] 
					};
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	
	/**
	 * Returns a unit vector, pointing out of the PhysicalObject if origin at location
	 * specified in argument.
	 * @param positionInLocalCoordinates the origin of the normal vector (local cartesian coord)
	 * @return a vector pointing "out", of unitary norm (absolute cartesian coord) 
	 */
	abstract public double[] getUnitNormalVector(double[] positionInPolarCoordinates);


	// Physical Bonds ...................................................................

	/** Simply adds the argument to the vector containing all the PhysicalBonds of this 
	 * PhysicalObject.*/  
	public void addPhysicalBond(PhysicalBond pb){
		getRwLock().writeLock().lock();
		physicalBonds.add(pb);
		getRwLock().writeLock().unlock();
	}

	/** Simply removes the argument from the vector containing all the PhysicalBonds of this 
	 * PhysicalObject. */  
	public void removePhysicalBond(PhysicalBond pb){
		getRwLock().writeLock().lock();
		physicalBonds.remove(pb);
		getRwLock().writeLock().unlock();
	}

	/** Returns true if there is a PhysicalBond that fixes me to this other PhysicalObject.*/
	public boolean getHasAPhysicalBondWith(PhysicalObject po){
		for (PhysicalBond pb : physicalBonds) {
			if(po == pb.getOppositePhysicalObject(this))
				return true;
		}
		return false;
	}

	/**
	 * Creates a new PhysicalBond between this PhysicalObject and the one given as argument. 
	 * The newly created PhysicalBond is inserted into the physical bon's list of both objects.
	 * @param po
	 * @return
	 */
	public PhysicalBond makePhysicalBondWith(PhysicalObject po){
		try
		{
			getRwLock().readLock().lock();
			PhysicalBond pb = new PhysicalBond(this,po);
//			this.physicalBonds.add(pb);
//			po.addPhysicalBond(pb);
			return pb;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	/**
	 * If there is a PhysicalBond between this PhysicalObject and po,
	 * it is removed (in both objects).  
	 * @param po the other PhysicalObject we want to test with
	 * @param removeThemAll if true, makes multiple removals (if multiple bonds)
	 * @return true if at least one PhysicalBond was removed
	 */
	public boolean removePhysicalBondWith(PhysicalObject po, boolean removeThemAll){
		try
		{
			getRwLock().writeLock().lock();
			boolean thereWasABond = false;
			for (int i = 0; i < physicalBonds.size(); i++) {
				PhysicalBond pb = physicalBonds.get(i);
				if(po == pb.getOppositePhysicalObject(this)){
					physicalBonds.remove(i);
					po.physicalBonds.remove(pb);
					if(!removeThemAll){
						return true;
					}else{
						thereWasABond = true;
						i--; // we continue to check, and since we removed the ith
					}
				}			
			}
			return thereWasABond;
		}
		finally
		{
			getRwLock().writeLock().unlock();
		}
	}

	// *************************************************************************************
	// *      METHODS FOR DIFFUSION (INTRA-CELLULAR & MEMBRANE-BOUNDED SUBSTANCES)         *
	// *************************************************************************************

	/** Compute diffusion of <code>IntracellularSubstances</code> with relatives in the neuron 
	 * tree structure, and perform diffusion processes according to one simulation time step.*/
	public abstract void runIntracellularDiffusion();

	/** 
	 * Returns the concentration of an <code>IntracellularSubstance</code> in this
	 * PhysicalObject. If not present at all, zhe value 0 is returned.
	 * @param substanceId
	 * @return
	 */
	public double getIntracellularConcentration(String substanceId){
		try
		{
			getRwLock().readLock().lock();
			Substance s = intracellularSubstances.get(substanceId);
			if(s == null){
				return 0;
			}else{
				return s.getConcentration();
			}
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
		
	}


	/** Modifies the quantity (increases or decreases) of an IntracellularSubstance. 
	 * If this <code>PhysicalNode</code> already has an <code>IntracellularSubstance</code> 
	 * instance corresponding to the type given as argument (with the same id), the fields 
	 * quantity and concentration in it will be modified, based on a computation depending
	 * on the simulation time step and the space volume (for the latter : only if volumeDependant
	 * is true in the IntracellularSubstance). If there is no such IntracellularSubstance
	 * instance already, a new instance is requested from ECM. 
	 * <p>
	 * This method is not used for diffusion, but only by biological classes...
	 * @param id the name of the Substance to change.
	 * @param quantityPerTime the rate of quantity production
	 */
	public void modifyIntracellularQuantity(String id, double quantityPerTime){
		getRwLock().writeLock().lock(); 
		
		IntracellularSubstance s = intracellularSubstances.get(id);
		if(s==null){
			s = ecm.intracellularSubstanceInstance(id);
			intracellularSubstances.put(id, s);
		}
		double deltaQ = quantityPerTime*Param.SIMULATION_TIME_STEP;
		s.changeQuantityFrom(deltaQ);
		if(s.isVolumeDependant()){
			s.updateConcentrationBasedOnQuantity(volume);
		}else{
			s.updateConcentrationBasedOnQuantity(this.getLength());
//			s.updateConcentrationBasedOnQuantity(1.0);
		}
		getRwLock().writeLock().unlock();
		  
	}

	/* adding an IntracellularSubstance instance (CAUTION : should not be used for biologic production,
	 * and therefore is not a public method. Instead , this method is used for filling up a new 
	 * PhysicalObject in case of extension).
	 */
	protected void addNewIntracellularSubstance(IntracellularSubstance s){
		getRwLock().writeLock().lock(); 
		intracellularSubstances.put(s.getId(), s);
		getRwLock().writeLock().unlock();
	}

	/** Returns the concentration of a membrane bound IntracellularSubstance on
	 * this PhysicalObject. Recall that by definition, the PhysicalObject are 
	 * considered as expressing a cell type specific protein as well as the universal
	 * marker "U".
	 * @param id
	 * @return
	 */
	public double getMembraneConcentration(String id){
		try
		{
			getRwLock().readLock().lock();
			// is it the substance that we have at our membrane because of our cell type?
			if( id == "U"){
				return 1.0;
			}
			// otherwise : do we have it on board ?
			IntracellularSubstance s = intracellularSubstances.get(id);
			if(s == null){
				return 0.0;
			}else{
				// if yes, is it a membrane substance ?
				if(!s.isVisibleFromOutside())
					return 0.0;
				return s.getConcentration();
			}
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}


	/** Modifies the quantity (increases or decreases) of an membrane-bound chemical.
	 *
	 * This method is not used for diffusion, but only by biological classes...
	 * @param id the name of the Substance to change.
	 * @param quantityPerTime the rate of quantity production
	 */

	public void modifyMembraneQuantity(String id, double quantityPerTime){
		// for now, the intracellular and membrane bound Substances are the same.
		modifyIntracellularQuantity(id, quantityPerTime); 
	}

	/* Returns the INSTANCE of IntracellularSubstance in this PhysicalObject with the same id
	 * than the IntracellularSubstance given as argument. If there is no such instance, a
	 * new one with similar properties is created, inserted into the intracellularSubstances
	 * vector and then returned. Only used between subclasses of physicalObject for intracellular
	 * diffusion. C.f. very similar method : PhysicalNode.giveYourSubstanceInstance.
	 */ 
	IntracellularSubstance giveYouIntracellularSubstanceInstance(IntracellularSubstance templateS){
		try
		{
			getRwLock().writeLock().lock(); //possible change ahead!
			IntracellularSubstance s = intracellularSubstances.get(templateS.getId());
			if(s == null){
				s = new IntracellularSubstance(templateS);
				intracellularSubstances.put(s.getId(), s);
			}
			return s;
		}
		finally
		{
			getRwLock().writeLock().unlock();
		}
	}


	/* Diffusion of diffusible IntracellularSubstances between two PhysicalObjects.
	 */
	protected void diffuseWithThisPhysicalObjects(PhysicalObject po, double distance){	

		// We store these temporary variable, because we still don't know if the
		// Substances depend on volumes or not
//		ReadWriteLock r1; 
//		ReadWriteLock r2;
//		if(po.ID != this.ID) System.out.println("not the same id!!!" + po.ID+ " "+this.ID);
//		else
//		{
//			System.out.println(" The same id!!!" + po.ID+ " "+this.ID);
//		}
//		if(po.ID>this.ID)
//		{
//			r1=po.getRwLock();
//			r2=this.getRwLock();
//		}
//		else
//		{
//			r1=this.getRwLock();
//			r2=po.getRwLock();
//		}
//		r1.writeLock().lock();
//		r2.writeLock().lock();
		
		double vA_v = this.volume;
		double vB_v = po.getVolume();
		double pre_a_v = (1.0/distance);
		double pre_m_v = (1.0/distance) * (1.0/vA_v + 1.0/vB_v);
		double vA_l = this.getLength();
		double vB_l = po.getLength();
		double pre_a_l = (1.0/distance);
		double pre_m_l = (1.0/distance) * (1.0/vA_l + 1.0/vB_l);
		
		// the variable we are effectively using
		double vA;
		double vB;
		double pre_a;
		double pre_m;

		for (Enumeration<IntracellularSubstance> substancesEnumeration = intracellularSubstances.elements(); substancesEnumeration.hasMoreElements();) {
			// for a given substance
			IntracellularSubstance sA = substancesEnumeration.nextElement();

			// does the substance depend on volumes or not ?
			if(sA.isVolumeDependant()){
				vA = vA_v;
				vB = vB_v;
				pre_a = pre_a_v;
				pre_m = pre_m_v;
			}else{
				vA = vA_l;
				vB = vB_l;
				pre_a = pre_a_l;
				pre_m = pre_m_l;
			}

			double sAConcentration = sA.getConcentration();

			// stop here if 1) non diffusible substance or 2) concentration very low:
			double diffusionConstant = sA.getDiffusionConstant();
			if(diffusionConstant<10E-14 || sAConcentration<Param.MINIMAL_CONCENTRATION_FOR_INTRACELLULAR_DIFFUSION){ 
				continue; // to avoid a division by zero in the n/m if the diff const = 0;
			}

			// find the counterpart in po
			IntracellularSubstance sB = po.giveYouIntracellularSubstanceInstance(sA);
			double sBConcentration = sB.getConcentration();

			// saving time : no diffusion if almost no difference;
			double absDiff = Math.abs(sAConcentration-sBConcentration);
			if( (absDiff<Param.MINIMAL_DIFFERENCE_CONCENTRATION_FOR_INTRACELLULAR_DIFFUSION) || 
					(absDiff/sAConcentration<Param.MINIMAL_DC_OVER_C_FOR_INTRACELLULAR_DIFFUSION)){
				continue; 
			}
			// TODO : if needed, when we come here, we have to re-put ourselves on the
			// scheduler list for intra-cellular diffusion.

			// analytic computation of the diffusion between these two PhysicalObjects
			// (cf document "Diffusion" by F.Zubler for explanation).
			double qA = sA.getQuantity();
			double qB = sB.getQuantity();
			double Tot = qA + qB;
			double a = pre_a*diffusionConstant;
			double m = pre_m*diffusionConstant;
			double n = a*Tot/vB;
			double nOverM = n/m;
			double K = qA -nOverM;
			qA = K*Math.exp(-m*Param.SIMULATION_TIME_STEP) + nOverM;
			qB = Tot - qA;

			sA.setQuantity(qA);
			sB.setQuantity(qB);
			// and update their concentration again
			// (Recall : if volumeDependant = false, vA and vB are not the true volume, 
			// but rather the length of the physical object)
			sA.updateConcentrationBasedOnQuantity(vA); 
			sB.updateConcentrationBasedOnQuantity(vB);
		}
//		r1.writeLock().unlock();
//		r2.writeLock().unlock();
	}

	/**
	 * Updates the concentration of substances, based on the volume of the object.
	 * Is usually called after change of the volume (and therefore we don't modify it here)
	 */
	protected abstract void updateIntracellularConcentrations();
	
	
	// *************************************************************************************
	// *      GETTERS & SETTERS                                                            *
	// *************************************************************************************

	/** Returns the <code>java.awt.Color</code> used to draw this PhysicalObject in the GUI. */
	public Color getColor() {
		try
		{
			getRwLock().readLock().lock();
			return color;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	/** Sets the <code>java.awt.Color</code> used to draw this PhysicalObject in the GUI. */
	public void setColor(Color color) {
		getRwLock().writeLock().lock();
		this.color = color;
		getRwLock().writeLock().unlock();
	}

	/** Returns a copy of the masslocation.*/
	public double[] getMassLocation() {
		try
		{
			getRwLock().readLock().lock();
			return  new double[] {massLocation[0],massLocation[1],massLocation[2]};
		}
		finally
		{
			getRwLock().readLock().unlock();
		}

	}

	/**
	 * - CAUTION : Never use this method to move a PhysicalObject, because the physics is not updated. 
	 * <p>
	 * - Never?
	 * <p>
	 *  - I said NEVER !
	 * @param massLocation the massLocation to set
	 */
	public void setMassLocation(double[] massLocation) {
		getRwLock().writeLock().lock();
		this.massLocation = massLocation.clone();
		getRwLock().writeLock().unlock();
	}

	/** Returns the "Up" direction for a Sphere, and the axis direction for a Cylinder*/
	public abstract double[] getAxis();

	
	/** Returns the first axis of the local coordinate system.*/
	public double[] getXAxis() {
		try
		{
			getRwLock().readLock().lock();
			return xAxis.clone();
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	/** Sets the first axis of the local coordinate system. Should have a norm of 1.*/
	public void setXAxis(double[] axis) {
		getRwLock().writeLock().lock();
		xAxis = axis.clone();
		getRwLock().writeLock().unlock();
	}
	/** Returns the second axis of the local coordinate system.*/
	public double[] getYAxis() {
		try
		{
			getRwLock().readLock().lock();
			return yAxis.clone();
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	/** Sets the second axis of the local coordinate system. Should have a norm of 1*/
	public void setYAxis(double[] axis) {
		getRwLock().writeLock().lock();
		yAxis = axis.clone();
		getRwLock().writeLock().unlock();
	}

	/** Returns the third axis of the local coordinate system.*/
	public double[] getZAxis() {
		try
		{
			getRwLock().readLock().lock();
			return zAxis;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	/** Sets the third axis of the local coordinate system. Should have a norm of 1*/
	public void setZAxis(double[] axis) {
		getRwLock().writeLock().lock();
		zAxis = axis.clone();
		getRwLock().writeLock().unlock();
	}


	/** Only for GUI display. Total force on this objects point mass, last time it was computed.
	 * 3 first components give the x,y,z coord, and last one if movement was applied (<0 means no).*/
	public double[] getTotalForceLastTimeStep(){
		try
		{
			getRwLock().readLock().lock();
			return totalForceLastTimeStep;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/** Returns true if this object still plays a role in the simulation. 
	 * For instance a PhysicalObject associated with a neurite that just retracted 
	 * is not exxisting. */
	public boolean isStillExisting() {
		try
		{
			getRwLock().readLock().lock();
			return stillExisting;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/** The role of the method is to indicate that an object is about to
	 * be garbage Collected. Caution: don't use this method! */
	public void setStillExisting(boolean stillExists) {
		getRwLock().writeLock().lock();
		this.stillExisting = stillExists;
		getRwLock().writeLock().unlock();
	}

	/** If true, this PhysicalObject will be run by the Scheduler on the next occasion.*/
	public boolean isOnTheSchedulerListForPhysicalObjects() {
		try
		{
			getRwLock().readLock().lock();
			return onTheSchedulerListForPhysicalObjects;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/** If true, this PhysicalObject will be run by the Scheduler on the next occasion.*/
	public void setOnTheSchedulerListForPhysicalObjects(
			boolean onTheSchedulerListForPhysicalObjects) {
		getRwLock().writeLock().lock();
		this.onTheSchedulerListForPhysicalObjects = onTheSchedulerListForPhysicalObjects;
		getRwLock().writeLock().unlock();
	}

	/** Returns the vector containing all the PhysicalBonds of this PhysicalObject.*/
	public Vector<PhysicalBond> getPhysicalBonds() {
		try
		{
			getRwLock().readLock().lock();
			return (Vector<PhysicalBond>) physicalBonds.clone();
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/** Sets the vector containing all the PhysicalBonds of this PhysicalObject.
	 * This methof should not be used during the simulation. */
	public void setPhysicalBonds(Vector<PhysicalBond> physicalBonds) {
		getRwLock().writeLock().lock();
		this.physicalBonds = (Vector<PhysicalBond>) physicalBonds.clone();
		getRwLock().writeLock().unlock();
	}

	/** Returns the vector containing all the Excrescences (PhysicalSpine, PhysicalBouton).*/
	public Vector<Excrescence> getExcrescences(){
		try
		{
			getRwLock().readLock().lock();
			return excrescences;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}


	/** Sets the vector containing all the Excrescences (PhysicalSpine, PhysicalBouton).
	 * This method should not be used during a simulation. */
	public void setExcrescences(Vector<Excrescence> excrescences) {
		getRwLock().writeLock().lock();
		this.excrescences = (Vector<Excrescence>) excrescences.clone();
		getRwLock().writeLock().unlock();
	}

	/** Returns the adherence to the extracellular matrix, i.e. the static friction 
	 * (the minimum force amplitude needed for triggering a movement). */
	public double getAdherence() {
		try
		{
			getRwLock().readLock().lock();
			return adherence;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}


	/** Sets the adherence to the extracellular matrix, i.e. the static friction 
	 * (the minimum force amplitude needed for triggering a movement). */
	public void setAdherence(double adherence) {
		getRwLock().writeLock().lock();
		this.adherence = adherence;
		getRwLock().writeLock().unlock();
	}


	/** Returns the mass, i.e. the kinetic friction 
	 * (scales the movement amplitude, therefore is considered as the mass).*/
	public double getMass() {
		try
		{
			getRwLock().readLock().lock();
			return mass;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}


	/** Sets the mass, i.e. the kinetic friction 
	 * (scales the movement amplitude, therefore is considered as the mass).*/
	public void setMass(double mass) {
		getRwLock().writeLock().lock();
		this.mass = mass;
		getRwLock().writeLock().unlock();
	}

	public double getDiameter() {
		try
		{
			getRwLock().readLock().lock();
			return diameter;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * Sets the diameter to a new value, and update the volume accordingly.
	 * is equivalent to setDiamater(diameter, true)
	 * @param diameter
	 */
	public void setDiameter(double diameter){
		
		setDiameter(diameter,true);
		
	}
	
	/** 
	 * Sets the diameter. The volume is sets accordingly if desired. 
	 * 
	 * @param diameter the new diameter
	 * @param updateVolume if true, the volume is set to match the new diameter.
	 */
	public void setDiameter(double diameter, boolean updateVolume) {
		getRwLock().writeLock().lock();
		this.diameter = diameter;
		if(updateVolume){
			updateVolume();
		}
		getRwLock().writeLock().unlock();
	}

	/** Returns the volume of this PhysicalObject.*/
	public double getVolume(){
		try
		{
			getRwLock().readLock().lock();
			return volume;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * Sets the volume, and (optionally) recomputes an new diameter.
	 * THIS METHOD SHOULD NOT BE USED TO INITIALIZE A PHYSICAL_OBJECT. 
	 * DEFINE DIMENSIONS, AND THE VOLUME WILL BE COMPUTED.
	 * @param volume the new volume
	 * @param updateDiameter if true, the diameter will be updated.
	 */
	public void setVolume(double volume, boolean updateDiameter){
		getRwLock().writeLock().lock();
		this.volume = volume;
		updateIntracellularConcentrations();
		if(updateDiameter){
			updateDiameter();
		}
		getRwLock().writeLock().unlock();
	}
	
	/**
	 * Sets the volume, and recomputes an new diameter.
	 * THIS METHOD SHOULD NOT BE USED TO INITIALIZE A PHYSICAL_OBJECT. 
	 * DEFINE DIMENSIONS, AND THE VOLUME WILL BE COMPUTED.
	 * @param volume the new volume
	 */
	public void setVolume(double volume){
		setVolume(volume,true);
	}

	/** Returns the length of a cylinder, or the diameter of a sphere.*/
	public abstract double getLength();
	
	/** Get an intracellular and membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
	public IntracellularSubstance getIntracellularSubstance(String id) {
		try
		{
			getRwLock().readLock().lock();
			return intracellularSubstances.get(id);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	/** Add an intracellular or membrane-bound chemicals 
	 *  in this PhysicalNode. */
	public void addIntracellularSubstance(IntracellularSubstance is) {
		try
		{
			getRwLock().readLock().lock();
			intracellularSubstances.put(is.getId(), is);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	/** Remove an intracellular or membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
	public void removeIntracellularSubstance(IntracellularSubstance is) {
		try
		{
			getRwLock().readLock().lock();
			intracellularSubstances.remove(is);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	/** All the intracellular and membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
	public Hashtable<String, IntracellularSubstance> getIntracellularSubstances() {
		try
		{
			getRwLock().readLock().lock();
			return intracellularSubstances;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}


	/** All the intracellular and membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
	public void setIntracellularSubstances(
			Hashtable<String, IntracellularSubstance> intracellularSubstances) {
		getRwLock().writeLock().lock();
		this.intracellularSubstances = (Hashtable<String, IntracellularSubstance>) intracellularSubstances.clone();
		getRwLock().writeLock().unlock();
	}

	/** The class computing the inter object force.*/
	public static InterObjectForce getInterObjectForce() {
		return interObjectForce;
	}
	
	/** The class computing the inter object force.*/
	public static void setInterObjectForce(InterObjectForce interObjectForce) {
		PhysicalObject.interObjectForce = interObjectForce;
	}

	public abstract double getInterObjectForceCoefficient();

	public abstract void setInterObjectForceCoefficient(double interObjectForceCoefficient);
	
	
}
