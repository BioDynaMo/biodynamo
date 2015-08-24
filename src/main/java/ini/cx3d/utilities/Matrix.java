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

import java.util.Random;



/**
 * The Matrix class contains several methods for manipulation of vectors and matrices of <code>double</code>. 
 * It doesn't have to be instantiated.
 * A vector is an one-dimensional array of <code>double</code>, with no difference between line and column vectors. 
 * A matrix is a bi-dimensional array, the first index representing the row, the second one the column. 
 * The methods return new <code>double[]</code> or <code>double[][]</code>, i.e. the arrays given as argument are not modified.
 * <p>
 * <B>Example 1 : </B>
 * <p>
 * 
 * <dir>
 * y = x/||x||<br>
 * z = Idenity*x
 * <p>
 * 
 * <code>
 * double[] x = new double[] {3.0,4.0,7.3};<br> 
 * double[] y = Matrix.normalize(x);<br> 
 * double[][] I = Matrix.id(3,3);<br> 
 * double[] z = Matrix.mult(I,y);<br> 
 * z = Matrix.scalarMult(10.0,z);
 * <code>
 * </dir>
 * 
 * <p>
 * In this example, the variable <code>x</code> refers to the same array, left unchanged,  after the call of the <code>normalize</code> methods,
 * the result being stored in a new variable. But the variable <code>z</code> references a new array after the call of <code>scalarMult</code>,
 * since it is used to store the result of the method. Note that the initial <code>double[]</code> that z used to refer to is left unchanged. 
 * If there is no other reference to it, it will be garbage collected
 * 
 * <p>
 * Although it might be clearer to write one operation per line, the different methods can naturally be combined. 
 * To avoid the typing of <code>Matrix</code> before each command, you can use the static import 
 * (by writing "<code>import static Matrix.*;</code>" in your import declaration).
 * <p>
 * <B>Example 2 : </B>
 * <p>
 * <dir>
 * r = ( ||v||*&lt;a,b&gt; ) * ( c^d - <e,f>*f/||f|| ) 
 * <p>
 * <code>double[] result = scalarMult(norm(v)*dot(a,b),subtract(crossProduct(c,d),projectionOnto(e,f)));</code>
 * </dir>
 * <p>
 * PLEASE KEEP IN MIND THAT THIS CLASS IS UNDER CONSTRUCTION AND NO GUARANTY IS GIVEN.
 * @author fredericzubler
 * 
 */

public abstract class Matrix {	
	
	static private Random random = new Random();
	
	/**
	 * @return a random number between 0 and 1;
	 */
	public static double getRandomDouble(){
		return random.nextDouble();
	}
	
	/**
	 * Initializes the random number generator. 
	 * @param seed
	 */
	public static void setRandomSeedTo(long seed){
		random = new Random(seed);
	}
	
	
	
	/**
	 * Returns the vector resulting from the addition of two vectors.
	 * 
	 * <p>
	 * There is no dimension check. It <code>a.length is smaller than b.length</code>, the subtraction will occur with
	 * the first elements of vectB. In the opposite case, a IndexOutOfBoundsException will be thrown.
	 * 
	 * @param a the first vector
	 * @param b the second vector
	 * @return vectA + vectB.
	 * 
	 */
	public static double[] add(double[] a, double[] b){
	    double[] vectResult = new double [a.length];  
		for(int i=0;i<vectResult.length;i++)              
	            vectResult[i]=a[i]+b[i];
	
		return vectResult;
	}

	/**
	 * Returns the vector resulting from the addition of a variable number of vectors.
	 * 
	 * <p>
	 * <B>Example: </B>
 	 * <p>
	 * <code>
	 * double[] g = {1,2,3};<br>
	 * double[] h = {4,5,6};<br>
	 * double[] j = {7,8,9};<br>
	 * double[] total = add(g,h,j);
	 * </code>
	 * 
	 */
	
	public static double[] add(double[] ... V){
	    double[] vectResult = new double [V[0].length];
	    for (int i = 0; i < V.length; i++) {
	    	for(int j=0; j<vectResult.length; j++)              
	            vectResult[j]+=V[i][j];
		}
		return vectResult;
	}
	

	/**
	 * Returns the matrix resulting from the addition of two matrices.
	 * 
	 * @param A the first matrix
	 * @param B the second matrix
	 * @return A + B
	 * 
	 */
	public static double[][] add(double[][] A,double[][] B){
	    double[][] matC = new double [A.length][A[0].length];  
	
	    for(int i=0;i<matC.length;i++)              
	        for(int j=0;j<matC[i].length;j++)
	            matC[i][j]=A[i][j]+B[i][j];
	
	return matC;
	}


	/**
	 * Returns a vector of <code>double</code> of length n with all elements having the value k.
	 * 
	 * @param n the length of the desired vector
	 * @param k the value of the elements 
	 */
	public static double[] allComp(int n, double k){
		double[] vectA = new double [n];
	    for(int i=0; i<vectA.length; i++)                   
	    	vectA[i] = k;
	    return vectA;
	}


	/**
	 * Returns a (n x m) matrix of <code>double</code> with all elements having the value k.
	 * 
	 * @param n the number of lines.
	 * @param m the number of columns.
	 * @param k the value of the elements 
	 */
	public static double[][] allComp(int n, int m, double k){
	    double[][] matA = new double [n][m];
	    for(int i=0; i<n; i++)              
	        for(int j=0; j<m; j++)       
	            matA[i][j] = k;
	            return matA;
	}


	/**
	 * Returns the angle (in degree) between two vectors.
	 * 
	 * @param a the first vector
	 * @param b the second vector
	 * @return the angle between them.
	 */
	public static double angleDegree(double[] a, double[] b){
		double angle = Matrix.angleRadian(a, b); 
		angle=angle*180/Math.PI;
	    return angle;
	}


	
	/**
	 * Returns the angle (in radian) between two vectors.
	 * 
	 * @param a the first vector
	 * @param b the second vector
	 * @return the angle between them.
	 */
	public static double angleRadian(double[] a, double[] b){
		double angle = Math.acos(Matrix.dot(a,b)/(Matrix.norm(a)*Matrix.norm(b)));
	    return angle;
	}


