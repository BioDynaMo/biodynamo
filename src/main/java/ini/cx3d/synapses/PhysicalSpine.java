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

import static ini.cx3d.SimStateSerializationUtil.keyValue;
import static ini.cx3d.SimStateSerializationUtil.removeLastChar;
import static ini.cx3d.utilities.Matrix.distance;

import ini.cx3d.physics.factory.PhysicalBondFactory;

import ini.cx3d.synapses.interfaces.Excrescence;
import ini.cx3d.synapses.interfaces.BiologicalSpine;

public class PhysicalSpine extends Excrescence2 implements ini.cx3d.synapses.interfaces.PhysicalSpine {

	BiologicalSpine biologicalSpine;

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{"); //fixme bug: should call super.simStateToJson

		keyValue(sb, "biologicalSpine", biologicalSpine);

		removeLastChar(sb);
		sb.append("}");
		return sb;
	}

	public PhysicalSpine() {
		super();
		ini.cx3d.swig.simulation.PhysicalSpine.registerJavaObject(this);
		super.type = SPINE;
	}
	
	public PhysicalSpine(ini.cx3d.physics.interfaces.PhysicalObject po, double[] origin, double length) {
		super();
		super.type = SPINE;
		super.po = po;
		super.positionOnPO = origin;
		super.length = length;
	}
	
	@Override
	public BiologicalSpine getBiologicalSpine() {
		return biologicalSpine;
	}

	@Override
	public void setBiologicalSpine(BiologicalSpine biologicalSpine) {
		this.biologicalSpine = biologicalSpine;
	}

	@Override
	public boolean synapseWith(ini.cx3d.synapses.interfaces.Excrescence otherExcressence, boolean createPhysicalBond) {
		// only if the other Excrescence is a bouton
		if(otherExcressence.getType() != BOUTON){
			(new Throwable(this+" is a spine, and thus can't synapse with "+otherExcressence)).printStackTrace();
			return false;
		}
		// making the references
		this.ex = otherExcressence;
		ex.setEx(this);
		// if needed, formation of the PhysicalBound
		if(createPhysicalBond){
			ini.cx3d.physics.interfaces.PhysicalBond pb = PhysicalBondFactory.create(
					super.po, super.positionOnPO,
					ex.getPo(), ex.getPositionOnPO(),
					distance(super.getPo().transformCoordinatesPolarToGlobal(super.getPositionOnPO()),
							ex.getPo().transformCoordinatesPolarToGlobal(ex.getPositionOnPO())),
					1);
			// that we insert into the two PhysicalObjects
//			super.po.addPhysicalBond(pb);
//			ex.getPo().addPhysicalBond(pb);
		}
		// debugg : 
		System.out.println("*PhysicalSpine: We made a synapse between "+
				this.type+ " of "+
				po.getCellElement().getCell().getNeuroMLType()+ " and "+
				ex.getType()+" of "+
				ex.getPo().getCellElement().getCell().getNeuroMLType());
		return true;
	}

	@Override
	public boolean synapseWithSoma(ini.cx3d.synapses.interfaces.Excrescence otherExcrescence,
								   boolean creatPhysicalBond) {
		return false;
	}
	

	@Override
	public boolean synapseWithShaft(ini.cx3d.localBiology.interfaces.NeuriteElement otherNe, double maxDis, int nrSegments,
									boolean createPhysicalBond) {
		return false;
	}

}
