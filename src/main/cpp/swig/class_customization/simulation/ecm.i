/**
 * This file contains code generation customizations for class ECM.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations, based on two preprocessor
 * variables. (ECM_NATIVE and ECM_DEBUG)
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/simulation/ecm.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"

%define %ECM_cx3d_shared_ptr()
  %cx3d_shared_ptr(ECM,
                   ini/cx3d/simulations/interfaces/ECM,
                   cx3d::simulation::ECM);
%enddef

%define %ECM_java()
  %java_defined_class(cx3d::simulation::ECM,
                      ECM,
                      ECM,
                      ini.cx3d.simulations.interfaces.ECM,
                      ini/cx3d/simulations/interfaces/ECM);
  %typemap(javacode) cx3d::simulation::ECM %{
    private static java.util.Map<Long, ini.cx3d.simulations.interfaces.ECM> javaObjectMap = new java.util.HashMap<Long, ini.cx3d.simulations.interfaces.ECM>();

    /**
     * This method must be called in each constructor of the Java Defined Class.
     * It registers the newly created Java object. Thus it is possible to retrieve
     * it if it is returned from the C++ side, or needed as a parameter in a Director
     * method.
     */
    public static synchronized void registerJavaObject(ini.cx3d.simulations.interfaces.ECM o) {
      long objPtr = ((ECM) o).getObjectPtr(o);
      javaObjectMap.put(objPtr, o);
    }

    /**
     * Returns the Java object given the C++ object pointer
     */
    public static synchronized ini.cx3d.simulations.interfaces.ECM getJavaObject(long objPtr) {
      if (objPtr == 0){
        return null;
      }
      else {
        return javaObjectMap.get(objPtr);
      }
    }


      @Override
      public void addArtificialGaussianConcentrationZ(ini.cx3d.physics.interfaces.Substance substance, double maxConcentration, double zCoord, double sigma) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addArtificialGaussianConcentrationZ(String substanceName, double maxConcentration, double zCoord, double sigma) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addArtificialLinearConcentrationZ(ini.cx3d.physics.interfaces.Substance substance, double maxConcentration, double zCoordMax, double zCoordMin) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addArtificialLinearConcentrationZ(String substanceName, double maxConcentration, double zCoordMax, double zCoordMin) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addArtificialGaussianConcentrationX(ini.cx3d.physics.interfaces.Substance substance, double maxConcentration, double xCoord, double sigma) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addArtificialGaussianConcentrationX(String substanceName, double maxConcentration, double xCoord, double sigma) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addArtificialLinearConcentrationX(ini.cx3d.physics.interfaces.Substance substance, double maxConcentration, double xCoordMax, double xCoordMin) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addArtificialLinearConcentrationX(String substanceId, double maxConcentration, double xCoordMax, double xCoordMin) {
          throw new UnsupportedOperationException();
      }

      @Override
      public double getValueArtificialConcentration(ini.cx3d.physics.interfaces.Substance substance, double[] position) {
          throw new UnsupportedOperationException();
      }
      @Override
      public void setECMtime(double ECMtime) {
          throw new UnsupportedOperationException();
      }
      @Override
      public double getGradientArtificialConcentration(ini.cx3d.physics.interfaces.Substance s, double[] position) {
          throw new UnsupportedOperationException();
      }

      @Override
      public String nicelyWrittenECMtime() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addNewCellTypeColor(String cellType, Color color) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void removePhysicalSphere(ini.cx3d.physics.interfaces.PhysicalSphere oldSphere) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addPhysicalNode(ini.cx3d.physics.interfaces.PhysicalNode newPhysicalNode) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void removePhysicalNode(ini.cx3d.physics.interfaces.PhysicalNode oldPhysicalNode) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addECMChemicalReaction(ECMChemicalReaction chemicalReaction) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void removeECMChemicalReaction(ECMChemicalReaction chemicalReaction) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void resetTime() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void clearAll() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addNewSubstanceTemplate(ini.cx3d.physics.interfaces.Substance s) {

      }

      @Override
      public void addNewIntracellularSubstanceTemplate(ini.cx3d.physics.interfaces.IntracellularSubstance s) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setArtificialWallsForCylinders(boolean artificialWallsForCylinders) {
          throw new UnsupportedOperationException();
      }

      @Override
      public boolean isAnyArtificialGradientDefined() {
          throw new UnsupportedOperationException();
      }

      @Override
      public Hashtable<ini.cx3d.physics.interfaces.Substance, double[]> getGaussianArtificialConcentrationZ() {
          throw new UnsupportedOperationException();
      }

      @Override
      public Hashtable<ini.cx3d.physics.interfaces.Substance, double[]> getLinearArtificialConcentrationZ() {
          throw new UnsupportedOperationException();
      }

      @Override
      public Hashtable<ini.cx3d.physics.interfaces.Substance, double[]> getGaussianArtificialConcentrationX() {
          throw new UnsupportedOperationException();
      }

      @Override
      public Hashtable<ini.cx3d.physics.interfaces.Substance, double[]> getLinearArtificialConcentrationX() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void removeSomaElement(ini.cx3d.localBiology.interfaces.SomaElement oldSoma) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void removeCell(ini.cx3d.cells.interfaces.Cell oldCell) {
          throw new UnsupportedOperationException();
      }

      @Override
      public SpatialOrganizationNode<ini.cx3d.physics.interfaces.PhysicalNode> getSpatialOrganizationNodeInstance(SpatialOrganizationNode<ini.cx3d.physics.interfaces.PhysicalNode> n, double[] position, ini.cx3d.physics.interfaces.PhysicalNode userObject) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void addGridOfPhysicalNodes(double x1, double x2, double y1, double y2, double z1, double z2, double d) {
          throw new UnsupportedOperationException();
      }

      @Override
      public ini.cx3d.physics.interfaces.PhysicalNode getPhysicalNodeInstance(double[] nodeLocation) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setBoundaries(double Xmin, double Xmax, double Ymin, double Ymax, double Zmin, double Zmax) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setArtificialWallsForSpheres(boolean artificialWallsForSpheres) {
          throw new UnsupportedOperationException();
      }
      @Override
      public View getView() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setView(View view) {
          throw new UnsupportedOperationException();
      }

      @Override
      public boolean isSimulationOnPause() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setSimulationOnPause(boolean simulationOnPause) {
          throw new UnsupportedOperationException();
      }

      @Override
      public boolean isContinuouslyRotating() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setContinuouslyRotating(boolean continuouslyRotating) {
          throw new UnsupportedOperationException();
      }

      @Override
      public boolean isTakingSnapshotAtEachTimeStep() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setTakingSnapshotAtEachTimeStep(boolean takingSnapshotAtEachTimeStep) {
          throw new UnsupportedOperationException();
      }

      @Override
      public boolean isTakingSnapshotEach100TimeSteps() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setTakingSnapshotEach100TimeSteps(boolean takingSnapshotEach100TimeSteps) {
          throw new UnsupportedOperationException();
      }
      @Override
      public void setCellList(AbstractSequentialList<ini.cx3d.cells.interfaces.Cell> cellList) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setIsLast(boolean b) {
          throw new UnsupportedOperationException();
      }

      @Override
      public Hashtable<String, ini.cx3d.physics.interfaces.IntracellularSubstance> getIntracelularSubstanceTemplates() {
          throw new UnsupportedOperationException();
      }

      @Override
      public Hashtable<String, ini.cx3d.physics.interfaces.Substance> getSubstanceTemplates() {
          throw new UnsupportedOperationException();
      }

      @Override
      public double[] getMinBounds() {
          throw new UnsupportedOperationException();
      }

      @Override
      public double[] getMaxBounds() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void saveToFile(String file) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void setPicturesName(String picturesName) {
          throw new UnsupportedOperationException();
      }

      @Override
      public String getPicturesName() {
          throw new UnsupportedOperationException();
      }

      @Override
      public NativeStringBuilder simStateToJson(NativeStringBuilder sb) {
          throw new UnsupportedOperationException();
      }

      @Override
      public View createGUI(int x, int y, int width, int height) {
          throw new UnsupportedOperationException();
      }

      @Override
      public void dumpImage() {
          throw new UnsupportedOperationException();
      }

      @Override
      public void dumpImage(String name) {
          throw new UnsupportedOperationException();
      }

      @Override
      public ini.cx3d.localBiology.interfaces.NeuriteElement getNeuriteElement(int i) {
          throw new UnsupportedOperationException();
      }

      @Override
      public ini.cx3d.localBiology.interfaces.SomaElement getSomaElement(int i) {
          throw new UnsupportedOperationException();
      }
      @Override
      public void addNewCellTypeColor(String cellType, java.awt.Color color) {
          throw new UnsupportedOperationException();
      }
  %}
