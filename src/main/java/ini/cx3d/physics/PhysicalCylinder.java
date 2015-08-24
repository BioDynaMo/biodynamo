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



import static ini.cx3d.utilities.Matrix.*;

import javax.swing.text.html.InlineView;

import ini.cx3d.Param;
import ini.cx3d.localBiology.CellElement;
import ini.cx3d.localBiology.LocalBiologyModule;
import ini.cx3d.localBiology.NeuriteElement;
import ini.cx3d.simulations.ECM;
import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
import ini.cx3d.synapses.Excrescence;



/**
 * A cylinder can be seen as a normal cylinder, with two end points and a diameter. It is oriented; 
 * the two points are called proximal and distal. The PhysicalCylinder is be part of a tree-like 
 * structure with (one and only) one Physical object at its proximal point and (up to) two physical Objects at
 * its distal end. If there is only one daughter,
 * it is the left one. If <code>daughterLeft == null</code>, there is no distal cylinder (this
 * is a terminal cylinder). The presence of a <code>daugtherRight</code> means that this branch has a bifurcation
 * at its distal end.
 * <p>
 * All the mass of this cylinder is concentrated at the distal point. Only the distal end is moved
 * by a PhysicalCylinder. All the forces in a cylinder that are applied to the proximal node (belonging to the 
 * mother PhysicalNode) are transmitted to the mother element
 * 
 * @author fredericzubler
 *
 */
public class PhysicalCylinder extends PhysicalObject{

	/* Local biology object associated with this PhysicalCylinder.*/
	private NeuriteElement neuriteElement = null;


	/* Parent node in the neuron tree structure (can be PhysicalSphere or PhysicalCylinder)*/  
	private PhysicalObject mother = null;
	/* First child node in the neuron tree structure (can only be PhysicalCylinder)*/
	private PhysicalCylinder daughterLeft = null;   
	/* Second child node in the neuron tree structure. (only PhysicalCylinder) */
	private PhysicalCylinder daughterRight = null; 
	/* number of branching points from here to the soma (root of the neuron tree-structure).*/
	private int branchOrder = 0;

	/* The part of the inter-object force transmitted to the mother (parent node) -- c.f. runPhysics() */
	private double[] forceToTransmitToProximalMass = {0.0, 0.0, 0.0}; 


	/* Vector from the attachment point to the massLocation (proximal -> distal).  */
	private double[] springAxis = new double[3];  	
	/* Real length of the PhysicalCylinder (norm of the springAxis). */
	private double actualLength = Param.NEURITE_DEFAULT_ACTUAL_LENGTH;
	/* Tension in the cylinder spring.*/
	private double tension = Param.NEURITE_DEFAULT_TENSION;
	/* Spring constant per distance unit (springConstant * restingLength  = "real" spring constant).*/
	private double springConstant = Param.NEURITE_DEFAULT_SPRING_CONSTANT;
	/* The length of the internal spring where tension would be zero. */
	private double restingLength = springConstant*actualLength/(tension+springConstant);	 // T = k*(A-R)/R --> R = k*A/(T+K)

	// DEBUGG
	private double oldActualLength=0;


	/** No argument constructor, initializing fields of <code>PhysicalObject</code> 
	 * with <code>Param</code> values.*/ 
	public PhysicalCylinder() {
		super();
		getRwLock().writeLock().lock();
		super.adherence = 	Param.NEURITE_DEFAULT_ADHERENCE; 
		super.mass = 		Param.NEURITE_DEFAULT_MASS; 
		super.diameter = 	Param.NEURITE_DEFAULT_DIAMETER; 
		updateVolume();
		getRwLock().writeLock().unlock();
	}

	/** Returns a <code>PhysicalCylinder</code> with all fields similar than in this 
	 * <code>PhysicalCylinder</code>. Note that the relatives in the tree structure, the
	 * tension, the volume, the  
	 * <code>CellElement</code>, as well as<code>Excrescences</code> and the 
	 * <code>IntracellularSubstances</code> are not copied. */
	public PhysicalCylinder getCopy(){
		
		PhysicalCylinder newCylinder = new PhysicalCylinder();

		// PhysicalObject variables
		getRwLock().readLock().lock();
		newCylinder.setAdherence(super.adherence);
		newCylinder.setMass(super.mass);
		newCylinder.setDiameter(super.diameter, true);  // re - computes also volumes
		newCylinder.setColor(super.color);		
		newCylinder.setStillExisting(super.isStillExisting());
		
		newCylinder.xAxis = super.xAxis.clone();
		newCylinder.yAxis = super.yAxis.clone();
		newCylinder.zAxis = super.zAxis.clone();
		// this class variable	
		newCylinder.springAxis = this.springAxis.clone();
		newCylinder.setBranchOrder(this.branchOrder);
		newCylinder.setSpringConstant(this.springConstant);
		getRwLock().readLock().unlock();
		return newCylinder;
	}



	// *************************************************************************************
	// *      METHODS FOR NEURON TREE STRUCTURE                                            *
	// *************************************************************************************



