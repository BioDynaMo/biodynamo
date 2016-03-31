package ini.cx3d.physics.interfaces;

import ini.cx3d.localBiology.interfaces.CellElement;
import ini.cx3d.synapses.Excrescence;

import java.awt.*;
import java.util.AbstractSequentialList;
import java.util.Hashtable;

/**
 * Created by lukas on 09.03.16.
 */
public interface PhysicalObject extends PhysicalNode {
	@Override
	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	/** Returns true because this object is a PhysicalObject.*/
	boolean isAPhysicalObject();

	/**
	 * Returns true if this <code>PhysicalObject</code> and the <code>PhysicalObject</code> given as
	 * argument have a mother-daughter or sister-sister (e.g. two daughters of a same mother) relation.
	 */
	boolean isRelative(PhysicalObject po);

	/** Returns the <code>CellElement</code>linked to this <code>PhysicalObject</code>.*/
	CellElement getCellElement();

	/** Adds an <code>Excrescence</code> instance to the Excrescence list of this
	 * <code>PhysicalObject</code>.*/
	void addExcrescence(Excrescence ex);

	/** Removes an <code>Excrescence</code> instance to the Excrescence list of this
	 * <code>PhysicalObject</code>.*/
	void removeExcrescence(Excrescence ex);

	/** Active displacement of the point mass of this <code>PhysicalObject</code>. ("active" means
	 * "triggered by a biological process" like in growth or migration). This method MUST NOT be used
	 * for displacement by purely passive (physical) force.
	 * @param speed in microns/hour
	 * @param direction a vector indicating the direction of movement
	 */
	void movePointMass(double speed, double[] direction);

	/** Compute physical forces, and move accordingly to one simulation time step.*/
	void runPhysics();

	/**
	 * Returns the inter-object force that the <code>PhysicalObject</code> in which the method is called applies
	 * onto the <code>PhysicalSphere</code> given as argument.
	 * @param s
	 * @return
	 */
	double[] getForceOn(ini.cx3d.physics.interfaces.PhysicalSphere s);

	/**
	 * Returns true if this <code>PhysicalObject</code> is in contact, i.e. if it is
	 * close enough to the <code>PhysicalObject</code> given as argument.
	 * @param o
	 * @return
	 */
	boolean isInContact(PhysicalObject o);

	/**
	 * Returns all the neighboring objects considered as being in contact with this PhysicalObject.
	 * @return
	 */
	AbstractSequentialList<PhysicalObject> getPhysicalObjectsInContact();

	void changeDiameter(double speed);

	void changeVolume(double speed);

	/**
	 * Returns the position in the global coordinate system (cartesian coordinates)
	 * of a point expressed in polar coordinates (cylindrical or spherical).
	 * @param positionInPolarCoordinates a point defined in polar coordinate system of a PhysicalObject
	 * @return [x,y,z] the absolute value in space
	 */
	double[] transformCoordinatesPolarToGlobal(double[] positionInPolarCoordinates);

	/**
	 * Returns the position in the polar coordinate system (cylindrical or spherical)
	 * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
	 * @param positionInAbsoluteCoordinates the [x,y,z] cartesian values
	 * @return the position in local coord.
	 */
	double[] transformCoordinatesGlobalToPolar(double[] positionInAbsoluteCoordinates);

	/**
	 * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
	 * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
	 * @param positionInGlobalCoord
	 * @return
	 */
	double[] transformCoordinatesGlobalToLocal(double[] positionInGlobalCoord);

	/**
	 * Returns the position in in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
	 * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
	 * @param positionInLocalCoord
	 * @return
	 */
	double[] transformCoordinatesLocalToGlobal(double[] positionInLocalCoord);

	/**
	 * Returns a unit vector, pointing out of the PhysicalObject if origin at location
	 * specified in argument.
	 * @param positionInPolarCoordinates the origin of the normal vector (local cartesian coord)
	 * @return a vector pointing "out", of unitary norm (absolute cartesian coord)
	 */
	double[] getUnitNormalVector(double[] positionInPolarCoordinates);

	/** Simply adds the argument to the vector containing all the PhysicalBonds of this
	 * PhysicalObject.*/
	void addPhysicalBond(PhysicalBond pb);

	/** Simply removes the argument from the vector containing all the PhysicalBonds of this
	 * PhysicalObject. */
	void removePhysicalBond(PhysicalBond pb);

	/** Returns true if there is a PhysicalBond that fixes me to this other PhysicalObject.*/
	boolean getHasAPhysicalBondWith(PhysicalObject po);

	/**
	 * Creates a new PhysicalBond between this PhysicalObject and the one given as argument.
	 * The newly created PhysicalBond is inserted into the physical bon's list of both objects.
	 * @param po
	 * @return
	 */
	PhysicalBond makePhysicalBondWith(PhysicalObject po);

