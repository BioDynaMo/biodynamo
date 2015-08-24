/*
Copyright (C) 2009 Frédéric Zubler, Rodney J. Douglas,
Dennis Göhlsdorf, Toby Weston, Andreas Hauri, Roman Bauer,
Sabina Pfister & Adrian M. Whatley.

This file is part of CX3D.

CX3D is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CX3D is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with CX3D.  If not, see <http://www.gnu.org/licenses/>.
*/

package ini.cx3d.synapses;

import static ini.cx3d.utilities.Matrix.*;
import java.util.*;

import ini.cx3d.Param;
import ini.cx3d.localBiology.*;
import ini.cx3d.physics.PhysicalBond;
import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.PhysicalObject;
import ini.cx3d.physics.PhysicalSphere;

public class PhysicalBouton extends Excrescence {

	BiologicalBouton biologicalBouton;

	public PhysicalBouton() {
		super();
		super.type = BOUTON;
	}

	public PhysicalBouton(PhysicalObject po, double[] origin, double length) {
		super();
		super.type = BOUTON;
		super.po = po;
		super.positionOnPO = origin;
		super.length = length;
	}

	public BiologicalBouton getBiologicalBouton() {
		return biologicalBouton;
	}

	public void setBiologicalBouton(BiologicalBouton biologicalBouton) {
		this.biologicalBouton = biologicalBouton;
	}

	@Override
	public boolean synapseWith(Excrescence otherExcressence,
			boolean createPhysicalBond) {
		// only if the other Excrescence is a bouton
		if (otherExcressence.getType() != SPINE) {
			(new Throwable(this + " is a bouton, and thus can't synapse with "
					+ otherExcressence)).printStackTrace();
			return false;
		}
		// no autapses
		if (super.po.getCellElement()!=null && otherExcressence.po.getCellElement()!=null) {
			if (super.po.getCellElement().getCell() == otherExcressence.po.getCellElement().getCell()) {
				return false;
			}
		}
		// making the references
		this.ex = otherExcressence;
		ex.setEx(this);
		// if needed, formation of the PhysicalBound
		if (createPhysicalBond) {
			PhysicalBond pb = new PhysicalBond(super.po, super.positionOnPO, ex
					.getPo(), ex.getPositionOnPO(), distance(
					super.getPo().transformCoordinatesPolarToGlobal(
							super.getPositionOnPO()), ex.getPo()
							.transformCoordinatesPolarToGlobal(
									ex.getPositionOnPO())),
			// 15,
					1);
			// that we insert into the two PhysicalObjects
			// super.po.addPhysicalBond(pb);
			// ex.getPo().addPhysicalBond(pb);
		}
		// debugg :
		System.out.println("PhysicalBouton: We made a synapse between "
				+ this.type + " of "
				+ po.getCellElement().getCell().getNeuroMLType() + " and "
				+ ex.getType() + " of "
				+ ex.getPo().getCellElement().getCell().getNeuroMLType());
		return true;
	}

	// Roman: Method for making synapses directly on the soma
	public boolean synapseWithSoma(Excrescence otherExcrescence,
			boolean createPhysicalBond) {
		// only if the other Excrescence is a bouton
		if (otherExcrescence.getType() == BOUTON) {
			(new Throwable(this + " is a bouton, and thus can't synapse with "
					+ otherExcrescence)).printStackTrace();
			return false;
		}
		// making the references
		this.ex = otherExcrescence;
		ex.setEx(this);
		// if needed, formation of the PhysicalBound
		if (createPhysicalBond) {
			PhysicalBond pb = new PhysicalBond(super.po, super.positionOnPO, ex
					.getPo(), ex.getPositionOnPO(), distance(
					super.getPo().transformCoordinatesPolarToGlobal(
							super.getPositionOnPO()), ex.getPo()
							.transformCoordinatesPolarToGlobal(
									ex.getPositionOnPO())),
			// 15,
					1);
			// that we insert into the two PhysicalObjects
			// super.po.addPhysicalBond(pb);
			// ex.getPo().addPhysicalBond(pb);
		}
		// debugg :
		System.out.println("PhysicalBouton: We made a synapse between "
				+ this.type + " of "
				+ po.getCellElement().getCell().getNeuroMLType() + " and "
				+ ex.getType() + " of "
				+ ex.getPo().getCellElement().getCell().getNeuroMLType());
		return true;
	}

	public boolean synapseWithShaft(NeuriteElement otherNe, double maxDis,
			int nrSegments, boolean createPhysicalBond) {
		PhysicalCylinder pc = otherNe.getPhysicalCylinder();
		double neLength = pc.getActualLength();
		double dx = neLength / nrSegments;
		double[] currPos;
		double[] currVec;

		double[] currDir = pc.getUnitaryAxisDirectionVector();

		// for (double dX = dx; dX<neLength; dX=dX+dx) {
		double dX = neLength * Math.random();
		currVec = new double[] { -currDir[0] * dX, -currDir[1] * dX,
				-currDir[2] * dX };

		// currPos = add(pc.getMassLocation(),currVec);
		currPos = otherNe.getPhysicalCylinder()
				.transformCoordinatesLocalToGlobal(currVec);
		if ((norm(subtract(currPos, this.getProximalEnd())) < maxDis)
				&& (createPhysicalBond)) {
			System.out.println("The distance was: "
					+ norm(subtract(currPos, super.getProximalEnd())));

			// double[] p1 =
			// super.po.transformCoordinatesPolarToGlobal(super.positionOnPO);
			double[] p1 = super.getProximalEnd();
			// double[] p2 =
			// otherNe.getPhysicalCylinder().transformCoordinatesLocalToGlobal(currVec);

			System.out
					.println("Between Bouton and neuriteelement with mass location at {"
							+ pc.getMassLocation()[0]
							+ ","
							+ pc.getMassLocation()[1]
							+ ","
							+ pc.getMassLocation()[2] + "}");
			System.out.println("Physical Bond Begin at:" + p1[0] + ", " + p1[1]
					+ ", " + p1[2]);
			System.out.println("Physical Bond End at:" + currPos[0] + ", "
					+ currPos[1] + ", " + currPos[2]);

			// PhysicalBond pb = new PhysicalBond(super.po, super.positionOnPO,
			// otherNe.getPhysicalCylinder(), currVec ,
			// distance(super.getPo().transformCoordinatesPolarToGlobal(super.getPositionOnPO()),
			// currPos),1);
			PhysicalBond pb = new PhysicalBond(super.po, super.positionOnPO,
					otherNe.getPhysicalCylinder(), otherNe
							.getPhysicalCylinder()
							.transformCoordinatesLocalToPolar(currVec),
					distance(super.getProximalEnd(), currPos), 1);
			super.neShaft = otherNe;

		}
		//}

		return true;
	}

}
