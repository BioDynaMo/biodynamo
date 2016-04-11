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

import static ini.cx3d.SimStateSerializationUtil.*;
import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.crossProduct;
import static ini.cx3d.utilities.Matrix.dot;
import static ini.cx3d.utilities.Matrix.norm;
import static ini.cx3d.utilities.Matrix.normalize;
import static ini.cx3d.utilities.Matrix.printlnLine;
import static ini.cx3d.utilities.Matrix.randomNoise;
import static ini.cx3d.utilities.Matrix.rotAroundAxis;
import static ini.cx3d.utilities.Matrix.scalarMult;
import static ini.cx3d.utilities.Matrix.subtract;
import static ini.cx3d.utilities.StringUtilities.toStr;

import ini.cx3d.Param;
import ini.cx3d.cells.Cell;
import ini.cx3d.cells.CellFactory;
import ini.cx3d.localBiology.interfaces.CellElement;
import ini.cx3d.physics.factory.PhysicalCylinderFactory;
import ini.cx3d.physics.factory.PhysicalObjectFactory;
import ini.cx3d.physics.factory.PhysicalSphereFactory;
import ini.cx3d.simulations.ECM;
import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
import ini.cx3d.spatialOrganization.interfaces.SpaceNode;
import ini.cx3d.swig.physics.physics;
import ini.cx3d.physics.interfaces.PhysicalCylinder;

import java.util.*;

import ini.cx3d.physics.interfaces.PhysicalObject;


/**
 * The embodiment of SomaElement. Contains
 *
 * The spherical coordinates (r, phi, theta) are defined as:
 * r >= 0 is the distance from the origin to a given point P.
 * 0 <= phi <= pi is the angle between the positive z-axis and the line formed between the origin and P.
 * 0 <= theta < 2pi is the angle between the positive x-axis and the line from the origin
 * to the P projected onto the xy-plane.
 *
 * @author fredericzubler
 *
 */

public class PhysicalSphere extends physics.PhysicalSphereBase implements ini.cx3d.physics.interfaces.PhysicalSphere {//ini.cx3d.swig.physics.PhysicalSphere{//ini.cx3d.physics.PhysicalObject2 {


	/* Local biology object associated with this PhysicalSphere.*/
	private ini.cx3d.localBiology.interfaces.SomaElement somaElement = null;

	/* The PhysicalCylinders attached to this sphere*/
	AbstractSequentialList<ini.cx3d.physics.interfaces.PhysicalCylinder> daughters = new LinkedList<>();
	/* Position in local coordinates (PhysicalObject's xAxis,yAxis,zAxis) of
	 * the attachment point of my daughters.*/
	Hashtable<ini.cx3d.physics.interfaces.PhysicalCylinder, double[]> daughtersCoord = new Hashtable<ini.cx3d.physics.interfaces.PhysicalCylinder, double[]>();

	/* Plays the same role than mass and adherence, for rotation around center of mass. */
	private double rotationalInertia = 0.5; // 5.0

	private double interObjectForceCoefficient = 0.15;

	/* Force applied by the biology. Is taken into account during runPhysics(), and the set to 0.*/
	protected double[] tractorForce = {0,0,0};

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		superSuperSimStateToJson(sb);

		// somaElement is circular reference;
		unorderedCollection(sb, "daughters", daughters);
		mapOfDoubleArray(sb, "daughtersCoord", daughtersCoord);
		keyValue(sb, "rotationInertia", rotationalInertia);
		keyValue(sb, "interObjectForceCoefficient", interObjectForceCoefficient);
		keyValue(sb, "tractorForce", tractorForce);


