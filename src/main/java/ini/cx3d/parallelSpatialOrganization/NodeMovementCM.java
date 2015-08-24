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

package ini.cx3d.parallelSpatialOrganization;

import ini.cx3d.spatialOrganization.PositionNotAllowedException;
import static ini.cx3d.utilities.Matrix.*;

public class NodeMovementCM<T> extends CacheManager<T> {
	ManagedObjectReference<T> nodeReference;
	double[] delta;
	public NodeMovementCM(ManagedObjectReference<T> nodeReference, double[] delta, SpatialOrganizationManager<T> mySOM) {
		super(mySOM);
		this.delta = delta;
		this.nodeReference = nodeReference;
	}
	
	public String descriptionString() {
		return "Movement #"+id+" (Node "+nodeReference+")";
	}

	@Override
	boolean executeTask() throws NodeLockedException {
		if (nodeReference.getSOM().isNodePending(nodeReference))
			return false;
		// get a local copy of the SpaceNode that should be moved:
		SpaceNode<T> localNode = getNode(nodeReference);
		try {
			localNode.moveFrom(delta);
			return true;
		} catch (ManagedObjectDoesNotExistException e) {
			throw new RuntimeException("A modifying process tried to access a managed object that didn't exist!");
		}
		catch (PositionNotAllowedException e) {
			// TODO: This is the most simple way to catch these exceptions... 
			// simply accept the proposed position.
			this.delta = subtract(e.getProposedPosition(), localNode.getPosition());
			return false;
		}
//		mySOM.addAction(new NodeMovement<T>(nodeReference, delta));
 	}

	@Override
	public int getPriority() {
		return 10;
	}

}
