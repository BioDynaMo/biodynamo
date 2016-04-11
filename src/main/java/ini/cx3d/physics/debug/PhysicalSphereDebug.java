package ini.cx3d.physics.debug;


import ini.cx3d.localBiology.interfaces.CellElement;
import ini.cx3d.localBiology.SomaElement;
import ini.cx3d.physics.interfaces.PhysicalCylinder;
import ini.cx3d.physics.interfaces.PhysicalSphere;
import ini.cx3d.physics.interfaces.PhysicalObject;
import ini.cx3d.swig.NativeStringBuilder;
import ini.cx3d.utilities.DebugUtil;

import java.util.AbstractSequentialList;

public class PhysicalSphereDebug extends ini.cx3d.physics.PhysicalSphere {

    @Override
    public double getLength() {
        DebugUtil.logMethodCall("getLength", this, new Object[]{});
        double ret = super.getLength();
        DebugUtil.logMethodReturn("getLength", this, ret);
        return ret;

    }

    @Override
    public NativeStringBuilder simStateToJson(NativeStringBuilder sb) {
        DebugUtil.logMethodCall("simStateToJson", this, new Object[]{sb});
        NativeStringBuilder ret = super.simStateToJson(sb);
        DebugUtil.logMethodReturn("simStateToJson", this, ret);
        return ret;

    }

//    @Override
//    public String toString() {
//        DebugUtil.logMethodCall("toString", this, new Object[]{});
//        String ret = super.toString();
//        DebugUtil.logMethodReturn("toString", this, ret);
//        return ret;
//
//    }

    public PhysicalSphereDebug() {
        super();
//        System.out.println("DBG diameter: "+getDiameter());
//        DebugUtil.logMethodCall("ini.cx3d.physics.debug.PhysicalSphereDebug", this, new Object[]{});

    }

    @Override
    public double getInterObjectForceCoefficient() {
        DebugUtil.logMethodCall("getInterObjectForceCoefficient", this, new Object[]{});
        double ret = super.getInterObjectForceCoefficient();
        DebugUtil.logMethodReturn("getInterObjectForceCoefficient", this, ret);
        return ret;

    }

    @Override
    public void setInterObjectForceCoefficient(double interObjectForceCoefficient) {
        DebugUtil.logMethodCall("setInterObjectForceCoefficient", this, new Object[]{interObjectForceCoefficient});
        super.setInterObjectForceCoefficient(interObjectForceCoefficient);
        DebugUtil.logMethodReturnVoid("setInterObjectForceCoefficient", this);

    }

    @Override
    public double getRotationalInertia() {
        DebugUtil.logMethodCall("getRotationalInertia", this, new Object[]{});
        double ret = super.getRotationalInertia();
        DebugUtil.logMethodReturn("getRotationalInertia", this, ret);
        return ret;

    }

    @Override
    public void setRotationalInertia(double rotationalInertia) {
        DebugUtil.logMethodCall("setRotationalInertia", this, new Object[]{rotationalInertia});
        super.setRotationalInertia(rotationalInertia);
        DebugUtil.logMethodReturnVoid("setRotationalInertia", this);

    }

    @Override
    public boolean isAPhysicalSphere() {
        DebugUtil.logMethodCall("isAPhysicalSphere", this, new Object[]{});
        boolean ret = super.isAPhysicalSphere();
        DebugUtil.logMethodReturn("isAPhysicalSphere", this, ret);
        return ret;

    }

    @Override
    public void movePointMass(double speed, double[] direction) {
        DebugUtil.logMethodCall("movePointMass", this, new Object[]{speed, direction});
        super.movePointMass(speed, direction);
        DebugUtil.logMethodReturnVoid("movePointMass", this);

    }

    @Override
    public double[] originOf(PhysicalObject daughterWhoAsks) {
        DebugUtil.logMethodCall("originOf", this, new Object[]{daughterWhoAsks});
        double[] ret = super.originOf(daughterWhoAsks);
        DebugUtil.logMethodReturn("originOf", this, ret);
        return ret;

    }

    @Override
    public double[] forceTransmittedFromDaugtherToMother(PhysicalObject motherWhoAsks) {
        DebugUtil.logMethodCall("forceTransmittedFromDaugtherToMother", this, new Object[]{motherWhoAsks});
        double[] ret = super.forceTransmittedFromDaugtherToMother(motherWhoAsks);
        DebugUtil.logMethodReturn("forceTransmittedFromDaugtherToMother", this, ret);
        return ret;

    }

    @Override
    public void removeDaugther(PhysicalObject daughterToRemove) {
        DebugUtil.logMethodCall("removeDaugther", this, new Object[]{daughterToRemove});
        super.removeDaugther(daughterToRemove);
        DebugUtil.logMethodReturnVoid("removeDaugther", this);

    }