	/**
	 * If there is a PhysicalBond between this PhysicalObject and po,
	 * it is removed (in both objects).
	 * @param po the other PhysicalObject we want to test with
	 * @param removeThemAll if true, makes multiple removals (if multiple bonds)
	 * @return true if at least one PhysicalBond was removed
	 */
	boolean removePhysicalBondWith(PhysicalObject po, boolean removeThemAll);

	/** Compute diffusion of <code>IntracellularSubstances</code> with relatives in the neuron
	 * tree structure, and perform diffusion processes according to one simulation time step.*/
	void runIntracellularDiffusion();

	/**
	 * Returns the concentration of an <code>IntracellularSubstance</code> in this
	 * PhysicalObject. If not present at all, zhe value 0 is returned.
	 * @param substanceId
	 * @return
	 */
	double getIntracellularConcentration(String substanceId);

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
	void modifyIntracellularQuantity(String id, double quantityPerTime);

	/** Returns the concentration of a membrane bound IntracellularSubstance on
	 * this PhysicalObject. Recall that by definition, the PhysicalObject are
	 * considered as expressing a cell type specific protein as well as the universal
	 * marker "U".
	 * @param id
	 * @return
	 */
	double getMembraneConcentration(String id);

	/** Modifies the quantity (increases or decreases) of an membrane-bound chemical.
	 *
	 * This method is not used for diffusion, but only by biological classes...
	 * @param id the name of the Substance to change.
	 * @param quantityPerTime the rate of quantity production
	 */

	void modifyMembraneQuantity(String id, double quantityPerTime);

	/** Returns the <code>java.awt.Color</code> used to draw this PhysicalObject in the GUI. */
	Color getColor();

	/** Sets the <code>java.awt.Color</code> used to draw this PhysicalObject in the GUI. */
	void setColor(Color color);

	/** Returns a copy of the masslocation.*/
	double[] getMassLocation();

	/**
	 * - CAUTION : Never use this method to move a PhysicalObject, because the physics is not updated.
	 * <p>
	 * - Never?
	 * <p>
	 *  - I said NEVER !
	 * @param massLocation the massLocation to set
	 */
	void setMassLocation(double[] massLocation);

	/** Returns the "Up" direction for a Sphere, and the axis direction for a Cylinder*/
	double[] getAxis();

	/** Returns the first axis of the local coordinate system.*/
	double[] getXAxis();

	/** Sets the first axis of the local coordinate system. Should have a norm of 1.*/
	void setXAxis(double[] axis);

	/** Returns the second axis of the local coordinate system.*/
	double[] getYAxis();

	/** Sets the second axis of the local coordinate system. Should have a norm of 1*/
	void setYAxis(double[] axis);

	/** Returns the third axis of the local coordinate system.*/
	double[] getZAxis();

	/** Sets the third axis of the local coordinate system. Should have a norm of 1*/
	void setZAxis(double[] axis);

	/** Only for GUI display. Total force on this objects point mass, last time it was computed.
	 * 3 first components give the x,y,z coord, and last one if movement was applied (<0 means no).*/
	double[] getTotalForceLastTimeStep();

	/** Returns true if this object still plays a role in the simulation.
	 * For instance a PhysicalObject associated with a neurite that just retracted
	 * is not exxisting. */
	boolean isStillExisting();

	/** The role of the method is to indicate that an object is about to
	 * be garbage Collected. Caution: don't use this method! */
	void setStillExisting(boolean stillExists);

	/** If true, this PhysicalObject will be run by the Scheduler on the next occasion.*/
	boolean isOnTheSchedulerListForPhysicalObjects();

	/** If true, this PhysicalObject will be run by the Scheduler on the next occasion.*/
	void setOnTheSchedulerListForPhysicalObjects(
			boolean onTheSchedulerListForPhysicalObjects);

	/** Returns the vector containing all the PhysicalBonds of this PhysicalObject.*/
	AbstractSequentialList<PhysicalBond> getPhysicalBonds();

	/** Sets the vector containing all the PhysicalBonds of this PhysicalObject.
	 * This methof should not be used during the simulation. */
//	void setPhysicalBonds(Vector<PhysicalBond> physicalBonds);

	/** Returns the vector containing all the Excrescences (PhysicalSpine, PhysicalBouton).*/
	AbstractSequentialList<Excrescence> getExcrescences();

	/** Sets the vector containing all the Excrescences (PhysicalSpine, PhysicalBouton).
	 * This method should not be used during a simulation. */
//	void setExcrescences(Vector<Excrescence> excrescences);

	/** Returns the adherence to the extracellular matrix, i.e. the static friction
	 * (the minimum force amplitude needed for triggering a movement). */
	double getAdherence();

	/** Sets the adherence to the extracellular matrix, i.e. the static friction
	 * (the minimum force amplitude needed for triggering a movement). */
	void setAdherence(double adherence);

