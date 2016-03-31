package ini.cx3d.physics.debug;

import ini.cx3d.localBiology.NeuriteElement;
import ini.cx3d.localBiology.interfaces.CellElement;
import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.interfaces.PhysicalObject;
import ini.cx3d.physics.interfaces.PhysicalSphere;
import ini.cx3d.utilities.DebugUtil;

public class PhysicalCylinderDebug extends ini.cx3d.physics.PhysicalCylinder {
    @Override
    public PhysicalCylinder getCopy() {
        DebugUtil.logMethodCall("getCopy", this, new Object[]{});
        PhysicalCylinder ret = super.getCopy();
        DebugUtil.logMethodReturn("getCopy", this, ret);
        return ret;

    }

    @Override
    public boolean isRelative(PhysicalObject po) {
        DebugUtil.logMethodCall("isRelative", this, new Object[]{po});
        boolean ret = super.isRelative(po);
        DebugUtil.logMethodReturn("isRelative", this, ret);
        return ret;

    }

    @Override
    public double[] originOf(PhysicalObject daughterWhoAsks) {
        DebugUtil.logMethodCall("originOf", this, new Object[]{daughterWhoAsks});
        double[] ret = super.originOf(daughterWhoAsks);
        DebugUtil.logMethodReturn("originOf", this, ret);
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
    public double[] forceTransmittedFromDaugtherToMother(PhysicalObject motherWhoAsks) {
        DebugUtil.logMethodCall("forceTransmittedFromDaugtherToMother", this, new Object[]{motherWhoAsks});
        double[] ret = super.forceTransmittedFromDaugtherToMother(motherWhoAsks);
        DebugUtil.logMethodReturn("forceTransmittedFromDaugtherToMother", this, ret);
        return ret;

    }

    @Override
    public boolean runDiscretization() {
        DebugUtil.logMethodCall("runDiscretization", this, new Object[]{});
        boolean ret = super.runDiscretization();
        DebugUtil.logMethodReturn("runDiscretization", this, ret);
        return ret;

    }

    @Override
    public void updateSpatialOrganizationNodePosition() {
        DebugUtil.logMethodCall("updateSpatialOrganizationNodePosition", this, new Object[]{});
        super.updateSpatialOrganizationNodePosition();
        DebugUtil.logMethodReturnVoid("updateSpatialOrganizationNodePosition", this);

    }

    @Override
    public void extendCylinder(double speed, double[] direction) {
        DebugUtil.logMethodCall("extendCylinder", this, new Object[]{speed, direction});
        super.extendCylinder(speed, direction);
        DebugUtil.logMethodReturnVoid("extendCylinder", this);

    }

    @Override
    public void movePointMass(double speed, double[] direction) {
        DebugUtil.logMethodCall("movePointMass", this, new Object[]{speed, direction});
        super.movePointMass(speed, direction);
        DebugUtil.logMethodReturnVoid("movePointMass", this);

    }

    @Override
    public boolean retractCylinder(double speed) {
        DebugUtil.logMethodCall("retractCylinder", this, new Object[]{speed});
        boolean ret = super.retractCylinder(speed);
        DebugUtil.logMethodReturn("retractCylinder", this, ret);
        return ret;

    }

    @Override
    public PhysicalCylinder[] bifurcateCylinder(double length, double[] direction_1, double[] direction_2) {
        DebugUtil.logMethodCall("bifurcateCylinder", this, new Object[]{length, direction_1, direction_2});
        PhysicalCylinder[] ret = super.bifurcateCylinder(length, direction_1, direction_2);
        DebugUtil.logMethodReturn("bifurcateCylinder", this, ret);
        return ret;

    }

    @Override
    public PhysicalCylinder branchCylinder(double length, double[] direction) {
        DebugUtil.logMethodCall("branchCylinder", this, new Object[]{length, direction});
        PhysicalCylinder ret = super.branchCylinder(length, direction);
        DebugUtil.logMethodReturn("branchCylinder", this, ret);
        return ret;

    }

    @Override
    public void setRestingLengthForDesiredTension(double tensionWeWant) {
        DebugUtil.logMethodCall("setRestingLengthForDesiredTension", this, new Object[]{tensionWeWant});
        super.setRestingLengthForDesiredTension(tensionWeWant);
        DebugUtil.logMethodReturnVoid("setRestingLengthForDesiredTension", this);

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
    public void runPhysics() {
        DebugUtil.logMethodCall("runPhysics", this, new Object[]{});
        super.runPhysics();
        DebugUtil.logMethodReturnVoid("runPhysics", this);

    }

    @Override
    public double[] getForceOn(PhysicalSphere s) {
        DebugUtil.logMethodCall("getForceOn", this, new Object[]{s});
        double[] ret = super.getForceOn(s);
        DebugUtil.logMethodReturn("getForceOn", this, ret);
        return ret;

    }

    @Override
    public double[] getForceOn(ini.cx3d.physics.interfaces.PhysicalCylinder c) {
        DebugUtil.logMethodCall("getForceOn", this, new Object[]{c});
        double[] ret = super.getForceOn(c);
        DebugUtil.logMethodReturn("getForceOn", this, ret);
        return ret;

    }

    @Override
    public boolean isInContactWithSphere(PhysicalSphere s) {
        DebugUtil.logMethodCall("isInContactWithSphere", this, new Object[]{s});
        boolean ret = super.isInContactWithSphere(s);
        DebugUtil.logMethodReturn("isInContactWithSphere", this, ret);
        return ret;

    }

    @Override
    public boolean isInContactWithCylinder(ini.cx3d.physics.interfaces.PhysicalCylinder c) {
        DebugUtil.logMethodCall("isInContactWithCylinder", this, new Object[]{c});
        boolean ret = super.isInContactWithCylinder(c);
        DebugUtil.logMethodReturn("isInContactWithCylinder", this, ret);
        return ret;

    }

    @Override
    public double[] closestPointTo(double[] p) {
        DebugUtil.logMethodCall("closestPointTo", this, new Object[]{p});
        double[] ret = super.closestPointTo(p);
        DebugUtil.logMethodReturn("closestPointTo", this, ret);
        return ret;

    }

    @Override
    public void runIntracellularDiffusion() {
        DebugUtil.logMethodCall("runIntracellularDiffusion", this, new Object[]{});
        super.runIntracellularDiffusion();
        DebugUtil.logMethodReturnVoid("runIntracellularDiffusion", this);

    }

    @Override
    public double[] getUnitNormalVector(double[] positionInPolarCoordinates) {
        DebugUtil.logMethodCall("getUnitNormalVector", this, new Object[]{positionInPolarCoordinates});
        double[] ret = super.getUnitNormalVector(positionInPolarCoordinates);
        DebugUtil.logMethodReturn("getUnitNormalVector", this, ret);
        return ret;

    }

    @Override
    public void updateLocalCoordinateAxis() {
        DebugUtil.logMethodCall("updateLocalCoordinateAxis", this, new Object[]{});
        super.updateLocalCoordinateAxis();
        DebugUtil.logMethodReturnVoid("updateLocalCoordinateAxis", this);

    }

    @Override
    public void updateDependentPhysicalVariables() {
        DebugUtil.logMethodCall("updateDependentPhysicalVariables", this, new Object[]{});
        super.updateDependentPhysicalVariables();
        DebugUtil.logMethodReturnVoid("updateDependentPhysicalVariables", this);

    }

    @Override
    public void updateDiameter() {
        DebugUtil.logMethodCall("updateDiameter", this, new Object[]{});
        super.updateDiameter();
        DebugUtil.logMethodReturnVoid("updateDiameter", this);

    }

//    @Override
//    public void updateVolume() {
//        DebugUtil.logMethodCall("updateVolume", this, new Object[]{});
//        super.updateVolume();
//        DebugUtil.logMethodReturnVoid("updateVolume", this);
//
//    }
//
//    @Override
//    public void updateIntracellularConcentrations() {
//        DebugUtil.logMethodCall("updateIntracellularConcentrations", this, new Object[]{});
//        super.updateIntracellularConcentrations();
//        DebugUtil.logMethodReturnVoid("updateIntracellularConcentrations", this);
//    }

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
    public CellElement getCellElement() {
        DebugUtil.logMethodCall("getCellElement", this, new Object[]{});
        CellElement ret = super.getCellElement();
        DebugUtil.logMethodReturn("getCellElement", this, ret);
        return ret;

    }

    @Override
    public NeuriteElement getNeuriteElement() {
        DebugUtil.logMethodCall("getNeuriteElement", this, new Object[]{});
        NeuriteElement ret = super.getNeuriteElement();
        DebugUtil.logMethodReturn("getNeuriteElement", this, ret);
        return ret;

    }

    @Override
    public void setNeuriteElement(NeuriteElement neuriteElement) {
        DebugUtil.logMethodCall("setNeuriteElement", this, new Object[]{neuriteElement});
        super.setNeuriteElement(neuriteElement);
        DebugUtil.logMethodReturnVoid("setNeuriteElement", this);

    }

    @Override
    public PhysicalCylinder getDaughterLeft() {
        DebugUtil.logMethodCall("getDaughterLeft", this, new Object[]{});
        PhysicalCylinder ret = super.getDaughterLeft();
        DebugUtil.logMethodReturn("getDaughterLeft", this, ret);
        return ret;

    }

    @Override
    public PhysicalCylinder getDaughterRight() {
        DebugUtil.logMethodCall("getDaughterRight", this, new Object[]{});
        PhysicalCylinder ret = super.getDaughterRight();
        DebugUtil.logMethodReturn("getDaughterRight", this, ret);
        return ret;

    }

//    @Override
//    public PhysicalObject getMother() {
//        DebugUtil.logMethodCall("getMother", this, new Object[]{});
//        PhysicalObject ret = super.getMother();
//        DebugUtil.logMethodReturn("getMother", this, ret);
//        return ret;
//
//    }

    @Override
    public void setMother(PhysicalObject mother) {
        DebugUtil.logMethodCall("setMother", this, new Object[]{mother});
        super.setMother(mother);
        DebugUtil.logMethodReturnVoid("setMother", this);

    }

    @Override
    public void setDaughterLeft(ini.cx3d.physics.interfaces.PhysicalCylinder daughterLeft) {
        DebugUtil.logMethodCall("setDaughterLeft", this, new Object[]{daughterLeft});
        super.setDaughterLeft(daughterLeft);
        DebugUtil.logMethodReturnVoid("setDaughterLeft", this);

    }

    @Override
    public void setDaughterRight(ini.cx3d.physics.interfaces.PhysicalCylinder daughterRight) {
        DebugUtil.logMethodCall("setDaughterRight", this, new Object[]{daughterRight});
        super.setDaughterRight(daughterRight);
        DebugUtil.logMethodReturnVoid("setDaughterRight", this);

    }

//    @Override
//    public void setBranchOrder(int branchOrder) {
//        DebugUtil.logMethodCall("setBranchOrder", this, new Object[]{branchOrder});
//        super.setBranchOrder(branchOrder);
//        DebugUtil.logMethodReturnVoid("setBranchOrder", this);
//
//    }

    @Override
    public int getBranchOrder() {
        DebugUtil.logMethodCall("getBranchOrder", this, new Object[]{});
        int ret = super.getBranchOrder();
        DebugUtil.logMethodReturn("getBranchOrder", this, ret);
        return ret;

    }

    @Override
    public double getActualLength() {
        DebugUtil.logMethodCall("getActualLength", this, new Object[]{});
        double ret = super.getActualLength();
        DebugUtil.logMethodReturn("getActualLength", this, ret);
        return ret;

    }

    @Override
    public void setActualLength(double actualLength) {
        DebugUtil.logMethodCall("setActualLength", this, new Object[]{actualLength});
        super.setActualLength(actualLength);
        DebugUtil.logMethodReturnVoid("setActualLength", this);

    }

    @Override
    public double getRestingLength() {
        DebugUtil.logMethodCall("getRestingLength", this, new Object[]{});
        double ret = super.getRestingLength();
        DebugUtil.logMethodReturn("getRestingLength", this, ret);
        return ret;

    }

    @Override
    public void setRestingLength(double restingLength) {
        DebugUtil.logMethodCall("setRestingLength", this, new Object[]{restingLength});
        super.setRestingLength(restingLength);
        DebugUtil.logMethodReturnVoid("setRestingLength", this);

    }

    @Override
    public double[] getSpringAxis() {
        DebugUtil.logMethodCall("getSpringAxis", this, new Object[]{});
        double[] ret = super.getSpringAxis();
        DebugUtil.logMethodReturn("getSpringAxis", this, ret);
        return ret;

    }

    @Override
    public void setSpringAxis(double[] springAxis) {
        DebugUtil.logMethodCall("setSpringAxis", this, new Object[]{springAxis});
        super.setSpringAxis(springAxis);
        DebugUtil.logMethodReturnVoid("setSpringAxis", this);

    }

    @Override
    public double getSpringConstant() {
        DebugUtil.logMethodCall("getSpringConstant", this, new Object[]{});
        double ret = super.getSpringConstant();
        DebugUtil.logMethodReturn("getSpringConstant", this, ret);
        return ret;

    }

//    @Override
//    public void setSpringConstant(double springConstant) {
//        DebugUtil.logMethodCall("setSpringConstant", this, new Object[]{springConstant});
//        super.setSpringConstant(springConstant);
//        DebugUtil.logMethodReturnVoid("setSpringConstant", this);
//
//    }

    @Override
    public double getTension() {
        DebugUtil.logMethodCall("getTension", this, new Object[]{});
        double ret = super.getTension();
        DebugUtil.logMethodReturn("getTension", this, ret);
        return ret;

    }

    @Override
    public double[] getUnitaryAxisDirectionVector() {
        DebugUtil.logMethodCall("getUnitaryAxisDirectionVector", this, new Object[]{});
        double[] ret = super.getUnitaryAxisDirectionVector();
        DebugUtil.logMethodReturn("getUnitaryAxisDirectionVector", this, ret);
        return ret;

    }

    @Override
    public boolean isTerminal() {
        DebugUtil.logMethodCall("isTerminal", this, new Object[]{});
        boolean ret = super.isTerminal();
        DebugUtil.logMethodReturn("isTerminal", this, ret);
        return ret;

    }

    @Override
    public boolean bifurcationPermitted() {
        DebugUtil.logMethodCall("bifurcationPermitted", this, new Object[]{});
        boolean ret = super.bifurcationPermitted();
        DebugUtil.logMethodReturn("bifurcationPermitted", this, ret);
        return ret;

    }

    @Override
    public boolean branchPermitted() {
        DebugUtil.logMethodCall("branchPermitted", this, new Object[]{});
        boolean ret = super.branchPermitted();
        DebugUtil.logMethodReturn("branchPermitted", this, ret);
        return ret;

    }

//    @Override
//    public double[] proximalEnd() {
//        DebugUtil.logMethodCall("proximalEnd", this, new Object[]{});
//        double[] ret = super.proximalEnd();
//        DebugUtil.logMethodReturn("proximalEnd", this, ret);
//        return ret;
//
//    }
//
//    @Override
//    public double[] distalEnd() {
//        DebugUtil.logMethodCall("distalEnd", this, new Object[]{});
//        double[] ret = super.distalEnd();
//        DebugUtil.logMethodReturn("distalEnd", this, ret);
//        return ret;
//
//    }

    @Override
    public double lengthToProximalBranchingPoint() {
        DebugUtil.logMethodCall("lengthToProximalBranchingPoint", this, new Object[]{});
        double ret = super.lengthToProximalBranchingPoint();
        DebugUtil.logMethodReturn("lengthToProximalBranchingPoint", this, ret);
        return ret;

    }

//    @Override
//    public boolean isAPhysicalCylinder() {
//        DebugUtil.logMethodCall("isAPhysicalCylinder", this, new Object[]{});
//        boolean ret = super.isAPhysicalCylinder();
//        DebugUtil.logMethodReturn("isAPhysicalCylinder", this, ret);
//        return ret;
//
//    }

    @Override
    public double getLength() {
        DebugUtil.logMethodCall("getLength", this, new Object[]{});
        double ret = super.getLength();
        DebugUtil.logMethodReturn("getLength", this, ret);
        return ret;

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
    public double[] getAxis() {
        DebugUtil.logMethodCall("getAxis", this, new Object[]{});
        double[] ret = super.getAxis();
        DebugUtil.logMethodReturn("getAxis", this, ret);
        return ret;

    }
}
