package ini.cx3d.spatialOrganization.interfaces;

/**
 * Common interface for Rational implementations
 */
public interface Rational {
	String toString();

	boolean isZero();

	Rational negate();

	Rational add(Rational otherValue);

	Rational increaseBy(Rational otherValue);

	Rational subtract(Rational otherValue);

	Rational decreaseBy(Rational otherValue);

	Rational multiply(Rational otherValue);

	Rational multiplyBy(Rational otherValue);

	Rational divide(Rational otherValue);

	Rational divideBy(Rational otherValue);

	double doubleValue();

	int compareTo(Rational obj);
}
