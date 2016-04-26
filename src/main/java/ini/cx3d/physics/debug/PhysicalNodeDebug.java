package ini.cx3d.physics.debug;


import ini.cx3d.physics.PhysicalNode;
import ini.cx3d.physics.interfaces.Substance;
import ini.cx3d.spatialOrganization.interfaces.SpaceNode;
import ini.cx3d.swig.simulation.simulation;
import ini.cx3d.utilities.DebugUtil;

import java.util.AbstractSequentialList;
import java.util.Hashtable;
import java.util.concurrent.locks.ReadWriteLock;

public class PhysicalNodeDebug extends PhysicalNode{
    public PhysicalNodeDebug() {
        super();
//        DebugUtil.logMethodCall("PhysicalNode created", this, new Object[]{});
    }

    @Override
    public boolean isAPhysicalObject() {
        DebugUtil.logMethodCall("isAPhysicalObject", this, new Object[]{});
        boolean ret =  super.isAPhysicalObject();
        DebugUtil.logMethodReturn("isAPhysicalObject", this, ret);
        return ret;

    }

    @Override
    public boolean isAPhysicalCylinder() {
        DebugUtil.logMethodCall("isAPhysicalCylinder", this, new Object[]{});
        boolean ret =  super.isAPhysicalCylinder();
        DebugUtil.logMethodReturn("isAPhysicalCylinder", this, ret);
        return ret;

    }

    @Override
    public boolean isAPhysicalSphere() {
        DebugUtil.logMethodCall("isAPhysicalSphere", this, new Object[]{});
        boolean ret =  super.isAPhysicalSphere();
        DebugUtil.logMethodReturn("isAPhysicalSphere", this, ret);
        return ret;

    }

    @Override
    public double getExtracellularConcentration(String id) {
        DebugUtil.logMethodCall("getExtracellularConcentration", this, new Object[]{id});
        double ret =  super.getExtracellularConcentration(id);
        DebugUtil.logMethodReturn("getExtracellularConcentration", this, ret);
        return ret;

    }

    @Override
    public double getConvolvedConcentration(String id) {
        DebugUtil.logMethodCall("getConvolvedConcentration", this, new Object[]{id});
        double ret =  super.getConvolvedConcentration(id);
        DebugUtil.logMethodReturn("getConvolvedConcentration", this, ret);
        return ret;

    }

    @Override
    public double getExtracellularConcentration(String id, double[] location) {
        DebugUtil.logMethodCall("getExtracellularConcentration", this, new Object[]{id, location});
        double ret =  super.getExtracellularConcentration(id, location);
        DebugUtil.logMethodReturn("getExtracellularConcentration", this, ret);
        return ret;

    }

    @Override
    public double[] getExtracellularGradient(String id) {
        DebugUtil.logMethodCall("getExtracellularGradient", this, new Object[]{id});
        double[] ret =  super.getExtracellularGradient(id);
        DebugUtil.logMethodReturn("getExtracellularGradient", this, ret);
        return ret;

    }

    @Override
    public void modifyExtracellularQuantity(String id, double quantityPerTime) {
        DebugUtil.logMethodCall("modifyExtracellularQuantity", this, new Object[]{id, quantityPerTime});
        super.modifyExtracellularQuantity(id, quantityPerTime);
        DebugUtil.logMethodReturnVoid("modifyExtracellularQuantity", this);

    }

    @Override
    public void runExtracellularDiffusion() {
        DebugUtil.logMethodCall("runExtracellularDiffusion", this, new Object[]{});
        super.runExtracellularDiffusion();
        DebugUtil.logMethodReturnVoid("runExtracellularDiffusion", this);

    }

    @Override
    public void degradate(double currentEcmTime) {
        DebugUtil.logMethodCall("degradate", this, new Object[]{currentEcmTime});
        super.degradate(currentEcmTime);
        DebugUtil.logMethodReturnVoid("degradate", this);

    }

    @Override
    public Substance getSubstanceInstance(Substance templateS) {
        DebugUtil.logMethodCall("getSubstanceInstance", this, new Object[]{templateS});
        Substance ret =  super.getSubstanceInstance(templateS);
        DebugUtil.logMethodReturn("getSubstanceInstance", this, ret);
        return ret;

    }

    @Override
    public double computeConcentrationAtDistanceBasedOnGradient(Substance s, double[] dX) {
        DebugUtil.logMethodCall("computeConcentrationAtDistanceBasedOnGradient", this, new Object[]{s, dX});
        double ret =  super.computeConcentrationAtDistanceBasedOnGradient(s, dX);
        DebugUtil.logMethodReturn("computeConcentrationAtDistanceBasedOnGradient", this, ret);
        return ret;

    }