	/**
	 * Returns a <code>String</code> containing all the elements separated by commas.
	 * @param v a vector of double
	 * @return a String
	 */
	public static String arrayToString(double[] v){
			String myString = "";
	    for(int i=0; i<v.length -1; i++)
	       myString += v[i] + ", ";
	    
	    myString += v[v.length-1];
		return myString;
	}
	
	public static double[] concat(double[] a, double[] b){
		int l = a.length;
		int m = b.length;
		double[] c = new double[l+m];
		for (int i = 0; i < l; i++) {
			c[i] = a[i];
		}
		for (int i = 0; i < m; i++) {
			c[i+l] = b[i];
		}
		return c;
	}
	

	/**
	 * Returns the cross product of two vectors. 
	 * <p>
	 * There is no dimension check. If one of the vectors has more than 3 elements, only the 3 first
	 * will be taken into account. If one vector is smaller, an <code>IndexOutOfBoundsException</code> 
	 * will be thrown.
	 * 
	 * @param a
	 * @param b
	 * @return the cross product of a and b (a x b)
	 */
	public static double[] crossProduct(double[] a, double[] b){
	    double[] vectResult= new double [a.length];  
		
		vectResult[0] = a[1]*b[2] - a[2]*b[1];
		vectResult[1] = a[2]*b[0] - a[0]*b[2];
		vectResult[2] = a[0]*b[1] - a[1]*b[0];
	
	    return vectResult;
	}

	
	/**
	 * Returns the LU decomposition of a matrix. Returns two matrices : L is a
	 * lower triangular matrix with diagonal elements equal to 1 and R is an upper triangular matrix
	 * so that A = L*R 
	 * <p>
	 * The LU decomposition is only defined on a square matrix. But here there is no dimension check. 
	 * If the number of columns is bigger than the 
	 * number of lines of the second, the methods returns the determinant of a truncated matrix.
	 * In the opposite case, an <code>IndexOutOfBoundsException</code> will be thrown.
	 * 
	 * @param A the matrix
	 * @return a <code>double[][][]</code> [L,R].
	 * 
	 */
	public static double[][][] decompLU(double[][] A){
		// based on the chapter IV of the course "Analyse numerique" taught by Pr Ernst Hairer at the University of Geneva
		
		int length = A.length;
		double[][] R = new double[length][length];
		double[][] L = new double[length][length];

		// R = copy of A (the matrix on which we make the operations, so we leave A unchanged)
		for (int i = 0; i < length; i++) {
			for (int j = 0; j < length; j++) {
				R[i][j] = A[i][j];
			}
		}
		
		for(int i=0; i<length-1; i++){			
			for(int j = i+1; j<length; j++){
				L[i][i]=1.0;
				double l = R[j][i]/R[i][i];
				L[j][i] = l;
				for(int k = i; k<length; k++){
					R[j][k] = R[j][k] - l*R[i][k];
				}
			}
		}
		L[length-1][length-1] = 1.0;
		
		return new double[][][] {L,R};
	}
	
	
	/**
	 * Returns the LU decomposition of a matrix with pivot. Returns three matrices L,R,P where L is a
	 * lower triangular matrix with diagonal elements equal to 1, R is an upper triangular matrix and
	 * P is a permutation matrix so that P*A = L*R 
	 * <p>
	 * The LU decomposition is only defined on a square matrix. But here is no dimension check. If the number of columns is bigger than the 
	 * number of lines of the second, the methods returns the determinant of a truncated matrix.
	 * In the opposite case, an <code>IndexOutOfBoundsException</code> will be thrown.
	 * 
	 * @param A the matrix
	 * @return [L,R,P].
	 * 
	 */
	public static double[][][] decompLUP(double[][] A){
		// based on the chapter IV of the course "Analyse numerique" taught by Pr Ernst Hairer at the University of Geneva
		
		int length = A.length;
		double[][] R = new double[length][length];
		double[][] L = new double[length][length];
		double[][] P = new double[length][length];;
		int nbOfSwaps = 1;
		// to keep track of the permutations :
		int[] orderOfPermutations = new int[length];
		for (int i = 0; i < length; i++) {
			orderOfPermutations[i] = i;
		}
 
		// R = copy of A (the matrix on which we make the operations, so we leave A unchanged)
		for (int i = 0; i < length; i++) {
			for (int j = 0; j < length; j++) {
				R[i][j] = A[i][j];
			}
		}
		
		for(int i=0; i<length-1; i++){
			// Find pivot and swap lines
			double a = Math.abs(R[i][i]); 
			int ligne = i;
			for (int j = i+1; j < length; j++) {
				if(Math.abs(R[j][i]) > a){
					a = Math.abs(R[j][i]);
					ligne = j;
					// A) one possibility would be to swap each line of R, L each time we find a temporary bigger value
//					double[] tempLine = R[j]; R[j] = R[i] ; R[i] = tempLine;
//					temp = L[ligne]; L[ligne] = L[i] ; L[i] = temp;
//					int tempValue = orederOfPermutations[j]; orederOfPermutations[j] = orederOfPermutations[i] ; orederOfPermutations[i] = tempValue; // if we want the P
//					nbOfSwaps = -nbOfSwaps;
				}
			}
			// B) Other possibility : to make only one swap at the end of the pivot finding
			if(ligne!=i){
				// swap R
				double[] temp = R[ligne]; R[ligne] = R[i] ; R[i] = temp;
				// swap L
				temp = L[ligne]; L[ligne] = L[i] ; L[i] = temp;
				// store which line we take 
				int tempValue = orderOfPermutations[ligne]; orderOfPermutations[ligne] = orderOfPermutations[i] ; orderOfPermutations[i] = tempValue;
				// store that we made a supplementary line swap
				nbOfSwaps = -nbOfSwaps;
			}

			// the elimination algorithm : finding L and R
			for(int j = i+1; j<length; j++){
				L[i][i]=1.0;
				double l = R[j][i]/R[i][i];
				L[j][i] = l;
				for(int k = i; k<length; k++){
					R[j][k] = R[j][k] - l*R[i][k];
				}
			}
		}
		L[length-1][length-1] = 1.0;
		
		double determinant = nbOfSwaps;
		for(int i=0; i<length; i++){
			determinant *= R[i][i];
		}
		
		// computing P out of the sequence of permutations
		for (int i = 0; i < length; i++) {
			P[i][orderOfPermutations[i]] = 1;
		}
		
		return new double[][][] {L,R,P};
	} 
	
	

