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

package ini.cx3d.spatialOrganization;

import ini.cx3d.spatialOrganization.interfaces.Rational;
import ini.cx3d.spatialOrganization.factory.RationalFactory;

/**
 * NOTE: This class has been replaced by a native implementation! It is merely used for debugging purposes!
 *
 * This class is used to store vectors in 3D-space. It also provides basic
 * linear algebra functions. All functions are performed using precise arithmetics by
 * using instances of {@link RationalJava} to store the entries and to perform the calculations.
 * 
 * This class is exclusively designed to store 3 elements per vector.
 * 
 * @author Dennis Goehlsdorf
 *
 */
public class ExactVector extends ini.cx3d.swig.simulation.ExactVector implements ini.cx3d.spatialOrganization.interfaces.ExactVector {

	/**
	 *  Stores the elements of this vector.
	 */
	Rational[] elements;

	/**
	 * Creates a new vector from an array of rational numbers.
	 * @param values The entries for this vector. The length of this array is expected to be 3.
	 */
	public ExactVector(Rational[] values) {
		registerJavaObject(this);
		if ((values == null) || (values.length != 3)){
			throw new IllegalArgumentException("This class only permits vectors with 3 entries!");
		}
		this.elements = values;
	}
	
	/**
	 * Creates a new vector from an array of double values. The constructor {@link RationalJava#RationalJava(double)} is used
	 * to transform the given double values into rational values.
	 * @param values The entries for this vector. The length of this array is expected to be 3.
	 */
	public ExactVector(double[] values) {
		registerJavaObject(this);
		if ((values == null) || (values.length != 3)){
			throw new IllegalArgumentException("This class only permits vectors with 3 entries!");
		}
		this.elements = new Rational[] { new RationalFactory().create(values[0]), new RationalFactory().create(values[1]), new RationalFactory().create(values[2])};
	}

	/**
	 * Computes the square of the length of this vector. 
	 * @return The square of the Euclidean length of this vector.
	 */
	@Override
	public Rational squaredLength() {
		Rational ret = new RationalFactory().create(0,1);
		for (int i = 0; i < 3; i++) {
			ret.add(this.elements[i].multiply(this.elements[i]));
		}
		return ret;
	}

	@Override
	public ini.cx3d.spatialOrganization.interfaces.ExactVector add(ini.cx3d.spatialOrganization.interfaces.ExactVector other) {
		if (!(other instanceof ExactVector)) {
			throw new RuntimeException("otherValue must be of type ini.cx3d.spatialOrganization.ExactVector");
		}
		return	add((ExactVector) other);
	}

	@Override
	public ini.cx3d.spatialOrganization.interfaces.ExactVector increaseBy(ini.cx3d.spatialOrganization.interfaces.ExactVector other) {
		if (!(other instanceof ExactVector)) {
			throw new RuntimeException("otherValue must be of type ini.cx3d.spatialOrganization.ExactVector");
		}
		return	increaseBy((ExactVector) other);
	}

	@Override
	public ini.cx3d.spatialOrganization.interfaces.ExactVector subtract(ini.cx3d.spatialOrganization.interfaces.ExactVector other) {
		if (!(other instanceof ExactVector)) {
			throw new RuntimeException("otherValue must be of type ini.cx3d.spatialOrganization.ExactVector");
		}
		return	subtract((ExactVector) other);
	}

	@Override
	public ini.cx3d.spatialOrganization.interfaces.ExactVector decreaseBy(ini.cx3d.spatialOrganization.interfaces.ExactVector other) {
		if (!(other instanceof ExactVector)) {
			throw new RuntimeException("otherValue must be of type ini.cx3d.spatialOrganization.ExactVector");
		}
		return	decreaseBy((ExactVector) other);
	}

