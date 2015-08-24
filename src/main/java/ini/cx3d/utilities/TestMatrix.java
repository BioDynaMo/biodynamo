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

package ini.cx3d.utilities;

import static ini.cx3d.utilities.Matrix.*;

import static ini.cx3d.utilities.SystemUtilities.*;
import ini.cx3d.Param;
import ini.cx3d.physics.*;

//import java.rmi.dgc.Lease;

//import javax.vecmath.Matrix3d;

//import cern.colt.matrix.DoubleMatrix2D;
//import cern.colt.matrix.impl.DenseDoubleMatrix2D;
//import cern.colt.matrix.linalg.Algebra;

public class TestMatrix {

	
	static double[] xAxis = {-1,0,0};
	static double[] yAxis = {0,-1,0};
	static double[] zAxis = {0,0,-1};
	
	
	public static void main(String[] args) {
		
		
		
		// Grahm - Schmidt
//		xAxis = normalize(xAxis);
//		yAxis = subtract(yAxis, scalarMult(dot(xAxis,yAxis), xAxis));
//		yAxis = normalize(yAxis);
//		zAxis = subtract(zAxis, scalarMult(dot(xAxis,zAxis), xAxis));
//		zAxis = subtract(zAxis, scalarMult(dot(yAxis,zAxis), yAxis));
//		zAxis = normalize(zAxis);
		
		double[][] A = randomNoise(1.0,1000,1000);
		tic();
//		double det = det(A);
		double[][] B = inv(A);
		tac();
		
		double [][] C = mult(A,B);
		System.out.println(C[3][3] +"\n"+C[300][300] +"\n"+C[999][999] +"\n\n"+C[0][3] +"\n"+C[117][456] +"\n"+C[600][500]);
		
		
		
	}
	
	public static double[] globalToLocal(double[] vectorInGlobalCoord){
		return new double[] { 
				dot(vectorInGlobalCoord,xAxis), 
				dot(vectorInGlobalCoord,yAxis),
				dot(vectorInGlobalCoord,zAxis)
				};
	}
	
	public static double[] localToGlobal(double[] vectorInLocalCoord){
		return new double[] { 
				vectorInLocalCoord[0]*xAxis[0] + vectorInLocalCoord[1]*yAxis[0] + vectorInLocalCoord[2]*zAxis[0], 
				vectorInLocalCoord[0]*xAxis[1] + vectorInLocalCoord[1]*yAxis[1] + vectorInLocalCoord[2]*zAxis[1], 
				vectorInLocalCoord[0]*xAxis[2] + vectorInLocalCoord[1]*yAxis[2] + vectorInLocalCoord[2]*zAxis[2] 
				};
	}
	
}
