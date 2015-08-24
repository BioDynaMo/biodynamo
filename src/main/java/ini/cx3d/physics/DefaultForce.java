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

import java.util.concurrent.locks.ReadWriteLock;


public class DefaultForce implements InterObjectForce{
	
	// =============================================================================================

	/** Interaction (repulsive or attractive) between two spheres, based on their distance and radius. 
	 * Even if the two spheres don't interpenetrate, there might be a force. The interaction is based
	 * on : 
	 * <p>Pattana S., <i> Aspects mecaniques de la division cellulaire : Modelisation numerique </i>, 
	 * http://www.lirmm.fr/doctiss04/art/M07.pdf
	 * <p>
	 * I modified the method by using virtual spheres with larger radii, to allow distant interactions,
	 * and get a desired density.
	 */
	public double[] forceOnASphereFromASphere(PhysicalSphere sphere1, PhysicalSphere sphere2) {
		// defining center and radius of spheres
		ReadWriteLock rwl1;
		ReadWriteLock rwl2;
		if(sphere1.getID()>sphere2.getID())
		{
			rwl1 =  sphere1.getRwLock();
			rwl2 = sphere2.getRwLock();
		}
		else
		{
			rwl1 =  sphere2.getRwLock();
			rwl2 =  sphere1.getRwLock();
		}
		try
		{
			rwl1.readLock().lock();
			rwl2.readLock().lock();
			double[] c1 = sphere1.getMassLocation();
			double r1 = 0.5*sphere1.getDiameter();
			double[] c2 = sphere2.getMassLocation();
			double r2 = 0.5*sphere2.getDiameter();
			// We take virtual bigger radii to have a distant interaction, to get a desired density.
			double additionalRadius = 10.0*Math.min(sphere1.getInterObjectForceCoefficient(), sphere2.getInterObjectForceCoefficient());
			r1 += additionalRadius;
			r2 += additionalRadius;
			// the 3 components of the vector c2 -> c1
			double comp1 = c1[0]-c2[0];   
			double comp2 = c1[1]-c2[1];
			double comp3 = c1[2]-c2[2];
			double distanceBetweenCenters = Math.sqrt(comp1*comp1 +comp2*comp2 +comp3*comp3);
			// the overlap distance (how much one penetrates in the other) 
			double delta = r1 + r2 -distanceBetweenCenters;
			// if no overlap : no force
			if(delta<0)
				return new double[] {0,0,0};
			// to avoid a division by 0 if the centers are (almost) at the same location
			if (distanceBetweenCenters<0.00000001){  
				double[] force2on1 = ini.cx3d.utilities.Matrix.randomNoise(3,3);
				return force2on1;
			}else{
				// the force itself
				double R = (r1*r2)/(r1+r2);
				double gamma = 1;			// attraction coeff
				double k = 2;     			// repulsion coeff
				double F = k*delta - gamma*Math.sqrt(R*delta);
				
				
				double module = F/distanceBetweenCenters;
				double[] force2on1 = {module*comp1, module*comp2, module*comp3};
				return force2on1;
			}
		}
		finally
		{
			rwl1.readLock().unlock();
			rwl2.readLock().unlock();
		}
	}
	