%enddef

%define %ECM_native()
  %native_defined_class(cx3d::simulations::ECM,
                      ECM,
                      ini.cx3d.simulations.interfaces.ECM,
                      ECM,
                      ;);
%enddef

/**
 * apply customizations
 */
%ECM_cx3d_shared_ptr();
#ifdef ECM_NATIVE
%pragma(java) modulecode=%{
    public static boolean useNativeECM = true;
%}
#else
// %pragma(java) modulecode=%{
//     public static boolean useNativeECM = false;
// %}
  %ECM_java();
#endif
%typemap(javainterfaces) cx3d::simulation::ECM "ini.cx3d.simulations.interfaces.ECM"
%typemap(javaimports) cx3d::simulation::ECM %{
  import ini.cx3d.swig.NativeStringBuilder;
  import ini.cx3d.swig.spatialOrganization.SpaceNodeT_PhysicalNode;
  import ini.cx3d.swig.physics.*;

  import ini.cx3d.graphics.View;
  import ini.cx3d.physics.ECMChemicalReaction;
  import ini.cx3d.physics.interfaces.*;
  import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
  import ini.cx3d.swig.physics.IntracellularSubstance;
  import ini.cx3d.swig.physics.PhysicalBond;
  import ini.cx3d.swig.physics.PhysicalCylinder;
  import ini.cx3d.swig.physics.PhysicalNode;
  import ini.cx3d.swig.physics.PhysicalObject;
  import ini.cx3d.swig.physics.PhysicalSphere;
  import ini.cx3d.swig.physics.Substance;

  import java.util.AbstractSequentialList;
  import java.util.Hashtable;
%}

%ignore cx3d::simulation::ECM::getGaussianArtificialConcentrationZ;
%ignore cx3d::simulation::ECM::getLinearArtificialConcentrationZ;
%ignore cx3d::simulation::ECM::getGaussianArtificialConcentrationX;
%ignore cx3d::simulation::ECM::getLinearArtificialConcentrationX;
%ignore cx3d::simulation::ECM::getIntracelularSubstanceTemplates;
%ignore cx3d::simulation::ECM::getSubstanceTemplates;

// %ignore cx3d::simulation::ECM::getCellList;
// %ignore cx3d::simulation::ECM::getPhysicalNodeList;
// %ignore cx3d::simulation::ECM::getPhysicalSphereList;
// %ignore cx3d::simulation::ECM::getPhysicalCylinderList;
// %ignore cx3d::simulation::ECM::getNeuriteElementList;
// %ignore cx3d::simulation::ECM::getSomaElementList;
