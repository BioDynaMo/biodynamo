package ini.cx3d.physics.interfaces;

import ini.cx3d.SimStateSerializable;
import ini.cx3d.xml.XMLSerializable;
import org.w3c.dom.Node;

import java.awt.*;
import java.io.Serializable;
import java.util.concurrent.locks.ReadWriteLock;

/**
 * common interface for Substance
 */
public interface Substance extends Serializable, SimStateSerializable {
	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

	/**
	 * Increases or decreases the quantity. Makes sure the quantity is never negative.
	 * @param deltaQ
	 */
	void changeQuantityFrom(double deltaQ);

	/** Well, as the name says, it multiplies the quantity and the concentration
	 * by a certain value. This method is mainly used for degradation .*/
	void multiplyQuantityAndConcentrationBy(double factor);

	/**
	 * Computes the quantity represented when the current concentration is maintained
	 * but the volume changes. Important when PhysicalNodes are moved.
	 * @param volume
	 */
	void updateQuantityBasedOnConcentration(double volume);

	/**
	 * Computes the new concentration if the current quantity is distributed in a given volume.
	 *  Important when PhysicalNodes are moved.
	 * @param volume
	 */
	void updateConcentrationBasedOnQuantity(double volume);

	/**
	 * Determines whether an other object is equal to this Substance.
	 * <br>The result is <code>true</code> if and only if the argument
	 * is not null and is a Substance object with the same id, color,
	 * degradationConstant and diffusionConstant. The
	 * quantity and concentration are note taken into account.
	 */
	boolean equals(Object o);

	/**
	 * Returns the color scaled by the concentration. Useful for painting PhysicalObjects / PhysicalNode
	 * based on their Substance concentrations.
	 * @return scaled Color
	 */
	Color getConcentrationDependentColor();

	String getId();

	void setId(String id);

	double getDiffusionConstant();

	void setDiffusionConstant(double diffusionConstant);

	double getDegradationConstant();

	void setDegradationConstant(double degradationConstant);

	Color getColor();

	void setColor(Color color);

	double getConcentration();

	void setConcentration(double concentration);

	double getQuantity();

	void setQuantity(double quantity);

//	XMLSerializable fromXML(Node xml);
//
//	StringBuilder toXML(String ident);
//
//	ReadWriteLock getRwLock();

	Substance getCopy();
}