	// is only used by sphere-cylinder and cylinder-cylinder....
	private double[] computeForceOfASphereOnASphere(double[] c1, double r1, double[] c2, double r2){
		// the 3 components of the vector c2 -> c1
		double comp1 = c1[0]-c2[0];   
		double comp2 = c1[1]-c2[1];
		double comp3 = c1[2]-c2[2];
		double distanceBetweenCenters = Math.sqrt(comp1*comp1 +comp2*comp2 +comp3*comp3);
		// the overlap distance (how much one penetrates in the other) 
		double a = r1 + r2 -distanceBetweenCenters;
		// if no overlap : no force
		if(a<0)
			return new double[] {0,0,0};
		// to avoid a division by 0 if the centers are (almost) at the same location
		if (distanceBetweenCenters<0.00000001){  
			double[] force2on1 = ini.cx3d.utilities.Matrix.randomNoise(3,3);
			return force2on1;
		}else{
			// the force is prop to the square of the interpentration distance and to the radii.
			double module = a/distanceBetweenCenters;
			double[] force2on1 = {module*comp1, module*comp2, module*comp3};
			return force2on1;
		}
	}

	
	// =============================================================================================
	
	
	public double[] forceOnACylinderFromASphere(PhysicalCylinder cylinder,PhysicalSphere sphere) {
		ReadWriteLock rwl1;
		ReadWriteLock rwl2;
		if(sphere.getID()>cylinder.getID())
		{
			rwl1 =  sphere.getRwLock();
			rwl2 = cylinder.getRwLock();
		}
		else
		{
			rwl1 =  cylinder.getRwLock();
			rwl2 =  sphere.getRwLock();
		}
		try
		{
			rwl1.readLock().lock();
			rwl2.readLock().lock();
			// define some geometrical values
			double[] pP = cylinder.proximalEnd(); 
			double[] pD = cylinder.distalEnd();
			double[] axis = cylinder.getSpringAxis();
			double actualLength = norm(axis);
			double d = cylinder.getDiameter();
			double[] c = sphere.getMassLocation(); 
			double r = 0.5*sphere.getDiameter();
			
			// I. If the cylinder is small with respect to the sphere: 
			// we only consider the interaction between the sphere and the point mass 
			// (i.e. distal point) of the cylinder - that we treat as a sphere.
			if (actualLength < r && true){
				return computeForceOfASphereOnASphere(pD, d*0.5, c, r); 
			}
	
			// II. If the cylinder is of the same scale or bigger than the sphere,
			// we look at the interaction between the sphere and the closest point 
			// (to the sphere center) on the cylinder. This interaction is distributed to
			// the two ends of the cylinder: the distal (point mass of the segment) and
			// the proximal (point mass of the mother of the segment).
	
			// 1) 	Finding cc : the closest point to c on the line pPpD ("line" and not "segment")
			// 		It is the projection of the vector pP->c onto the vector pP->pD (=axis)
			double[] pPc = subtract(c,pP);
	
			// 		projection of pPc onto axis = (pPc.axis)/norm(axis)^2  * axis
			// 		length of the projection = (pPc.axis)/norm(axis)
	
			double pPcDotAxis = pPc[0]*axis[0] + pPc[1]*axis[1] + pPc[2]*axis[2];
			double K = pPcDotAxis/(actualLength*actualLength);
			//		cc = pP + K* axis 
			double[] cc  = new double[] {pP[0]+K*axis[0], pP[1]+K*axis[1], pP[2]+K*axis[2]}; 
	
	
			// 2)	Look if c -and hence cc- is (a) between pP and pD, (b) before pP or (c) after pD
			double proportionTransmitedToProximalEnd;
			if(K<=1.0 && K>=0.0){
				// 		a) 	if cc (the closest point to c on the line pPpD) is between pP and pD
				//			the force is distributed to the two nodes
				proportionTransmitedToProximalEnd = 1.0 - K;
			}else if(K<0){
				// 		b) 	if the closest point to c on the line pPpD is before pP
				//			the force is only on the proximal end (the mother point mass)
				proportionTransmitedToProximalEnd = 1.0;
				cc = pP;
			}else {   	// if(K>1)
				// 		c) if cc is after pD, the force is only on the distal end (the segment's point mass).	 
				proportionTransmitedToProximalEnd = 0.0;
				cc = pD;
			}
	
			// 3) 	If the smallest distance between the cylinder and the center of the sphere
			//		is larger than the radius of the two objects , there is no interaction:
			double penetration = d/2 + r -distance(c,cc);		 
			if(penetration<=0) {
				return new double[] {0.0, 0.0, 0.0};
			}
			double[] force = computeForceOfASphereOnASphere(cc, d*0.5, c, r);
			return new double[] {force[0], force[1], force[2], proportionTransmitedToProximalEnd};
		}
		finally
		{
			rwl1.readLock().unlock();
			rwl2.readLock().unlock();
		}
	}