    @Override
    public void updateRelative(PhysicalObject oldRelative, PhysicalObject newRelative) {
        DebugUtil.logMethodCall("updateRelative", this, new Object[]{oldRelative, newRelative});
        super.updateRelative(oldRelative, newRelative);
        DebugUtil.logMethodReturnVoid("updateRelative", this);

    }

    @Override
    public SomaElement getSomaElement() {
        DebugUtil.logMethodCall("getSomaElement", this, new Object[]{});
        SomaElement ret = super.getSomaElement();
        DebugUtil.logMethodReturn("getSomaElement", this, ret);
        return ret;

    }

    @Override
    public void setSomaElement(SomaElement somaElement) {
        DebugUtil.logMethodCall("setSomaElement", this, new Object[]{somaElement});
        super.setSomaElement(somaElement);
        DebugUtil.logMethodReturnVoid("setSomaElement", this);

    }

    @Override
    public void changeVolume(double speed) {
        DebugUtil.logMethodCall("changeVolume", this, new Object[]{speed});
        super.changeVolume(speed);
        DebugUtil.logMethodReturnVoid("changeVolume", this);

    }

    @Override
    public void changeDiameter(double speed) {
        DebugUtil.logMethodCall("changeDiameter", this, new Object[]{speed});
        super.changeDiameter(speed);
        DebugUtil.logMethodReturnVoid("changeDiameter", this);

    }

    @Override
    public void updateDependentPhysicalVariables() {
        DebugUtil.logMethodCall("updateDependentPhysicalVariables", this, new Object[]{});
        super.updateDependentPhysicalVariables();
        DebugUtil.logMethodReturnVoid("updateDependentPhysicalVariables", this);

    }

//    @Override
//    public void updateIntracellularConcentrations() {
//        DebugUtil.logMethodCall("updateIntracellularConcentrations", this, new Object[]{});
//        super.updateIntracellularConcentrations();
//        DebugUtil.logMethodReturnVoid("updateIntracellularConcentrations", this);
//
//    }

//    @Override
//    public void updateVolume() {
//        DebugUtil.logMethodCall("updateVolume", this, new Object[]{});
//        super.updateVolume();
//        DebugUtil.logMethodReturnVoid("updateVolume", this);
//
//    }

    @Override
    public void updateDiameter() {
        DebugUtil.logMethodCall("updateDiameter", this, new Object[]{});
        super.updateDiameter();
        DebugUtil.logMethodReturnVoid("updateDiameter", this);

    }

    @Override
    public PhysicalCylinder addNewPhysicalCylinder(double newLength, double phi, double theta) {
        DebugUtil.logMethodCall("addNewPhysicalCylinder", this, new Object[]{newLength, phi, theta});
        PhysicalCylinder ret = super.addNewPhysicalCylinder(newLength, phi, theta);
        DebugUtil.logMethodReturn("addNewPhysicalCylinder", this, ret);
        return ret;

    }

    @Override
    public ini.cx3d.physics.interfaces.PhysicalSphere divide(double vr, double phi, double theta) {
        DebugUtil.logMethodCall("divide", this, new Object[]{vr, phi, theta});
        PhysicalSphere ret = super.divide(vr, phi, theta);
        DebugUtil.logMethodReturn("divide", this, ret);
        return ret;

    }

    @Override
    public boolean isInContactWithSphere(ini.cx3d.physics.interfaces.PhysicalSphere s) {
        DebugUtil.logMethodCall("isInContactWithSphere", this, new Object[]{s});
        boolean ret = super.isInContactWithSphere(s);
        DebugUtil.logMethodReturn("isInContactWithSphere", this, ret);
        return ret;

    }

    @Override
    public boolean isInContactWithCylinder(PhysicalCylinder c) {
        DebugUtil.logMethodCall("isInContactWithCylinder", this, new Object[]{c});
        boolean ret = super.isInContactWithCylinder(c);
        DebugUtil.logMethodReturn("isInContactWithCylinder", this, ret);
        return ret;

    }

    @Override
    public double[] getForceOn(PhysicalCylinder c) {
        DebugUtil.logMethodCall("getForceOn", this, new Object[]{c});
        double[] ret = super.getForceOn(c);
        DebugUtil.logMethodReturn("getForceOn", this, ret);
        return ret;

    }

    @Override
    public double[] getForceOn(ini.cx3d.physics.interfaces.PhysicalSphere s) {
        DebugUtil.logMethodCall("getForceOn", this, new Object[]{s});
        double[] ret = super.getForceOn(s);
        DebugUtil.logMethodReturn("getForceOn", this, ret);
        return ret;

    }

