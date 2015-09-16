package ini.cx3d.spatialOrganization.interfaces;

/**
 * Common interface for ExactVector implementations
 */
public interface ExactVector {
	/**
	 * Computes the square of the length of this vector.
	 * @return The square of the Euclidean length of this vector.
	 */
	Rational squaredLength();

	/**
	 * Computes the sum of this vector with another vector and returns a new instance of <code>ExactVector</code>
	 * containing the result.
	 * @param other The second argument of the addition.
	 * @return A new instance of <code>ExactVector</code>.
	 */
	ExactVector add(ExactVector other);

	/**
	 * Adds another vector to this vector.
	 * @param other The vector by which this vector should be increased.
	 * @return A reference to <code>this</code>, which has been modified.
	 */
	ExactVector increaseBy(ExactVector other);

	/**
	 * Computes the result of this vector minus another vector and returns a new instance of <code>ExactVector</code>
	 * containing the result.
	 * @param other The second argument of the subtraction.
	 * @return A new instance of <code>ExactVector</code>.
	 */
	ExactVector subtract(ExactVector other);

	/**
	 * Decreases this vector by another vector.
	 * @param other The vector by which this vector should be decreased.
	 * @return A reference to <code>this</code>, which has been modified.
	 */
	ExactVector decreaseBy(ExactVector other);

	/**
	 * Computes the product of this vector with a rational number and returns a new instance of <code>ExactVector</code>
	 * containing the result. This vector itself remains unchanged.
	 * @param factor The constant by which all entries of this vector should be multiplied.
	 * @return A new instance of <code>ExactVector</code>.
	 */
	ExactVector multiply(Rational factor);

	/**
	 * Multiplies this vector with a rational number. The result is stored in this vector itself.
	 * @param factor The constant by which all entries of this vector should be multiplied.
	 * @return A reference to <code>this</code>, which has been modified.
	 */
	ExactVector multiplyBy(Rational factor);

	/**
	 * Computes the division of this vector with a rational number and returns a new instance of <code>ExactVector</code>
	 * containing the result. This vector itself remains unchanged.
	 * @param factor The constant by which all entries of this vector should be divided.
	 * @return A new instance of <code>ExactVector</code>.
	 */
	ExactVector divide(Rational factor);

	/**
	 * Divides this vector by a rational number. The result is stored in this vector itself.
	 * @param factor The constant by which all entries of this vector should be divided.
	 * @return A reference to <code>this</code>, which has been modified.
	 */
	ExactVector divideBy(Rational factor);

	/**
	 * Computes the dot product of this vector with another one. The result is stored in a new instance of <code>RationalJava</code>.
	 * @param other The other argument of this dot product.
	 * @return A new instance of <code>RationalJava</code> containing the computed dot product.
	 */
	Rational dotProduct(ExactVector other);

	/**
	 * Multiplies this vector by -1. This vector itself is modified during that process.
	 * @return A reference to this vector itself.
	 */
	ExactVector negate();

	/**
	 * Returns the cross-product of this vector and another.
	 *
	 * @param  other The vector with which the cross-product should be calculated
	 * @return The cross-product of this vector and <code>other<\code>, stored in a new instance of <code>ExactVector</code>
	 */
	ExactVector crossProduct(ExactVector other);
}