	/**
	 * Returns the determinant of a matrix.
	 * <p>
	 * For 3-by-3 or smaller matrices, the method performs an explicit computation based on the definition of the
	 * determinant. For matrices of size bigger or equal to 4, a LR-decomposition with pivot is performed (the method
	 * <code>detLR()</code> is called.
	 * <p>
	 * The determinant is only defined on a square matrix. But here is no dimension check. If the number of columns is bigger than the 
	 * number of lines of the second, the methods returns the determinant of a truncated matrix.
	 * In the opposite case, an <code>IndexOutOfBoundsException</code> will be thrown.
	 * 
	 * @param A the matrix
	 * @return its determinant.
	 * 
	 */
	public static double det(double[][] A){
	    int n =  A.length;
	    
	    if(n==1)
    		return A[0][0];
	   
	    if(n==2)
    		return A[0][0]*A[1][1]-A[1][0]*A[0][1];
    		
	    if(n==3){
	    	return 	A[0][0]*(A[1][1]*A[2][2]-A[1][2]*A[2][1]) -
					A[0][1]*(A[1][0]*A[2][2]-A[1][2]*A[2][0]) +
					A[0][2]*(A[1][0]*A[2][1]-A[1][1]*A[2][0]) ;
		}else{
	    	return detLR(A);
		}
	}


