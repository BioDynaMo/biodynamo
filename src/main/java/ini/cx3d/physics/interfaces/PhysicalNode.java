package ini.cx3d.physics.interfaces;

import ini.cx3d.SimStateSerializable;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;

import java.io.Serializable;
import java.util.Hashtable;

/**
 * Created by lukas on 26.02.16.
 */
public interface PhysicalNode extends Serializable, SimStateSerializable {
	@Override
	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	/** Returns true if this PhysicalNode is a PhysicalObject.*/
	boolean isAPhysicalObject();

	/** Returns true if this PhysicalNode is a PhysicalCylinder.*/
	boolean isAPhysicalCylinder();

	/** Returns true if this PhysicalNode is a PhysicalSphere.*/
	boolean isAPhysicalSphere();

	/**
	 * Returns the concentration of an extra-cellular Substance at this PhysicalNode.
	 * @param id the name of the substance
	 * @return the concentration
	 */
	double getExtracellularConcentration(String id);

	/**
	 * Returns the average concentration of for a PhysicalNode and all its neighbors,
	 * weighted by physical nodes volumes. Diminishes local fluctuations due to irregular
	 * triangulation, but is more expensive to compute.
	 * @param id the name of the substance
	 * @return the concentration
	 */
	double getConvolvedConcentration(String id);

	/**
	 * Returns the concentration of an extra-cellular Substance outside this PhysicalNode.
	 * @param id the name of the substance
	 * @param location the place where concentration is probed
	 * @return the concentration
	 */
	double getExtracellularConcentration(String id, double[] location);

	/**
	 * Returns the gradient at the space node location for a given substance.
	 * The way this method is implemented was suggested by Andreas Steimer.
	 * @param id the name of the Substance we have to compute the gradient of.
	 * @return [dc/dx, dc/dy, dc/dz]
	 */
	double[] getExtracellularGradient(String id);

	/** Modifies the quantity (increases or decreases) of an extra-cellular Substance.
	 * If this <code>PhysicalNode</code> already has an <code>Substance</code> instance
	 * corresponding to the type given as argument (with the same id), the fields
	 * quantity and concentration in it will be modified, based on a computation depending
	 * on the simulation time step and the space volume. If there is no such Substance
	 * instance already, a new instance is requested from ECM.
	 * <p>
	 * This method is not used for diffusion, but only by biological classes...
	 * @param id the name of the Substance to change.
	 * @param quantityPerTime the rate of quantity production
	 */
	void modifyExtracellularQuantity(String id, double quantityPerTime);

	/**
	 * Degradate (according to degrad. constant) and diffuse 8according to diff. constant)
	 * all the <code>Substance</code> stored in this <code>PhysicalNode</code>.
	 */
	void runExtracellularDiffusion();

	/** Returns the INSTANCE of Substance stored in this node, with the same id
	 * than the Substance given as argument. If there is no such Instance, a new one
	 * is created, inserted into the list extracellularSubstances and returned.
	 * Used for diffusion and for ECMChemicalReaction.*/
	Substance getSubstanceInstance(Substance templateS);

	/** Returns the position of the <code>SpatialOrganizationNode</code>.
	 * Equivalent to getSoNode().getPosition(). */
	// not a real getter...
	double[] soNodePosition();

	/**
	 * returns all <code>PhysicalNodes</code> considered as neighbors.
	 */
	Iterable<PhysicalNode> getNeighboringPhysicalNodes();

	/** Sets the SpatialOrganizationNode (vertex in the triangulation neighboring system).*/
	SpatialOrganizationNode<PhysicalNode> getSoNode();

	/** Returns the SpatialOrganizationNode (vertex in the triangulation neighboring system).*/
	void setSoNode(SpatialOrganizationNode<PhysicalNode> son);

	/** if <code>true</code>, the PhysicalNode will be run by the Scheduler.**/
	boolean isOnTheSchedulerListForPhysicalNodes();

	/** if <code>true</code>, the PhysicalNode will be run by the Scheduler.**/
	void setOnTheSchedulerListForPhysicalNodes(
			boolean onTheSchedulerListForPhysicalNodes);

	/** Add an extracellular or membrane-bound chemicals
	 *  in this PhysicalNode. */
	void addExtracellularSubstance(Substance is);

	/** Remove an extracellular or membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
	void removeExtracellularSubstance(Substance is);

	/** All the (diffusible) chemicals that are present in the space defined by this physicalNode. */
	Hashtable<String, Substance> getExtracellularSubstances();

	/** All the (diffusible) chemicals that are present in the space defined by this physicalNode. */
	void setExtracellularSubstances(
			Hashtable<String, Substance> extracellularSubstances);

	/** Solely used by the PhysicalNodeMovementListener to update substance concentrations.**/
	void setMovementConcentratioUpdateProcedure(
			int movementConcentratioUpdateProcedure);

	/** Returns a unique ID for this PhysicalNode.*/
	int getID();

	/**
	 *  Interpolation of the concentration value at a certain distance. Used to update
	 * the chemicals in case of a displacement or for the creation of new nodes, in
	 * PhysicalNodeMovementListener, in case barycentric interpolation is not possible.
	 * @param s the substance
	 * @param dX the distance from this nodes's location
	 * @return
	 */
	double computeConcentrationAtDistanceBasedOnGradient(ini.cx3d.physics.interfaces.Substance s, double[] dX);

	/** Solely used by the PhysicalNodeMovementListener to update substance concentrations.**/
	int getMovementConcentratioUpdateProcedure();
}