    @Override
    public double[] soNodePosition() {
        DebugUtil.logMethodCall("soNodePosition", this, new Object[]{});
        double[] ret =  super.soNodePosition();
        DebugUtil.logMethodReturn("soNodePosition", this, ret);
        return ret;

    }

    @Override
    public SpaceNode<ini.cx3d.physics.interfaces.PhysicalNode> getSoNode() {
        DebugUtil.logMethodCall("getSoNode", this, new Object[]{});
        SpaceNode ret =  super.getSoNode();
        DebugUtil.logMethodReturn("getSoNode", this, ret);
        return ret;

    }

    @Override
    public void setSoNode(SpaceNode son) {
        DebugUtil.logMethodCall("setSoNode", this, new Object[]{son});
        super.setSoNode(son);
        DebugUtil.logMethodReturnVoid("setSoNode", this);

    }

    @Override
    public boolean isOnTheSchedulerListForPhysicalNodes() {
        DebugUtil.logMethodCall("isOnTheSchedulerListForPhysicalNodes", this, new Object[]{});
        boolean ret =  super.isOnTheSchedulerListForPhysicalNodes();
        DebugUtil.logMethodReturn("isOnTheSchedulerListForPhysicalNodes", this, ret);
        return ret;

    }

    @Override
    public void setOnTheSchedulerListForPhysicalNodes(boolean onTheSchedulerListForPhysicalNodes) {
        DebugUtil.logMethodCall("setOnTheSchedulerListForPhysicalNodes", this, new Object[]{onTheSchedulerListForPhysicalNodes});
        super.setOnTheSchedulerListForPhysicalNodes(onTheSchedulerListForPhysicalNodes);
        DebugUtil.logMethodReturnVoid("setOnTheSchedulerListForPhysicalNodes", this);

    }

    @Override
    public int getMovementConcentratioUpdateProcedure() {
        DebugUtil.logMethodCall("getMovementConcentratioUpdateProcedure", this, new Object[]{});
        int ret =  super.getMovementConcentratioUpdateProcedure();
        DebugUtil.logMethodReturn("getMovementConcentratioUpdateProcedure", this, ret);
        return ret;

    }

    @Override
    public void setMovementConcentratioUpdateProcedure(int movementConcentratioUpdateProcedure) {
        DebugUtil.logMethodCall("setMovementConcentratioUpdateProcedure", this, new Object[]{movementConcentratioUpdateProcedure});
        super.setMovementConcentratioUpdateProcedure(movementConcentratioUpdateProcedure);
        DebugUtil.logMethodReturnVoid("setMovementConcentratioUpdateProcedure", this);

    }

    @Override
    public void addExtracellularSubstance(Substance is) {
        DebugUtil.logMethodCall("addExtracellularSubstance", this, new Object[]{is});
        super.addExtracellularSubstance(is);
        DebugUtil.logMethodReturnVoid("addExtracellularSubstance", this);

    }

    @Override
    public void removeExtracellularSubstance(Substance is) {
        DebugUtil.logMethodCall("removeExtracellularSubstance", this, new Object[]{is});
        super.removeExtracellularSubstance(is);
        DebugUtil.logMethodReturnVoid("removeExtracellularSubstance", this);

    }

    @Override
    public AbstractSequentialList<Substance> getExtracellularSubstances() {
        DebugUtil.logMethodCall("getExtracellularSubstances", this, new Object[]{});
        AbstractSequentialList<Substance> ret =  super.getExtracellularSubstances();
        DebugUtil.logMethodReturn("getExtracellularSubstances", this, ret);
        return ret;

    }

    @Override
    public Substance getExtracellularSubstance(String key) {
        DebugUtil.logMethodCall("getExtracellularSubstance", this, new Object[]{key});
        Substance ret =  super.getExtracellularSubstance(key);
        DebugUtil.logMethodReturn("getExtracellularSubstance", this, ret);
        return ret;

    }

    @Override
    public int getID() {
        DebugUtil.logMethodCall("getID", this, new Object[]{});
        int ret =  super.getID();
        DebugUtil.logMethodReturn("getID", this, ret);
        return ret;

    }

//    @Override
//    public boolean equals(Object o) {
//        DebugUtil.logMethodCall("equals", this, new Object[]{o});
//        boolean ret =  super.equals(o);
//        DebugUtil.logMethodReturn("equals", this, ret);
//        return ret;
//
//    }
}