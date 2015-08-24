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

/**
 * A reference to a node owned by a remote SpatialOrganisationManager.
 *
 * @author Toby Weston
 * @version 0.1
 * @Date: 13/6/2008
 * 
 * 
 * @deprecated Use ManagedObjectReference only.
 */

public class NodeReference<T> extends ManagedObjectReference<T> {

	/**
	 * Create a new NodeReference
	 */
	public NodeReference(SpatialOrganizationManager<T> som, long address){
		super(address,som);
	}
	
	/**
	 * @return a reference to the Managed Object
	 */
	public ManagedObjectReference<T> getReference(){
		NodeReference<T> ret = new NodeReference(som, address);
		return ret; 
		
	}
	
	/**
	 * @return a local copy of the SpaceNode that this reference is pointing to.
	 * @deprecated
	 */
    public SpaceNode<T> getLocalCopy() {
    		try{
    			return som.getCopyOfSpaceNode(this, null);
		}catch(NodeLockedException nle){
			//TODO - something here
			return null;
		}catch (ManagedObjectDoesNotExistException e) {
			return null;
		}
    }

}