		removeLastChar(sb);
		sb.append("}");
		return sb;
	}

	@Override
	public String toString() {
		StringBuilder sb = new StringBuilder();
		sb.append("(");
		sb.append(toStr(getMass()));
		sb.append(", ");
		sb.append(toStr(getVolume()));
		sb.append(", ");
		sb.append(toStr(getAdherence()));
		sb.append(", ");
		sb.append(toStr(getTotalForceLastTimeStep()));
		sb.append(", ");
		sb.append(toStr(getPhysicalBonds()));
//		sb.append(", ");
//		sb.append(toStr(getExcrescences()));
		sb.append(", ");
		sb.append(toStr(getXAxis()));
		sb.append(", ");
		sb.append(toStr(getYAxis()));
		sb.append(", ");
		sb.append(toStr(getZAxis()));
		sb.append(", ");
		sb.append(toStr(getDiameter()));
		sb.append(", ");
		sb.append(toStr(getMassLocation()));
		sb.append(", ");
		sb.append(toStr(daughters));
		sb.append(", ");
		sb.append(toStr(daughtersCoord));
		sb.append(", ");
		sb.append(toStr(rotationalInertia));
		sb.append(", ");
		sb.append(toStr(interObjectForceCoefficient));
		sb.append(", ");
		sb.append(toStr(tractorForce));
		sb.append(", ");
		sb.append(somaElement);
		sb.append(")");
		return sb.toString();
	}

	public PhysicalSphere() {
		super();
		//getRwLock().writeLock().lock();
		ini.cx3d.swig.physics.PhysicalNode.registerJavaObject(this);
		ini.cx3d.swig.physics.PhysicalObject.registerJavaObject(this);
		ini.cx3d.swig.physics.PhysicalSphere.registerJavaObject(this);
		setMass(1);
		setAdherence(Param.SPHERE_DEFAULT_ADHERENCE);
		setDiameter(Param.SPHERE_DEFAULT_DIAMETER, false);
		updateVolume();
		//getRwLock().writeLock().unlock();
	}


	@Override
	public double getInterObjectForceCoefficient() {

		try{
			//getRwLock().readLock().lock();
			return interObjectForceCoefficient;
		}
		finally
		{
			//getRwLock().readLock().unlock();
		}
	}

	@Override
	public void setInterObjectForceCoefficient(double interObjectForceCoefficient) {


		if(interObjectForceCoefficient<0.0){
			interObjectForceCoefficient = 0.0;
		}else if(interObjectForceCoefficient>1.0){
			interObjectForceCoefficient = 1.0;
		}
		//getRwLock().writeLock().lock();
		this.interObjectForceCoefficient = interObjectForceCoefficient;
		//getRwLock().writeLock().unlock();

	}


	@Override
	public double getRotationalInertia() {
		//getRwLock().readLock().lock();
		try{
			return rotationalInertia;
		}
		finally
		{
			//getRwLock().readLock().unlock();
		}
	}

	@Override
	public void setRotationalInertia(double rotationalInertia) {
		//getRwLock().writeLock().lock();
		this.rotationalInertia = rotationalInertia;
		//getRwLock().writeLock().unlock();

	}

	/** returns true because this object is a PhysicalSphere */
	@Override
	public boolean isAPhysicalSphere(){
		return true;
	}


	// *************************************************************************************
	//   RELATIONS WITH MOTHER & DAUGHTERS
	// *************************************************************************************




	@Override
	public void movePointMass(double speed, double[] direction){
			// NOTE :
			// a) division by norm(direction), so we get a pure direction
			// b) multiplication by the mass, because the total force is divide
			//  	by the mass in runPhysics().
			// c) multiplication by speed for obvious reasons...
			// d) the scaling for simulation time step occurs in the runPhysics() method

	//		double factor = speed*mass/norm(direction);
		    double n = norm(direction);
		    if(n==0) return;
			double factor = speed/n;

			//getRwLock().writeLock().lock();
			tractorForce[0] = factor*(direction[0]);
			tractorForce[1] = factor*(direction[1]);
			tractorForce[2] = factor*(direction[2]);
			//getRwLock().writeLock().unlock();
			// if we are told to move (by our SomaElement for instance), we will update
			// our physics.
			if(speed > 1E-10){
				super.setOnTheSchedulerListForPhysicalObjects(true);
			}

		}


	/**
	 *
	 *
	 * @param daughterWhoAsks .
	 *
	 */
	@Override
	public double[] originOf(ini.cx3d.physics.interfaces.PhysicalObject daughterWhoAsks) {
		try
		{
			//getRwLock().readLock().lock();

			double[] xyz = daughtersCoord.get(daughterWhoAsks);

			double radius = getDiameter()*.5;
			xyz = scalarMult(radius, xyz);
			double[] ml = getMassLocation();
			double[] xa = getXAxis();
			double[] ya = getYAxis();
			double[] za = getZAxis();
			double[] origin =  new double[] {	ml[0] + xyz[0]*xa[0] + xyz[1]*ya[0] + xyz[2]*za[0] ,
					ml[1] + xyz[0]*xa[1] + xyz[1]*ya[1] + xyz[2]*za[1] ,
					ml[2] + xyz[0]*xa[2] + xyz[1]*ya[2] + xyz[2]*za[2] };
			return origin;
			}
		finally
		{
			//getRwLock().readLock().unlock();
		}
	}

	/**
	 * A PhysicalSphere has no mother that could call, so this method is not used.
	 */
	@Override
	public double[] forceTransmittedFromDaugtherToMother(ini.cx3d.physics.interfaces.PhysicalObject motherWhoAsks) {
		return null;
	}

	@Override
	public void removeDaugther(ini.cx3d.physics.interfaces.PhysicalObject daughterToRemove) {
		//getRwLock().writeLock().lock();
		daughters.remove(daughterToRemove);
		daughtersCoord.remove(daughterToRemove);
		//getRwLock().writeLock().unlock();
	}

	@Override
	public void updateRelative(ini.cx3d.physics.interfaces.PhysicalObject oldRelative, ini.cx3d.physics.interfaces.PhysicalObject newRelative) {
		//getRwLock().writeLock().lock();
		double[] coordOfTheNeuriteThatChanges = daughtersCoord.get(oldRelative);
		daughters.remove(oldRelative);
		daughters.add((ini.cx3d.physics.interfaces.PhysicalCylinder) newRelative);
		daughtersCoord.put((ini.cx3d.physics.interfaces.PhysicalCylinder) newRelative, coordOfTheNeuriteThatChanges);
		//getRwLock().writeLock().unlock();
	}

	// *************************************************************************************
	//   DISCRETIZATION , SPATIAL NODE, CELL ELEMENT
	// *************************************************************************************

	/* Move the SpatialOrganizationNode in the center of this PhysicalSphere. If it is
	 * only a small distance off (half of the radius), there is no movement.
	 */
	private void updateSpatialOrganizationNodePosition() {
		//getRwLock().writeLock().lock();
		double[] currentCenterPosition = getSoNode().getPosition();
		double[] ml = getMassLocation();
		double displacementOfTheCenter[] = new double[] {	ml[0] - currentCenterPosition[0],
				ml[1] - currentCenterPosition[1],
				ml[2] - currentCenterPosition[2]  };
		double offset = norm(displacementOfTheCenter);
		if(offset>getDiameter()*0.25 || offset > 5){

			// TODO : do we need this ?
			displacementOfTheCenter = add(displacementOfTheCenter,randomNoise(getDiameter()*0.025, 3));
			try {
				getSoNode().moveFrom(displacementOfTheCenter);
			} catch (PositionNotAllowedException e) {
				e.printStackTrace();
			}
		}
		//getRwLock().writeLock().unlock();
	}



	// *************************************************************************************
	//   BIOLOGY (growth, division, new neurite... )
	// *************************************************************************************


	/**
	 * @return the somaElement
	 */
	@Override
	public ini.cx3d.localBiology.interfaces.SomaElement getSomaElement() {

		try{
			//getRwLock().readLock().lock();
			return somaElement;
		}
		finally
		{
			//getRwLock().readLock().unlock();
		}
	}

	/**
	 * @param somaElement the somaElement to set
	 */
	@Override
	public void setSomaElement(ini.cx3d.localBiology.interfaces.SomaElement somaElement) {

		if (somaElement != null) {
			//getRwLock().writeLock().lock();
			this.somaElement = somaElement;
			//getRwLock().writeLock().unlock();
		} else {
			System.out.println("ERROR  PhysicalSphere: somaElement already exists");
		};
	}


	/**
	 * Progressive modification of the volume. Updates the diameter, the intracellular concentration
	 * @param speed cubic micron/ h
	 */
	@Override
	public void changeVolume(double speed) {
		//getRwLock().writeLock().lock();
		//scaling for integration step
		double dV = speed*(Param.SIMULATION_TIME_STEP);
		setVolumeOnly(getVolume() + dV);
		if(getVolume() < 5.2359877E-7 ){	// minimum volume, corresponds to minimal diameter
			System.err.println("PhysicalSphere.changeVolume() : volume is " + getVolume());
			setVolumeOnly(5.2359877E-7);
		}
		//getRwLock().writeLock().unlock();

		updateDiameter();
		updateIntracellularConcentrations();
		scheduleMeAndAllMyFriends();

	}

	/**
	 * Progressive modification of the diameter. Updates the volume, the intracellular concentration
	 * @param speed micron/ h
	 */
	@Override
	public void changeDiameter(double speed) {
		//scaling for integration step
		//getRwLock().writeLock().lock();
		double dD = speed*(Param.SIMULATION_TIME_STEP);
		setDiameter(getDiameter() + dD, false);
		if(getDiameter() < 0.01 ){
			System.err.println("PhysicalSphere.changeDiameter() : diameter is " + getDiameter());
			setDiameter(0.01, false); // minimum diameter
		}
		//getRwLock().writeLock().unlock();

		updateVolume();
		// no call to updateIntracellularConcentrations() cause it's done by updateVolume().
		scheduleMeAndAllMyFriends();

	}


	@Override
	public void updateDependentPhysicalVariables() {
		updateVolume();
	}


	/**
	 * Updates the concentration of substances, based on the volume of the object.
	 * Is usually called after change of the volume (and therefore we don't modify it here)
	 */
	@Override
	public void updateIntracellularConcentrations(){

		//getRwLock().readLock().lock();
		for (ini.cx3d.physics.interfaces.IntracellularSubstance s : getIntracellularSubstances().values() ) {
			if(s.isVolumeDependant()){
				s.updateConcentrationBasedOnQuantity(getVolume());
			}else{
				s.updateConcentrationBasedOnQuantity(getDiameter());
//				s.updateConcentrationBasedOnQuantity(1.0);
			}
		}
		//getRwLock().readLock().unlock();
	}

	/**
	 * Recompute volume after diameter has changed.
	 */
	@Override
	public void updateVolume(){
		//getRwLock().writeLock().lock();
		double d = getDiameter();
		setVolumeOnly(0.523598776 * d * d * d); 	// 0,523598776 = (4/3)*pi*(1/(2^3))
		//getRwLock().writeLock().unlock();
		updateIntracellularConcentrations();
	}

	/* Recompute diameter, after volume has been changed and recompute then concentration of
	 * IntracellularSubstances.*/
	@Override
	public void updateDiameter(){
		// V = (4/3)*pi*r^3 = (pi/6)*diameter^3
		//getRwLock().writeLock().lock();
		setDiameter(Math.cbrt(getVolume() * 1.90985932), false);   		// 1.90985932 = 6/pi
		//getRwLock().writeLock().unlock();
	}


	/**
	 * Extension of a PhysicalCylinder as a daughter of this PhysicalSphere. The position on the sphere where
	 * the cylinder will be attached is specified in spherical coordinates with respect to the
	 * cx3d.cells Axis with two angles. The cylinder that is produced is specified by the object (usually
	 * a SomaElement) that calls this method. Indeed, unlike PhysicalCylinder.insertProximalCylinder() for instance,
	 * this method is called for biological reasons, and not for discretization purposes.
	 *
	 * @param phi the angle from the zAxis
	 * @param theta the angle from the xAxis around the zAxis
	 */
	@Override
	public PhysicalCylinder addNewPhysicalCylinder(double newLength, double phi, double theta){
		double radius = 0.5*getDiameter();
		// position in cx3d.cells coord
		double xCoord = Math.cos(theta)*Math.sin(phi);
		double yCoord = Math.sin(theta)*Math.sin(phi);
		double zCoord = Math.cos(phi);
		double[] xa = getXAxis();
		double[] ya = getYAxis();
		double[] za = getZAxis();
		double[] axisDirection = {
				xCoord*xa[0] + yCoord*ya[0] + zCoord*za[0] ,
				xCoord*xa[1] + yCoord*ya[1] + zCoord*za[1] ,
				xCoord*xa[2] + yCoord*ya[2] + zCoord*za[2] };

		// positions & axis in cartesian coord
		double[] newCylinderBeginingLocation = add(getMassLocation(), scalarMult(radius,axisDirection));
		double[] newCylinderSpringAxis = scalarMult(newLength,axisDirection);

		double[] newCylinderMassLocation = add(newCylinderBeginingLocation, newCylinderSpringAxis);
		double[] newCylinderCentralNodeLocation = add(newCylinderBeginingLocation, scalarMult(0.5,newCylinderSpringAxis));
		// new PhysicalCylinder
		ini.cx3d.physics.interfaces.PhysicalCylinder cyl = PhysicalCylinderFactory.create();
		cyl.setSpringAxis(newCylinderSpringAxis);
		cyl.setMassLocation(newCylinderMassLocation);
		cyl.setActualLength(newLength);
		cyl.setRestingLengthForDesiredTension(Param.NEURITE_DEFAULT_TENSION);
		cyl.updateLocalCoordinateAxis();
		cyl.setDiameter(Param.NEURITE_DEFAULT_DIAMETER, true);
		// Color
		cyl.setColor(getColor());
		// familly relations

		//getRwLock().writeLock().lock();
		daughters.add(cyl);
		cyl.setMother(this);
		daughtersCoord.put(cyl, new double[] {xCoord, yCoord, zCoord});

		// SpaceNode
		SpatialOrganizationNode<ini.cx3d.physics.interfaces.PhysicalNode> newSON = null;
		try {
			newSON = getSoNode().getNewInstance(newCylinderCentralNodeLocation, cyl);
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
		}
		cyl.setSoNode((SpaceNode) newSON);
		//ECM.getInstance()
		ECM.getInstance().addPhysicalCylinder(cyl);
		//getRwLock().writeLock().unlock(); //wrong spot changeomorrow!! but solved I guess
		return cyl;
	}



	/**
	 * Division of the sphere into two spheres. The one in which the method is called becomes
	 * one the 1st daughter sphere (it keeps its Soma); a new PhysicalSphere is instantiated
	 * and becomes the 2nd daughter (and the Soma given as argument is attributed to it
	 * as CellElement). One can specify the relative size of the daughters (2nd/1st).
	 * In asymmetrical division the cx3d.cells that divides stays the progenitor, so the ratio is
	 * smaller than 1.
	 * @param vr ratio of the two volumes (vr = v2/v1)
	 * @param phi the angle from the zAxis (for the division axis)
	 * @param theta the angle from the xAxis around the zAxis (for the division axis)
	 * @return the other daughter (new sphere)
	 */
	@Override
	public ini.cx3d.physics.interfaces.PhysicalSphere divide(double vr, double phi, double theta){
		// A) Defining some values ..................................................................
		double oldVolume = getVolume();
		// defining the two radii s.t total volume is conserved ( R^3 = r1^3 + r2^3 ; vr = r2^3 / r1^3 )
		double radius = getDiameter()*0.5;
		double r1 = radius / Math.pow(1.0+vr, 1.0/3.0);
		double r2 = radius / Math.pow(1.0+1/vr, 1.0/3.0);
		// define an axis for division (along which the nuclei will move) in cx3d.cells Coord
		double xCoord = Math.cos(theta)*Math.sin(phi);
		double yCoord = Math.sin(theta)*Math.sin(phi);
		double zCoord = Math.cos(phi);
		double TOTAL_LENGTH_OF_DISPLACEMENT = radius/4.0;
//		TOTAL_LENGTH_OF_DISPLACEMENT = 5;
		double[] xa = getXAxis();
		double[] ya = getYAxis();
		double[] za = getZAxis();
		double[] axisOfDivision = {
				TOTAL_LENGTH_OF_DISPLACEMENT*(xCoord*xa[0] + yCoord*ya[0] + zCoord*za[0]) ,
				TOTAL_LENGTH_OF_DISPLACEMENT*(xCoord*xa[1] + yCoord*ya[1] + zCoord*za[1]) ,
				TOTAL_LENGTH_OF_DISPLACEMENT*(xCoord*xa[2] + yCoord*ya[2] + zCoord*za[2]) };
		// two equations for the center displacement :
		//  1) d2/d1= v2/v1 = vr (each sphere is shifted inver. proportionally to its volume)
		// 	2) d1 + d2 = TOTAL_LENGTH_OF_DISPLACEMENT
		double d2 = (TOTAL_LENGTH_OF_DISPLACEMENT/(vr+1));
		double d1 = TOTAL_LENGTH_OF_DISPLACEMENT-d2;

		// B) Instantiating a new sphere = 2nd daughter................................................
		// getting a new sphere
		PhysicalSphere newSphere = (PhysicalSphere) PhysicalSphereFactory.create();

		// super class variables (except masLocation, filled below)
		newSphere.setXAxis(getXAxis());
		newSphere.setYAxis(getYAxis());
		newSphere.setZAxis(getZAxis());
		newSphere.setColor(getColor());
		newSphere.setAdherence(getAdherence());
		newSphere.setMass(getMass());
		newSphere.setStillExisting(this.isStillExisting());

		// this class variables (except radius/diameter)
		newSphere.rotationalInertia = rotationalInertia;
		newSphere.setAdherence(getAdherence());
		newSphere.setInterObjectForceCoefficient(this.interObjectForceCoefficient);
		newSphere.setDiameter(r2 * 2, false);
		double d = newSphere.getDiameter();
		newSphere.setVolumeOnly(0.523598776 * d*d*d); 	// 0,523598776 = (4/3)*pi*(1/(2^3))

		// Mass Location
		double[] ml = getMassLocation();
		double[] newSphereMassLocation = new double[] {
				ml[0] + d2*axisOfDivision[0],
				ml[1] + d2*axisOfDivision[1],
				ml[2] + d2*axisOfDivision[2]  };
		newSphere.setMassLocation(newSphereMassLocation);

		// C) Request a SpaceNode
		double[] newSphereSpaceNodeLocation = newSphereMassLocation.clone();
		SpatialOrganizationNode<ini.cx3d.physics.interfaces.PhysicalNode> newSON = null;
		try {
			newSON = getSoNode().getNewInstance(newSphereSpaceNodeLocation, newSphere);
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
		}
		newSphere.setSoNode((SpaceNode) newSON);

		// D) register new Sphere to ECM
		//getRwLock().writeLock().lock();
		ECM.getInstance().addPhysicalSphere(newSphere);  // this method also adds the PhysicalNode

		// E) This sphere becomes the 1st daughter.....................................................
		// move this cx3d.cells on opposite direction (move the centralNode & the massLocation)
		try {
			getSoNode().moveFrom(new double[]{-d1 * axisOfDivision[0],
					-d1 * axisOfDivision[1],
					-d1 * axisOfDivision[2]}) ;
		} catch (PositionNotAllowedException e) {
			e.printStackTrace();
		}
		ml = getMassLocation();
		ml[0] -= d1*axisOfDivision[0];
		ml[1] -= d1*axisOfDivision[1];
		ml[2] -= d1*axisOfDivision[2];
		setMassLocation(ml);

		// F) change properties of this cell
		setDiameter(r1*2, false);
		d = getDiameter();
		setVolumeOnly(.523598776 * d*d*d); 	// 0,523598776 = (4/3)*pi*(1/(2^3))


		// G) Copy the intracellular and membrane bound Substances
		for (ini.cx3d.physics.interfaces.IntracellularSubstance sub : getIntracellularSubstances().values()) {
				ini.cx3d.physics.interfaces.IntracellularSubstance subCopy = (ini.cx3d.physics.interfaces.IntracellularSubstance)sub.getCopy(); 	//copy substance
				sub.distributeConcentrationOnDivision(subCopy);
				sub.updateQuantityBasedOnConcentration(getVolume());
				subCopy.updateQuantityBasedOnConcentration(newSphere.getVolume());
				newSphere.addIntracellularSubstance(subCopy);

		}

		// G) Copy the intracellular and membrane bound Substances (old Fred's)
//		for (IntracellularSubstance sub : intracellularSubstances.values()) {
//				IntracellularSubstance subCopy = new IntracellularSubstance(sub); 	//copy substance
//				newSphere.intracellularSubstances.put(subCopy.getId(), subCopy);
//				subCopy.setConcentration(sub.getConcentration());					// concentration doesn't change
//				double oldQuantity = sub.getQuantity();								// quantity changes proportional to the volume
//				sub.setQuantity(oldQuantity*(volume/oldVolume));
//				subCopy.setQuantity(oldQuantity*(newSphere.getVolume()/oldVolume));
//		}



		//getRwLock().writeLock().unlock();
		return newSphere;
	}


	// *************************************************************************************
	//   PHYSICS
	// *************************************************************************************

	/**
	 * Tells if a sphere is in the detection range of an other sphere.
	 */
	@Override
	public boolean isInContactWithSphere(ini.cx3d.physics.interfaces.PhysicalSphere s){
		try{
			//getRwLock().readLock().lock();
			double[] force = PhysicalObjectFactory.getInterObjectForce().forceOnASphereFromASphere(this, s);
			if(norm(force)>1E-15){
				return true;
			}else{
				return false;
			}
		}
		finally
		{
			//getRwLock().readLock().unlock();
		}
//		double sumOfTheRadius = 0.5*(super.diameter + s.getDiameter() );
//		// if larger force-field, we increase the detection range
//		if(this.forceFieldType == Param.TISSUE_CELL_FORCE_FIELD && s.getForceFieldType() == Param.TISSUE_CELL_FORCE_FIELD){
////			sumOfTheRadius += 3.5;
//		}
//		double additionalRadius = 10.0*Math.min(s.getInterObjectForceCoefficient(), this.getInterObjectForceCoefficient());
//		sumOfTheRadius += 2*additionalRadius; // 2 times: one for each sphere
//		sumOfTheRadius *= 1; // some extra range
//
//		double distanceBetweenSphereCenters = distance(massLocation, s.getMassLocation());
//		if(distanceBetweenSphereCenters -sumOfTheRadius < Param.SPHERE_SPHERE_DETECTION_RANGE ){
//			return true;
//		}
//		return false;
	}

	@Override
	public boolean isInContactWithCylinder(PhysicalCylinder c) {

		//getRwLock().readLock().lock();

		double[] force = PhysicalObjectFactory.getInterObjectForce().forceOnACylinderFromASphere(c,this);
		//getRwLock().readLock().unlock();
		if(norm(force)>1E-15){
			return true;
		}else{
			return false;
		}


//		// detailed comments on the method used to find the closest point on a line from a point
//		// in Force.forceOnACylinderFromASphere().
//		double[] pP = c.proximalEnd();
//		double[] pD = c.getMassLocation();
//		double[] axis = c.getSpringAxis();
//		double actualLength = c.getActualLength();
//		double[] pPc = subtract(massLocation,pP);
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
//		double penetration = 0.5*( c.getDiameter() + this.diameter ) -distance(massLocation,cc) ;
//		if(penetration > -Param.SPHERE_CYLINDER_DETECTION_RANGE) {
//			return true;
//		}
//		return false;
	}


	@Override
	public double[] getForceOn(PhysicalCylinder c) {
		// get force on a Cylinder from a sphere
//		double[] f = Force.forceOnACylinderFromASphere(
//				c.proximalEnd(),
//				c.getMassLocation(),
//				c.getSpringAxis(),
//				c.getActualLength(),
//				c.getDiameter(),
//				massLocation,
//				this.getDiameter()*0.5 );
		try{
			//getRwLock().readLock().lock();
			return PhysicalObjectFactory.getInterObjectForce().forceOnACylinderFromASphere(c, this);
		}
		finally
		{
			//getRwLock().readLock().unlock();
		}
//		return f;
	}


	@Override
	public double[] getForceOn(ini.cx3d.physics.interfaces.PhysicalSphere s) {
//		double[] f;
//		if(this.forceFieldType == Param.TISSUE_CELL_FORCE_FIELD && s.getForceFieldType() == Param.TISSUE_CELL_FORCE_FIELD){
//			f = Force.tissueInteractionForceOnASphereFromASphere(
//					s.getMassLocation(), s.getDiameter()*0.5, massLocation, diameter*0.5);
//		}else{
//			f = Force.forceOnASphereFromASphere(
//					s.getMassLocation(), s.getDiameter()*0.5, massLocation, diameter*0.5);
//		}
//		return f;
		try{
			//getRwLock().readLock().lock();
			return PhysicalObjectFactory.getInterObjectForce().forceOnASphereFromASphere(s, this);
		}
		finally
		{
			//getRwLock().readLock().unlock();
		}
	}




	@Override
	public void runPhysics() {
		// Basically, the idea is to make the sum of all the forces acting
		// on the Point mass. It is stored in translationForceOnPointMass.
		// There is also a computation of the torque (only applied
		// by the daughter neurites), stored in rotationForce.

		// TODO : There might be a problem, in the sense that the biology is not applied
		// if the total Force is smaller than adherence. Once, I should look at this more carefully.

		// If we detect enough forces to make us  move, we will re-schedule us

		super.setOnTheSchedulerListForPhysicalObjects(false);

		//getRwLock().readLock().lock();
		double [] xAxis = this.getXAxis();
		double [] yAxis = this.getYAxis();
		double [] zAxis = this.getZAxis();
		double [] tractorForce = this.tractorForce.clone();
		//getRwLock().readLock().unlock();


		// the 3 types of movement that can occur
		boolean biologicalTranslation = false;
		boolean physicalTranslation = false;
		boolean physicalRotation = false;

		double h = Param.SIMULATION_TIME_STEP;
		double[] movementAtNextStep = new double[3];



		// BIOLOGY :
		// 0) Start with tractor force : What the biology defined as active movement------------
		movementAtNextStep[0]+= h*tractorForce[0]  ;
		movementAtNextStep[1]+= h*tractorForce[1];
		movementAtNextStep[2]+= h*tractorForce[2];


		// PHYSICS
		// the physics force to move the point mass
		double[] translationForceOnPointMass = {0,0,0};
		// the physics force to rotate the cell
		double[] rotationForce = {0,0,0};



		// 1) "artificial force" to maintain the sphere in the ECM.getInstance() simulation boundaries--------

		if(ECM.getInstance().getArtificialWallForSpheres()){
			double[] forceFromArtificialWall = ECM.getInstance().forceFromArtificialWall(getMassLocation(), getDiameter() * 0.5);
			translationForceOnPointMass[0] += forceFromArtificialWall[0];
			translationForceOnPointMass[1] += forceFromArtificialWall[1];
			translationForceOnPointMass[2] += forceFromArtificialWall[2];
		}




		// 2) Spring force from my neurites (translation and rotation)--------------------------
		for (int i = 0; i < daughters.size(); i++) {
			ini.cx3d.physics.interfaces.PhysicalCylinder c = daughters.get(i);
			double[] forceFromDaughter = c.forceTransmittedFromDaugtherToMother(this);
			// for mass translation
			translationForceOnPointMass[0]+= forceFromDaughter[0];
			translationForceOnPointMass[1]+= forceFromDaughter[1];
			translationForceOnPointMass[2]+= forceFromDaughter[2];
			// for rotation
			double[] xyz = daughtersCoord.get(c);
			double[] r = {	xyz[0]*xAxis[0] + xyz[1]*yAxis[0] + xyz[2]*zAxis[0] ,
					xyz[0]*xAxis[1] + xyz[1]*yAxis[1] + xyz[2]*zAxis[1] ,
					xyz[0]*xAxis[2] + xyz[1]*yAxis[2] + xyz[2]*zAxis[2] };
			rotationForce = add(rotationForce, crossProduct(r, forceFromDaughter));
		}
		// 3) Object avoidance force -----------------------------------------------------------
	//**LOCK R
		//getRwLock().readLock().lock();
		//	(We check for every neighbor object if they touch us, i.e. push us away)
		AbstractSequentialList<ini.cx3d.physics.interfaces.PhysicalNode> neighbors = getSoNode().getNeighbors();
		for (int i = 0; i < neighbors.size(); i++) {
			ini.cx3d.physics.interfaces.PhysicalNode neighbor = neighbors.get(i);
			if(neighbor == null) {
				System.out.println("neighbor is null - idx " + i);
				System.out.println("#neighbors: "+ neighbors.size());
				System.out.println("elements: "+ toStr(neighbors));
			}
			// of course, only if it is an instance of PhysicalObject
			if(neighbor.isAPhysicalObject()){
				ini.cx3d.physics.interfaces.PhysicalObject n = (ini.cx3d.physics.interfaces.PhysicalObject)neighbor;
				// if it is a direct relative, we don't take it into account
				if(daughters.contains(n)){  // no physical effect of a member of the family...
					continue;
				}
				// if we have a PhysicalBond with him, we also don't take it into account
				if(getPhysicalBonds() != null && getPhysicalBonds().size() != 0){
					for (ini.cx3d.physics.interfaces.PhysicalBond pb : getPhysicalBonds()) {
						if(Objects.equals(pb.getOppositePhysicalObject(this), neighbor)){
							continue;
						}
					}
				}
				double[] forceFromThisNeighbor = n.getForceOn(this);
				translationForceOnPointMass[0]+= forceFromThisNeighbor[0];
				translationForceOnPointMass[1]+= forceFromThisNeighbor[1];
				translationForceOnPointMass[2]+= forceFromThisNeighbor[2];
			}
		}

		// 4) PhysicalBonds--------------------------------------------------------------------
		for (ini.cx3d.physics.interfaces.PhysicalBond pb : getPhysicalBonds()) {
			double[] forceFromThisPhysicalBond = pb.getForceOn(this);
			// for mass translation only (no rotation)
			translationForceOnPointMass[0]+= forceFromThisPhysicalBond[0];
			translationForceOnPointMass[1]+= forceFromThisPhysicalBond[1];
			translationForceOnPointMass[2]+= forceFromThisPhysicalBond[2];
		}
	//**UNLOCK R
		//getRwLock().readLock().unlock();
		// How the physics influences the next displacement--------------------------------------------------------
	//**LOCK W
		//getRwLock().writeLock().lock();
		double[] tflts = getTotalForceLastTimeStep();
		tflts[0]=translationForceOnPointMass[0]; // for Force display in View
		tflts[1]=translationForceOnPointMass[1];
		tflts[2]=translationForceOnPointMass[2];
		tflts[3]= -1;   // we don't know yet if it will result in a movement
		setTotalForceLastTimeStep(tflts);
		//getRwLock().writeLock().unlock();
	//**UNLOCK W
		double normOfTheForce = Math.sqrt(	translationForceOnPointMass[0]*translationForceOnPointMass[0]+
				translationForceOnPointMass[1]*translationForceOnPointMass[1]+
				translationForceOnPointMass[2]*translationForceOnPointMass[2] );


		// is there enough force to :
		//  - make us biologically move (Tractor) :
		if(norm(tractorForce)>0.01){
			biologicalTranslation = true;
		}
		//  - break adherence and make us translate ?
		if(normOfTheForce>getAdherence()) {
			physicalTranslation = true;
		}
		//  - make us rotate ?
		double r = norm(rotationForce);
		if(r > rotationalInertia){
			physicalRotation = true;
		}

		double mh = h/getMass();
		// adding the physics translation (scale by weight) if important enough
		if(physicalTranslation){

			// We scale the move with mass and time step
			movementAtNextStep[0] += translationForceOnPointMass[0]*mh;
			movementAtNextStep[1] += translationForceOnPointMass[1]*mh;
			movementAtNextStep[2] += translationForceOnPointMass[2]*mh;
		}

		//getRwLock().writeLock().lock();
		// Performing the translation itself :
		if(physicalTranslation || biologicalTranslation){
			tflts = getTotalForceLastTimeStep();
			tflts[3]=1; // it does become a movement
			setTotalForceLastTimeStep(tflts);

			// but we want to avoid huge jumps in the simulation, so there are maximum distances possible
			if(normOfTheForce*mh > Param.SIMULATION_MAXIMAL_DISPLACEMENT){
				movementAtNextStep = scalarMult(Param.SIMULATION_MAXIMAL_DISPLACEMENT, normalize(movementAtNextStep));
			}

			// The translational movement itself
			double[] ml = getMassLocation();
			ml[0] += movementAtNextStep[0];
			ml[1] += movementAtNextStep[1];
			ml[2] += movementAtNextStep[2];
			setMassLocation(ml);

		}
		// Performing the rotation
		if(physicalRotation){
			double rotationAngle = 3.14*Param.SIMULATION_TIME_STEP;
			setXAxis(rotAroundAxis(xAxis, rotationAngle, rotationForce));
			setYAxis(rotAroundAxis(yAxis, rotationAngle, rotationForce));
			setZAxis(rotAroundAxis(zAxis, rotationAngle, rotationForce));
		}
		//getRwLock().writeLock().unlock();
		// updating some values :
		if(biologicalTranslation || physicalTranslation || physicalRotation){
			// re-centering my SpatialOrganizationNode
			updateSpatialOrganizationNodePosition();
			// if I have daughters, I update their length and tension
			for (int i = 0; i < daughters.size(); i++) {
				daughters.get(i).updateDependentPhysicalVariables();
			}

			// Re-schedule me and every one that has something to do with me :
			super.setOnTheSchedulerListForPhysicalObjects(true);
			// daughters :
			for (int i = 0; i < daughters.size(); i++) {
				daughters.get(i).setOnTheSchedulerListForPhysicalObjects(true);
			}
			// neighbors :
			//getRwLock().readLock().lock();
			for (Object neighbor : getSoNode().getNeighbors()) {
				if( ((ini.cx3d.physics.interfaces.PhysicalNode) neighbor).isAPhysicalObject()){
					((ini.cx3d.physics.interfaces.PhysicalObject)neighbor).setOnTheSchedulerListForPhysicalObjects(true);
				}
			}
			//getRwLock().readLock().unlock();
			// physical objects at the other side of a PhysicalBond:
			//getRwLock().readLock().lock();
			for (ini.cx3d.physics.interfaces.PhysicalBond pb : getPhysicalBonds()) {
				pb.getOppositePhysicalObject(this).setOnTheSchedulerListForPhysicalObjects(true);
			}
			//getRwLock().readLock().unlock();
		}


		// Reset biological movement to 0.
		// (Will need new instruction from SomaElement in order to move again)
		//getRwLock().writeLock().lock();
		this.tractorForce[0] = 0; this.tractorForce[1] = 0; this.tractorForce[2] = 0;
		//getRwLock().writeLock().unlock();
	}

	private void scheduleMeAndAllMyFriends(){

		// Re-schedule me and every one that has something to do with me :
		super.setOnTheSchedulerListForPhysicalObjects(true);
		// daughters :
		for (int i = 0; i < daughters.size(); i++) {
			daughters.get(i).setOnTheSchedulerListForPhysicalObjects(true);
		}
		// neighbors :
		//getRwLock().readLock().lock();
		for (Object neighbor : getSoNode().getNeighbors()) {
			if(((ini.cx3d.physics.interfaces.PhysicalNode) neighbor).isAPhysicalObject()){
				((ini.cx3d.physics.interfaces.PhysicalObject)neighbor).setOnTheSchedulerListForPhysicalObjects(true);
			}
		}
		//getRwLock().readLock().unlock();
		// physical objects at the other side of a PhysicalBond:
		//getRwLock().readLock().lock();
		for (ini.cx3d.physics.interfaces.PhysicalBond pb : getPhysicalBonds()) {
			pb.getOppositePhysicalObject(this).setOnTheSchedulerListForPhysicalObjects(true);
		}
		//getRwLock().readLock().unlock();
	}

	@Override
	public double[] getAxis() {
		try
		{
			//getRwLock().readLock().lock();
			return getZAxis().clone();
		}
		finally
		{
			//getRwLock().readLock().unlock();
		}
	}

	/**
	 * @return the daughters
	 */
	@Override
	public AbstractSequentialList<ini.cx3d.physics.interfaces.PhysicalCylinder> getDaughters() {
		try
		{
			//getRwLock().readLock().lock();
			return daughters;
		}
		finally
		{
			//getRwLock().readLock().unlock();
		}
	}


	@Override
	public void runIntracellularDiffusion() {
		//getRwLock().readLock().lock();

		// 1) Degradation according to the degradation constant for each chemical
		for (ini.cx3d.physics.interfaces.IntracellularSubstance is: getIntracellularSubstances().values())
		{
//			if (is.getId().equals("GeneStart_1")){
				is.degrade();
				if(is.isVolumeDependant()){
					is.updateQuantityBasedOnConcentration(this.getVolume());
				}else{
					is.updateQuantityBasedOnConcentration(this.getLength());
				}
//			}
		}


	// TODO: check if does not change to much in the simulations!!!

//	for (Substance s : intracellularSubstances.values()) {
//	double decay = Math.exp(-s.getDegradationConstant()*Param.SIMULATION_TIME_STEP);
//	s.multiplyQuantityAndConcentrationBy(decay);
//	s.setConcentration(s.getConcentration()-s.getDegradationConstant()*s.getConcentration());
//	s.updateQuantityBasedOnConcentration(this.getVolume());
//	}

	//getRwLock().readLock().unlock();
	//	2) Diffusion in Physical cylinders
	// TODO : scramble daughters so that we don't always go in same order.
//	AbstractSequentialList<PhysicalCylinder> daugters = (AbstractSequentialList<PhysicalCylinder>) daughters.clone();
	for (PhysicalCylinder cyl : daughters) {
		// To be sure that we diffuse all the chemicals,
		// the direction (i.e.who calls diffuseWithThisPhysicalObject() )
		// is chosen randomly.
		PhysicalObject po1 = this;
		PhysicalObject po2 = cyl;
		if(ECM.getRandomDouble()<0.5){
			po1 = cyl;
			po2 = this;
		}
		// now we call the diffusion function in the super class
		po1.diffuseWithThisPhysicalObjects(po2, cyl.getActualLength());
	}

}



	// *************************************************************************************
	//   Coordinates transform
	// *************************************************************************************

	/*
	 * 3 systems of coordinates :
	 *
	 * Global :		cartesian coord, defined by orthogonal axis (1,0,0), (0,1,0) and (0,0,1)
	 * 				with origin at (0,0,0).
	 * Local :		defined by orthogonal axis xAxis, yAxis and zAxis,
	 * 				with origin at center of the sphere (point mass location)
	 * Polar :		spherical coordinates [r,phi,theta] with
	 * 				r >= 0 is the distance between the origin (point mass) O and a given point P,
	 * 				0 <= phi <= pi is the angle between the positive z-axis and the line OP.
	 * 				0 <= theta < 2pi is the angle between the positive x-axis and the projection of OP onto the xy-plane
	 *
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

	public static void main(String[] args) {
		double p = Math.PI;

		double phi = p/4;
		double theta = p*17;

		Cell c = CellFactory.getCellInstance(new double[] {10,-0.14,30});
		ini.cx3d.localBiology.interfaces.SomaElement soma = c.getSomaElement();
		ini.cx3d.physics.interfaces.PhysicalSphere sphere = soma.getPhysicalSphere();
//		NeuriteElement ne = c.getSomaElement().extendNewNeurite(2, phi, theta);
//		PhysicalCylinder pc = ne.getPhysicalCylinder();

		double[] pol = new double[] {3,phi,theta};
//		double[] glo = pc.proximalEnd();
		double[] glo = sphere.transformCoordinatesPolarToGlobal(pol);
		double[] polagain = sphere.transformCoordinatesGlobalToPolar(glo);

//		printlnLine("glo",glo);
//		printlnLine("polagain",polagain);

		double[] gloAgain = sphere.transformCoordinatesPolarToGlobal(polagain);
//		printlnLine("gloAgain",gloAgain);
//		Scheduler.simulate();

		ini.cx3d.physics.interfaces.PhysicalSphere cyl = sphere;
		double[] P = {10,p*(0.1), -p*1.5};

		System.out.println("=== 1) Test P->L L->P  =====================");
		printlnLine("P", P);
		double[] trans = cyl.transformCoordinatesPolarToLocal(P);
		printlnLine("trans", trans);
		double[] back = cyl.transformCoordinatesLocalToPolar(trans);
		printlnLine("back", back);

		System.out.println("=== 2) Test L->P P->L  =====================");
		printlnLine("P", P);
		trans = cyl.transformCoordinatesLocalToPolar(P);
		printlnLine("trans", trans);
		back = cyl.transformCoordinatesPolarToLocal(trans);
		printlnLine("back", back);


		System.out.println("=== 3) Test L->G G->L =====================");
		printlnLine("P", P);
		trans = cyl.transformCoordinatesLocalToGlobal(P);
		printlnLine("trans", trans);
		back = cyl.transformCoordinatesGlobalToLocal(trans);
		printlnLine("back", back);

		System.out.println("=== 4) Test G->L L->G =====================");
		printlnLine("P", P);
		trans = cyl.transformCoordinatesGlobalToLocal(P);
		printlnLine("trans", trans);
		back = cyl.transformCoordinatesLocalToGlobal(trans);
		printlnLine("back", back);

		System.out.println("=== 5) Test G->P P->G =====================");
		printlnLine("P", P);
		trans = cyl.transformCoordinatesGlobalToPolar(P);
		printlnLine("trans", trans);
		back = cyl.transformCoordinatesPolarToGlobal(trans);
		printlnLine("back", back);

		System.out.println("=== 6) Test P->G G->P =====================");
		printlnLine("P", P);
		trans = cyl.transformCoordinatesPolarToGlobal(P);
		printlnLine("trans", trans);
		back = cyl.transformCoordinatesGlobalToPolar(trans);
		printlnLine("back", back);

		System.out.println("===Verif==========================");

		printlnLine("P", P);
		double[] PL = cyl.transformCoordinatesPolarToLocal(P);
		printlnLine("PL", PL);

		double[] LG = cyl.transformCoordinatesLocalToGlobal(PL);
		printlnLine("LG", LG);

		double[] GL = cyl.transformCoordinatesGlobalToLocal(LG);
		printlnLine("GL", GL);

		double[] LP = cyl.transformCoordinatesLocalToPolar(GL);
		printlnLine("LP", LP);














	}
	// G -> L
	/**
	 * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
	 * of a point expressed in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1]).
	 * @param positionInGlobalCoord
	 * @return
	 */
	@Override
	public double[] transformCoordinatesGlobalToLocal(double[] positionInGlobalCoord){
		try
		{
			//getRwLock().readLock().lock();
			positionInGlobalCoord = subtract(positionInGlobalCoord, getMassLocation());
			return new double[] {
					dot(positionInGlobalCoord,getXAxis()),
					dot(positionInGlobalCoord,getYAxis()),
					dot(positionInGlobalCoord,getZAxis())
					};
		}
		finally{
			//getRwLock().readLock().unlock();
		}
	}
	// L -> G
	/**
	 * Returns the position in global cartesian coordinates ([1,0,0],[0,1,0],[0,0,1])
	 * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
	 * @param positionInLocalCoord
	 * @return
	 */
	@Override
	public double[] transformCoordinatesLocalToGlobal(double[] positionInLocalCoord){
		try
		{
			//getRwLock().readLock().lock();
			double [] xa = getXAxis();
			double [] ya = getYAxis();
			double [] za = getZAxis();
			double[] glob = new double[] {
					positionInLocalCoord[0]*xa[0] + positionInLocalCoord[1]*ya[0] + positionInLocalCoord[2]*za[0],
					positionInLocalCoord[0]*xa[1] + positionInLocalCoord[1]*ya[1] + positionInLocalCoord[2]*za[1],
					positionInLocalCoord[0]*xa[2] + positionInLocalCoord[1]*ya[2] + positionInLocalCoord[2]*za[2]
					};
			return add(glob, getMassLocation());
		}
		finally{
			//getRwLock().readLock().unlock();
		}
	}

	// L -> P
	/**
	 * Returns the position in spherical coordinates (r,phi,theta)
	 * of a point expressed in the local coordinate system (xAxis, yXis, zAxis).
	 * @param positionInLocalCoordinates
	 * @return
	 */
	@Override
	public double[] transformCoordinatesLocalToPolar(double[] positionInLocalCoordinates){
		try
		{
			//getRwLock().readLock().lock();
			return new double[] {
					Math.sqrt(positionInLocalCoordinates[0]*positionInLocalCoordinates[0] + positionInLocalCoordinates[1]*positionInLocalCoordinates[1] + positionInLocalCoordinates[2]*positionInLocalCoordinates[2]),
					Math.atan2(Math.sqrt(positionInLocalCoordinates[0]*positionInLocalCoordinates[0] + positionInLocalCoordinates[1]*positionInLocalCoordinates[1]) , positionInLocalCoordinates[2]),
					Math.atan2(positionInLocalCoordinates[1],positionInLocalCoordinates[0])
			};
		}
		finally{
			//getRwLock().readLock().unlock();
		}
	}
	// P -> L
	/**
	 * Returns the position in the local coordinate system (xAxis, yXis, zAxis)
	 * of a point expressed in spherical coordinates (r,phi,theta).
	 * @param positionInPolarCoordinates
	 * @return
	 */
	@Override
	public double[] transformCoordinatesPolarToLocal(double[] positionInPolarCoordinates){
		try
		{
			//getRwLock().readLock().lock();
			return new double[] {
					positionInPolarCoordinates[0]*Math.cos(positionInPolarCoordinates[2])*Math.sin(positionInPolarCoordinates[1]),
					positionInPolarCoordinates[0]*Math.sin(positionInPolarCoordinates[2])*Math.sin(positionInPolarCoordinates[1]),
					positionInPolarCoordinates[0]*Math.cos(positionInPolarCoordinates[1])
			};
		}
		finally{
			//getRwLock().readLock().unlock();
		}

	}

	@Override
	public double[] transformCoordinatesPolarToGlobal(double[] positionInPolarCoordinates) {

		double localX = positionInPolarCoordinates[0]*Math.cos(positionInPolarCoordinates[2])*Math.sin(positionInPolarCoordinates[1]);
		double localY = positionInPolarCoordinates[0]*Math.sin(positionInPolarCoordinates[2])*Math.sin(positionInPolarCoordinates[1]);
		double localZ = positionInPolarCoordinates[0]*Math.cos(positionInPolarCoordinates[1]);
		try
		{
			//getRwLock().readLock().lock();
			double[] xa = getXAxis();
			double[] ya = getYAxis();
			double[] za = getZAxis();
			double[] ml = getMassLocation();
			return new double[] {
					ml[0] + localX*xa[0] + localY*ya[0] + localZ*za[0] ,
					ml[1] + localX*xa[1] + localY*ya[1] + localZ*za[1] ,
					ml[2] + localX*xa[2] + localY*ya[2] + localZ*za[2] };
		}
		finally{
			//getRwLock().readLock().unlock();
		}
	}


	@Override
	public double[] transformCoordinatesGlobalToPolar(double[] positionInGlobalCoordinates) {
		try
		{
			//getRwLock().readLock().lock();
			double[] vectorToPoint = subtract(positionInGlobalCoordinates,getMassLocation());
			double[] localCartesian = {
					dot(getXAxis(),vectorToPoint),
					dot(getYAxis(),vectorToPoint),
					dot(getZAxis(),vectorToPoint) };
			return new double[] {
					Math.sqrt(localCartesian[0]*localCartesian[0] + localCartesian[1]*localCartesian[1] + localCartesian[2]*localCartesian[2]),
					Math.atan2(Math.sqrt(localCartesian[0]*localCartesian[0] + localCartesian[1]*localCartesian[1]) , localCartesian[2]),
					Math.atan2(localCartesian[1],localCartesian[0])
			};
		}
		finally{
			//getRwLock().readLock().unlock();
		}
	}


	@Override
	public double[] getUnitNormalVector(double[] positionInPolarCoordinates) {
		try
		{
			//getRwLock().readLock().lock();
			double[] positionInLocalCoordinates = transformCoordinatesPolarToLocal(positionInPolarCoordinates);
			double[] xa = getXAxis();
			double[] ya = getYAxis();
			double[] za = getZAxis();
			return new double[] {
					positionInLocalCoordinates[0]*xa[0] + positionInLocalCoordinates[1]*ya[0] + positionInLocalCoordinates[2]*za[0] ,
					positionInLocalCoordinates[0]*xa[1] + positionInLocalCoordinates[1]*ya[1] + positionInLocalCoordinates[2]*za[1] ,
					positionInLocalCoordinates[0]*xa[2] + positionInLocalCoordinates[1]*ya[2] + positionInLocalCoordinates[2]*za[2] };
		}
		finally{
			//getRwLock().readLock().unlock();
		}
	}


	@Override
	public CellElement getCellElement() {
		return getSomaElement();
	}


	@Override
	public boolean isRelative(ini.cx3d.physics.interfaces.PhysicalObject po) {
			if(daughters.contains(po))
				return true;
			return false;
	}


	@Override
	public double getLength() {
		try{
			//getRwLock().readLock().lock();
			return getDiameter();
		}
		finally{
			//getRwLock().readLock().unlock();
		}
	}


}
