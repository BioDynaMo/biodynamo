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
import static ini.cx3d.utilities.Matrix.norm;
import static ini.cx3d.utilities.Matrix.printlnLine;
import static ini.cx3d.utilities.Matrix.scalarMult;
import static ini.cx3d.utilities.Matrix.subtract;

import java.util.concurrent.locks.ReadWriteLock;

import ini.cx3d.Param;


/**
 * This class represents an elastic bond between two physical objects.
 * It can be used (1) to represent a cell adhesion mechanism - zip/anchor- and
 * in this case is permanent, or (2) to force two cylinders that crossed
 * each other to come back on the right side, and in this case it vanishes
 * when the right conformation is re-established.
 * 
 * It works as a spring, with
 * a resting length and a spring constant, used to compute a force along the vector
 * joining the two ends, depending on the actual length. (Note that it is considered as a
 * real unique spring and not a linear spring constant as in PhysicalCylinder)
 *  
 * @author fredericzubler
 *
 */

public class PhysicalBond {
	
	private PhysicalObject a;
	private PhysicalObject b;
	private double[] originOnA;
	private double[] originOnB;
	private double restingLength;
	private double springConstant = 10;
	private double maxTension = 50;
	private double dumpingConstant = 0;
	
	private double pastLenght;
	
	/** If true, allows the physical bond to "slide" if b is a chain of PhysicalCylinders
	 * It can be seen as the migration of a along b. */
	private boolean slidingAllowed = false;
	/** If false, there is no force transmitted on the first PhysicalObject (a).*/
	private boolean hasEffectOnA = true;
	/** If false, there is no force transmitted on the second PhysicalObject (b).*/
	private boolean hasEffectOnB = true;
	
	public PhysicalBond(){
	}
	
	/** Creates a PhysicalBond between the point masses of the two PhysicalObjects
	 * given as argument, with resting length their actual distance from oneanother.
	 * @param a
	 * @param b
	 */
	public PhysicalBond(PhysicalObject a, PhysicalObject b){
		dolocking(a, b);
		this.originOnA = a.transformCoordinatesGlobalToPolar(a.getMassLocation());
		this.originOnB = b.transformCoordinatesGlobalToPolar(b.getMassLocation());
		this.restingLength = norm(subtract(a.getMassLocation(), b.getMassLocation()));
		this.springConstant = 10;
		this.dumpingConstant = 0;
		this.slidingAllowed = false;
		this.pastLenght = restingLength;
	}

	private void dolocking(PhysicalObject a, PhysicalObject b) {
		ReadWriteLock rwl1;
		ReadWriteLock rwl2;
		if(a.getID()>b.getID())
		{
			rwl1 = a.getRwLock();
			rwl2 = b.getRwLock();
		}
		else
		{
			rwl1 =  a.getRwLock();
			rwl2 =  b.getRwLock();
		}
		rwl1.writeLock().lock();
		rwl2.writeLock().lock();
		this.a = a;
		this.b = b;
		a.addPhysicalBond(this);
		b.addPhysicalBond(this);
		rwl1.writeLock().unlock();
		rwl2.writeLock().unlock();
	}
	
	public PhysicalBond(PhysicalObject a, double[] positionOnA, PhysicalObject b , double[] positionOnB, double restingLength, double springConstant) {
		dolocking(a, b);
		this.originOnA = positionOnA;
		this.originOnB = positionOnB;
		this.restingLength = restingLength;
		this.springConstant = springConstant;
		this.slidingAllowed = false;
		this.pastLenght = restingLength;
	}
	
	public synchronized PhysicalObject getFirstPhysicalObject() {
		return a;
	}
	
	public synchronized PhysicalObject getSecondPhysicalObject() {
		return b;
	}
	
	public synchronized void setFirstPhysicalObject(PhysicalObject a) {
		this.a = a;
	}
	
	public synchronized void setSecondPhysicalObject(PhysicalObject b) {
		this.b = b;
	}
	
	/** If false, the first PhysicalObject doesn't feel the influence of this PhysicalBond.*/
	public synchronized boolean isHasEffectOnA() {
		return hasEffectOnA;
	}
	/** If false, the first PhysicalObject doesn't feel the influence of this PhysicalBond.*/
	public synchronized void setHasEffectOnA(boolean hasEffectOnA) {
		this.hasEffectOnA = hasEffectOnA;
	}
	/** If false, the second PhysicalObject doesn't feel the influence of this PhysicalBond.*/
	public synchronized  boolean isHasEffectOnB() {
		return hasEffectOnB;
	}
	/** If false, the second PhysicalObject doesn't feel the influence of this PhysicalBond.*/
	public synchronized void setHasEffectOnB(boolean hasEffectOnB) {
		this.hasEffectOnB = hasEffectOnB;
	}
	/** If true, allows the physical bond to "slide" from b to b's mother or daughter left,
	 * if b is a chain of PhysicalCylinders. It can be seen as the migration of a along b.*/
	public synchronized boolean isSlidingAllowed() {
		return slidingAllowed;
	}
	/** 
	 * If true, allows the physical bond to "slide" from b to b's mother or daughter left,
	 * if b is a chain of PhysicalCylinders. It can be seen as the migration of a along b.
	 * @param slidingAllowed
	 */
	public synchronized void setSlidingAllowed(boolean slidingAllowed) {
		this.slidingAllowed = slidingAllowed;
	}
	
