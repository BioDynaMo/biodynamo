package ini.cx3d.simulations.interfaces;

import ini.cx3d.SimStateSerializable;
import ini.cx3d.graphics.View;
import ini.cx3d.localBiology.interfaces.NeuriteElement;
import ini.cx3d.physics.ECMChemicalReaction;
import ini.cx3d.physics.interfaces.PhysicalCylinder;
import ini.cx3d.physics.interfaces.PhysicalNode;
import ini.cx3d.physics.interfaces.PhysicalSphere;
import ini.cx3d.physics.interfaces.Substance;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
import ini.cx3d.spatialOrganization.interfaces.SpaceNode;

import java.awt.*;
import java.util.AbstractSequentialList;
import java.util.Hashtable;

/**
 * Created by lukas on 25.04.16.
 */
public interface ECM extends SimStateSerializable {

	int getPhysicalNodeListSize();

	int getPhysicalCylinderListSize();

	int getPhysicalSphereListSize();

	int getSomaElementListSize();

	int getNeuriteElementListSize();

	int getCellListSize();

	 ini.cx3d.cells.interfaces.Cell getCell(int i);

	double getRandomDouble1();

	void viewRepaint();

	ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb);

//	double getRandomDouble();

//	double getGaussianDouble(double mean, double standardDeviation);

	void setBoundaries(double Xmin, double Xmax, double Ymin, double Ymax, double Zmin, double Zmax);

	void setArtificialWallsForSpheres(boolean artificialWallsForSpheres);

	boolean getArtificialWallForSpheres();

	void setArtificialWallsForCylinders(boolean artificialWallsForCylinders);

	boolean getArtificialWallForCylinders();

	double[] forceFromArtificialWall(double[] location, double radius);

	SpaceNode<PhysicalNode> getSpatialOrganizationNodeInstance(double[] position, PhysicalNode userObject);

	SpaceNode<PhysicalNode> getSpatialOrganizationNodeInstance(
			SpaceNode<PhysicalNode> n, double[] position, PhysicalNode userObject);

	void addGridOfPhysicalNodes(double x1, double x2, double y1, double y2, double z1, double z2, double d);

	PhysicalNode getPhysicalNodeInstance(double[] nodeLocation);

	void addPhysicalCylinder(PhysicalCylinder newCylinder);

	void removePhysicalCylinder(PhysicalCylinder oldCylinder);

	void addPhysicalSphere(ini.cx3d.physics.interfaces.PhysicalSphere newSphere);

	void removePhysicalSphere(ini.cx3d.physics.interfaces.PhysicalSphere oldSphere);

	void addPhysicalNode(PhysicalNode newPhysicalNode);

	void removePhysicalNode(PhysicalNode oldPhysicalNode);

	void addECMChemicalReaction(ECMChemicalReaction chemicalReaction);

	void removeECMChemicalReaction(ECMChemicalReaction chemicalReaction);

	void addCell(ini.cx3d.cells.interfaces.Cell newCell);

	void removeCell(ini.cx3d.cells.interfaces.Cell oldCell);

	// Cell Elements--------------------------------------------------
	void addSomaElement(ini.cx3d.localBiology.interfaces.SomaElement newSoma);

	void removeSomaElement(ini.cx3d.localBiology.interfaces.SomaElement oldSoma);

	void addNeuriteElement(ini.cx3d.localBiology.interfaces.NeuriteElement newNE);

	void removeNeuriteElement(ini.cx3d.localBiology.interfaces.NeuriteElement oldNE);

	void resetTime();

	void clearAll();

	void addNewSubstanceTemplate(ini.cx3d.physics.interfaces.Substance s);

	void addNewIntracellularSubstanceTemplate(ini.cx3d.physics.interfaces.IntracellularSubstance s);

	ini.cx3d.physics.interfaces.Substance substanceInstance(String id);

	ini.cx3d.physics.interfaces.IntracellularSubstance intracellularSubstanceInstance(String id);

	// *********************************************************************
	// *** Pre-defined cellType colors
	// *********************************************************************
	void addNewCellTypeColor(String cellType, Color color);

	Color cellTypeColor(String cellType);

	boolean thereAreArtificialGradients();

	void addArtificialGaussianConcentrationZ(ini.cx3d.physics.interfaces.Substance substance, double maxConcentration, double zCoord, double sigma);

	void addArtificialGaussianConcentrationZ(String substanceName, double maxConcentration, double zCoord, double sigma);

	void addArtificialLinearConcentrationZ(ini.cx3d.physics.interfaces.Substance substance, double maxConcentration, double zCoordMax, double zCoordMin);

	void addArtificialLinearConcentrationZ(String substanceName, double maxConcentration, double zCoordMax, double zCoordMin);

	void addArtificialGaussianConcentrationX(ini.cx3d.physics.interfaces.Substance substance, double maxConcentration, double xCoord, double sigma);

	void addArtificialGaussianConcentrationX(String substanceName, double maxConcentration, double xCoord, double sigma);

	void addArtificialLinearConcentrationX(ini.cx3d.physics.interfaces.Substance substance, double maxConcentration, double xCoordMax, double xCoordMin);

	void addArtificialLinearConcentrationX(String substanceId, double maxConcentration, double xCoordMax, double xCoordMin);

	double getValueArtificialConcentration(String nameOfTheChemical, double[] position);

	double getValueArtificialConcentration(ini.cx3d.physics.interfaces.Substance substance, double[] position);

	double[] getGradientArtificialConcentration(String nameOfTheChemical, double[] position);

	double getGradientArtificialConcentration(ini.cx3d.physics.interfaces.Substance s, double[] position);

	// **************************************************************************
	// Simulation Time
	// **************************************************************************
	String nicelyWrittenECMtime();

	// **************************************************************************
	// GUI & pause
	// **************************************************************************
	void createGUI();

	View createGUI(int x, int y, int width, int height);

	void dumpImage();

	void dumpImage(String name);

	AbstractSequentialList<PhysicalNode> getPhysicalNodeList();

	AbstractSequentialList<ini.cx3d.cells.interfaces.Cell> getCellList();

	View getView();

	void setView(View view);

	boolean isSimulationOnPause();

	void setSimulationOnPause(boolean simulationOnPause);

	boolean isContinuouslyRotating();

	void setContinuouslyRotating(boolean continuouslyRotating);

	boolean isTakingSnapshotAtEachTimeStep();

	void setTakingSnapshotAtEachTimeStep(boolean takingSnapshotAtEachTimeStep);

	boolean isTakingSnapshotEach100TimeSteps();

	void setTakingSnapshotEach100TimeSteps(boolean takingSnapshotEach100TimeSteps);

	AbstractSequentialList<PhysicalSphere> getPhysicalSphereList();

	AbstractSequentialList<PhysicalCylinder> getPhysicalCylinderList();

	PhysicalCylinder getPhysicalCylinder(int i);

	ini.cx3d.localBiology.interfaces.NeuriteElement getNeuriteElement(int i);

	PhysicalSphere getPhysicalSphere(int i);

	PhysicalNode getPhysicalNode(int i);

	ini.cx3d.localBiology.interfaces.SomaElement getSomaElement(int i);

	AbstractSequentialList<NeuriteElement> getNeuriteElementList();

	boolean isAnyArtificialGradientDefined();

	Hashtable<Substance, double[]> getGaussianArtificialConcentrationZ();

	Hashtable<Substance, double[]> getLinearArtificialConcentrationZ();

	Hashtable<Substance, double[]> getGaussianArtificialConcentrationX();

	Hashtable<Substance, double[]> getLinearArtificialConcentrationX();

	double getECMtime();

	void setECMtime(double ECMtime);

	void increaseECMtime(double deltaT);

	void setCellList(AbstractSequentialList<ini.cx3d.cells.interfaces.Cell> cellList);

	void setIsLast(boolean b);

	Hashtable<String, ini.cx3d.physics.interfaces.IntracellularSubstance> getIntracelularSubstanceTemplates();

	Hashtable<String, Substance> getSubstanceTemplates();

	double[] getMinBounds();

	double[] getMaxBounds();

	void saveToFile(String file);

	void setPicturesName(String picturesName);

	String getPicturesName();

	AbstractSequentialList<ini.cx3d.localBiology.interfaces.SomaElement> getSomaElementList();
}