	/**
	 * Computes the sum of this vector with another vector and returns a new instance of <code>ExactVector</code>
	 * containing the result.
	 * @param other The second argument of the addition.
	 * @return A new instance of <code>ExactVector</code>.
	 */
	public ExactVector add(ExactVector other) {
//		if (anotherVector == null)
//			throw new IllegalArgumentException("Cannot add a vector that is null!");
		return new ExactVector(new Rational[] {
				elements[0].add(other.elements[0]),
				elements[1].add(other.elements[1]),
				elements[2].add(other.elements[2])});
	}

	/**
	 * Adds another vector to this vector.
	 * @param other The vector by which this vector should be increased.
	 * @return A reference to <code>this</code>, which has been modified. 
	 */
	public ExactVector increaseBy(ExactVector other) {
//		if (anotherVector == null)
//			throw new IllegalArgumentException("Cannot add a vector that is null!");
		for (int i = 0; i < elements.length; i++) {
			elements[i].increaseBy(other.elements[i]);
		}
		return this;
	}
	
	/**
	 * Computes the result of this vector minus another vector and returns a new instance of <code>ExactVector</code> 
	 * containing the result.
	 * @param other The second argument of the subtraction.
	 * @return A new instance of <code>ExactVector</code>.
	 */
	public ExactVector subtract(ExactVector other) {
//		if (anotherVector == null)
//			throw new IllegalArgumentException("Cannot add a vector that is null!");
		return new ExactVector(new Rational[] {
				elements[0].subtract(other.elements[0]),
				elements[1].subtract(other.elements[1]),
				elements[2].subtract(other.elements[2])});
	}

	/**
	 * Decreases this vector by another vector.
	 * @param other The vector by which this vector should be decreased.
	 * @return A reference to <code>this</code>, which has been modified. 
	 */
	public ExactVector decreaseBy(ExactVector other) {
//		if (anotherVector == null)
//			throw new IllegalArgumentException("Cannot add a vector that is null!");
		for (int i = 0; i < elements.length; i++) {
			elements[i].decreaseBy(other.elements[i]);
		}
		return this;
	}
	
	/**
	 * Computes the product of this vector with a rational number and returns a new instance of <code>ExactVector</code>
	 * containing the result. This vector itself remains unchanged.
	 * @param factor The constant by which all entries of this vector should be multiplied.
	 * @return A new instance of <code>ExactVector</code>.
	 */
	@Override
	public ExactVector multiply(Rational factor) {
		return new ExactVector(new Rational[] {
				elements[0].multiply(factor),
				elements[1].multiply(factor),
				elements[2].multiply(factor)});
	}
	
	/**
	 * Multiplies this vector with a rational number. The result is stored in this vector itself.
	 * @param factor The constant by which all entries of this vector should be multiplied.
	 * @return A reference to <code>this</code>, which has been modified. 
	 */
	@Override
	public ExactVector multiplyBy(Rational factor) {
		for (int i = 0; i < elements.length; i++) {
			elements[i].multiplyBy(factor);
		}
		return this;
	}
	
	/**
	 * Computes the division of this vector with a rational number and returns a new instance of <code>ExactVector</code>
	 * containing the result. This vector itself remains unchanged.
	 * @param factor The constant by which all entries of this vector should be divided.
	 * @return A new instance of <code>ExactVector</code>.
	 */
	@Override
	public ExactVector divide(Rational factor) {
		return new ExactVector(new Rational[] {
				elements[0].divide(factor),
				elements[1].divide(factor),
				elements[2].divide(factor)});
	}

	/**
	 * Divides this vector by a rational number. The result is stored in this vector itself.
	 * @param factor The constant by which all entries of this vector should be divided.
	 * @return A reference to <code>this</code>, which has been modified. 
	 */
	@Override
	public ExactVector divideBy(Rational factor) {
		for (int i = 0; i < elements.length; i++) {
			elements[i].divideBy(factor);
		}
		return this;
	}

	@Override
	public Rational dotProduct(ini.cx3d.spatialOrganization.interfaces.ExactVector other) {
		if (!(other instanceof ExactVector)) {
			throw new RuntimeException("otherValue must be of type ini.cx3d.spatialOrganization.ExactVector");
		}
		return	dotProduct((ExactVector) other);
	}

