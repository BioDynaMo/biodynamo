package ini.cx3d.localBiology.interfaces;

import ini.cx3d.SimStateSerializable;
import ini.cx3d.cells.Cell;
import ini.cx3d.localBiology.LocalBiologyModule;

import java.util.AbstractSequentialList;
import java.util.Vector;

/**
 * Created by lukas on 23.03.16.
 */
public interface CellElement extends SimStateSerializable {
	/** Adds the argument to the <code>LocalBiologyModule</code> list, and registers this as it's
	 * <code>CellElements</code>.*/
	void addLocalBiologyModule(LocalBiologyModule m);

	/** Removes the argument from the <code>LocalBiologyModule</code> list.*/
	void removeLocalBiologyModule(LocalBiologyModule m);

	/** Removes all the <code>LocalBiologyModule</code> in this <code>CellElements</code>.*/
	void cleanAllLocalBiologyModules();

	/** Returns the localBiologyModule List (not a copy).*/
	AbstractSequentialList<LocalBiologyModule> getLocalBiologyModulesList();

	/** Sets the localBiologyModule List.*/
	void setLocalBiologyModulesList(AbstractSequentialList<LocalBiologyModule> localBiologyModulesList);

	/**
	 * Sets the <code>Cell</code> this <code>CellElement</code> is part of.
	 * @param cell
	 */
	void setCell(Cell cell);

	/**
	 *
	 * @return the <code>Cell</code> this <code>CellElement</code> is part of.
	 */
	Cell getCell();

	/** Returns <code>true</code> if is a <code>NeuriteElement</code>.*/
	boolean isANeuriteElement();

	/** Returns <code>true</code> if is a <code>SomaElement</code>.*/
	boolean isASomaElement();

	ini.cx3d.physics.interfaces.PhysicalObject getPhysical();

	void setPhysical(ini.cx3d.physics.interfaces.PhysicalObject po);

	void move(double speed, double[] direction);
}