    @Override
    public void runPhysics() {
        DebugUtil.logMethodCall("runPhysics", this, new Object[]{});
        super.runPhysics();
        DebugUtil.logMethodReturnVoid("runPhysics", this);

    }

    @Override
    public double[] getAxis() {
        DebugUtil.logMethodCall("getAxis", this, new Object[]{});
        double[] ret = super.getAxis();
        DebugUtil.logMethodReturn("getAxis", this, ret);
        return ret;

    }

    @Override
    public AbstractSequentialList<ini.cx3d.physics.interfaces.PhysicalCylinder> getDaughters() {
        DebugUtil.logMethodCall("getDaughters", this, new Object[]{});
        AbstractSequentialList<ini.cx3d.physics.interfaces.PhysicalCylinder> ret = super.getDaughters();
        DebugUtil.logMethodReturn("getDaughters", this, ret);
        return ret;

    }

    @Override
    public void runIntracellularDiffusion() {
        DebugUtil.logMethodCall("runIntracellularDiffusion", this, new Object[]{});
        super.runIntracellularDiffusion();
        DebugUtil.logMethodReturnVoid("runIntracellularDiffusion", this);

    }

    @Override
    public double[] transformCoordinatesGlobalToLocal(double[] positionInGlobalCoord) {
        DebugUtil.logMethodCall("transformCoordinatesGlobalToLocal", this, new Object[]{positionInGlobalCoord});
        double[] ret = super.transformCoordinatesGlobalToLocal(positionInGlobalCoord);
        DebugUtil.logMethodReturn("transformCoordinatesGlobalToLocal", this, ret);
        return ret;

    }

    @Override
    public double[] transformCoordinatesLocalToGlobal(double[] positionInLocalCoord) {
        DebugUtil.logMethodCall("transformCoordinatesLocalToGlobal", this, new Object[]{positionInLocalCoord});
        double[] ret = super.transformCoordinatesLocalToGlobal(positionInLocalCoord);
        DebugUtil.logMethodReturn("transformCoordinatesLocalToGlobal", this, ret);
        return ret;

    }

    @Override
    public double[] transformCoordinatesLocalToPolar(double[] positionInLocalCoordinates) {
        DebugUtil.logMethodCall("transformCoordinatesLocalToPolar", this, new Object[]{positionInLocalCoordinates});
        double[] ret = super.transformCoordinatesLocalToPolar(positionInLocalCoordinates);
        DebugUtil.logMethodReturn("transformCoordinatesLocalToPolar", this, ret);
        return ret;

    }

    @Override
    public double[] transformCoordinatesPolarToLocal(double[] positionInPolarCoordinates) {
        DebugUtil.logMethodCall("transformCoordinatesPolarToLocal", this, new Object[]{positionInPolarCoordinates});
        double[] ret = super.transformCoordinatesPolarToLocal(positionInPolarCoordinates);
        DebugUtil.logMethodReturn("transformCoordinatesPolarToLocal", this, ret);
        return ret;

    }

    @Override
    public double[] transformCoordinatesPolarToGlobal(double[] positionInPolarCoordinates) {
        DebugUtil.logMethodCall("transformCoordinatesPolarToGlobal", this, new Object[]{positionInPolarCoordinates});
        double[] ret = super.transformCoordinatesPolarToGlobal(positionInPolarCoordinates);
        DebugUtil.logMethodReturn("transformCoordinatesPolarToGlobal", this, ret);
        return ret;

    }

    @Override
    public double[] transformCoordinatesGlobalToPolar(double[] positionInGlobalCoordinates) {
        DebugUtil.logMethodCall("transformCoordinatesGlobalToPolar", this, new Object[]{positionInGlobalCoordinates});
        double[] ret = super.transformCoordinatesGlobalToPolar(positionInGlobalCoordinates);
        DebugUtil.logMethodReturn("transformCoordinatesGlobalToPolar", this, ret);
        return ret;

    }

    @Override
    public double[] getUnitNormalVector(double[] positionInPolarCoordinates) {
        DebugUtil.logMethodCall("getUnitNormalVector", this, new Object[]{positionInPolarCoordinates});
        double[] ret = super.getUnitNormalVector(positionInPolarCoordinates);
        DebugUtil.logMethodReturn("getUnitNormalVector", this, ret);
        return ret;

    }

    @Override
    public CellElement getCellElement() {
        DebugUtil.logMethodCall("getCellElement", this, new Object[]{});
        CellElement ret = super.getCellElement();
        DebugUtil.logMethodReturn("getCellElement", this, ret);
        return ret;

    }

    @Override
    public boolean isRelative(PhysicalObject po) {
        DebugUtil.logMethodCall("isRelative", this, new Object[]{po});
        boolean ret = super.isRelative(po);
        DebugUtil.logMethodReturn("isRelative", this, ret);
        return ret;

    }
}
