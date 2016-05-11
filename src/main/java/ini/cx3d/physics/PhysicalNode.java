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

import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.distance;
import static ini.cx3d.utilities.Matrix.dot;
import static ini.cx3d.utilities.Matrix.solve;
import static ini.cx3d.utilities.Matrix.subtract;
import static ini.cx3d.utilities.StringUtilities.toStr;

import ini.cx3d.Param;
import ini.cx3d.SimStateSerializationUtil;
import ini.cx3d.physics.factory.SubstanceFactory;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.spatialOrganization.SpatialOrganizationEdge;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;

import ini.cx3d.physics.interfaces.Substance;
import ini.cx3d.spatialOrganization.interfaces.Edge;
import ini.cx3d.spatialOrganization.interfaces.SpaceNode;
import ini.cx3d.utilities.StringUtilities;

import java.util.*;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;



/**
 * PhysicalNode represents a piece of the simulation space, whether it contains a physical object of not.
 * As such, it contains a list of all the chemicals (<code>Substance</code>) that are present at this place,
 * as well as the methods for the diffusion. In order to be able to diffuse chemicals, it contains a node
 * that is part of the neighboring system (eg triangulation). A <code>PhysicalNode</code> can only diffuse
 * to and receive from the neighbouring <code>PhysicalNode</code>s.
 * <p>
 * The <code>PhysiacalObject</code> sub-classes (<code>PhysicalSphere</code>, <code>PhysicalCylinder</code>)
 * inherit from this class. This emphasize the fact that they take part in the definition of space and
 * that diffusion of chemical occurs across them.
 * <p>
 * As all the CX3D runnable objects, the PhysicalNode updates its state (i.e. diffuses and degradates) only
 * if needed. The private field <code>onTheSchedulerListForPhysicalNodes</code> is set to <code>true</code>
 * in this case. (For degradation, there is an update mechanism, catching up from the last time it was performed).
 *
 * @author fredericzubler
 *
 */
public class PhysicalNode implements ini.cx3d.physics.interfaces.PhysicalNode {
	// NOTE : all the "protected" fields are not "private" because they have to be accessible by subclasses

	/* Unique identification for this CellElement instance. Used for marshalling/demarshalling*/
	int ID = 0;
	static AtomicInteger idCounter = new AtomicInteger(0);


	/* Reference to the ECM. */
	protected static ini.cx3d.simulations.interfaces.ECM ecm = ECMFacade.getInstance();

	/* If true, the PhysicalNode will be run by the Scheduler.*/
	private boolean onTheSchedulerListForPhysicalNodes = true;

	/* Stores the time at which degradation was performed last (important for later catch up).*/
	private double lastECMTimeDegradateWasRun = ecm.getECMtime();

	/* Used by the PhysicalNodeMovementListener to update substance concentrations.*/
	private int movementConcentratioUpdateProcedure = -999;

	/* All the (diffusible) chemicals that are present in the space defined by this physicalNode. */
	private Hashtable<String, ini.cx3d.physics.interfaces.Substance> extracellularSubstances = new Hashtable<String, ini.cx3d.physics.interfaces.Substance>();

	/* My anchor point in the neighboring system */
	protected SpatialOrganizationNode<ini.cx3d.physics.interfaces.PhysicalNode> soNode;