	public void exchangePhysicalObject(PhysicalObject oldPo, PhysicalObject newPo){
		ReadWriteLock rwl1;
		ReadWriteLock rwl2;
		if(a.getID()>b.getID())
		{
			rwl1 = a.getRwLock();
			rwl2 = b.getRwLock();
		}
		else
		{
			rwl1 =  a.getRwLock();
			rwl2 =  b.getRwLock();
		}
		
		if(oldPo == a){
			a = newPo;
		}else{
			b = newPo;
		}
		oldPo.removePhysicalBond(this);
		newPo.addPhysicalBond(this);
		
		rwl1.writeLock().unlock();
		rwl2.writeLock().unlock();
	}
	
	public void vanish(){
		ReadWriteLock rwl1;
		ReadWriteLock rwl2;
		if(a.getID()>b.getID())
		{
			rwl1 = a.getRwLock();
			rwl2 = b.getRwLock();
		}
		else
		{
			rwl1 =  a.getRwLock();
			rwl2 =  b.getRwLock();
		}
		rwl1.writeLock().lock();
		rwl2.writeLock().lock();
		a.removePhysicalBond(this);
		b.removePhysicalBond(this);
		rwl1.writeLock().unlock();
		rwl2.writeLock().unlock();
	}
	
	public synchronized PhysicalObject getOppositePhysicalObject(PhysicalObject po) {
		if(po == a){
			return b;
		}else{
			return a;
		}
	}
	
	public synchronized  void setPositionOnObjectInLocalCoord(PhysicalObject po, double[] positionInLocalCoordinates){
		if(po == a){
			originOnA = positionInLocalCoordinates;
		}else{
			originOnB = positionInLocalCoordinates;
		}
	}
	
	public synchronized double[] getPositionOnObjectInLocalCoord(PhysicalObject po){
		if(po == a){
			return originOnA;
		}else{
			return originOnB;
		}
	}
	
	/**
	 * Returns the force that this PhysicalBond is applying to a PhsicalObject.
	 * The function also returns the proportion of the mass that is applied to the 
	 * proximal end (mother's point mass) in case of PhysicalCylinder.
	 * (For PhysicalSpheres, the value p is meaningless).
	 * 
	 * @param po the PhysicalObject to which the force is being applied.
	 * @return [Fx,Fy,Fz,p] an array with the 3 force components and the proportion
	 * applied to the proximal end - in case of a PhysicalCylinder.
	 */
	public double[] getForceOn(PhysicalObject po){
		// 0. Find if this physicalBound has an effect at all on the object
		if( (po==a && hasEffectOnA==false) || (po==b && hasEffectOnB==false) )
			return new double[] {0,0,0};
		// 1. Find the other object
		PhysicalObject otherPo = getOppositePhysicalObject(po);
		// 2. Find the two insertion points of the bond
		double[] pointOnOtherPo = otherPo.transformCoordinatesPolarToGlobal( getPositionOnObjectInLocalCoord(otherPo) );
		double[] pointOnPo = po.transformCoordinatesPolarToGlobal( getPositionOnObjectInLocalCoord(po) );
		// 3. Compute the force
		double[] forceDirection = subtract(pointOnOtherPo, pointOnPo);
		// 3'. If sliding along the other object is possible, 
		// only the component perpendicular to the xAxis of the other object is taken into account
		if(po ==a & slidingAllowed && otherPo instanceof PhysicalCylinder){
			PhysicalCylinder pc = (PhysicalCylinder) otherPo;
			double projNorm = dot(forceDirection,otherPo.getXAxis());
			double[] parallelComponent = scalarMult(projNorm, otherPo.getXAxis());
//			forceDirection = subtract(forceDirection,parallelComponent);
			double[] newPositionOnOtherPo = getPositionOnObjectInLocalCoord(otherPo);
			newPositionOnOtherPo[0] -= projNorm;
			if(newPositionOnOtherPo[0] > pc.getActualLength() +1){
				PhysicalCylinder dL = pc.getDaughterLeft();
				if(dL != null){
					exchangePhysicalObject(pc, dL);
					newPositionOnOtherPo[0] = newPositionOnOtherPo[0]-pc.getActualLength(); 
				}else{
					newPositionOnOtherPo[0] += projNorm;
				}
			}else if(newPositionOnOtherPo[0] < -1){
				PhysicalObject mo = pc.getMother();
				if(mo instanceof PhysicalCylinder){
					PhysicalCylinder m = (PhysicalCylinder)mo;
					exchangePhysicalObject(pc, m);
//					newPositionOnOtherPo[0] = m.actualLength - (pc.actualLength-newPositionOnOtherPo[0]); 
					newPositionOnOtherPo[0] = m.getActualLength() + newPositionOnOtherPo[0]; 
				}else{
					newPositionOnOtherPo[0] += projNorm;
				}
			}
		}
		double actualLength = norm(forceDirection);
		if(actualLength == 0){  // should never be the case, but who knows... then we avoid division by 0
			return new double[] {0,0,0,0};
		}

		double springSpeed = (actualLength-this.pastLenght)/Param.SIMULATION_TIME_STEP;
		this.pastLenght = actualLength;
		
		double tension = springConstant*(actualLength-restingLength) + dumpingConstant*springSpeed;  // (Note: different than in PhysicalCylinder)
		double[] force = scalarMult(tension/actualLength, forceDirection);

		// 4. Return it
		// TODO : cleaner way to to this....
		if (po instanceof PhysicalCylinder) {
			PhysicalCylinder pc = (PhysicalCylinder) po;
//			return new double[] {force[0], force[1], force[2], 1-(getPositionOnObjectInLocalCoord(po)[0]/pc.getActualLength()) };
			double p = 1-(getPositionOnObjectInLocalCoord(po)[0]/pc.getActualLength());
			if(p>0.8)
				p=0.8;
			else if(p<0.2)
				p=0.2;
			// Note : if p=0.5 all the times, there is some displacements that can't be justified
			// and if it is allowed to be 0 or 1, we get kinkings...
			return new double[] {force[0], force[1], force[2], p };
		}
		return new double[] {force[0], force[1], force[2], 0};
	}
	