	/** Returns the mass, i.e. the kinetic friction
	 * (scales the movement amplitude, therefore is considered as the mass).*/
	double getMass();

	/** Sets the mass, i.e. the kinetic friction
	 * (scales the movement amplitude, therefore is considered as the mass).*/
	void setMass(double mass);

	double getDiameter();

	/**
	 * Sets the diameter to a new value, and update the volume accordingly.
	 * is equivalent to setDiamater(diameter, true)
	 * @param diameter
	 */
	void setDiameter(double diameter);

	/**
	 * Sets the diameter. The volume is sets accordingly if desired.
	 *
	 * @param diameter the new diameter
	 * @param updateVolume if true, the volume is set to match the new diameter.
	 */
	void setDiameter(double diameter, boolean updateVolume);

	/** Returns the volume of this PhysicalObject.*/
	double getVolume();

	/**
	 * Sets the volume, and (optionally) recomputes an new diameter.
	 * THIS METHOD SHOULD NOT BE USED TO INITIALIZE A PHYSICAL_OBJECT.
	 * DEFINE DIMENSIONS, AND THE VOLUME WILL BE COMPUTED.
	 * @param volume the new volume
	 * @param updateDiameter if true, the diameter will be updated.
	 */
	void setVolume(double volume, boolean updateDiameter);

	/**
	 * helper function that only sets the volume attribute to the specified value
	 * @param volume
	 */
	void setVolumeOnly(double volume);

	/**
	 * Sets the volume, and recomputes an new diameter.
	 * THIS METHOD SHOULD NOT BE USED TO INITIALIZE A PHYSICAL_OBJECT.
	 * DEFINE DIMENSIONS, AND THE VOLUME WILL BE COMPUTED.
	 * @param volume the new volume
	 */
	void setVolume(double volume);

	/** Returns the length of a cylinder, or the diameter of a sphere.*/
	double getLength();

	/** Get an intracellular and membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
	IntracellularSubstance getIntracellularSubstance(String id);

	/** Add an intracellular or membrane-bound chemicals
	 *  in this PhysicalNode. */
	void addIntracellularSubstance(IntracellularSubstance is);

	/** Remove an intracellular or membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
	void removeIntracellularSubstance(IntracellularSubstance is);

	/** All the intracellular and membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
	Hashtable<String, IntracellularSubstance> getIntracellularSubstances();

	/** All the intracellular and membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
//	void setIntracellularSubstances(
//			Hashtable<String, IntracellularSubstance> intracellularSubstances);

	double getInterObjectForceCoefficient();

	void setInterObjectForceCoefficient(double interObjectForceCoefficient);

	/**
	 * Convenient way to change family links in the neuron tree structure
	 * (mother, daughter branch). This method is useful during elongation and
	 * retraction for intercalation/removal of elements.
	 * of elements.
	 */
	void updateRelative(ini.cx3d.physics.interfaces.PhysicalObject oldRelative, ini.cx3d.physics.interfaces.PhysicalObject newRelative);

	/**
	 * Returns the absolute coordinates of the location where a <code>PhysicalObject</code> is attached
	 * to this <code>PhysicalObject</code>. Does not necessarily contain a check of the identity of the
	 *  element that makes the request.
	 *
	 * @param daughterWhoAsks the PhysicalObject attached to us.
	 * @return the coord
	 */
	double[] originOf(ini.cx3d.physics.interfaces.PhysicalObject daughterWhoAsks);

	/**
	 * Removal of a <code>PhysicalObject</code> from the list of our daughters.
	 * (Mainly in case of complete retraction of the daughter.*/
	void removeDaugther(ini.cx3d.physics.interfaces.PhysicalObject daughterToRemove);

	/**
	 * Resets some computational and physical properties (like the tension, volume) after
	 * a displacement
	 */
	void updateDependentPhysicalVariables();

	/* Diffusion of diffusible IntracellularSubstances between two PhysicalObjects.
	 */
	void diffuseWithThisPhysicalObjects(PhysicalObject po, double distance);

	/* Returns the INSTANCE of IntracellularSubstance in this PhysicalObject with the same id
	 * than the IntracellularSubstance given as argument. If there is no such instance, a
	 * new one with similar properties is created, inserted into the intracellularSubstances
	 * vector and then returned. Only used between subclasses of physicalObject for intracellular
	 * diffusion. C.f. very similar method : PhysicalNode.giveYourSubstanceInstance.
	 */
	IntracellularSubstance giveYouIntracellularSubstanceInstance(IntracellularSubstance templateS);

	/**
	 * Returns the inter-object force that the <code>PhysicalObject</code> in which the method is called applies
	 * onto the <code>PhysicalCylinder</code> given as argument.
	 * @param c
	 * @return
	 */
	double[] getForceOn(PhysicalCylinder c);


	void setTotalForceLastTimeStep(double[] totalForceLastTimeStep);

}