	// =============================================================================================
	
	public double[] forceOnASphereFromACylinder(PhysicalSphere sphere, PhysicalCylinder cylinder) {
		// it is the opposite of force on a cylinder from sphere:
		double[] temp = forceOnACylinderFromASphere(cylinder, sphere);
		return new double[] {-temp[0], -temp[1], -temp[2]};
	}

	// =============================================================================================
	

	public double[] forceOnACylinderFromACylinder(PhysicalCylinder cylinder1, PhysicalCylinder cylinder2) {
		ReadWriteLock rwl1;
		ReadWriteLock rwl2;
		if(cylinder1.getID()>cylinder2.getID())
		{
			rwl1 =  cylinder1.getRwLock();
			rwl2 = cylinder2.getRwLock();
		}
		else
		{
			rwl1 =  cylinder2.getRwLock();
			rwl2 =  cylinder1.getRwLock();
		}
		try
		{	
			rwl1.readLock().lock();
			rwl2.readLock().lock();
		
			// define some geometrical values
			double[] A = cylinder1.proximalEnd();
			double[] B = cylinder1.getMassLocation();
			double d1 = cylinder1.getDiameter();
			double[] C = cylinder2.proximalEnd();
			double[] D = cylinder2.getMassLocation();
			double d2 = cylinder2.getDiameter();
			
			double K = 0.5; // part devoted to the distal node
	
			//	looking for closest point on them
			// (based on http://local.wasp.uwa.edu.au/~pbourke/geometry/lineline3d/)
			double p13x = A[0]-C[0];
			double p13y = A[1]-C[1];
			double p13z = A[2]-C[2];
			double p43x = D[0]-C[0];
			double p43y = D[1]-C[1];
			double p43z = D[2]-C[2];
			double p21x = B[0]-A[0];
			double p21y = B[1]-A[1];
			double p21z = B[2]-A[2]; 
	
			double d1343 = p13x*p43x + p13y*p43y + p13z*p43z;
			double d4321 = p21x*p43x + p21y*p43y + p21z*p43z;
			double d1321 = p21x*p13x + p21y*p13y + p21z*p13z;
			double d4343 = p43x*p43x + p43y*p43y + p43z*p43z;
			double d2121 = p21x*p21x + p21y*p21y + p21z*p21z;
	
			double[] P1, P2;
	
			double denom = d2121*d4343 - d4321*d4321;
			
			//if the two segments are not ABSOLUTLY parallel
			if(denom > 0.000000000001){
				double numer = d1343*d4321 - d1321*d4343;
	
				double mua = numer/denom;
				double mub = (d1343 + mua*d4321)/d4343;
	
				if(mua<0){
					P1 = A;
					K = 1;
				}else if(mua>1){
					P1 = B;
					K = 0;
				}else{
					P1 = new double[] {A[0]+mua*p21x, A[1]+mua*p21y, A[2]+mua*p21z };
					K = 1-mua;
				}
	
				if(mub<0){
					P2 = C;
				}else if(mub>1){
					P2 = D;
				}else{
					P2 = new double[] {C[0]+mub*p43x, C[1]+mub*p43y, C[2]+mub*p43z };
				}
	
			}else{
				P1 = add(A,scalarMult(0.5, subtract(B,A) ));
				P2 = add(C,scalarMult(0.5, subtract(D,C) ));
			}
			
			// W put a virtual sphere on the two cylinders
			double[] force =  scalarMult(10,computeForceOfASphereOnASphere(P1,d1+0,P2,d2+0));
	
			return new double[] {force[0], force[1], force[2], K};
		}
		finally
		{
			rwl1.readLock().unlock();
			rwl2.readLock().unlock();
		}
	}

	

}