	/**
	 * Returns the determinant of a matrix, computed by a LR-decomposition with pivot. 
	 * For matrices 3-by-3 or smaller, the method <code>det()</code> is faster.
	 * <p>
	 * The determinant is only defined on a square matrix. But here is no dimension check. 
	 * If the number of columns is bigger than the number of lines of the second, 
	 * the methods returns the determinant of a truncated matrix.
	 * In the opposite case, an <code>IndexOutOfBoundsException</code> will be thrown.
	 * 
	 * @param A the matrix
	 * @return its determinant.
	 * 
	 */
	public static double detLR(double[][] A){
		// based on the chapter IV of the course "Analyse numerique" taught by Pr Ernst Hairer at the University of Geneva
		
		int n = A.length;
		double[][] R = new double[n][n];
		int nbOfSwaps = 1;
		// although we perform a LU decomposition with pivot, we don't need to store L and P
		
		// R = copy of A (the matrix on which we make the operations, so we leave A unchanged)
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				R[i][j] = A[i][j];
			}
		}
		// Triangulation of R
		for(int i=0; i<n-1; i++){
			// Find pivot and swap lines
			double a = Math.abs(R[i][i]); 
			for (int j = i+1; j < n; j++) {
				if(Math.abs(R[j][i]) > a){
					a = Math.abs(R[j][i]);
					double[] tempLine = R[j]; R[j] = R[i] ; R[i] = tempLine;
					nbOfSwaps = -nbOfSwaps;
				}
			}
			// Elimination of (i+i)th element of each line >i
			for(int j = i+1; j<n; j++){
				double l = R[j][i]/R[i][i];
				for(int k = i; k<n; k++){
					R[j][k] = R[j][k] - l*R[i][k];
				}
			}
		}
		// Compute the det of the triangular Matrix R
		double determinant = nbOfSwaps;   // to keep the correct signe (inverted at each swap)
		for(int i=0; i<n; i++){
			determinant *= R[i][i];
		}
		return determinant;
	}
	
	/**
	 * Computes the inner product (also called dot product) of two vectors.
	 * @param a
	 * @param b
	 * @return a.b
	 */
	public static double dot(double[] a, double[] b){
	    double prod = 0;  
		for(int i=0;i<a.length;i++)
			prod += a[i]*b[i];
	    
		return prod;
	}


	/** 
	 * Returns the euclidean distance between two points. Equivalent to the 2-norm of (b-a).
	 * @param a
	 * @param b
	 * @return ||a-b||
	 */
	public static double distance(double[] a, double[] b) {
			return norm(subtract(a, b));
	}
	
	
	/**
	 * Returns a (n x n) identity matrix. 
	 * (The elements on the diagonal have value <code>1.0</code>, all others are set to <code>0.0</code>).
	 * 
	 * @param n the size of the matrix
	 * @return the identity matrix
	 */
	public static double[][] id(int n){
	    double[][] A = new double [n][n];
	    for(int i=0; i<n; i++)              
	        for(int j=0; j<n; j++){
	            if (i==j) A[i][j] = 1.0;
	            else A[i][j] = 0.0;
	            }
	    return A;
	}

	/**
	 * Returns the inverse of a matrix. The method uses a LU decomposition with pivot. 
	 * There is no check on the size (is it a square matrix) or the rank (is it inversible, ie det doesn't equal 0).
	 * @param A
	 * @return A^(-1)
	 */
	public static double[][] inv(double[][] A){
		// based on the chapter IV of the course "Analyse numerique" 
		// taught by Pr Ernst Hairer at the University of Geneva
	
		int n = A.length;
		double[][] invertedA = new double[n][n];
		double[][] R = new double[n][n];
		
		double[][] basis = id(n);
		
		// R = copy of A (the matrix on which we make the operations, so we leave A unchanged)
		for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				R[i][j] = A[i][j];
			}
		}
		// Triangulation of R
		for(int i=0; i<n-1; i++){
			// Find pivot and swap lines
			double a = Math.abs(R[i][i]); 
			for (int j = i+1; j < n; j++) {
				if(Math.abs(R[j][i]) > a){
					a = Math.abs(R[j][i]);
					double[] tempLine = R[j]; R[j] = R[i] ; R[i] = tempLine;
					tempLine = basis[j]; basis[j] = basis[i] ; basis[i] = tempLine;
				}
			}
			// Elimination of (i+i)th element of each line >i in R
			for(int j = i+1; j<n; j++){
				double l = R[j][i]/R[i][i];
				for(int k = i; k<n; k++){  // CAUTION we start at k = i !!
					R[j][k] = R[j][k] - l*R[i][k];
				}
				for(int k = 0; k<n; k++){  // CAUTION we start at k = 0 !!
					basis[j][k] = basis[j][k] - l*basis[i][k];
				}
			}
		}
		
		for (int k = 0; k < basis.length; k++) {		
			for (int i = n - 1; i > -1; i--) {
				double sum = 0.0;
				for (int j = i + 1; j < n; j++)
					sum += R[i][j] * invertedA[j][k];
				invertedA[i][k] = (basis[i][k] - sum) / R[i][i];
			}
		}
		return invertedA;
	}

	/** Returns the coefficient of the polynome of degree n that fits the best the pairs (xi,yi). The
	 * coefficient are given in ascending power (p[0] + p[1]*x + p[2]*x^2 + ... ).
	 * @param x first values of the pairs
	 * @param y second values
	 * @param n the degree of the desired polynome
	 * @return its coefficients (a + b*x + c*x^2 +...)
	 */
	public static double[] leastSquare(double[] x, double[] y, int n){
		int m = x.length;
		double[][] A = new double[m][n+1];
		for(int i = 0; i<m; i++){
			A[i][0] = 1;
			A[i][1] = x[i];
			for(int j = 2; j<n+1; j++){
				A[i][j] = A[i][j-1]*x[i];
			}
		}
		return solve(A,y);
	}
	
	
	
	/**
	 * Returns a (3x3) rotation matrix, representing a rotation of angle phi around the x-axis in the positive
	 * mathematical sense (clockwise when looking toward the origin).
	 * <p>
	 * To rotate the vector around the x-axis, you have to multiply it by the rotation matrix:
	 * <p>
	 * <B>Example: </B>
 	 * <p>
	 * <code>
	 * double[] vectorToRotate = {0.1, 0.6, 3.2};<br>
	 * double[] resultOfTheRotation = mult( matRotAroundX(Math.PI), vectorToRotate);<br>
	 * </code>
	 * 
	 * @param phi the rotation angle in radians.
	 * @return a rotation matrix of R^3, of angle phi around the (1,0,0)-axis.
	 * 
	 */ 
	public static double[][] matRotAroundX(double phi){
		double[][] Rx = new double[3][3];  
	   	Rx[0][0] = 1.0; 	Rx[0][1] = 0.0;				Rx[0][2] = 0.0;
	  	Rx[1][0] = 0.0; 	Rx[1][1] = Math.cos(phi);	Rx[1][2] = Math.sin(phi);
	   	Rx[2][0] = 0.0; 	Rx[2][1] =-Rx[1][2];		Rx[2][2] = Rx[1][1];
	    return Rx;
	}
	
	/**
	 * Returns a (3x3) rotation matrix, representing a rotation of angle phi around the y-axis in the positive
	 * mathematical sense (clockwise when looking toward the origin).
	 * 
	 * @param phi the rotation angle in radians.
	 * @return a rotation matrix in R^3, of angle phi around the (0,1,0)-axis.
	 * 
	 */ 
	public static double[][] matRotAroundY(double phi){
		double[][] Ry = new double[3][3];
		Ry[0][0] = Math.cos(phi);		Ry[0][1] = 0;		Ry[0][2] = Math.sin(phi);
		Ry[1][0] = 0;					Ry[1][1] = 1;		Ry[1][2] = 0;
		Ry[2][0] = -Ry[0][2];			Ry[2][1] = 0;		Ry[2][2] = Ry[0][0];
		return Ry;
	}

	/**
	 * Returns a (3x3) rotation matrix, representing a rotation of angle phi around the z-axis in the positive
	 * mathematical sense (clockwise when looking toward the origin).
	 * 
	 * @param phi the rotation angle in radians.
	 * @return a rotation matrix in R^3, of angle phi around the (0,0,1)-axis.
	 * 
	 */ 
	public static double[][] matRotAroundZ(double phi){
		double[][] Rz = new double[3][3];
		Rz[0][0] = Math.cos(phi);		Rz[0][1] = Math.sin(phi);	Rz[0][2] = 0;
		Rz[1][0] = -Rz[0][1];			Rz[1][1] = Rz[0][0];		Rz[1][2] = 0;
		Rz[2][0] = 0;					Rz[2][1] = 0;				Rz[2][2] = 1;
		return Rz;
	}


	/**
	 * Returns the vector resulting from the multiplication of a vector by a matrix.
	 * <p>
	 * There is no dimension check. If the number of columns of the matrix is smaller then the length of
	 * the vector, only the first elements of the vector will be taken into account. It the number of colums
	 * of the matrix is bigger, an <code>IndexOutOfBoundsException</code> will be thrown.
	 * 
	 * @param A the matrix
	 * @param b the vector
	 * @return A * b
	 * 
	 */ 
	public static double[] mult(double[][] A,double[] b){
	    double[] vectR = new double [A.length];  
	
	    for(int i=0;i<vectR.length;i++){
	     vectR[i]=0;
	        for(int j=0;j<b.length;j++){
	           vectR[i]+=A[i][j]*b[j];           
	        }
	    }
	    
	    return vectR;
	}


	/**
	 * Returns the matrix resulting from the multiplication of two matrices.
	 * <p>
	 * There is no dimension check. If the number of columns of the first matrix is smaller then the 
	 * number of lines of the second, the second matrix will be truncated. In the opposite case,
	 * an <code>IndexOutOfBoundsException</code> will be thrown.
	 * 
	 * @param A the first matrix
	 * @param B the second matrix
	 * @return A * B
	 * 
	 */ 
	public static double[][] mult(double[][] A,double[][] B){
	    double[][] C = new double [A.length][B[0].length];  
	
	    for(int i=0;i<C.length;i++){
	        for(int j=0;j<C[i].length;j++){
	            C[i][j]=0;
	            for(int k=0;k<A[1].length;k++){
	                C[i][j]+=A[i][k]*B[k][j];           
	            }
	        }
	    }
	return C;
	}


	/**
	 * Returns the euclidean norm of a vector.
	 * @param a vector
	 * @return it's norm
	 */
	public static double norm(double[] a){
		double norme = 0;
	    for(int i=0;i<a.length;i++)
			norme += a[i]*a[i];
		norme = Math.sqrt(norme);             
	    return norme;
	}
	
	/**
	 * Returns the L1 norm of a vector (i.e. sum of absolute values of its components).
	 * @param a vector
	 * @return it's L1 norm
	 */
	public static double norm1(double[] a){
		double norme = 0;
	    for(int i=0;i<a.length;i++)
			norme += Math.abs(a[i]);           
	    return norme;
	}
	
	/**
	 * Returns the euclidean norm of a vector.
	 * @param a vector
	 * @return it's norm
	 */
	public static double norm2(double[] a){
		double norme = 0;
	    for(int i=0;i<a.length;i++)
			norme += a[i]*a[i];
		norme = Math.sqrt(norme);             
	    return norme;
	}
	
	/**
	 * Returns the L-infinity norm (the absolute value of the largest component of the vector).
	 * @param a vector
	 * @return it's L infinity norm
	 */
	public static double normInfinity(double[] a){
		double max = 0;
	    for(int i=0;i<a.length;i++){
			double temp = Math.abs(a[i]);
			if(temp>max) max = temp;
	    }          
	    return max;
	}
	
	
	

	/**
	 * Normalizes a vector.
	 * @param a a vector
	 * @return the vector divided by its norm
	 */
	public static double[] normalize(double[] a){
		double[] vectResult = new double[a.length];
		double norme = Matrix.norm(a);
		if(norme == 0.0)
			norme = 1.0;
	    for(int i=0;i<a.length;i++)
			vectResult[i]=a[i] /norme;
	    return vectResult;
	}

	
	/**
	 * Returns a vector of norm 1 perpendicular to a 3D vector. As usual there is not length check.
	 * @param a
	 * @return a vector perpendicular 
	 */
	public static double[] perp3(double[] a) {
			double[] vectPerp = new double[3];
		if (a[0]==0.0) {					
			vectPerp[0] = 1.0;
			vectPerp[1] = 0.0;
			vectPerp[2] = 0.0;
			vectPerp = rotAroundAxis(vectPerp,6.35*random.nextDouble(),a);
		}
		else {
			vectPerp[0] = a[1];
			vectPerp[1] = -a[0];
			vectPerp[2] = 0.0;
			vectPerp = normalize(vectPerp);
			vectPerp = rotAroundAxis(vectPerp,6.35*random.nextDouble(),a);
		}
		return vectPerp;
	}

	
	/**
	 * Writes a vector in column in the output stream.
	 * @param a the vect to write
	 * @see printLine
	 */
	public static void print(double[] a){
	    for(int i=0; i<a.length; i++)
	        System.out.println(a[i]);
	}


	/**
	 * Writes a matrix in the output stream. The disposition is consistent with the usual mathematical conventions
	 * (elements <code>[i][j]</code> will be at line i, column element.
	 * 
	 * @param  A the matrix to write
     */
    public static void print(double[][] A){
        for(int i=0; i<A.length; i++){
            for(int j=0; j<A[i].length; j++){
            	System.out.print(A[i][j]+"     ");
            }           
            System.out.print("\n");
        }
    }
	
    
    /**
	 * Writes a vector in square brackets with its name, in the output stream. 
	 * It is particularly convenient if you want to paste it directly into Matlab.
	 * 
	 * @param  a the vector to write
     */
	public static void print(String name, double[] a){
		System.out.println("\n"+name+" = [");
	    for(int i=0; i<a.length; i++)
	        System.out.println(a[i]);
	    System.out.println("]");
	}
	
	/**
	 * Writes a matrix in square brackets with its name, in the output stream. 
	 * It is particularly convenient if you want to paste it directly into Matlab.
	 * 
	 * @param  A the matrix to write
     */
    public static void print(String name, double[][] A){
    	System.out.println("\n"+name+" = [");
    	for(int i=0; i<A.length; i++){
            for(int j=0; j<A[i].length; j++){
            	System.out.print(A[i][j]+"     ");
            }           
            System.out.print("\n");
        }
    	System.out.println("];");
    }
    
    /**
	 * Writes a vector in line in the output stream.
	 * @param a the vect to write
	 */
	public static void printLine(double[] a){
	    for(int i=0; i<a.length; i++)
	        System.out.print(a[i] + ", ");
	}


	 /**
	 * Writes a vector in line in the output stream, and terminates the line.
	 * @param a the vect to write
	 */
	public static void printlnLine(double[] a){
	    for(int i=0; i<a.length -1; i++)
	        System.out.print(a[i] + ", ");
		System.out.println(a[a.length-1]);
	}
	
	public static void printlnLine(String name, double[] a){
		System.out.print(name+" = [ ");
	    for(int i=0; i<a.length -1; i++)
	        System.out.print(a[i] + ", ");
		System.out.println(a[a.length-1]+" ];");
	}

	
	/**
	 * Returns the projection of the first vector onto the second one.
	 * @param a
	 * @param b
	 * @return the projection of a onto b
	 */
	public static double[] projectionOnto(double[] a, double[] b){
		double k = (dot(a,b) / dot(b,b));
		return scalarMult(k,b);
	}


	/**
	 * Returns a vector of length n of random <code>double</code> between 0 and 1.
     */
	public static double[] random(int n){
	    double[] a = new double [n];
	    for(int i=0; i<n; i++)             
	    	a[i] = (double)random.nextDouble();
	    return a;
	}


	/**
	 * Returns a (n x m) matrix of random <code>double</code> between 0 and 1.
	 * @param n the number of lines
	 * @param m the number of columns 
     */
    public static double[][] random(int n, int m){
        double[][] A = new double [n][m];
        for(int i=0; i<n; i++)              
            for(int j=0; j<m; j++)       
                A[i][j] = (double)random.nextDouble();
                return A;
    }
    
    /**
     * Returns a vector (of length n) of random <code>double</code> between -k and k.
     * @param k the upper bound (absolute value) of range for the ramdom elements
     * @param n the length of the vector
     * @return a vector with random elements between -k and k
     */
	public static double[] randomNoise(double k, int n){
	    double[] a = new double [n];
	    for(int i=0; i<n; i++)             
	    	a[i] = -k + 2*k*(double)random.nextDouble();
	    return a;
	}


	/**
	 * Returns a (n x m) matrix of random <code>double</code> between -k and k.
	 * 
	 * @param k the upper bound (absolute value) of range for the ramdom elements.
	 * @param n the number of lines.
	 * @param m the number of columns 
     */
    public static double[][] randomNoise(double k, int n, int m){
        double[][] A = new double [n][m];
        for(int i=0; i<n; i++){              
            for(int j=0; j<m; j++){
            	A[i][j] = -k + 2*k*(double)random.nextDouble();
            	
            }
        }
        return A; 
    }
    
    
    /**
	 * Returns a random vector of <code>double</code> being either 0.0 or 1.0 .
	 * 
	 * @param n the length of the vector.
     */
	public static double[] randomZeroOne(int n){
	    double[] a = new double [n];
	    for(int i=0; i<n; i++)              
	        a[i] = (double)Math.rint(random.nextDouble());
	    return a; 
	}


	/**
	 * Returns a (n x m) random matrix of <code>double</code> being either 0.0 or 1.0 .
	 * 
	 * @param n the number of lines
	 * @param m the number of columns 
     */
    public static double[][] randomZeroOne(int n, int m){
        double[][] A = new double [n][m];
        for(int i=0; i<n; i++)              
            for(int j=0; j<m; j++)       
                A[i][j] = (double)Math.rint(random.nextDouble());
                return A;
    }
    
    /**
	 * Returns a vector of length n with elements randomly chosen to be either -1.0 or +1.0 .
	 * 
	 * @param n the vector's length
     */
	public static double[] randomOneMinusOne(int n){
	    double[] A = new double [n];
	    for(int i=0; i<n; i++) 
	    		if (random.nextDouble() >0.5)
	    			A[i] = 1.0;
	    		else
	    			A[i] = -1.0;
	    return A; 
	}

	/**
	 * Returns a (n x m) random matrix with elements randomly chosen to be either -1.0 or +1.0.
	 * 
	 * @param n the number of lines
	 * @param m the number of columns 
     */
    public static double[][] randomOneMinusOne(int n, int m){
        double[][] A = new double [n][m];
        for(int i=0; i<n; i++)              
            for(int j=0; j<m; j++)       
            	if (random.nextDouble() >0.5)
	    			A[i][j] = 1.0;
	    		else
	    			A[i][j] = -1.0;
        return A;
    }

    /**
     * Returns a vector of length n, with elements randomly chosen between 0 and d. (In fact, 
     * you can also give a negative d, and then get a vector with only negative values).
     * @param d
     * @param n
     * @return
     */
	public static double[] randomPositiveNoise(double d, int n){
	    double[] a = new double [n];
	    for(int i=0; i<n; i++)             
	    	a[i] = (double)random.nextDouble()*d;
	    return a;
	}

    
    
	/**
	 * Performs a rotation of a 3D vector a around a given axis b, in the positive mathematical sens.
	 * As usual : there is no dimension check (only works with vectors of length 3), the arrays
	 * given as argument are left unchanged.
	 * <p>
	 * <B>Example: </B>
 	 * <p>
	 * <code>
	 * double[] axis = {1.0,1.0,0.0};<br>
	 * double[] vectToRotate = {4,5,6};<br>
	 * double[] resultOfTheRotation = rotAroundAxis(vectToRotate, Math.PI, axis);<br>
	 * </code>
	 * @param a the vector we want to rotate
	 * @param theta the amplitude of rotation (in radian)
	 * @param b the axis (also a vector)
	 * @return the vector after rotation
	 */
	public static double[] rotAroundAxis(double[] a, double theta, double[] b){
		
		double[] axis = Matrix.normalize(b);
	
		double[] temp1 = scalarMult(dot(a,axis) , axis);
		double[] temp2 = scalarMult(Math.cos(-theta), subtract(a, temp1));
		double[] temp3 = scalarMult(Math.sin(-theta), crossProduct(a,axis));
	   	
	    return add(temp1,temp2,temp3);
	}

	/**
	 * Solves a linear system of equation A*x = b of n equations with m unknowns. If n = m
	 * an exact solution is given. If m is bigger than n, we look for a vector x of length n
	 * s.t. ||A*x - b|| is minimal. (This method only works for coefficient matrices A with maximal rank;
	 * this condition is not tested).
	 * @param A
	 * @param b
	 * @return x
	 */
	public static double[] solve(double[][] A, double[] b){
		int m = A.length;
		int n = A[0].length;
		if(n<m){
			return solveQR(A,b);
		}
		if(n<4){
			return solveCramer(A,b);
		}
		return solveGauss(A,b);
	}
	
	/**
	 * Solves a systems of linear equations A*x = b with Cramer's rule. The method is only defined
	 * for n = 2 or 3. (As usual, there is no check of the dimensions or the rank of the matrix)..
	 * @param A 
	 * @param b
	 * @return x
	 */
	public static double[] solveCramer(double[][] A, double[] b){
		int n = A.length;
		if(n==2){
	    	double a = A[0][0];	double bb = A[0][1];
    		double c = A[1][0];	double d = A[1][1];
    		double e = b[0];	double f = b[1];
    		double det = a*d-bb*c;
    		return new double[] { (e*d-bb*f)/det, (a*f-e*c)/det}; 
		}
	    if(n==3){
	    	double a = A[0][0]; double bb = A[0][1]; double c = A[0][2];
    		double d = A[1][0];	double e = A[1][1];	double f = A[1][2];
    		double g = A[2][0];	double h = A[2][1];	double i = A[2][2];
    		double j = b[0];	double k = b[1];	double l = b[2];
    		
    		double eihf = (e*i-h*f);
    		double bich = (bb*i-c*h);
    		double bfce = (bb*f-c*e);
    		double det_1 = 1/(a*eihf - d*bich + g*bfce);
    		double x = det_1*(j*eihf - k*bich + l*bfce);
    		double y = det_1*( a*(k*i-l*f) - d*(j*i-c*l) + g*(j*f-c*k)  );
    		double z = det_1*( a*(e*l-h*k) - d*(bb*l-h*j) + g*(k*bb-j*e)  );
    		return new double[] {x,y,z}; 
		}else{
	    	return null;
		}
	}
	
	/**
	 * Solves a systems of linear equations A*x = b with Gaussian elimination. 
	 * For systems with n = 2 or 3
	 * it is faster to use <code>solveCramer(A,b)</code>.
	 * (As usual, there is no check of the dimensions or the rank of the matrix).
	 * @param A 
	 * @param b
	 * @return x
	 */
	public static double[] solveGauss(double[][] A, double[] b){
		// based on the chapter IV of the course "Analyse numerique" taught by Pr Ernst Hairer at the University of Geneva
		
		int length = A.length;
		double[][] R = new double[length][length];
		double[] c = new double[length];
		double[] x = new double[length];
		// although we perform a LU decomposition with pivot, we don't need to store L and P
		
		// R = copy of A (the matrix on which we make the operations, so we leave A unchanged)
		// c = copy of b (same reason)
		for (int i = 0; i < length; i++) {
			c[i] = b[i];
			for (int j = 0; j < length; j++) {
				R[i][j] = A[i][j];
			}
		}
		// Triangulation of R
		for(int i=0; i<length-1; i++){
			// Find pivot and swap lines
			double a = Math.abs(R[i][i]); 
			for (int j = i+1; j < length; j++) {
				if(Math.abs(R[j][i]) > a){
					a = Math.abs(R[j][i]);
					double[] tempLine = R[j]; R[j] = R[i] ; R[i] = tempLine;
					double tempc = c[j]; c[j] = c[i]; c[i] = tempc;
				}
			}
			// Elimination of (i+i)th element of each line >i
			for(int j = i+1; j<length; j++){
				double l = R[j][i]/R[i][i];
				c[j] -= l * c[i];
				for(int k = i; k<length; k++){
					R[j][k] = R[j][k] - l*R[i][k];
				}
			}
			
		}
		// Find x
		for (int i = length - 1; i > -1; i--) {
	        double sum = 0.0;
	        for (int j = i + 1; j < length; j++)
	            sum += R[i][j] * x[j];
	        x[i] = (c[i] - sum) / R[i][i];
	    }

		return x;
	}
	
	/**
	 * Returns the best "solution" (least square method) 
	 * to an overdetermined system (of n linear equations with m unknowns), by a QR decomposition.
	 * If the system has more equations than unknowns, in general there is no solution.
	 * We look for a vector x s.t ||A*x - b|| is minimal. 
	 * <p>
	 * As usual, there is no check on the rank
	 * or the dimensions of A and b.
	 * @param A an m x n matrix (m>=n)
	 * @param b a vector of length m
	 * @return x a vector of length n
	 */
	public static double[] solveQR(double[][] A, double[] b){
		int m = A.length;
		int n = A[0].length;
		double[] c = new double[m];
		// R = copy of A (so we don't alter A
		// c = copy of b
		double[][] R = new double[m][n];
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < n; j++) {
				R[i][j] = A[i][j];
			}
			c[i] = b[i];
		}
		// n steps if n < m, (n-1) steps if n = m
		int numberOfSteps = n;
		if(n==m)
			numberOfSteps = n-1;
		// loop for the different multiplication with matrix H
		for (int j = 0; j < numberOfSteps; j++) {
			// A1 = first non-zero column of A
			double[] A1 = new double[m-j];
			double[] cc = new double[m-j];
			for (int i = 0; i < A1.length; i++) {
				A1[i] = R[i+j][j];  // the jth column is the first non zero. We take it from it's jth element
				cc[i] = c[i+j];
			}
			// Alpha (7.8)
			double sum = 0;
			for (int i = 0; i < A1.length; i++) {
				sum+= A1[i]*A1[i];
			}
			double norm = Math.sqrt(sum);	// norm of this (sub-)column
			double alpha = -Math.signum(A1[0])*norm; 
			// vector vj
			double[] v = new double[A1.length];
			for (int i = 0; i < A1.length; i++) {
				v[i]= A1[i];
			}
			v[0] -= alpha;
			// Beta (7.10)
			double beta = -1/(alpha*(A1[0]-alpha));
			// H1*A (7.9)
			// Aj = jth column of A
			
			double[] Aj = new double[m-j];
			for (int jj = j; jj < n; jj++) {
				for (int i = 0; i < m-j; i++) {
					Aj[i]= R[i+j][jj];
				}
				double[] HA = subtract(Aj , scalarMult( beta*dot(v,Aj) , v) );
				for (int i = j; i < m; i++) {
					R[i][jj] = HA[i-j];
				}
			}
			cc = subtract(cc , scalarMult( beta*dot(v,cc) , v) );
			for (int i = j; i < m; i++) {
				c[i] = cc[i-j];
			}
		}
		double[] x = new double[n];
		for (int i = n - 1; i > -1; i--) {
	        double sum = 0.0;
	        for (int j = i + 1; j < n; j++)
	            sum += R[i][j] * x[j];
	        x[i] = (c[i] - sum) / R[i][i];
	    }
		return x;
	}

	/**
     * Multiplication of (all the elements of) a vector by a scalar value.
     *
     * @param  k a scalar
     * @param  a the vector we want to multiply
     * @return k*vectA
     */
	public static double[] scalarMult(double k, double[] a){
		double[] vectResult = new double [a.length];
	    for(int i=0;i<a.length;i++)              
	    	vectResult[i]=a[i] * k;
	    return vectResult;
	}


    /**
     * Multiplication of (all the elements of) a matrix by a scalar value.
     *
     * @param  k a scalar
     * @param  A the matrix we want to multiply
     * @return k*matA
     */
    public static double[][] scalarMult(double k, double[][] A){
		double[][] matR = new double[A.length][A[0].length];
        for(int i=0;i<A.length;i++)              
            for(int j=0;j<A[i].length;j++)
	    		matR[i][j] = k*A[i][j];
        return matR;
    }
    
    /** 
     * Random permutation of the elements of a vector.
     * @param v an array of doubles
     * @return an other array with the same elements, in random order
     */
    public static double[] scramble(double[] v){
    	// associate each element with a random number
    	int n = v.length;
    	java.util.Hashtable<Double,Double> table = new java.util.Hashtable<Double,Double>(v.length);
		double[] vRank = new double[n];
		java.util.Random r = new java.util.Random();
		for (int i = 0; i < v.length; i++) {
			double d = r.nextDouble();
			vRank[i] = d;
			table.put(d, v[i]);
		}
    	// sort the random numbers
		java.util.Arrays.sort(vRank);
    	// get the elements associated with the random numbers  : they are in random order
		double[] vScrambled = new double[n];
		for (int i = 0; i < n; i++) {
			vScrambled[i] = table.get(vRank[i]);
		}
		return vScrambled;
    }
    
    
    
	/**
	 * Returns the angle (in radian) between two vectors of length 2, with a value between -pi and +pi. 
	 * @param a
	 * @param b
	 * @return the signed angle between a and b
	 */
	public static double signedAngleRadian(double[] a, double[] b){
		double angle = Math.acos(Matrix.dot(a,b)/(Matrix.norm(a)*Matrix.norm(b)));
		
		double determin = det(new double[][] { {a[0], a[1]},
											 {b[0], b[1]} }); 
		if (determin>0){
			return angle;
		}else{
			return -angle;
		}
	}


	/**
	 * Returns a sub-matrix, obtained by removing the line k and the column l of a matrix.
	 *
	 * @param A the matrix
	 * @param k the index of the line to remove
	 * @param l the index of the column to remove
	 * @return a submatrix
	 * 
	 */
	public static double[][] subMatrix(double[][] A, int k, int l){
		int iLength = A.length;
		int jLength = A[0].length;
		 
		double[][] C = new double[iLength-1][jLength-1];  
		for(int i=0;i<k;i++) {              
			for(int j=0;j<l;j++){
	        	C[i][j] = A[i][j];
		 	}
		 	for(int j=l+1;j<jLength;j++){
		 		C[i][j-1] = A[i][j];
		 	}
		}
		for(int i=k+1;i<iLength;i++) {              
			for(int j=0;j<l;j++){
				C[i-1][j] = A[i][j];
		 	}
		 	for(int j=l+1;j<jLength;j++){
		 		C[i-1][j-1] = A[i][j];
		 	}
		}
		return C;
	}


	/**
	 * Returns the matrix resulting from the subtraction of two vectors. Without dimension check.
	 * @param a
	 * @param b
	 * @return a-b
	 */
	public static double[] subtract(double[] a, double[] b){
			int VectALength = a.length;
	    double[] vectResult = new double [VectALength];  
		for(int i=0;i<VectALength;i++)              
	            vectResult[i]=a[i]-b[i];
	
		return vectResult;
	}

	
	/**
	 * Returns the matrix resulting from the subtraction of two matrices. Without dimension check.
	 * 
	 * 
	 * @param A the first matrix.
	 * @param B the second matrix
	 * @return A - B.
	 * 
	 */
	public static double[][] subtract(double[][] A, double[][] B){
	    double[][] matC = new double [A.length][A[0].length];  
	    for(int i=0;i<matC.length;i++)              
	        for(int j=0;j<matC[i].length;j++)
	            matC[i][j]=A[i][j]-B[i][j];
	
	return matC;
	}
	

	/**
	 * Returns the matrix resulting from the transposition of the elements of of a matrix.
	 * @param A
	 * @return At = the transpose of A
	 */
    public static double[][] transp(double[][] A){
        double[][] C = new double [A[0].length][A.length];  

        for(int i=0;i<C.length;i++)              
            for(int j=0;j<C[i].length;j++)
                    C[i][j]=A[j][i];           
        return C;
    }
    
    
    /**
     * Returns the Vandermonde matrix associated with a vector.
     * @param x the second column coefficient
     * @return the associated Vanedermonde matrix
     */
    public static double[][] vandermonde(double[] x){
    	double[][] V = new double[x.length][x.length]; 
    	for (int i = 0; i < x.length; i++) {
    		V[i][0] = 1;
			V[i][1] = x[i];
			for (int j = 2; j < x.length; j++) {
				V[i][j] = V[i][j-1]*x[i];
			}
		}
    	return V;
    }
    
    /**
     * Returns the Hilbert matrix of order n. Hilbert matrices are examples of ill-conditioned matrices.
     * @param n the size of the matrix
     * @return the Hilbert matrix
     */
    public static double[][] hilbert(int n){
    	double[][] H = new double[n][n]; 
    	for (int i = 0; i < n; i++) {
			for (int j = 0; j < n; j++) {
				H[i][j] = 1.0/(i+j+1);
			}
		}
    	return H;
    }
    
    public static double[] zeros(int n){
    	double[] z = new double[n];
    	return z;
    }
    
    
      
    public static void main(String[] args){
    	double[] direction = {0.0, 0.0, 0.0};
    	direction = normalize(direction);
    	printlnLine("dir",direction);
    	
    	
    }
}
