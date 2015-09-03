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

import ini.cx3d.SimStateSerializable;

import static ini.cx3d.SimStateSerializationUtil.keyValue;

public class BiologicalSpine implements SimStateSerializable{
	PhysicalSpine physicalSpine;

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{");

		//physical spine is circular reference
//		keyValue(sb, "physicalSpine", physicalSpine);

//		removeLastChar(sb);
		sb.append("}");
		return sb;
	}

	public PhysicalSpine getPhysicalSpine() {
		return physicalSpine;
	}

	public void setPhysicalSpine(PhysicalSpine physicalSpine) {
		this.physicalSpine = physicalSpine;
	}
	
}