	/**
	 * Returns true if the <code>PhysicalObject</code> given as argument is a mother, daughter
	 * or sister branch.*/
	@Override
	public boolean isRelative(PhysicalObject po) {
		try
		{
			getRwLock().readLock().lock();
			// mother-daughter
			if( (po==mother)||(po==daughterLeft)||(po==daughterLeft) )
				return true;
			// sister-sister
			if(po.isAPhysicalCylinder()){
				if(((PhysicalCylinder)po).getMother() == this.mother)
					return true;
			}
			return false;
		}finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * Returns the location in absolute coordinates of where the <code>PhysicalObject</code> 
	 * given as argument is attached on this where the <code>PhysicalCylinder</code>  
	 * If the argument is one of our daughter <code>PhysicalCylinder</code>, the point mass location
	 * is returned. Otherwise, the return is <code>null</code>.
	 * 
	 * @param daughterWhoAsks the PhysicalObject requesting it's origin.
	 * 
	 */
	@Override
	public double[] originOf(PhysicalObject daughterWhoAsks) {
		// TODO : consider remove the check 
		try
		{
			getRwLock().readLock().lock();
			if(daughterWhoAsks == daughterLeft || daughterWhoAsks == daughterRight){
				return massLocation;
			}
		}finally
		{
			getRwLock().readLock().unlock();
		}
		System.out.println(this +" PhysicalCylinder.getOrigin() says : this is not one of my relatives !!!");
		(new Throwable()).printStackTrace();
		return null;
	
	}


	@Override
	protected void removeDaugther(PhysicalObject daughterToRemove) {
		// If there is another daughter than the one we want to remove,
		// we have to be sure that it will be the daughterLeft.
		
		
		if(daughterToRemove == daughterRight){
			getRwLock().writeLock().lock();
			daughterRight = null;
			getRwLock().writeLock().unlock();
			return;
		}
		
		if(daughterToRemove == daughterLeft){
			getRwLock().writeLock().lock();
			daughterLeft = daughterRight;
			daughterRight = null;
			getRwLock().writeLock().unlock();
			return;
		}
		System.out.println("PhysicalCylinder.daughterToRemove() says : this is not one of my relatives !!!");
		(new Throwable()).printStackTrace();
	}

	@Override
	protected void updateRelative(PhysicalObject oldRelative, PhysicalObject newRelative) {
		if(oldRelative == mother){
			setMother(newRelative);
			return;
		}
		if(oldRelative == daughterLeft){
			setDaughterLeft((PhysicalCylinder) newRelative);
			return;
		}
		if(oldRelative == daughterRight){
			setDaughterRight((PhysicalCylinder) newRelative);
			return;
		}
		System.out.println("PhysicalCylinder.updateRelative() says : this is not one of my relatives !!!");
		(new Throwable()).printStackTrace();
	}

	/**
	 * returns the total force that this <code>PhysicalCylinder</code> exerts on it's mother.
	 * It is the sum of the spring force an the part of the inter-object force computed earlier in
	 * <code>runPhysics()</code>.
	 */
	double[] forceTransmittedFromDaugtherToMother(PhysicalObject motherWhoAsks) {
		try
		{
			getRwLock().readLock().lock();
			if(motherWhoAsks != mother){
				System.out.println("PhysicalCylinder.forceTransmittedFromDaugtherToMother() says : this is not my mother !!!");
				(new Throwable()).printStackTrace();
			}
			// The inner tension is added to the external force that was computed earlier.
			// (The reason for dividing by the actualLength is to normalize the direction : T = T * axis/ (axis length)
			double factor =  tension/actualLength;
			if(factor<0){
				factor = 0;
			}
			return new double[] { 	factor*springAxis[0] + forceToTransmitToProximalMass[0], 
					factor*springAxis[1] + forceToTransmitToProximalMass[1],
					factor*springAxis[2] + forceToTransmitToProximalMass[2]  } ;
		}finally
		{
			getRwLock().readLock().unlock();
		}
	}




	// *************************************************************************************
	//   DISCRETIZATION , SPATIAL NODE, CELL ELEMENT
	// *************************************************************************************


	/** 
	 * Checks if this <code>PhysicalCylinder</code> is either too long (and in this case it will insert 
	 * another <code>PhysicalCylinder</code>), or too short (and in this second case fuse it with the
	 * proximal element or even delete it).
	 * */
	public boolean runDiscretization() {
		
		if(actualLength>Param.NEURITE_MAX_LENGTH){
			getRwLock().writeLock().lock();
			if(daughterLeft== null){   // if terminal branch : 
				insertProximalCylinder(0.1);
			}else if(mother.isAPhysicalSphere()){ //if initial branch :
				insertProximalCylinder(0.9);
			}else{
				insertProximalCylinder(0.5);
			}
			getRwLock().writeLock().unlock();
			return true;
		}
		

		if(	actualLength<Param.NEURITE_MIN_LENGTH &&
				mother.isAPhysicalCylinder() &&
				((PhysicalCylinder)mother).restingLength < Param.NEURITE_MAX_LENGTH - this.restingLength - 1 &&
				((PhysicalCylinder)mother).daughterRight == null &&
				this.daughterLeft != null) {
			getRwLock().writeLock().lock();
			// if the previous branch is removed, we first remove its associated NeuriteElement
			((PhysicalCylinder)mother).neuriteElement.removeYourself();
			// then we remove it
			removeProximalCylinder();
			getRwLock().writeLock().unlock();
		}
		return true;
	}

	/**
	 * Divides the PhysicalCylinder into two PhysicalCylinders of equal length. The one in which the method is called becomes the distal half.
	 * A new PhysicalCylinder is instantiated and becomes the proximal part. All characteristics are transmitted.
	 * A new Neurite element is also instantiated, and assigned to the new proximal PhysicalCylinder
	 */
	private NeuriteElement insertProximalCylinder(){
		return insertProximalCylinder(0.5);
	}

	/**
	 * Divides the PhysicalCylinder into two PhysicalCylinders (in fact, into two instances of the derived class). 
	 * The one in which the method is called becomes the distal half, and it's length is reduced.
	 * A new PhysicalCylinder is instantiated and becomes the proximal part (=the mother). All characteristics are transmitted
	 * 
	 *@param distalPortion the fraction of the total old length devoted to the distal half (should be between 0 and 1).
	 */
	private NeuriteElement insertProximalCylinder(double distalPortion){ 
		// debugg :
		getRwLock().writeLock().lock();
		this.oldActualLength = 0;
		// location
		double[] newProximalCylinderMassLocation = new double[] {	massLocation[0] - distalPortion*springAxis[0],
				massLocation[1] - distalPortion*springAxis[1],
				massLocation[2] - distalPortion*springAxis[2]  };
		double temp = distalPortion+(1-distalPortion)/2.0;
		double[] newProximalCylinderSpatialNodeLocation = new double[] {	massLocation[0] - temp*springAxis[0],
				massLocation[1] - temp*springAxis[1],
				massLocation[2] - temp*springAxis[2]  };
		// creating a new PhysicalCylinder & a new NeuriteElement, linking them together
		PhysicalCylinder newProximalCylinder = getCopy();	
		NeuriteElement ne = neuriteElement.getCopy();
		ne.setPhysical(newProximalCylinder);
		newProximalCylinder.setMassLocation(newProximalCylinderMassLocation);
		// familly relations
		this.mother.updateRelative(this, newProximalCylinder);
		newProximalCylinder.setMother(this.getMother());
		this.setMother(newProximalCylinder);
		newProximalCylinder.setDaughterLeft(this);
		// SOM relation 
		SpatialOrganizationNode<PhysicalNode> newSON = null;
		try {
			newSON = soNode.getNewInstance(newProximalCylinderSpatialNodeLocation, newProximalCylinder);
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
		}
		newProximalCylinder.setSoNode(newSON);
		// registering the new cylinder with ecm
		ecm.addPhysicalCylinder(newProximalCylinder);
		// physics
		newProximalCylinder.restingLength = (1-distalPortion)*this.restingLength;
		this.restingLength *= distalPortion;

		// intracellularSubstances quantities .....................................
		// (concentrations are solved in updateDependentPhysicalVariables():
		for (IntracellularSubstance s : intracellularSubstances.values() ) {
			// if doesn't diffuse at all : all the substance stays in the distal part !
			if(s.getDiffusionConstant()<0.000000000001){
				continue;
			}
			// create similar IntracellularSubstance and insert it into the new cylinder
			double quantityBeforeDistribution = s.getQuantity(); 
			IntracellularSubstance s2 = new IntracellularSubstance(s);
			s2.setQuantity(quantityBeforeDistribution*(1-distalPortion));
			newProximalCylinder.addNewIntracellularSubstance(s2);
			// decrease value of IntracellularSubstance in this cylinder
			s.setQuantity(quantityBeforeDistribution*distalPortion);
		}
		this.updateDependentPhysicalVariables();
		newProximalCylinder.updateDependentPhysicalVariables();
		newProximalCylinder.updateLocalCoordinateAxis(); // has to come after updateDepend...

		// copy the LocalBiologicalModules (not done in NeuriteElement, because this creation of 
		// cylinder-neuriteElement is decided for physical and not biological reasons
		for (LocalBiologyModule module : neuriteElement.getLocalBiologyModulesList()) {
			if(module.isCopiedWhenNeuriteElongates())
				ne.addLocalBiologyModule(module.getCopy());
		}
		
		// deal with the excressences:
		for (int i = 0; i<excrescences.size() ; i++){
			Excrescence ex = excrescences.get(i);
			double[] pos = ex.getPositionOnPO();
			// transmitt them to proxymal cyl
			if(pos[0]<newProximalCylinder.actualLength){
				this.excrescences.remove(ex);
				newProximalCylinder.excrescences.add(ex);
				ex.setPo(newProximalCylinder);
				i--;
			}else{
				// or kep them here, depending on coordonate
				pos[0]-=newProximalCylinder.actualLength;
			}
		}
		
		
		getRwLock().writeLock().unlock();
		
		return ne;
	}


	/**
	 * Merges two Cylinders together. The one in which the method is called phagocytes it's mother.
	 * The CellElement of the PhysicalCylinder that is removed is also removed: it's removeYourself() method is called.
	 */
	private void removeProximalCylinder() {
		getRwLock().writeLock().lock();
		try
		{
			// The mother is removed if (a) it is a PhysicalCylinder and (b) it has no other daughter than this. 
			if( !(mother.isAPhysicalCylinder()) || (((PhysicalCylinder)mother).getDaughterRight() != null)  ){
				(new Throwable("removeProximalCylinder")).printStackTrace();
				return;
			}
			// The guy we gonna remove
			PhysicalCylinder proximalCylinder = (PhysicalCylinder)mother;
			// the ex-mother's neurite Element has to be removed
			proximalCylinder.getNeuriteElement().removeYourself();
			// Re-organisation of the PhysicalObject tree structure: by-passing proximalCylinder
			proximalCylinder.getMother().updateRelative(proximalCylinder, this);
			this.setMother(proximalCylinder.getMother());
	
	
			// collecting (the quantities of) the intracellular substances of the removed cylinder.
			for (IntracellularSubstance s : proximalCylinder.getIntracellularSubstances().values() ) {
				this.modifyIntracellularQuantity(s.getId(), s.getQuantity()/Param.SIMULATION_TIME_STEP);
				// divided by time step, because in the method the parameter is multiplied by time step...
				// and we want to change the quantity.
				// We don't change the concentration, it is done later by the call to updateVolume()
			}
	
	
			// Keeping the same tension :
			// (we don't use updateDependentPhysicalVariables(), because we have tension and want to 
			// compute restingLength, and not the opposite...)
			// T = k*(A-R)/R --> R = k*A/(T+K)
			this.springAxis = subtract(massLocation,mother.originOf(this));
			this.actualLength = norm(springAxis);
			this.restingLength = springConstant*actualLength/(tension+springConstant);
			// .... and volume
			updateVolume();
			// and local coord
			updateLocalCoordinateAxis();
			// ecm
			ecm.removePhysicalCylinder(proximalCylinder);
			
			// dealing with excressences:
			// mine are shifted up :
			for (int i = 0; i<excrescences.size() ; i++){
				double shift = this.actualLength-proximalCylinder.actualLength;
				Excrescence ex = excrescences.get(i);
				double[] pos = ex.getPositionOnPO();
				pos[0] += shift;
			}
			// I incorporate the ones of the previous cyl:
			for (int i = 0; i<proximalCylinder.excrescences.size() ; i++){
				Excrescence ex = excrescences.get(i);
				this.excrescences.add(ex);
				ex.setPo(this);
			}
			// TODO: take care of Physical Bonds 
			proximalCylinder.setStillExisting(false);
		}finally
		{
			getRwLock().writeLock().unlock();
		}
		// the SON
		updateSpatialOrganizationNodePosition();
		// TODO: CAUTION : for future parallel implementation. If a visitor is in the branch, it gets destroyed.... 

	}

	/**
	 * Repositioning of the SpatialNode location (usually a Delaunay vertex) at the barycenter of the cylinder. 
	 * If it is already closer than a quarter of the diameter of the cylinder, it is not displaced.
	 */
	void updateSpatialOrganizationNodePosition() {
		
		
		getRwLock().readLock().lock();
	
		double[] currentSpatialNodePosition = soNode.getPosition();
		
		double displacementOfTheCenter[] = new double[] {	massLocation[0] - 0.5*springAxis[0] - currentSpatialNodePosition[0],
				massLocation[1] - 0.5*springAxis[1] - currentSpatialNodePosition[1],
				massLocation[2] - 0.5*springAxis[2] - currentSpatialNodePosition[2]  };
		double diameter = this.diameter;
		getRwLock().readLock().unlock();
		// to save time in SOM operation, if the displacement is very small, we don't do it
		if(norm(displacementOfTheCenter)<diameter/4.0)
			return;
		// To avoid perfect alignment (pathologic position for Delaunay, eg), we add a small jitter
		// TODO remove next line when we have Dennis'stable Delaunay
		displacementOfTheCenter = add(displacementOfTheCenter,randomNoise(diameter/4.0, 3));
		
			// Tell the node to moves
		try{
			soNode.moveFrom(displacementOfTheCenter);	
		
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
		}
	

	}

	// *************************************************************************************
	//   ELONGATION, RETRACTION, BRANCHING
	// *************************************************************************************

	/** Method used for active extension of a terminal branch, representing the steering of a 
	 * growth cone. The movement should always be forward, otherwise no movement is performed.
	 * 
	 * @param speed of the growth rate (microns/hours).
	 * @direction the 3D direction of movement.
	 */
	public void extendCylinder(double speed, double[] direction){
		getRwLock().readLock().lock();
		double temp = dot(direction, springAxis);
		getRwLock().readLock().unlock();
		if(  temp> 0 )  {	
			movePointMass(speed, direction);
		}
	}

	/** Method used for active extension of a terminal branch, representing the steering of a 
	 * growth cone. There is no check for real extension (unlike in extendCylinder() ).
	 * 
	 * @param speed of the growth rate (microns/hours).
	 * @direction the 3D direction of movement.
	 */
	public void movePointMass(double speed, double[] direction){
		// check if is a terminal branch
		try{
			
			getRwLock().writeLock().lock();
			if(daughterLeft != null){      
				return;
			}
			
			// scaling for integration step
			double length = speed*Param.SIMULATION_TIME_STEP;
			direction = normalize(direction);
			double displacement[] = new double[] {	
					length*direction[0],
					length*direction[1],
					length*direction[2] };
			massLocation = add(displacement, massLocation);
			// here I have to define the actual length ..........
			double[] relativeML = mother.originOf(this); 	// massLocation of the mother
			springAxis[0] = massLocation[0] - relativeML[0];
			springAxis[1] = massLocation[1] - relativeML[1];
			springAxis[2] = massLocation[2] - relativeML[2];
			actualLength = Math.sqrt(springAxis[0]*springAxis[0] + springAxis[1]*springAxis[1] + springAxis[2]*springAxis[2]);
			oldActualLength = actualLength;
		}
		finally
		{
			getRwLock().writeLock().unlock();
		}
		// process of elongation : setting tension to 0 increases the restingLength :
		setRestingLengthForDesiredTension(0.0);
		
		// some physics and computation obligations....
		updateVolume(); // and update concentration of internal stuff.
		updateLocalCoordinateAxis();
		updateSpatialOrganizationNodePosition();
		// Make sure I'll be updated when I run my physics
		// but since I am actually moving, I have to update the neighbors 
		// (the relative would probably not be needed...).
		scheduleMeAndAllMyFriends();
	}

	/**
	 * Branch retraction by moving the distal end (i.e. the massLocation) toward the
	 * proximal end (the mother), maintaining the same tension in the PhysicalCylinder. The method
	 * shortens the actual and the resting length so that the result is a shorter
	 * cylinder with the same tension.
	 * - If this PhysicalCylinder is longer than the required shortening, it simply retracts. 
	 * - If it is shorter and its mother has no other daughter, it merges with it's mother and 
	 * the method is recursively called (this time the cylinder length is bigger because we have 
	 * a new PhysicalCylinder that resulted from the fusion of two).
	 * - If it is shorter and either the previous PhysicalCylinder has another daughter or the 
	 * mother is not a PhysicalCylinder, it disappears.
	 * 
	 * @param speed of the retraction (microns/hours).
	 * @return false if the neurite doesn't exist anymore (complete retraction)
	 */
	public boolean retractCylinder(double speed) {
		try{
			getRwLock().writeLock().lock();
			// check if is a terminal branch
			if(daughterLeft != null){      
				return true;
			}
			// TODO : what if there are some physical Bonds ??
			// scaling for integration step
			speed = speed*Param.SIMULATION_TIME_STEP;
	
			// if aL > length : retraction keeping the same tension
			// (putting a limit on how short a branch can be is absolutely necessary
			//   otherwise the tension might explode)
			if(actualLength>speed+0.1){ 
				double newActualLength = actualLength - speed;
				double factor = newActualLength/actualLength;
				actualLength = newActualLength;
				restingLength = springConstant*actualLength/(tension +springConstant); //cf removeproximalCylinder()
				springAxis = new double[] { factor*springAxis[0], factor*springAxis[1], factor*springAxis[2] };
				massLocation = add(mother.originOf(this), springAxis);
				updateVolume(); // and update concentration of internal stuff.
				updateSpatialOrganizationNodePosition();
				// be sure i'll run my physics :
				super.setOnTheSchedulerListForPhysicalObjects(true);
				return true;
				// if al < length and mother is a PhysicalCylinder with no other daughter : merge with mother
			} else if(mother.isAPhysicalCylinder() && ((PhysicalCylinder)mother).getDaughterRight() == null  ){
				removeProximalCylinder(); // also updates volume...
				// be sure i'll run my physics :
				super.setOnTheSchedulerListForPhysicalObjects(true);
				return retractCylinder(speed/Param.SIMULATION_TIME_STEP);
				// if mother is cylinder with other daughter or is not a cylinder : disappear.	
			}else{
				mother.removeDaugther(this);
				this.setStillExisting(false);
				ecm.removePhysicalCylinder(this);  // this method removes the SONode
				// and the associated neuriteElement also disappears :
				neuriteElement.removeYourself();
				// intracellularSubstances quantities 
				// (concentrations are solved in updateDependentPhysicalVariables():
				for (IntracellularSubstance s : intracellularSubstances.values() ) {
					mother.modifyIntracellularQuantity(s.getId(), s.getQuantity()/Param.SIMULATION_TIME_STEP);
					// (divide by time step because it is multiplied by it in the method)
				}
				mother.updateDependentPhysicalVariables();
				// extra-safe : make sure you'll not be run :
				super.setOnTheSchedulerListForPhysicalObjects(false);
				return false;
			}
		}
		finally
		{
			getRwLock().writeLock().unlock();
		}
	}


	/** 
	 * Bifurcation of the growth cone creating : adds the 2 <code>PhysicalCylinder</code> that become
	 * daughter left and daughter right
	 * @param length the length of the new branches 
	 * @param direction_1 of the first branch (if 
	 * @param newBranchL
	 * @param newBranchR
	 */

	public PhysicalCylinder[] bifurcateCylinder(double length, double[] direction_1, double[] direction_2) {
		// check it is a terminal branch
		if (daughterLeft != null){
			System.out.println("Not a terminal Branch");
			return null;
		}
		// create the cylinders
		PhysicalCylinder newBranchL = this.getCopy();
		PhysicalCylinder newBranchR = this.getCopy();
		// set family relations
		getRwLock().writeLock().lock();
		this.daughterLeft = newBranchL;
		newBranchL.setMother(this);
		this.daughterRight = newBranchR;
		newBranchR.setMother(this);
		getRwLock().writeLock().unlock();
		
		// check that the directions are not pointing backwards
		if( angleRadian(springAxis, direction_1)>Math.PI*0.5){
			double[] proj = projectionOnto(direction_1, springAxis);
			proj = scalarMult(-1,proj);
			direction_1 = add(direction_1, proj);
		}
		if( angleRadian(springAxis, direction_2)>Math.PI*0.5){
			double[] proj = projectionOnto(direction_2, springAxis);
			proj = scalarMult(-1,proj);
			direction_2 = add(direction_2, proj);
		}

		// mass location and spring axis
		newBranchL.springAxis = scalarMult(length,normalize(direction_1));
		newBranchL.setMassLocation( add(this.massLocation, newBranchL.springAxis) );
		newBranchL.updateLocalCoordinateAxis();  // (important so that xAxis is correct)

		newBranchR.springAxis = scalarMult(length,normalize(direction_2));
		newBranchR.setMassLocation( add(this.massLocation, newBranchR.springAxis) );
		newBranchR.updateLocalCoordinateAxis(); 
		
		// physics of tension : 
		newBranchL.setActualLength(length);
		newBranchR.setActualLength(length);
		newBranchR.setRestingLengthForDesiredTension(Param.NEURITE_DEFAULT_TENSION);
		newBranchL.setRestingLengthForDesiredTension(Param.NEURITE_DEFAULT_TENSION);

		// spatial organization node
		double[] newBranchCenterLocation = add(this.massLocation, scalarMult(0.5,newBranchL.springAxis));
		SpatialOrganizationNode<PhysicalNode> newSON = null;
		try {
			newSON = this.soNode.getNewInstance(newBranchCenterLocation, newBranchL);
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
		}
		newBranchL.setSoNode(newSON);

		newBranchCenterLocation = add(this.massLocation, scalarMult(0.5,newBranchR.springAxis));
		try {
			newSON = this.soNode.getNewInstance(newBranchCenterLocation, newBranchR);
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
		}
		newBranchR.setSoNode(newSON);	

		// register the new branches with ecm
		ecm.addPhysicalCylinder(newBranchL);
		ecm.addPhysicalCylinder(newBranchR);

		// set local coordinate axis in the new branches 
		newBranchL.updateLocalCoordinateAxis();
		newBranchR.updateLocalCoordinateAxis();

		// i'm scheduled to run physics next time :
		// (the daughters automatically are too, because they are new PhysicalObjects)
		super.setOnTheSchedulerListForPhysicalObjects(true);
		
		newBranchL.updateDependentPhysicalVariables();
		newBranchR.updateDependentPhysicalVariables();
		
		return new PhysicalCylinder[] {daughterLeft, daughterRight};
	}
	
	
	


	/**
	 * Makes a side branching by adding a second daughter to a non terminal <code>PhysicalCylinder</code>.
	 * The new <code>PhysicalCylinder</code> is perpendicular to the mother branch.
	 * @param direction the direction of the new neuriteLement (But will be automatically corrected if
	 * not al least 45 degrees from the cylinder's axis).
	 * @return the newly added <code>NeuriteSegment</code>
	 */
	public PhysicalCylinder branchCylinder(double length, double[] direction) {
		// we first split this cylinder into two pieces
		NeuriteElement ne = insertProximalCylinder();
		// then append a "daughter right" between the two 
		return ne.getPhysicalCylinder().extendSideCylinder(length, direction);
	}
	
	
	private PhysicalCylinder extendSideCylinder(double length, double[] direction) {
		PhysicalCylinder newBranch = this.getCopy();
		if(direction == null){
			direction = add( perp3(springAxis), randomNoise(0.1, 3) );    
		}else{
			// check that the direction is at least 45 degrees from the branch axis
			// TODO : better method ! 
			double angleWithSideBranch = angleRadian(springAxis, direction);
			if(angleWithSideBranch<0.78 || angleWithSideBranch > 2.35){  // 45-135 degrees
				double[] p = crossProduct(springAxis, direction);
				p = crossProduct(p, springAxis);
				direction = add(normalize(direction), normalize(p));
			}
		}
		// location of mass and computation center
		double[] newBranchSpringAxis = scalarMult(length, normalize(direction)); 
		double[] newBranchMassLocation = add(this.massLocation, newBranchSpringAxis);
		newBranch.massLocation = newBranchMassLocation;
		newBranch.springAxis = newBranchSpringAxis;
		// physics  
		newBranch.actualLength = length;
		newBranch.setRestingLengthForDesiredTension( Param.NEURITE_DEFAULT_TENSION); 
		newBranch.setDiameter(Param.NEURITE_DEFAULT_DIAMETER, true);
		newBranch.updateLocalCoordinateAxis();
		// family relations
		newBranch.setMother(this);
		getRwLock().writeLock().lock();
		this.daughterRight = newBranch;
		getRwLock().writeLock().unlock();
		// new CentralNode
		double[] newBranchCenterLocation = add(this.massLocation, scalarMult(0.5,newBranchSpringAxis));
		SpatialOrganizationNode<PhysicalNode> newSON = null;
		try {
			newSON = soNode.getNewInstance(newBranchCenterLocation, newBranch);
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
		}
		newBranch.setSoNode(newSON);
		// correct physical values (has to be after family relations and SON assignement).
		newBranch.updateDependentPhysicalVariables();
		// register to ecm
		ecm.addPhysicalCylinder(newBranch);
		
		
		// i'm scheduled to run physics next time :
		// (the side branch automatically is too, because it's a new PhysicalObject)
		super.setOnTheSchedulerListForPhysicalObjects(true);
		return newBranch;
	}


	public void setRestingLengthForDesiredTension(double tensionWeWant){
		getRwLock().writeLock().lock();
		this.tension = tensionWeWant;
		// T = k*(A-R)/R --> R = k*A/(T+K)
		restingLength = springConstant*actualLength/(tension+springConstant);
		getRwLock().writeLock().unlock();
	}

	
	/**
	 * Progressive modification of the volume. Updates the diameter, the intracellular concentration 
	 * @param speed cubic micron/ h
	 */
	public void changeVolume(double speed) {
		//scaling for integration step
		getRwLock().writeLock().lock();
		double dV = speed*(Param.SIMULATION_TIME_STEP);
		super.volume += dV;
		
		if(volume < 5.2359877E-7 ){	// minimum volume, corresponds to minimal diameter
			System.err.println("PhysicalSphere.changeVolume() : volume is "+volume);
			volume = 5.2359877E-7; 
		}
		getRwLock().writeLock().unlock();
		updateDiameter();
		updateIntracellularConcentrations();
		scheduleMeAndAllMyFriends();
	}

	/**
	 * Progressive modification of the diameter. Updates the volume, the intracellular concentration 
	 * @param speed micron/ h
	 */
	public void changeDiameter(double speed) {
		//scaling for integration step
		getRwLock().writeLock().lock();
		double dD = speed*(Param.SIMULATION_TIME_STEP);
		super.diameter += dD;
		if(diameter < 0.01 ){
			System.err.println("PhysicalCylinder.changeDiameter() : diameter is "+diameter);
		}
		getRwLock().writeLock().unlock();
		updateVolume();
		// no call to updateIntracellularConcentrations() cause it's done by updateVolume().
		scheduleMeAndAllMyFriends();
	}
	
	

	// *************************************************************************************
	//   Physics
	// *************************************************************************************

	
	
	
	/**
	 * 
	 */
	public void runPhysics() {
		// decide first if we have to split or fuse this cylinder. Usually only 
		// terminal branches (growth cone) do this.
		if(daughterLeft==null){
			runDiscretization();
		}

		// in case we don't move, we won't run physics the next time :
		super.setOnTheSchedulerListForPhysicalObjects(false);
		
		getRwLock().readLock().lock();
		double h = Param.SIMULATION_TIME_STEP;
		double[] forceOnMyPointMass = {0,0,0};
		double[] forceOnMyMothersPointMass  = {0,0,0};

		// 1) Spring force -------------------------------------------------------------------
		// 		Only the spring of this cylinder. The daughters spring also act on this mass,
		// 		but they are treated in point (2)

		double factor =  -tension/actualLength;  // the minus sign is important because the spring axis goes in the opposite direction
		forceOnMyPointMass[0]+= factor*springAxis[0];
		forceOnMyPointMass[1]+= factor*springAxis[1];
		forceOnMyPointMass[2]+= factor*springAxis[2];


		// 2) Force transmitted by daugthers (if they exist) ----------------------------------
		if(daughterLeft!=null) {
			double[] forceFromDaughter = daughterLeft.forceTransmittedFromDaugtherToMother(this);
			forceOnMyPointMass[0]+= forceFromDaughter[0];	
			forceOnMyPointMass[1]+= forceFromDaughter[1];
			forceOnMyPointMass[2]+= forceFromDaughter[2];
		}
		if(daughterRight!=null) {
			double[] forceFromDaughter = daughterRight.forceTransmittedFromDaugtherToMother(this);
			forceOnMyPointMass[0]+= forceFromDaughter[0];	
			forceOnMyPointMass[1]+= forceFromDaughter[1];
			forceOnMyPointMass[2]+= forceFromDaughter[2];
		} 
		
		// 3) Object avoidance force -----------------------------------------------------------
		//	(We check for every neighbor object if they touch us, i.e. push us away)
		for (PhysicalNode neighbor : soNode.getNeighbors()) {
			// of course, only if it is an instance of PhysicalObject
			if(neighbor.isAPhysicalObject()){
				PhysicalObject n = (PhysicalObject)neighbor;
				// if it is a direct relative, we don't take it into account
				if(neighbor == mother || neighbor == daughterLeft || neighbor == daughterRight) 
					continue;
				// if sister branch, we also don't take into account
				if (neighbor instanceof PhysicalCylinder) {
					PhysicalCylinder nCyl = (PhysicalCylinder) neighbor;
					if(nCyl.getMother() == this.mother){
						continue;
					}
				}
				// if we have a PhysicalBond with him, we also don't take it into account
				if(super.physicalBonds != null){
					for (PhysicalBond pb : physicalBonds) {
						if(pb.getOppositePhysicalObject(this) == neighbor){
							continue;
						}
					}
				}

				double[] forceFromThisNeighbor = n.getForceOn(this);
				
				// 1) "artificial force" to maintain the sphere in the ecm simulation boundaries--------
				if(ecm.getArtificialWallForCylinders()){
					double[] forceFromArtificialWall = ecm.forceFromArtificialWall(massLocation, diameter*0.5);
					forceOnMyPointMass[0] += forceFromArtificialWall[0];
					forceOnMyPointMass[1] += forceFromArtificialWall[1];
					forceOnMyPointMass[2] += forceFromArtificialWall[2];
				}

				if(forceFromThisNeighbor.length == 3){
					// (if all the force is transmitted to the (distal end) point mass : )
					forceOnMyPointMass[0]+= forceFromThisNeighbor[0];	
					forceOnMyPointMass[1]+= forceFromThisNeighbor[1];
					forceOnMyPointMass[2]+= forceFromThisNeighbor[2];
				}else{												
					// (if there is a part transmitted to the proximal end : )
					double partForMyPointMass = 1.0-forceFromThisNeighbor[3];
					forceOnMyPointMass[0]+= forceFromThisNeighbor[0]*partForMyPointMass;	
					forceOnMyPointMass[1]+= forceFromThisNeighbor[1]*partForMyPointMass;	
					forceOnMyPointMass[2]+= forceFromThisNeighbor[2]*partForMyPointMass;	
					forceOnMyMothersPointMass[0]+= forceFromThisNeighbor[0]*forceFromThisNeighbor[3];	
					forceOnMyMothersPointMass[1]+= forceFromThisNeighbor[1]*forceFromThisNeighbor[3];	
					forceOnMyMothersPointMass[2]+= forceFromThisNeighbor[2]*forceFromThisNeighbor[3];
				}		
			}
		}
		
		boolean antiKink = false;
		// TEST : anti-kink
		if(antiKink){
		double KK = 5;
		if(daughterLeft != null && daughterRight == null){
			if(daughterLeft.daughterLeft!=null){
				PhysicalCylinder downstream = daughterLeft.daughterLeft;
				double rresting = daughterLeft.getRestingLength()+downstream.getRestingLength();
//				double rresting = daughterLeft.getActualLength()+downstream.getActualLength();
				double[] downToMe = subtract(massLocation,downstream.massLocation);
				double aactual = norm(downToMe);
		
				forceOnMyPointMass = add(forceOnMyPointMass, scalarMult(KK*(rresting-aactual),normalize(downToMe)));
			}
		}
		
		if (daughterLeft!= null && mother instanceof PhysicalCylinder) {
			PhysicalCylinder motherCyl = (PhysicalCylinder) mother;
			double rresting = this.getRestingLength()+motherCyl.getRestingLength();
//			double rresting = this.getActualLength()+motherCyl.getActualLength();
			double[] downToMe = subtract(massLocation,motherCyl.proximalEnd());
			double aactual = norm(downToMe);
	
			forceOnMyPointMass = add(forceOnMyPointMass, scalarMult(KK*(rresting-aactual),normalize(downToMe)));
		}
		}
		// 4) PhysicalBond -----------------------------------------------------------
		for (int i=0; i< physicalBonds.size(); i++) {
			PhysicalBond pb = physicalBonds.get(i);
			double[] forceFromThisPhysicalBond = pb.getForceOn(this);

			if(forceFromThisPhysicalBond.length == 3){
				// (if all the force is transmitted to the (distal end) point mass : )
				forceOnMyPointMass[0]+= forceFromThisPhysicalBond[0];	
				forceOnMyPointMass[1]+= forceFromThisPhysicalBond[1];
				forceOnMyPointMass[2]+= forceFromThisPhysicalBond[2];
			}else{												
				// (if there is a part transmitted to the proximal end : )
				double partForMyPointMass = 1.0-forceFromThisPhysicalBond[3];
				forceOnMyPointMass[0]+= forceFromThisPhysicalBond[0]*partForMyPointMass;	
				forceOnMyPointMass[1]+= forceFromThisPhysicalBond[1]*partForMyPointMass;	
				forceOnMyPointMass[2]+= forceFromThisPhysicalBond[2]*partForMyPointMass;	
				forceOnMyMothersPointMass[0]+= forceFromThisPhysicalBond[0]*forceFromThisPhysicalBond[3];	
				forceOnMyMothersPointMass[1]+= forceFromThisPhysicalBond[1]*forceFromThisPhysicalBond[3];	
				forceOnMyMothersPointMass[2]+= forceFromThisPhysicalBond[2]*forceFromThisPhysicalBond[3];
			}	
		}
    	getRwLock().readLock().unlock();
		
		// 5) define the force that will be transmitted to the mother
		getRwLock().writeLock().lock();
		forceToTransmitToProximalMass = forceOnMyMothersPointMass;
		getRwLock().writeLock().unlock();
		// 6) Compute the movement of this neurite elicited by the resultant force----------------
		// 	6.0) In case we display the force
		getRwLock().writeLock().lock();
		super.totalForceLastTimeStep[0] = forceOnMyPointMass[0];
		super.totalForceLastTimeStep[1] = forceOnMyPointMass[1];
		super.totalForceLastTimeStep[2] = forceOnMyPointMass[2];
		super.totalForceLastTimeStep[3] = 1;
		getRwLock().writeLock().unlock();
		//	6.1) Define movement scale 
		double hOverM = h/mass;
		double normOfTheForce = norm(forceOnMyPointMass);
		// 	6.2) If is F not strong enough -> no movements
		if(normOfTheForce<adherence) {  
			getRwLock().writeLock().lock();
			super.totalForceLastTimeStep[3] = -1;
			getRwLock().writeLock().unlock();
			return;									
		}
		// So, what follows is only executed if we do actually move :

		// 	6.3) Since there's going be a move, we calculate it
		double[] displacement = scalarMult(hOverM, forceOnMyPointMass);
		double normOfDisplacement = normOfTheForce*hOverM;

		// 	6.4) There is an upper bound for the movement.
		if(normOfDisplacement > Param.SIMULATION_MAXIMAL_DISPLACEMENT){
			displacement = scalarMult(Param.SIMULATION_MAXIMAL_DISPLACEMENT/normOfDisplacement, displacement);
		}


		//		//7) We check if we don't intersect an other neurite with the planned move---------------
		//		double[] desiredNewMassLocation = add(massLocation,displacement);
		//		double s = 1;
		//		for (PhysicalNode neighbor : soNode.getNeighbors()) {
		//		// of course, only if it is an instance of PhysicalObject
		//		if(neighbor instanceof PhysicalCylinder){
		//		PhysicalCylinder n = (PhysicalCylinder)neighbor;
		//		// if it is a direct relative, we don't take it into account
		//		if(neighbor == mother || neighbor == daughterLeft || neighbor == daughterRight) 
		//		continue;

		//		if(distance(n.getSONode().getPosition(),this.getSONode().getPosition()) > (this.actualLength + n.actualLength + 0.4) ){
		//		continue;
		//		}

		//		CollisionCheck.addPhysicalBondIfCrossing(
		//		this.getProximalEnd(), 
		//		this.massLocation,
		//		desiredNewMassLocation, 
		//		this, 
		//		n);
		//		}
		//		}
		//		// But when we move our point mass, it moves the proximal end of the daugthers---
		//		if(daughterLeft instanceof PhysicalCylinder){
		//		PhysicalCylinder dl = (PhysicalCylinder) daughterLeft;
		//		for (PhysicalNode neighbor : dl.getSONode().getNeighbors()) {
		//		// of course, only if it is an instance of PhysicalObject
		//		if(neighbor instanceof PhysicalCylinder){
		//		PhysicalCylinder n = (PhysicalCylinder)neighbor;
		//		// if it is a direct relative, we don't take it into account
		//		if(neighbor == dl.mother || neighbor == dl.daughterLeft || neighbor == dl.daughterRight) 
		//		continue;
		//		if(distance(n.getSONode().getPosition(),dl.getSONode().getPosition()) > (dl.actualLength + n.actualLength + 0.4) ){
		//		continue;
		//		}
		//		CollisionCheck.addPhysicalBondIfCrossing(
		//		dl.massLocation, 
		//		this.massLocation,
		//		desiredNewMassLocation, 
		//		dl, 
		//		n);
		//		}
		//		}
		//		}

		double[] actualDisplacement = displacement;

		// 8) Eventually, we do perform the move--------------------------------------------------
		// 8.1) The move of our mass
		getRwLock().writeLock().lock();
		massLocation[0] += actualDisplacement[0];
		massLocation[1] += actualDisplacement[1];
		massLocation[2] += actualDisplacement[2];
		getRwLock().writeLock().unlock();
		// 8.2) Recompute length, tension and re-center the computation node, and redefine axis
		updateDependentPhysicalVariables();
		updateLocalCoordinateAxis();
		// 8.3) For the relatives: recompute the lenght, tension etc. (why for mother? have to think about that)
		getRwLock().readLock().lock();
		if(daughterLeft!=null){
			daughterLeft.updateDependentPhysicalVariables();
			daughterLeft.updateLocalCoordinateAxis();
		}
		if(daughterRight!=null){
			daughterRight.updateDependentPhysicalVariables();
			daughterRight.updateLocalCoordinateAxis();
		}
		getRwLock().readLock().unlock();
		// 8.4) next time we reschedule everyone :
		scheduleMeAndAllMyFriends();

	}

	/**
	 * Sets the scheduling flag onTheSchedulerListForPhysicalObjects to true
	 * for me and for all my neighbors, relative, things I share a physicalBond with
	 */
	private void scheduleMeAndAllMyFriends(){
		// me
		super.setOnTheSchedulerListForPhysicalObjects(true);
		// relatives :
		getRwLock().readLock().lock();
		mother.setOnTheSchedulerListForPhysicalObjects(true);
		if(daughterLeft != null){
			daughterLeft.setOnTheSchedulerListForPhysicalObjects(true);
			if(daughterRight != null){
				daughterRight.setOnTheSchedulerListForPhysicalObjects(true);
			}
		}
		// neighbors in the triangulation :
	
		for (PhysicalNode neighbor : soNode.getNeighbors()) {
			if(neighbor.isAPhysicalObject()){
				((PhysicalObject)neighbor).setOnTheSchedulerListForPhysicalObjects(true);
			}
		}
		for (int i=0; i< physicalBonds.size(); i++) {
			physicalBonds.get(i).getOppositePhysicalObject(this).setOnTheSchedulerListForPhysicalObjects(true);
		}
		getRwLock().readLock().unlock();
	}



	@Override
	public double[] getForceOn(PhysicalSphere s) {
		try
		{
			getRwLock().readLock().lock();
			// force from cylinder on sphere
	//		double[] f = Force.forceOnASphereFromACylinder(
	//				s.getMassLocation(),
	//				s.getDiameter()*0.5,
	//				subtract(this.massLocation,this.springAxis),
	//				this.massLocation,
	//				this.springAxis,
	//				this.actualLength,
	//				this.getDiameter() );
	//		return f;
			return interObjectForce.forceOnASphereFromACylinder(s, this);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	@Override
	public double[] getForceOn(PhysicalCylinder c) {
		try{
			getRwLock().readLock().lock();
			if(c.getMother() == this.mother){
				// extremely important to avoid that two sister branches start to
				// interact physically.
				return new double[] {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
			}
	//		//		double[] f = Force.forceOnACylinderFromACylinder2(
	//		//		subtract(c.massLocation,c.springAxis), c.massLocation, c.diameter,
	//		//		subtract(this.massLocation,this.springAxis), this.massLocation, this.diameter);
	//		double[] f = Force.forceOnACylinderFromACylinder2(
	//				c.proximalEnd(), c.getMassLocation(), c.diameter,
	//				this.proximalEnd(), this.massLocation, this.diameter);
	//		return f;
			return interObjectForce.forceOnACylinderFromACylinder(c, this);
		
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	@Override
	public boolean isInContactWithSphere(PhysicalSphere s) {
		getRwLock().readLock().lock();
		double[] force = PhysicalObject.interObjectForce.forceOnACylinderFromASphere(this,s);
		getRwLock().readLock().unlock();
		if(norm(force)>1E-15){
			return true;
		}else{
			return false;
		}
		
//		//		detailed comments on the method used to find the closest point on a line from a point
//		// in Force.forceOnACylinderFromASphere().
//		double[] c = s.getMassLocation();
//		double[] pP = mother.originOf(this);
//		double[] pD = massLocation;
//		double[] axis = springAxis;
//		double[] pPc = subtract(c,pP);
//
//		// 		projection of pPc onto axis = (pPc.axis)/norm(axis)^2  * axis
//		// 		length of the projection = (pPc.axis)/norm(axis)
//		double pPcDotAxis = pPc[0]*axis[0] + pPc[1]*axis[1] + pPc[2]*axis[2];
//		double K = pPcDotAxis/(actualLength*actualLength);
//		//		cc = pP + K* axis 
//		double[] cc  = new double[] {pP[0]+K*axis[0], pP[1]+K*axis[1], pP[2]+K*axis[2]}; 
//
//		if(K<0){ 	// if the closest point to c on the line pPpD is before pP
//			cc = pP;
//		}else {   	// if cc is after pD, the force is only on the distal end (the segment's point mass).	 
//			cc = pD;
//		}
//
//		double penetration = (diameter + s.getDiameter() )*0.5 -distance(c,cc);		 
//		if(penetration > -Param.SPHERE_CYLINDER_DETECTION_RANGE) {
//			return true;
//		}
//		return false;
	}

	@Override
	public boolean isInContactWithCylinder(PhysicalCylinder c) {
		
		getRwLock().readLock().lock();
		double[] force = PhysicalObject.interObjectForce.forceOnACylinderFromACylinder(this,c);
		getRwLock().readLock().unlock();
		if(norm(force)>1E-15){
			return true;
		}else{
			return false;
		}
		

//		//	2 cylinders, with extremities A,B and diameter d1, and B,C,d2	
//		//	based on http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline3d/
//		// (The same method is used in Force.forceOnACylinderFromACylinder2()
//
//		double[] A = this.distalEnd();
//		double[] B = this.proximalEnd();
//		double d1 = this.diameter;
//		double[] C = c.distalEnd();
//		double[] D = c.proximalEnd();
//		double d2 = c.getDiameter();
//
//		double p13x = A[0]-C[0];
//		double p13y = A[1]-C[1];
//		double p13z = A[2]-C[2];
//		double p43x = D[0]-C[0];
//		double p43y = D[1]-C[1];
//		double p43z = D[2]-C[2];
//		double p21x = B[0]-A[0];
//		double p21y = B[1]-A[1];
//		double p21z = B[2]-A[2]; 
//
//		double d1343 = p13x*p43x + p13y*p43y + p13z*p43z;
//		double d4321 = p21x*p43x + p21y*p43y + p21z*p43z;
//		double d1321 = p21x*p13x + p21y*p13y + p21z*p13z;
//		double d4343 = p43x*p43x + p43y*p43y + p43z*p43z;
//		double d2121 = p21x*p21x + p21y*p21y + p21z*p21z;
//
//		double[] P1, P2;
//
//		double denom = d2121*d4343 - d4321*d4321;
//		if(denom > 0.000000000001){
//			//			if the two segments are not absolutly parallel
//			double numer = d1343*d4321 - d1321*d4343;
//
//			double mua = numer/denom;
//			double mub = (d1343 + mua*d4321)/d4343;
//
//			if(mua<0){
//				P1 = A;
//			}else if(mua>1){
//				P1 = B;
//			}else{
//				P1 = new double[] {A[0]+mua*p21x, A[1]+mua*p21y, A[2]+mua*p21z };
//			}
//
//			if(mub<0){
//				P2 = C;
//			}else if(mub>1){
//				P2 = D;
//			}else{
//				P2 = new double[] {C[0]+mub*p43x, C[1]+mub*p43y, C[2]+mub*p43z };
//			}
//
//		}else{
//			P1 = add(A,scalarMult(0.5, subtract(B,A) ));
//			P2 = add(C,scalarMult(0.5, subtract(D,C) ));
//		}
//
//		return (distance(P1,P2)<d1+d2);
	}

	/** Returns the point on this cylinder's spring axis that is the closest to the point p.*/
	public double[] closestPointTo(double[] p){
		try{
			getRwLock().readLock().lock();
			double[] massToP = subtract(p, massLocation);
			
			printlnLine("massToP",massToP);
			
			double massToPDotMinusAxis = -massToP[0]*springAxis[0] - massToP[1]*springAxis[1] - massToP[2]*springAxis[2];
			System.out.println("massToPDotMinusAxis = "+massToPDotMinusAxis);
			
			double K = massToPDotMinusAxis/(actualLength*actualLength);
			
			System.out.println("K = "+K);
			
			double[] cc; // the closest point
			if(K<=1.0 && K>=0.0){
				cc  = new double[] {massLocation[0]-K*springAxis[0], massLocation[1]-K*springAxis[1], massLocation[2]-K*springAxis[2]};
			}else if(K<0){
				cc = massLocation;
			}else {   	
				cc = proximalEnd();
			}
			
			printlnLine("cc",cc);
			
			return cc;	
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	
	@Override
	public void runIntracellularDiffusion() {

		// 1) Degradation according to the degradation constant for each chemical
		
		getRwLock().readLock().lock();
		for (Substance s : intracellularSubstances.values()) {
			double decay = Math.exp(-s.getDegradationConstant()*Param.SIMULATION_TIME_STEP);
			s.multiplyQuantityAndConcentrationBy(decay);
		}

		// 2) each cylinder is responsible for diffusion with its distal relatives (i.e daughter left
		// and daughter right). The direction (i.e.who calls diffuseWithThisPhysicalObject() )
		// is chosen randomly. To be sure that new substances will be transmitted bi-directionally. 
		// For now we start first with dL and then dR. This might have to change...
		PhysicalCylinder daughterRight = this.daughterRight;
		PhysicalCylinder daughterLeft = this.daughterLeft;
		getRwLock().readLock().unlock();
		
		if(daughterRight != null){
			PhysicalObject po1 = this;
			PhysicalObject po2 = daughterRight;
			if(ECM.getRandomDouble()<0.5){
				po1 = daughterRight;
				po2 = this;
			}
			po1.diffuseWithThisPhysicalObjects(po2, ((PhysicalCylinder)daughterRight).getActualLength());
		}
		
		if(daughterLeft != null) {
			PhysicalObject po1 = this;
			PhysicalObject po2 = daughterLeft;
			if(ECM.getRandomDouble()<0.5){
				po1 = daughterLeft;
				po2 = this;
			}
			po1.diffuseWithThisPhysicalObjects(po2, ((PhysicalCylinder)daughterLeft).getActualLength());
		}
		
	}

	

	


	@Override
	public double[] getUnitNormalVector(double[] positionInPolarCoordinates) {
		try{
			getRwLock().readLock().lock();
			return add(	scalarMult(Math.cos(positionInPolarCoordinates[1]), yAxis),
					scalarMult(Math.sin(positionInPolarCoordinates[1]) ,zAxis)
			);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}



	/**
	 * Defines the three orthonormal local axis so that a cylindrical coordinate system
	 * can be used. The xAxis is aligned with the springAxis. The two other are in the
	 * plane perpendicular to springAxis. This method to update the axis was suggested by
	 * Matt Coock. - Although not perfectly exact, it is accurate enough for us to use. 
	 *
	 */
	public void updateLocalCoordinateAxis(){
		// x (new) = something new
		// z (new) = x (new) cross y(old)
		// y (new) = z(new) cross x(new)

		//		if(false)
		//		return;
		getRwLock().writeLock().lock();
		xAxis = normalize(this.springAxis);
		zAxis = crossProduct(xAxis, yAxis);
		double normOfZ = norm(zAxis);
		if(normOfZ<1E-10){
			// If new xAxis and old yAxis are aligned, we cannot use this scheme;
			// we start by re-defining new perp vectors. Ok, we loose the previous info, but
			// this should almost never happen.... 
			zAxis = perp3(xAxis);
		}else{
			zAxis = scalarMult((1/normOfZ),zAxis);
		}
		yAxis = crossProduct(zAxis, xAxis);
		getRwLock().writeLock().unlock();
	}



	/**
	 * Updates the spring axis, the actual length, the tension and the volume.
	 *  
	 * For tension, T = k*(aL-rL)/rL.  k = spring constant, 
	 * rL = resting length, aL = actual length. (Note the division by rL. 
	 * Otherwise we could have Cylinders with big aL and rL = 0). 
	 * <p>
	 * This method also automatically calls the <code>resetComputationCenterPosition()</code> 
	 * method at the end.
	 */
	protected  void updateDependentPhysicalVariables() {
		getRwLock().writeLock().lock();
		double[] relativeML = mother.originOf(this); 	// massLocation of the mother
		springAxis[0] = massLocation[0] - relativeML[0];
		springAxis[1] = massLocation[1] - relativeML[1];
		springAxis[2] = massLocation[2] - relativeML[2];
		actualLength = Math.sqrt(springAxis[0]*springAxis[0] + springAxis[1]*springAxis[1] + springAxis[2]*springAxis[2]);
		tension = springConstant * ( actualLength - restingLength ) / restingLength;
		getRwLock().writeLock().unlock();
		updateVolume();
		updateSpatialOrganizationNodePosition();
		
	}

	/* Recomputes diameter after volume has changed.*/
	@Override
	protected void updateDiameter() {
		getRwLock().writeLock().lock();
		diameter = Math.sqrt(volume * 1.27323954 / actualLength);  	// 1.27323 = 4/pi
		getRwLock().writeLock().unlock();
	}


	/* Recomputes volume, after diameter has been change. And makes a call for 
	 * recomputing then concentration of IntracellularSubstances.*/
	@Override
	protected void updateVolume() {						// 0.78539 = pi/4
		getRwLock().writeLock().lock();
		volume = 0.785398163 * diameter * diameter * actualLength;
		updateIntracellularConcentrations();
		getRwLock().writeLock().unlock();
	}



	/**
	 * Updates the concentration of substances, based on the volume of the object.
	 * Is usually called after change of the volume (and therefore we don't modify it here)
	 */
	protected void updateIntracellularConcentrations(){
		getRwLock().readLock().lock();
		for (IntracellularSubstance s : intracellularSubstances.values() ) {
			if(s.isVolumeDependant()){
				s.updateConcentrationBasedOnQuantity(volume);
			}
			else{
				s.updateConcentrationBasedOnQuantity(actualLength);
			}
		} 
		getRwLock().readLock().unlock();
	}
	


	// *************************************************************************************
	//   Coordinates transform
	// *************************************************************************************
	
	/*
	 * 3 systems of coordinates :
	 * 
	 * Global :		cartesian coord, defined by orthogonal axis (1,0,0), (0,1,0) and (0,0,1)
	 * 				with origin at (0,0,0).
	 * Local :		defined by orthogonal axis xAxis (=vect proximal to distal end), yAxis and zAxis,
	 * 				with origin at proximal end
	 * Polar :		cylindrical coordinates [h,theta,r] with 	
	 * 				h = first local coord (along xAxis), 
	 * 				theta = angle from yAxis,
	 * 				r euclidian distance from xAxis;
	 * 				with origin at proximal end
	 * 
	 *  Note: The methods below transform POSITIONS and not DIRECTIONS !!!
	 *  
	 * G -> L
	 * L -> G
	 * 
	 * L -> P
	 * P -> L
	 * 
	 * G -> P = G -> L, then L -> P
	 * P -> P = P -> L, then L -> G 
	 */
	
	// G -> L
	/**
	 * Returns the position in the local coordinate system (xAxis, yXis, zAxis) 
	 * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
	 * @param positionInGlobalCoord
	 * @return
	 */
	public double[] transformCoordinatesGlobalToLocal(double[] positionInGlobalCoord){
		try{
			getRwLock().readLock().lock();
			positionInGlobalCoord = subtract(positionInGlobalCoord, proximalEnd());
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
	// L -> G
	/**
	 * Returns the position in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
	 * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
	 * @param positionInLocalCoord
	 * @return
	 */
	public double[] transformCoordinatesLocalToGlobal(double[] positionInLocalCoord){
		try{
	
			getRwLock().readLock().lock();
			double[] glob = new double[] { 
					positionInLocalCoord[0]*xAxis[0] + positionInLocalCoord[1]*yAxis[0] + positionInLocalCoord[2]*zAxis[0], 
					positionInLocalCoord[0]*xAxis[1] + positionInLocalCoord[1]*yAxis[1] + positionInLocalCoord[2]*zAxis[1], 
					positionInLocalCoord[0]*xAxis[2] + positionInLocalCoord[1]*yAxis[2] + positionInLocalCoord[2]*zAxis[2] 
					};
			return add(glob,proximalEnd());
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	// L -> P
	/**
	 * Returns the position in cylindrical coordinates (h,theta,r)
	 * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
	 * @param positionInLocalCoord
	 * @return
	 */
	public double[] transformCoordinatesLocalToPolar(double[] positionInLocalCoordinates){
		try{
			getRwLock().readLock().lock();
			return new double[] {
					positionInLocalCoordinates[0],
					Math.atan2(positionInLocalCoordinates[2], positionInLocalCoordinates[1]),
					Math.sqrt(positionInLocalCoordinates[1]*positionInLocalCoordinates[1] + positionInLocalCoordinates[2]*positionInLocalCoordinates[2])
			};
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	// P -> L
	/**
	 * Returns the position in the local coordinate system (xAxis, yXis, zAxis) 
	 * of a point expressed in cylindrical coordinates (h,theta,r).
	 * @param positionInLocalCoord
	 * @return
	 */
	public double[] transformCoordinatesPolarToLocal(double[] positionInPolarCoordinates){
		try{
			getRwLock().readLock().lock();
			return new double[] {
					positionInPolarCoordinates[0],
					positionInPolarCoordinates[2]*Math.cos(positionInPolarCoordinates[1]),
					positionInPolarCoordinates[2]*Math.sin(positionInPolarCoordinates[1])
			};
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	// P -> G :    P -> L, then L -> G 
	@Override
	public double[] transformCoordinatesPolarToGlobal(double[] positionInPolarCoordinates) {
		try{
			getRwLock().readLock().lock();
			if(positionInPolarCoordinates.length==2){
				// the positionInLocalCoordinate is in cylindrical coord (h,theta,r)
				// with r being implicit (half the diameter)
				// We thus have h (along xAxis) and theta (the angle from the yAxis).
				double r = 0.5*diameter;
				positionInPolarCoordinates = new double[] {
						positionInPolarCoordinates[0],
						positionInPolarCoordinates[1],
						r
				};
			}
			double[] local = transformCoordinatesPolarToLocal(positionInPolarCoordinates);
			return transformCoordinatesLocalToGlobal(local);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	// G -> L :    G -> L, then L -> P 
	@Override
	public double[] transformCoordinatesGlobalToPolar(double[] positionInGlobalCoordinates) {
		try{
			getRwLock().readLock().lock();
			double[] local = transformCoordinatesGlobalToLocal(positionInGlobalCoordinates);
			return(transformCoordinatesLocalToPolar(local));
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	

	// *************************************************************************************
	//   GETTERS & SETTERS
	// *************************************************************************************


	/** Well, there is no field cellElement. We return neuriteElement.*/
	@Override
	public CellElement getCellElement() {
		try{
			getRwLock().readLock().lock();
			return neuriteElement;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * @return the neuriteElement
	 */
	public NeuriteElement getNeuriteElement() {
		try{
			getRwLock().readLock().lock();
			return neuriteElement;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}


	/**
	 * @param neuriteElement the neuriteElement to set
	 */
	public void setNeuriteElement(NeuriteElement neuriteElement) {

		if (neuriteElement != null) {
			getRwLock().writeLock().lock();
			this.neuriteElement = neuriteElement;
			getRwLock().writeLock().unlock();
		} else {System.out.println("ERROR  PhysicalCylinder: neuriteElement already exists");
		};
		
	}


	/**
	 * @return the daughterLeft
	 */
	public PhysicalCylinder getDaughterLeft() {
		try{
			getRwLock().readLock().lock();
			return daughterLeft;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * @return the daughterRight
	 */
	public PhysicalCylinder getDaughterRight() {
		try{
			getRwLock().readLock().lock();
			return daughterRight;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * @return the mother
	 */
	public PhysicalObject getMother() {
		try{
			getRwLock().readLock().lock();
			return mother;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * @param mother the mother to set
	 */
	public void setMother(PhysicalObject mother) {
		getRwLock().writeLock().lock();
		this.mother = mother;
		getRwLock().writeLock().unlock();
	}

	/**
	 * @param daughterLeft the daughterLeft to set
	 */
	public void setDaughterLeft(PhysicalCylinder daughterLeft) {
		getRwLock().writeLock().lock();
		this.daughterLeft = daughterLeft;
		getRwLock().writeLock().unlock();
	}

	/**
	 * @param daughterRight the daughterRight to set
	 */
	public void setDaughterRight(PhysicalCylinder daughterRight) {
		getRwLock().writeLock().lock();
		this.daughterRight = daughterRight;
		getRwLock().writeLock().unlock();
	}


	/**
	 * @param branchOrder the branchOrder to set
	 */
	public void setBranchOrder(int branchOrder) {
		getRwLock().writeLock().lock();
		this.branchOrder = branchOrder;
		getRwLock().writeLock().unlock();
	}

	/**
	 * @return the branchOrder
	 */
	public int getBranchOrder() {
		try{
			getRwLock().readLock().lock();
			return branchOrder;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	public double getActualLength() {
		try{
			getRwLock().readLock().lock();
			return actualLength;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * Should not be used, since the actual length depends on the geometry.
	 * @param actualLength
	 */
	public void setActualLength(double actualLength) {
		getRwLock().writeLock().lock();
		this.actualLength = actualLength;
		getRwLock().writeLock().unlock();
	}

	public double getRestingLength() {
		try{
			getRwLock().readLock().lock();
			return restingLength;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	public void setRestingLength(double restingLength) {
		getRwLock().writeLock().lock();
		this.restingLength = restingLength;
		getRwLock().writeLock().unlock();
	}

	public double[] getSpringAxis() {
		try{
			getRwLock().readLock().lock();
			return springAxis.clone();
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	

	public void setSpringAxis(double[] springAxis) {
		getRwLock().writeLock().lock();
		this.springAxis = springAxis.clone();
		getRwLock().writeLock().unlock();
	}

	public double getSpringConstant() {
		try{
			getRwLock().readLock().lock();
			return springConstant;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}

	public void setSpringConstant(double springConstant) {
	
		getRwLock().writeLock().lock();
		this.springConstant = springConstant;
		getRwLock().writeLock().unlock();
	}

	public double getTension() {
		try{
			getRwLock().readLock().lock();
			return tension;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}

	//	public void setTension(double tension) {
	//		this.tension = tension;
	//		setRestingLengthToSetTension(tension);
	//	}

	/**
	 * Gets a vector of length 1, with the same direction as the SpringAxis.
	 * @return a normalized springAxis
	 */
	// NOT A "REAL" GETTER
	public double[] getUnitaryAxisDirectionVector() {
		try{
			getRwLock().readLock().lock();
			double factor = 1.0/actualLength;
			return new double[] {factor*springAxis[0], factor*springAxis[1], factor*springAxis[2]};
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}

	/** Should return yes if the PhysicalCylinder is considered a terminal branch.
	 * @return is it a terminal branch
	 */
	public boolean isTerminal(){
		try{
			getRwLock().readLock().lock();
			return (daughterLeft==null);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}

	/**
	 * Returns true if a bifurcation is physicaly possible. That is if the PhysicalCylinder
	 * has no daughter and the actual length is bigger than the minimum required.
	 * @return
	 */
	public boolean bifurcationPermitted(){
		try{
			getRwLock().readLock().lock();
			if(daughterLeft == null && actualLength > Param.NEURITE_MINIMAL_LENGTH_FOR_BIFURCATION){
				//			if(daughterLeft == null && getLengthToProximalBranchingPoint() > 40){
				return true;
			}
			return false;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}
	/**
	 * Returns true if a side branch is physically possible. That is if this is not a terminal
	 * branch and if there is not already a second daughter.
	 * @return
	 */
	public boolean branchPermitted(){
		try{
			getRwLock().readLock().lock();
			if(this.daughterLeft!=null && this.daughterRight==null){
				return true;
			}else{
				return false;
			}
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}

	/**
	 * retuns the position of the proximal end, ie the massLocation minus the spring axis.
	 * Is mainly used for paint
	 * @return
	 */
	public double[] proximalEnd(){
		try{
			getRwLock().readLock().lock();
			return subtract(massLocation, springAxis);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}


	/**
	 * retuns the position of the distal end, ie the massLocation coordinates (but not the 
	 * actual massLocation array).
	 * Is mainly used for paint
	 * @return
	 */
	public double[] distalEnd(){
		try{
			getRwLock().readLock().lock();
			return new double[] {massLocation[0], massLocation[1], massLocation[2]};
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}

	/**
	 * Returns the total (actual) length of all the cylinders (including the one in which this method is 
	 * called) before the previous branching point. Used to decide if long enough to bifurcate or branch,
	 * independently of the discretization.
	 * @return 
	 */
	public double lengthToProximalBranchingPoint(){
		try{
			getRwLock().readLock().lock();
			double length = actualLength;
			if (mother.isAPhysicalCylinder()) {
				PhysicalCylinder previousCylinder = (PhysicalCylinder) mother;
				if(previousCylinder.getDaughterRight() == null){
					length += previousCylinder.lengthToProximalBranchingPoint();
				}
			}
			return length;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}

	/** returns true because this object is a PhysicalCylinder */
	public boolean isAPhysicalCylinder(){
		return true;
	}

	@Override
	public double getLength() {
		try{
			getRwLock().readLock().lock();
			return actualLength;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	@Override
	public double getInterObjectForceCoefficient() {
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public void setInterObjectForceCoefficient(
			double interObjectForceCoefficient) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public double[] getAxis() {
		try{
			getRwLock().readLock().lock();
			return xAxis.clone();
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
		
	}

}