	/**
	 * Gets the location in absolute cartesian coord of the first insertion point (on a).
	 * (Only for graphical display).Raises a NullPointerException if a == null.
	 * @return x,y,z coord of the insertion point of one end
	 */
	public double[] getFirstEndLocation(){
		return a.transformCoordinatesPolarToGlobal( getPositionOnObjectInLocalCoord(a) );
	}
	
	/**
	 * Gets the location in absolute cartesian coord of the first insertion point (on a).
	 * (Only for graphical display). Raises a NullPointerException if b == null.
	 * @return x,y,z coord of the insertion point of one end
	 */
	public double[] getSecondEndLocation(){
		return b.transformCoordinatesPolarToGlobal( getPositionOnObjectInLocalCoord(b) );
	}
	
	public static void main(String[] args){
		// creation of 2 cylinders of length 5
		PhysicalCylinder pc1 = new PhysicalCylinder();
		pc1.setMassLocation(new double[] {0,0,5} );
		pc1.setSpringAxis(new double[] {0,0,5});
		pc1.setActualLength(5);
		PhysicalCylinder pc2 = new PhysicalCylinder();
		pc2.setMassLocation(new double[] {1,0,5} );
		pc2.setSpringAxis(new double[] {0,0,5});
		pc2.setActualLength(5);
		// creation of a PhysicalBond
		PhysicalBond pb = new PhysicalBond(pc1, new double[] {2,3}, pc2, new double[] {4,3}, 0, 5);
		double[] force = pb.getForceOn(pc1);
		printlnLine("force",force);
	}
	
	public String toString(){
		return "My name is Bond, PhysicalBond ("+hashCode()+")";
	}

	/**
	 * @return the restingLength
	 */
	public synchronized double getRestingLength() {
		return restingLength;
	}

	/**
	 * @param restingLength the restingLength to set
	 */
	public synchronized void setRestingLength(double restingLength) {
		this.restingLength = restingLength;
	}

	/**
	 * @return the springConstant
	 */
	public synchronized double getSpringConstant() {
		return springConstant;
	}

	/**
	 * @param springConstant the springConstant to set
	 */
	public synchronized void setSpringConstant(double springConstant) {
		this.springConstant = springConstant;
	}

	/**
	 * @return the maxTension
	 */
	public synchronized double getMaxTension() {
		return maxTension;
	}

	/**
	 * @param maxTension the maxTension to set
	 */
	public synchronized void setMaxTension(double maxTension) {
		this.maxTension = maxTension;
	}

	/**
	 * @return the dumpingConstant
	 */
	public synchronized double getDumpingConstant() {
		return dumpingConstant;
	}

	/**
	 * @param dumpingConstant the dumpingConstant to set
	 */
	public synchronized void setDumpingConstant(double dumpingConstant) {
		this.dumpingConstant = dumpingConstant;
	}
	
	
}
