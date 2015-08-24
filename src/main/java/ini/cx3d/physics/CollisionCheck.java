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
import static ini.cx3d.utilities.Matrix.crossProduct;
import static ini.cx3d.utilities.Matrix.dot;
import static ini.cx3d.utilities.Matrix.norm;
import static ini.cx3d.utilities.Matrix.normalize;
import static ini.cx3d.utilities.Matrix.scalarMult;
import static ini.cx3d.utilities.Matrix.subtract;
import static ini.cx3d.utilities.Matrix.distance;
import static ini.cx3d.utilities.SystemUtilities.*;

import java.util.concurrent.locks.ReadWriteLock;


public class CollisionCheck {
	public double[] maximumMove(	double[] A, double[] B1, double[] B2, double d1,
			double[] C, double[] D, double d2) {
		// define a Triangle


		return D;

	}

	public static void main(String[] args){
		tic();
//		for (int i = 0; i < 1000; i++) {
		double s = howMuchCanWeMove(
				new double[] {0, 0, 0},
				new double[] {0.5, 1, 0},
				new double[] {2, 0, 0},
				new double[] {0.5/2.0, 0.1/2.0-3, 1},
				new double[] {0.5/2.0, 0.1/2.0+3, -1},
				0.1
		);
//		}
		System.out.println(s);
		System.out.println();
		tac();
	}

	/** Checks if a cylinder movement intersects an other cylinder. A PhysicalCylinder A-B has the extremeity B moved
	 * at point C. The method looks if this movement intersects the cylinder D-E. If yes, it returns the proportion 
	 * of the movement - ie the proportion on the segment B-C where you can move. Values are between 0 and 1.
	 * @param A the immobile end of the moving cylinder
	 * @param B the mobile end of the moving cylinder
	 * @param C	the desired end of the moving end
	 * @param D one end of the immobile cylinder
	 * @param E the other end of the cylinder
	 * @param dim = minimal distance betweeen the two central line of the cylinders
	 * @return the proportion of the desider movement of point B that is allowed before collision.
	 */ 
	public static double howMuchCanWeMove(double[] A, double[] B, double[] C, double[] D, double[] E, double dim) {
		double[] vec1, vec2, vec3, vec4, n, vecNormalA, vecNormalB, vecNormalC ;
		vec1=subtract(B,A);
		vec2=subtract(C,A);
		vec3=subtract(C,B);
		n = crossProduct(vec1,vec2);
		// plane through A and perpendicular to vecNormal (= plane through A,B and C):
		// (good summary on planes at http://mathworld.wolfram.com/Plane.html ).
		// Basically, the  plane perpendicular to n = (a,b,c) and containing the point Xo = (xo,yo,zo)
		// contains the points X = (x,y,z) satisfying : n dot (X-Xo) = 0
		// This last equation gives a*x + b*y + c*z = d, with d = a*xo + b*yo + c*zo
		// Here we take A as our point Xo.
		double a,b,c,d;
		a = n[0];
		b = n[1];
		c = n[2];
		d = dot(A,n);   
		// H = intersection point of previous plane and line DE  
		// line = E + lambda(D-E). We look for a lambda s.t for each component
		// the equation of the previous plane is satified
		double lambda = (d-(a*E[0] + b*E[1] + c*E[2])) / ( a*(D[0]-E[0]) + b*(D[1]-E[1]) + c*(D[2]-E[2]) );
		// Is the intersection point between D and E ?
		if(lambda > 1 || lambda < 0){
			return 1.0;
		}
		double[] H = add(E,scalarMult(lambda,subtract(D,E)));    
		// Is H inside the triangle ABC? yes if it satisfies three conditions: 
		double[] side1,side2;
		// i) H and C are on the same side of vec1
		side1 = subtract(H,A);
		side2 = subtract(C,A);
		vecNormalA = normalize(crossProduct(n,vec1));
		if(Math.signum( dot(vecNormalA,side1)*dot(vecNormalA,side2) )<0)
			return 1.0;
		// ii) H and B are on the same side of vec2
		side1 = subtract(H,A);
		side2 = subtract(B,A);
		vecNormalB = crossProduct(n,vec2);
		if(Math.signum( dot(vecNormalB,side1)*dot(vecNormalB,side2) )<0)
			return 1.0;	
		// iii) H and A are on the same side of vec3
		side1 = subtract(H,B);
		side2 = subtract(A,B);
		vecNormalC = crossProduct(n,vec3);
		if(Math.signum( dot(vecNormalC,side1)*dot(vecNormalC,side2) )<0)
			return 1.0;

		// If we arrive here it means that the cylinder D-E is inside the area swaped by the moving cylinder.
		// H is the point where D-E crosses the triangle ABC.
		// Now we look where on the line AB we can go so that the smaller triangle doesn't enters D-E
		// We use the 2 line segments algorithm described in http://mathworld.wolfram.com/Line-LineIntersection.html

		vec4 = subtract(H,A);
		vec1 = new double[] {-vec1[0] , -vec1[1], -vec1[2] };
		double[] temp =  crossProduct(vec3, vec4);

		double s = dot ( crossProduct(vec1, vec4) , temp);
		double sNumerator = temp[0]*temp[0] + temp[1]*temp[1] +  temp[2]*temp[2] ;
		s /= sNumerator;

		// We also want to take into account the diameter of the cylinders:
		// dim is the closest distance the two centerline of the cylinder can come.
		// If H was on the line B-C we would subtract dim/|BC| from s;
		// since H is closer from A, we multiply this ratio by |AB|/|AC|.

		double correctionTerm = dim*norm(vec1)/(norm(vec3)*norm(vec4));
		System.out.println("correctionTerm = "+correctionTerm);	

		s -= correctionTerm;
		if(s<0)
			s=0;

		return s;

	}