	/**
	 * Computes the dot product of this vector with another one. The result is stored in a new instance of <code>RationalJava</code>.
	 * @param other The other argument of this dot product.
	 * @return A new instance of <code>RationalJava</code> containing the computed dot product.
	 */
	public Rational dotProduct(ExactVector other) {
		Rational ret = new RationalFactory().create(0, 1);
		for (int i = 0; i < elements.length; i++) {
			ret = ret.add(other.elements[i].multiply(this.elements[i]));
		}
		return ret;
	}

	/**
	 * Multiplies this vector by -1. This vector itself is modified during that process.
	 * @return A reference to this vector itself. 
	 */
	@Override
	public ExactVector negate() {
		for (int i = 0; i < elements.length; i++) {
			elements[i].negate();
		}
		return this;
	}

	@Override
	public ini.cx3d.spatialOrganization.interfaces.ExactVector crossProduct(ini.cx3d.spatialOrganization.interfaces.ExactVector other) {
		if (!(other instanceof ExactVector)) {
			throw new RuntimeException("otherValue must be of type ini.cx3d.spatialOrganization.ExactVector");
		}
		return	crossProduct((ExactVector) other);
	}

	/**
	 * Returns a string representation of this vector.
	 * @return A <code>String</code> of the format "(element1, element2, element3)". 
	 */
	public String toString() {
		String ret = "(";
		for (int i = 0; i < elements.length; i++) {
			if (i != 0)
				ret += ", ";
			ret += elements[i].toString();
		}
		ret += ")";
		return ret;
	}

	/**
	 * Returns the cross-product of this vector and another.
	 * 
	 * @param  other The vector with which the cross-product should be calculated
	 * @return The cross-product of this vector and <code>other<\code>, stored in a new instance of <code>ExactVector</code>
	 */
	public ExactVector crossProduct(ExactVector other) {
		if (other == null) {
			throw new IllegalArgumentException ( "Attempt to compute the cross product with a vector that is null!");
		}
		Rational[] result = new Rational[elements.length];
		for (int i = 0; i < elements.length; i++) 
			result[i] = this.elements[((i+1)%3)].multiply(other.elements[((i+2)%3)]).subtract(
					    this.elements[((i+2)%3)].multiply(other.elements[((i+1)%3)]));
		return new ExactVector(result);
	}
	
	/**
	 * Computes the determinant of a 3x3 matrix. 
	 * @param vector An array of size 3 which contains the column vectors of the matrix.
	 * @return A new instance of <code>RationalJava</code> containing the determinant of the specified matrix.
	 */
	public static Rational det(ini.cx3d.spatialOrganization.interfaces.ExactVector[] vector) {
		if ((vector == null) || (vector.length != 3))
			throw new IllegalArgumentException("This function can only calculate the determinant of 3 vectors of size 3!");

		ExactVector[] c = new ExactVector[3];
		for (int i = 0; i < 3; i++) {
			if (!(vector[i] instanceof ExactVector)) {
				throw new RuntimeException("vector must be of type ini.cx3d.spatialOrganization.ExactVector");
			}
			c[i] = (ExactVector) vector[i];
		}

		return 	c[0].elements[0].multiply(c[1].elements[1]).multiply(c[2].elements[2]).add(
				c[0].elements[1].multiply(c[1].elements[2]).multiply(c[2].elements[0])).add(
				c[0].elements[2].multiply(c[1].elements[0]).multiply(c[2].elements[1])).subtract(
				c[0].elements[0].multiply(c[1].elements[2]).multiply(c[2].elements[1])).subtract(
				c[0].elements[1].multiply(c[1].elements[0]).multiply(c[2].elements[2])).subtract(
				c[0].elements[2].multiply(c[1].elements[1]).multiply(c[2].elements[0]));
	}
}
