/**
 * This file contains code generation customizations for class PhysicalNodeMovementListener.
 * At the top of this file it defines partial macro applications by fixing the
 * value of a number of arguments. Therefore, this macros are easier to use.
 * https://en.wikipedia.org/wiki/Partial_application
 * At the bottom it executes the customizations.
 * Documentation of macros and their arguments can be found in the included
 * files!
 *
 * SWIG modules that use the class simply include it:
 * %include "class_customization/physical_organization_node_movement_listener.i"
 */

%include "util.i"
%include "cx3d_shared_ptr.i"
%include "std_list_typemap.i"
%include "std_array_typemap.i"

%define %PhysicalNodeMovementListener_cx3d_shared_ptr()
  %cx3d_shared_ptr(PhysicalNodeMovementListener,
                   ini/cx3d/spatialOrganization/SpatialOrganizationNodeMovementListener,
                   cx3d::physics::PhysicalNodeMovementListener);
%enddef

%define %PhysicalNodeMovementListener_java()
  %java_defined_class(cx3d::physics::PhysicalNodeMovementListener,
                      PhysicalNodeMovementListener,
                      PhysicalNodeMovementListener,
                      ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener,
                      ini/cx3d/spatialOrganization/SpatialOrganizationNodeMovementListener);
%enddef

%define %PhysicalNodeMovementListener_native()
  %native_defined_class(cx3d::physics::PhysicalNodeMovementListener,
                      PhysicalNodeMovementListener,
                      ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener,
                      PhysicalNodeMovementListener,
                      ;);
%enddef

%define %PhysicalNodeMovementListener_stdlist()
  %stdlist_typemap(std::shared_ptr<cx3d::physics::PhysicalNodeMovementListener>,
                   PhysicalNodeMovementListener,
                   ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener);
%enddef

/**
 * apply customizations
 */
%PhysicalNodeMovementListener_cx3d_shared_ptr();
#ifdef PHYSICALNODEMOVEMENTLISTENER_NATIVE
  %PhysicalNodeMovementListener_native();
#else
  %PhysicalNodeMovementListener_java();
#endif
#ifdef PHYSICALNODEMOVEMENTLISTENER_DEBUG
  %setJavaDebugSwitch(PhysicalNodeMovementListener, true);
#else
  %setJavaDebugSwitch(PhysicalNodeMovementListener, false);
#endif
%PhysicalNodeMovementListener_stdlist();
%typemap(javabase) cx3d::physics::PhysicalNodeMovementListener "ini.cx3d.swig.simulation.SpatialOrganizationNodeMovementListenerT_PhysicalNode"
%typemap(javaimports) cx3d::physics::PhysicalNodeMovementListener "import ini.cx3d.swig.simulation.SpaceNodeT_PhysicalNode;"

// manual modifications because SWIG didn't pick up that PhysicalNodeMovementListener is a derived class
%typemap(javabody) cx3d::physics::PhysicalNodeMovementListener %{
  private long swigCPtr;
  private boolean swigCMemOwnDerived;

  protected PhysicalNodeMovementListener(long cPtr, boolean cMemoryOwn) {
    super(cPtr, true);
    swigCMemOwnDerived = cMemoryOwn;
    swigCPtr = cPtr;
  }

  public static long getCPtr(Object o) {
    if(!(o instanceof PhysicalNodeMovementListener)){
      throw new RuntimeException("Object " + o + " must be of type PhysicalNodeMovementListener. Use Proxy to wrap this object");
    }
    PhysicalNodeMovementListener obj = (PhysicalNodeMovementListener) o;
    return (obj == null) ? 0 : obj.swigCPtr;
  }

  public static PhysicalNodeMovementListener swigCreate(long cPtr, boolean cMemoryOwn) {
    return new PhysicalNodeMovementListener(cPtr, cMemoryOwn);
  }
%}