	public static void addPhysicalBondIfCrossing(double[] A, double[] B, double[] C, PhysicalCylinder moving, PhysicalCylinder still) {
		//the immobile end of the moving cylinder

		// the distal end of the immobile cylinder
		double[] D = still.getMassLocation();
		// the proximal end of the cylinder
		double[] E = still.proximalEnd();


		double[] vec1, vec2, vec3, n, vecNormalA, vecNormalB, vecNormalC ;
		vec1=subtract(B,A);
		vec2=subtract(C,A);
		vec3=subtract(C,B);
		n = crossProduct(vec1,vec2);
		// plane through A and perpendicular to vecNormal (= plane through A,B and C):
		// (good summary on planes at http://mathworld.wolfram.com/Plane.html ).
		// Basically, the  plane perpendicular to n = (a,b,c) and containing the point Xo = (xo,yo,zo)
		// contains the points X = (x,y,z) satisfying : n dot (X-Xo) = 0
		// This last equation gives a*x + b*y + c*z = d, with d = a*xo + b*yo + c*zo
		// Here we take A as our point Xo.
		double a,b,c,d;
		a = n[0];
		b = n[1];
		c = n[2];
		d = dot(A,n);   
		// H = intersection point of previous plane and line DE  
		// line = E + lambda(D-E). We look for a lambda s.t for each component
		// the equation of the previous plane is satified
		double lambda = (d-(a*E[0] + b*E[1] + c*E[2])) / ( a*(D[0]-E[0]) + b*(D[1]-E[1]) + c*(D[2]-E[2]) );
		// Is the intersection point between D and E ?
		if(lambda > 1 || lambda < 0){
			return;
		}
		double[] H = add(E,scalarMult(lambda,subtract(D,E)));    
		// Is H inside the triangle ABC? yes if it satisfies three conditions: 
		double[] side1,side2;
		// i) H and C are on the same side of vec1
		side1 = subtract(H,A);
		side2 = subtract(C,A);
		vecNormalA = normalize(crossProduct(n,vec1));
		if(Math.signum( dot(vecNormalA,side1)*dot(vecNormalA,side2) )<0)
			return;
		// ii) H and B are on the same side of vec2
		side1 = subtract(H,A);
		side2 = subtract(B,A);
		vecNormalB = crossProduct(n,vec2);
		if(Math.signum( dot(vecNormalB,side1)*dot(vecNormalB,side2) )<0)
			return;	
		// iii) H and A are on the same side of vec3
		side1 = subtract(H,B);
		side2 = subtract(A,B);
		vecNormalC = crossProduct(n,vec3);
		if(Math.signum( dot(vecNormalC,side1)*dot(vecNormalC,side2) )<0)
			return;

		// If we arrive here it means that the cylinder D-E is inside the area swaped by the moving cylinder.
		// and this means either that something illegal happened (and in this case, as correction, we put a PhysicalBond to force 
		// the branches to take their normal configuration later) or that a former forbidden situation has just been corrected
		// (and in this case, we remove the PhysicalBond between theses objects).

		boolean alreadyAPhysicalBond = false;
		ReadWriteLock rwl1;
		ReadWriteLock rwl2;
		if(moving.getID()>still.getID())
		{
			rwl1 = moving.getRwLock();
			rwl2 = still.getRwLock();
		}
		else
		{
			rwl1 =  still.getRwLock();
			rwl2 =  moving.getRwLock();
		}
		for (int i = 0; i < still.getPhysicalBonds().size(); i++) {
			PhysicalBond pbOnStill = still.getPhysicalBonds().get(i);
			    rwl1.readLock().lock();
			    rwl2.readLock().lock();
				if(pbOnStill.getOppositePhysicalObject(still)==moving){
					alreadyAPhysicalBond = true;
					pbOnStill.vanish();
					System.out.println("VANISH : CollisionCheck.addPhysicalBondIfCrossing() ***********************************");
				}
				rwl1.readLock().unlock();
				rwl2.readLock().unlock();
		}

		if(alreadyAPhysicalBond == false){
			PhysicalBond p = new PhysicalBond(still, new double[] {lambda*distance(E,D), 0}, moving, new double[] {0,0}, -10,10);
			p.setSlidingAllowed(true);
			System.out.println("ADD : CollisionCheck.addPhysicalBondIfCrossing() ***********************************");
		}
	}


}