	/* used for synchronisation for multithreading. introduced by Haurian*/
	private ReadWriteLock rwLock = new ReentrantReadWriteLock();

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{");

		SimStateSerializationUtil.keyValue(sb, "ID", ID);
		SimStateSerializationUtil.keyValue(sb, "idCounter", idCounter.get());
		SimStateSerializationUtil.keyValue(sb, "onTheSchedulerListForPhysicalNodes", onTheSchedulerListForPhysicalNodes);
		SimStateSerializationUtil.keyValue(sb, "lastECMTimeDegradateWasRun", lastECMTimeDegradateWasRun);
		SimStateSerializationUtil.keyValue(sb, "movementConcentratioUpdateProcedure", movementConcentratioUpdateProcedure);
		SimStateSerializationUtil.map(sb, "extracellularSubstances", extracellularSubstances);
		SimStateSerializationUtil.keyValue(sb, "soNode", soNode);

		SimStateSerializationUtil.removeLastChar(sb);
		sb.append("}");
		return sb;
	}

	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("(");
		sb.append(toStr(ID));
		sb.append(", ");
		sb.append(toStr(idCounter.get()));
		sb.append(", ");
		sb.append(onTheSchedulerListForPhysicalNodes);
		sb.append(", ");
		sb.append(toStr(lastECMTimeDegradateWasRun));
		sb.append(", ");
		sb.append(toStr(movementConcentratioUpdateProcedure));
		sb.append(", ");
		sb.append(toStr(extracellularSubstances));
		sb.append(", ");
		sb.append(soNode);
		sb.append(")");
		return sb.toString();
	}

	public PhysicalNode(){
//		registerJavaObject(this);
		getRwLock().writeLock().lock();
		this.ID = idCounter.incrementAndGet();
		getRwLock().writeLock().unlock();
	}

	// *************************************************************************************
	// *        instanceof-like methods                                                    *
	// *************************************************************************************


	/** Returns true if this PhysicalNode is a PhysicalObject.*/
	@Override
	public boolean isAPhysicalObject(){
		//The function is overwritten in PhysicalObject.
		return false;
	}

	/** Returns true if this PhysicalNode is a PhysicalCylinder.*/
	@Override
	public boolean isAPhysicalCylinder(){
		// The function is overwritten in PhysicalSphere.
		return false;
	}

	/** Returns true if this PhysicalNode is a PhysicalSphere.*/
	@Override
	public boolean isAPhysicalSphere(){
		// The function is overwritten in PhysicalSphere
		return false;
	}



	// *************************************************************************************
	// *        INTERACTION WITH PHYSICAL_OBJECTS (secretion, reading concentration etc.)  *
	// *************************************************************************************

	/**
	 * Returns the concentration of an extra-cellular Substance at this PhysicalNode.
	 * @param id the name of the substance
	 * @return the concentration
	 */
	@Override
	public double getExtracellularConcentration(String id){
		getRwLock().readLock().lock();
		try{
			double c = 0.0;
			if(ecm.thereAreArtificialGradients()){
				c += ecm.getValueArtificialConcentration(id, getSoNode().getPosition());
			}
			ini.cx3d.physics.interfaces.Substance s = extracellularSubstances.get(id);
			if(s==null){
				return c;
			}else{
				getRwLock().readLock().unlock();
				degradate(ecm.getECMtime()); // make sure you are up-to-date weight/ degradation
				getRwLock().readLock().lock();
				return c + s.getConcentration();

			}

		}
		finally{
			getRwLock().readLock().unlock();
		}

	}

	/**
	 * Returns the average concentration of for a PhysicalNode and all its neighbors,
	 * weighted by physical nodes volumes. Diminishes local fluctuations due to irregular
	 * triangulation, but is more expensive to compute.
	 * @param id the name of the substance
	 * @return the concentration
	 */
	@Override
	public double getConvolvedConcentration(String id) {
		getRwLock().readLock().lock();
		try{

			double volSum=0;
			double exC = 0;
			double currVol = this.getSoNode().getVolume();
			double currC = this.getExtracellularConcentration(id);
			volSum = volSum + currVol;
			exC = exC + currVol*currC;
			for (ini.cx3d.physics.interfaces.PhysicalNode pn : this.getSoNode().getNeighbors()) {
				currVol = pn.getSoNode().getVolume();
				currC = pn.getExtracellularConcentration(id);
				exC = exC + currVol*currC;
				volSum = volSum + currVol;
			}

			return exC/volSum;
		}
		finally{
			getRwLock().readLock().unlock();
		}
	}


	/**
	 * Returns the concentration of an extra-cellular Substance outside this PhysicalNode.
	 * @param id the name of the substance
	 * @param location the place where concentration is probed
	 * @return the concentration
	 */
	@Override
	public double getExtracellularConcentration(String id, double[] location){
		getRwLock().readLock().lock();
		try{
			double c = 0.0;
			if(ecm.thereAreArtificialGradients()){
				c += ecm.getValueArtificialConcentration(id, location);
			}

			Object[] vertices = soNode.getVerticesOfTheTetrahedronContaining(location);
			double concentrationAtLocation = 0;
			if(vertices != null){
				double[] barycentricCoord = getBarycentricCoordinates_java(location, vertices);
				for (int j = 0; j < 4; j++) {
					concentrationAtLocation += ((ini.cx3d.physics.interfaces.PhysicalNode)vertices[j]).getExtracellularConcentration(id)*barycentricCoord[j];
				}
			}
			return c + concentrationAtLocation;
		}
		finally{
			getRwLock().readLock().unlock();
		}

	}


	/**
	 * Returns the gradient at the space node location for a given substance.
	 * The way this method is implemented was suggested by Andreas Steimer.
	 * @param id the name of the Substance we have to compute the gradient of.
	 * @return [dc/dx, dc/dy, dc/dz]
	 */
	@Override
	public double[] getExtracellularGradient(String id){
		getRwLock().readLock().lock();
		try{
			// the gradient can be composed of diffusible Substances and artificial substances in ecm
			double[] grad;
			// 1. diffusible substance component
			ini.cx3d.physics.interfaces.Substance s = extracellularSubstances.get(id);
			if(s==null){
				grad = new double[] {0.0, 0.0, 0.0};
			} else {
				// distance to three neighbors
				double[][] vectorsToNeighbors = new double[3][];
				double[] differencesBetweenTheNeighborsAndThis = new double[3];
				int indexOfTheEquationWeGet=0;
				// loop through the neighbors until we have selected three of them (indexOfTheEquationWeGet)
				for (ini.cx3d.physics.interfaces.PhysicalNode n : soNode.getNeighbors()) {
					double substanceConcentrationInNeighbor = n.getExtracellularConcentration(id);
					// prepare the linear system to be solved
					vectorsToNeighbors[indexOfTheEquationWeGet] = subtract(n.soNodePosition(), this.soNode.getPosition());
					differencesBetweenTheNeighborsAndThis[indexOfTheEquationWeGet] = (substanceConcentrationInNeighbor-s.getConcentration());
					indexOfTheEquationWeGet++;
					// only three equations;
					if(indexOfTheEquationWeGet>2)
						break;
				}
				String param1 = StringUtilities.toStr(vectorsToNeighbors);
				String param2 = StringUtilities.toStr(differencesBetweenTheNeighborsAndThis);
				grad = solve(vectorsToNeighbors, differencesBetweenTheNeighborsAndThis);
				String ret = StringUtilities.toStr(grad);
			}
			// 2. ECM's artificial gradient component
			if(ecm.thereAreArtificialGradients()){
				grad = add(grad, ecm.getGradientArtificialConcentration(id, getSoNode().getPosition()));
			}
			return grad;
		}
		finally{
			getRwLock().readLock().unlock();
		}
	}

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
	@Override
	public void modifyExtracellularQuantity(String id, double quantityPerTime){
		getRwLock().writeLock().lock();

		ini.cx3d.physics.interfaces.Substance ss = extracellularSubstances.get(id);
		if(ss==null){
			ss = ecm.substanceInstance(id);
			extracellularSubstances.put(id, ss);
		}
		double deltaQ = quantityPerTime*Param.SIMULATION_TIME_STEP;
		double volume = soNode.getVolume();
		degradate(ecm.getECMtime()); // make sure you are up-to-date weight/ degradation
		ss.updateQuantityBasedOnConcentration(volume); // TODO : is this step really necessary ?
		ss.changeQuantityFrom(deltaQ);
		ss.updateConcentrationBasedOnQuantity(volume);
		// we will diffuse next time step
		onTheSchedulerListForPhysicalNodes = true;

		getRwLock().writeLock().unlock();
	}


	// *************************************************************************************
	// *                            RUN (diffusion, degradation)                           *
	// *************************************************************************************

	/**
	 * Degradate (according to degrad. constant) and diffuse 8according to diff. constant)
	 * all the <code>Substance</code> stored in this <code>PhysicalNode</code>.
	 */
	@Override
	public void runExtracellularDiffusion(){
		// 1) now that we are about to diffuse, a new diffusion should only be performed
		// if there is a good reason for that.
		onTheSchedulerListForPhysicalNodes = false;
		double currentEcmTime = ecm.getECMtime();
		// 2) Degradation according to the degradation constant for each chemical
		degradate(currentEcmTime);
		// 3) Diffusion (along every edge)
		AbstractSequentialList<Edge> edges = soNode.getEdges();
		for (int i = 0; i < edges.size(); i++) {
			diffuseEdgeAnalytically(edges.get(i), currentEcmTime);
		}
	}

	/**
	 * Runs the degradation of all Substances (only if it is not up-to-date). This method
	 * is called before each operation on Substances (
	 * @param currentEcmTime the current time of the caller
	 * (so that it doesn't require a call to ECM).
	 */
	public void degradate(double currentEcmTime){ //changed to proteceted

		// if we are up-to-date : we stop here.
		if(lastECMTimeDegradateWasRun > currentEcmTime){
			return;
		}

		//getRwLock().writeLock().lock();
		// Otherwise, degradation according to the degradation constant for each chemical
		double deltaT = currentEcmTime-lastECMTimeDegradateWasRun;
		for (ini.cx3d.physics.interfaces.Substance s : getExtracellularSubstances()) {
			double decay = Math.exp(-s.getDegradationConstant()*deltaT);
			s.multiplyQuantityAndConcentrationBy(decay);
		}
		// We store the current time as the last time we updated degradation
		// (+0.0000001, to be on the safe side of the double comparison)
		lastECMTimeDegradateWasRun= currentEcmTime+0.0000001;
		//getRwLock().writeLock().unlock();
	}

	/* Analytic solution of the diffusion process along the edge between two PhysicalNodes.
	 * dQA/dt = diffCst*(Area/distance)*(QB/VB-QA/VA)
	 */
	private void diffuseEdgeAnalytically(SpatialOrganizationEdge<ini.cx3d.physics.interfaces.PhysicalNode> e, double currentEcmTime) {
		try {

			// the two PhysicalNodes
			PhysicalNode nA = this;
			ini.cx3d.physics.interfaces.PhysicalNode tmp = e.getOppositeElement(this);
			if(e == null || tmp == null || !(tmp instanceof PhysicalNode)){
				throw new RuntimeException();
			}
			PhysicalNode nB = (PhysicalNode) e.getOppositeElement(this);

			//always lock highest first! to avoid deadlock!
			ReadWriteLock r1;
			ReadWriteLock r2;
			if (nA.ID > nB.ID) {
				r1 = nA.getRwLock();
				r2 = nB.getRwLock();
			} else {
				r1 = nB.getRwLock();
				r2 = nA.getRwLock();
			}
			r1.writeLock().lock();
			r2.writeLock().lock();

			// make sure the other one is up-to-date with degradation
			nB.degradate(currentEcmTime);
			// some values about space node distances, contact area and volume
			SpatialOrganizationNode<ini.cx3d.physics.interfaces.PhysicalNode> sonA = getSoNode();
			SpatialOrganizationNode<ini.cx3d.physics.interfaces.PhysicalNode> sonB = nB.getSoNode();
			double distance = distance(sonA.getPosition(), sonB.getPosition());
			double vA = sonA.getVolume();
			double vB = sonB.getVolume();
			double pre_a = (e.getCrossSection() / distance);
			double pre_m = (e.getCrossSection() / distance) * (1.0 / vA + 1.0 / vB);

			// diffusion of all the extracellularSubstances in A :
			for (Substance sA : getExtracellularSubstances() ) {
				// for a given substance
//				Substance sA = substancesEnumeration.nextElement();
				double sAConcentration = sA.getConcentration();
				// stop here if 1) non diffusible substance or 2) concentration very low:
				double diffusionConstant = sA.getDiffusionConstant();
				if (diffusionConstant < 10E-14 || sAConcentration < Param.MINIMAL_CONCENTRATION_FOR_EXTRACELLULAR_DIFFUSION) {
					continue; // to avoid a division by zero in the n/m if the diff const = 0;
				}
				// find the counterpart in B
				ini.cx3d.physics.interfaces.Substance sB = nB.getSubstanceInstance(sA);
				double sBConcentration = sB.getConcentration();
				// saving time : no diffusion if almost no difference;
				double absDiff = Math.abs(sAConcentration - sBConcentration);
				if ((absDiff < Param.MINIMAL_DIFFERENCE_CONCENTRATION_FOR_EXTRACELLULAR_DIFFUSION) ||
						(absDiff / sAConcentration < Param.MINIMAL_DC_OVER_C_FOR_EXTRACELLULAR_DIFFUSION)) {
					continue;
				}
				// If we reach this point, it means that it is worth performing the diffusion.
				// we thus put ourselves on the list for performing it again next time step.
				nB.setOnTheSchedulerListForPhysicalNodes(true);
				this.onTheSchedulerListForPhysicalNodes = true;

				// Analytical computation of the diffusion between these two nodes
				// (cf document "Diffusion" by F.Zubler for explanation).

				double qA = sA.getQuantity();
				double qB = sB.getQuantity();
				double Tot = qA + qB;
				double a = pre_a * diffusionConstant;
				double m = pre_m * diffusionConstant;

				double n = a * Tot / vB;
				double nOverM = n / m;
				double K = qA - nOverM;

				qA = K * Math.exp(-m * Param.SIMULATION_TIME_STEP) + nOverM;
				qB = Tot - qA;

				sA.setQuantity(qA);
				sB.setQuantity(qB);
				// and update their concentration again
				sA.updateConcentrationBasedOnQuantity(vA);
				sB.updateConcentrationBasedOnQuantity(vB);
			}

			r1.writeLock().unlock();
			r2.writeLock().unlock();
		} catch(Exception ex) {
			ex.printStackTrace();
		}
	}


	/** Returns the INSTANCE of Substance stored in this node, with the same id
	 * than the Substance given as argument. If there is no such Instance, a new one
	 * is created, inserted into the list extracellularSubstances and returned.
	 * Used for diffusion and for ECMChemicalReaction.*/
	@Override
	public ini.cx3d.physics.interfaces.Substance getSubstanceInstance(Substance templateS){
		try
		{
			//write lock because of possible changement of substances!
			getRwLock().writeLock().lock();
			ini.cx3d.physics.interfaces.Substance s = extracellularSubstances.get(templateS.getId());
			if(s == null){
				// if it doesn't exist, you create it (with concentration 0)
				s = SubstanceFactory.create(templateS);
				s.setConcentration(0);
				s.setQuantity(0);
				extracellularSubstances.put(s.getId(), s);
			}
			return s;
		}
		finally
		{
			getRwLock().writeLock().unlock();
		}
	}


	// *************************************************************************************
	// *      METHODS FOR INTERPOLATION (used only by PhysicalNodeMovementListener)        *
	// *************************************************************************************

	/*
	 * Finding the barycentric coordinates of a point Q with respect to the the four points P
	 * (based on http://www.devmaster.net/wiki/Barycentric_coordinates).
	 * @param Q
	 * @param P1
	 * @param P2
	 * @param P3
	 * @param P4
	 * @return array with the coordinate of point Q with respect Pi
	 */
	public static double[] getBarycentricCoordinates_java(double[] Q, double[] P1, double[] P2, double[] P3, double[] P4){

		// three linearly independent vectors
		double[] B1 = subtract(P2,P1);
		double[] B2 = subtract(P3,P1);
		double[] B3 = subtract(P4,P1);
		// finding how to express (Q-P1) with these three vectors : gives the 2nd, 3rd and 4th coordinate
		double[][] A = { 	{B1[0], B2[0], B3[0] },
				{B1[1], B2[1], B3[1] },
				{B1[2], B2[2], B3[2] } };
		double[] barycentricCoord =  solve(A,subtract(Q,P1));
		// to find the first component : the total = 1
		double firstCoord = 1-(barycentricCoord[0]+barycentricCoord[1]+barycentricCoord[2]);

		return new double[] {firstCoord, barycentricCoord[0], barycentricCoord[1], barycentricCoord[2]};
	}

	/*
	 * Finding the barycentric coordinates of a point Q with respect to the the four vertices.
	 * @param Q
	 * @param vertices
	 * @return
	 */
	public static double[] getBarycentricCoordinates_java(double[] Q, Object[] vertices){
		double[] a = ((ini.cx3d.physics.interfaces.PhysicalNode)vertices[0]).getSoNode().getPosition();
		double[] b = ((ini.cx3d.physics.interfaces.PhysicalNode)vertices[1]).getSoNode().getPosition();
		double[] c = ((ini.cx3d.physics.interfaces.PhysicalNode)vertices[2]).getSoNode().getPosition();
		double[] d = ((ini.cx3d.physics.interfaces.PhysicalNode)vertices[3]).getSoNode().getPosition();
		return getBarycentricCoordinates_java(Q,a,b,c,d);
	}


	/* Interpolation of the concentration value at a certain distance. Used to update
	 * the chemicals in case of a displacement or for the creation of new nodes, in
	 * PhysicalNodeMovementListener, in case barycentric interpolation is not possible.
	 * @param s the substance
	 * @param dX the distance from this nodes's location
	 * @return
	 */
	@Override
	public double computeConcentrationAtDistanceBasedOnGradient(ini.cx3d.physics.interfaces.Substance s, double[] dX){
		try
		{
			getRwLock().readLock().lock();

			// if the point that interests us is inside a tetrahedron, we interpolate the
			// value based on the tetrahedron

			// otherwise we compute the gradient, and multiply it with the displacement -> get value;
			double[] gradient = getExtracellularGradient(s.getId());
			double newConcentration = s.getConcentration() + dot(gradient,dX);
			if(newConcentration < 0)
				newConcentration = 0;
			return newConcentration;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	// *************************************************************************************
	// *      GETTERS & SETTERS                                                            *
	// *************************************************************************************


	/** Returns the position of the <code>SpatialOrganizationNode</code>.
	 * Equivalent to getSoNode().getPosition(). */
	// not a real getter...
	@Override
	public double[] soNodePosition(){
		try
		{
			getRwLock().readLock().lock();

			return soNode.getPosition();

		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/**
	 * returns all <code>PhysicalNodes</code> considered as neighbors.
	 */
//	@Override
//	public Iterable<ini.cx3d.physics.interfaces.PhysicalNode> getNeighboringPhysicalNodes(){
//		try
//		{
//			getRwLock().readLock().lock();
//			Vector<ini.cx3d.physics.interfaces.PhysicalNode> temp = new Vector<ini.cx3d.physics.interfaces.PhysicalNode>();
//			for(ini.cx3d.physics.interfaces.PhysicalNode o:soNode.getNeighbors())
//			{
//				temp.add(o);
//			}
//			return temp;
//		}
//		finally
//		{
//			getRwLock().readLock().unlock();
//		}
//
//
//	}


	/** Sets the SpatialOrganizationNode (vertex in the triangulation neighboring system).*/
	@Override
	public SpaceNode<ini.cx3d.physics.interfaces.PhysicalNode> getSoNode(){
		try
		{
			getRwLock().readLock().lock();
			return (SpaceNode<ini.cx3d.physics.interfaces.PhysicalNode>) soNode;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	/** Returns the SpatialOrganizationNode (vertex in the triangulation neighboring system).*/
	@Override
	public void setSoNode(SpaceNode son){

			getRwLock().writeLock().lock();
			this.soNode = son;
			getRwLock().writeLock().unlock();
	}


	/** if <code>true</code>, the PhysicalNode will be run by the Scheduler.**/
	@Override
	public boolean isOnTheSchedulerListForPhysicalNodes() {
		try
		{
			getRwLock().readLock().lock();
			return onTheSchedulerListForPhysicalNodes;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	/** if <code>true</code>, the PhysicalNode will be run by the Scheduler.**/
	@Override
	public void setOnTheSchedulerListForPhysicalNodes(
			boolean onTheSchedulerListForPhysicalNodes) {

			getRwLock().writeLock().lock();
			this.onTheSchedulerListForPhysicalNodes = onTheSchedulerListForPhysicalNodes;
			getRwLock().writeLock().unlock();

	}

	/** Solely used by the PhysicalNodeMovementListener to update substance concentrations.**/
	public int getMovementConcentratioUpdateProcedure() {
		try
		{
			getRwLock().readLock().lock();
			return movementConcentratioUpdateProcedure;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	/** Solely used by the PhysicalNodeMovementListener to update substance concentrations.**/
	@Override
	public void setMovementConcentratioUpdateProcedure(
			int movementConcentratioUpdateProcedure) {
		try
		{
			getRwLock().readLock().lock();
			this.movementConcentratioUpdateProcedure = movementConcentratioUpdateProcedure;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/** Add an extracellular or membrane-bound chemicals
	 *  in this PhysicalNode. */
	@Override
	public void addExtracellularSubstance(ini.cx3d.physics.interfaces.Substance is) {
		try
		{
			getRwLock().readLock().lock();
			extracellularSubstances.put(is.getId(), is);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/** Remove an extracellular or membrane-bound chemicals that are present
	 *  in this PhysicalNode. */
	@Override
	public void removeExtracellularSubstance(ini.cx3d.physics.interfaces.Substance is) {
		try
		{
			getRwLock().readLock().lock();
			extracellularSubstances.remove(is);
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	/** All the (diffusible) chemicals that are present in the space defined by this physicalNode. */
//	@Override
//	public Hashtable<String, ini.cx3d.physics.interfaces.Substance> getExtracellularSubstances() {
//		try
//		{
//			getRwLock().readLock().lock();
//			return (Hashtable<String, ini.cx3d.physics.interfaces.Substance>) extracellularSubstances.clone();
//		}
//		finally
//		{
//			getRwLock().readLock().unlock();
//		}
//	}
	@Override
	public AbstractSequentialList<ini.cx3d.physics.interfaces.Substance> getExtracellularSubstances() {
		AbstractSequentialList<ini.cx3d.physics.interfaces.Substance> list = new LinkedList<>();
		for(Substance s : extracellularSubstances.values()) {
			list.add(s);
		}
		return sort(list);
	}

	@Override
	public ini.cx3d.physics.interfaces.Substance getExtracellularSubstance(String key) {
		return extracellularSubstances.get(key);
	}

	/** All the (diffusible) chemicals that are present in the space defined by this physicalNode. */
	public void setExtracellularSubstances(
			Hashtable<String, ini.cx3d.physics.interfaces.Substance> extracellularSubstances) {
		getRwLock().writeLock().lock();
		this.extracellularSubstances = (Hashtable<String, ini.cx3d.physics.interfaces.Substance>) extracellularSubstances.clone();
		getRwLock().writeLock().unlock();
	}

	/** Returns a unique ID for this PhysicalNode.*/
	@Override
	public int getID() {
		return ID;
	}


	public ReadWriteLock getRwLock() {
		return rwLock;
	}

	private AbstractSequentialList<Substance> sort(AbstractSequentialList<Substance> list) {
		Collections.sort(list, (substance, t1) -> substance.toString().compareTo(t1.toString()));
		return list;
	}
}
