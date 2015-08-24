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

package ini.cx3d.physics;

import static ini.cx3d.utilities.Matrix.add;
import ini.cx3d.simulations.ECM;
import ini.cx3d.spatialOrganization.SpatialOrganizationNode;
import ini.cx3d.spatialOrganization.SpatialOrganizationNodeMovementListener;

import java.util.Hashtable;
import java.util.Iterator;
import java.util.Vector;

/**
 * The role of this class is a) to determinate the concentration of different chemicals 
 * inside a <code>PhysicalNode</code> when it is moved or added and b) to make sure that the total quantity
 * is conserved when <code>PhysicalNode</code> are added, moved or removed.
 * To be efficient, the methods have to be executed right before or after the node operation.
 * Therefore the class implements the SpatialOrganizationNodeMovementListener interface.
 * 
 * @author fredericzubler
 */
public class PhysicalNodeMovementListener implements SpatialOrganizationNodeMovementListener<PhysicalNode> {

	// flag we put into each neighboring PhysicalNode before the move
	// so we recognize a new neighbor after the movement has occurred.
	private  static int movementOperationId = (int)(10000*ECM.getRandomDouble());
	// all extracellularSubstances present in this PhysicalNode. 
	private Substance[] substancesInN;
	// respective quantity of the extracellularSubstances before the move. 
	private double[] q;
	// all the neighbors of the PhysicalNode.
	private Iterable<PhysicalNode> neighborsBefore;

	//----------------------------------------------------------------------
	// MASS CONSERVATION WHEN A POINT IS MOVED :
	// We compute the total of the quantity in the node to be moved and in all
	// the nodes that will be affected (it means neighbors before and neighbors
	// after the move, that are not always the same). The total quantity before
	// and after the move defines a ratio. We then multiply the concentration
	// in all the participants by this ration -> total mass by definition is the same.
	// (Implementation note : the neighbors after the movement are only known after
	// the move, i.e. when the second method is called).
	//----------------------------------------------------------------------

	public void nodeAboutToMove(SpatialOrganizationNode<PhysicalNode> n, double[] planedMovement) {
		PhysicalNode pn = n.getUserObject();
		neighborsBefore = n.getPermanentListOfNeighbors();
		Hashtable<String, Substance> extracellularSubstancesInPn = pn.getExtracellularSubstances();
		substancesInN = new Substance[extracellularSubstancesInPn.size()];
		q = new double[substancesInN.length];
		movementOperationId = (movementOperationId+1)%1000000;
		// 1) Quantities summation in pn & pn's neighbors before the move:
		// (for all extracellularSubstances in the moving node)
		int i = 0;
		for (Substance s : extracellularSubstancesInPn.values() ) {
			q[i] = s.getQuantity();
			substancesInN[i] = s;
			String sId = s.getId();
			for (PhysicalNode nn : neighborsBefore) {
				nn.setMovementConcentratioUpdateProcedure(movementOperationId);
				Substance ss = nn.getExtracellularSubstances().get(sId);
				if(ss!=null){
					q[i] += ss.getQuantity();
				}
			}
			i++;
		}
		// 2) computing the concentration at the future new location of pn (for every substance)

		// if possible : find the tetrahedron the new location belongs to :
		double[] futurePosition = add(pn.getSoNode().getPosition(), planedMovement);
		Object[] vertices = pn.getSoNode().getVerticesOfTheTetrahedronContaining(futurePosition);
		// if there is one : find by interpolation the new concentration :
		// we weight the concentration of each vertex by the barycentric coord of the new point
		if(vertices != null){
			double[] barycentricCoord = pn.getBarycentricCoordinates(futurePosition, vertices);
			for (i = 0; i < substancesInN.length; i++) { 
				String name = substancesInN[i].getId();
				double newConcentration = 0;
				for (int j = 0; j < 4; j++) {
					newConcentration += ((PhysicalNode)vertices[j]).getExtracellularConcentration(name)*barycentricCoord[j];
				}
				substancesInN[i].setConcentration(newConcentration);
			}
		}else{
			// if we can't find a tetra, we compute the gradient and multiply this by the displacement
			for (i = 0; i < substancesInN.length; i++) { 
				double newConcentration = pn.computeConcentrationAtDistanceBasedOnGradient(substancesInN[i], planedMovement);
				substancesInN[i].setConcentration(newConcentration);
			}
		}



	}

	public void nodeMoved(SpatialOrganizationNode<PhysicalNode> n) {
		PhysicalNode pn = n.getUserObject();
		Iterable<PhysicalNode> neighborsAfter = n.getNeighbors();
		Vector<PhysicalNode> newNeighbors = new Vector<PhysicalNode>();

		// 3) identifying the really new neighbors of n 
		// (i.e. the ones that were not neighbors before the movement)
		for (PhysicalNode nn : neighborsAfter) {
			if(nn.getMovementConcentratioUpdateProcedure() != movementOperationId){ 
				newNeighbors.add(nn);
			}
		}

		// 4) adding all the new neighbors contribution to the total quantity before the move:
		for (int i = 0; i < substancesInN.length; i++) {
			for (PhysicalNode nn : newNeighbors) {
				Substance ss = nn.getExtracellularSubstances().get(substancesInN[i].getId());
				if(ss!=null){
					q[i] += ss.getQuantity();
				}
			}
		}

		// For all extracellularSubstances:		
		for (int i = 0; i < substancesInN.length; i++) {	

			// 5) Update the quantities in every cell that has been affected, and sum it 
			// 5.a) in pn itself
			Substance s = substancesInN[i];
			s.updateQuantityBasedOnConcentration(pn.getSoNode().getVolume()); 
			double quantityAfter = s.getQuantity();
			// 5.b) in the old neighbors
			for (PhysicalNode nn : neighborsBefore) {
				Substance ss = nn.getExtracellularSubstances().get(s.getId());
				if(ss!=null){
					ss.updateQuantityBasedOnConcentration(nn.getSoNode().getVolume());  
					quantityAfter += ss.getQuantity();
				}
			}
			// 5.c) in the NEW neighbors
			for (Iterator<PhysicalNode> iter = newNeighbors.iterator(); iter.hasNext();) {
				PhysicalNode nn = iter.next();
				Substance ss = nn.getExtracellularSubstances().get(s.getId());
				if(ss!=null){
					ss.updateQuantityBasedOnConcentration(nn.getSoNode().getVolume());  
					quantityAfter += ss.getQuantity();
				}
			}

			// 6) defining a ratio of quantity change (quantity before / quantity after) for the i-th substance
			if(quantityAfter<1.0E-14){
				q[i] = 0; 		// (avoid division by 0)
			}else{
				q[i] /= quantityAfter;  
//				q[i] = 1;     	// de-comment this for DEACTIVATION !!!!!!!!!!!!!!!!!!!!!
			}

			// 7) changing the concentration of the i-th substance by its ratio
			// 7.a) in pn
			s.multiplyQuantityAndConcentrationBy(q[i]);
			// 7.b) in the old neighbors
			for (PhysicalNode nn : neighborsBefore) {
				Substance ss = nn.getExtracellularSubstances().get(s.getId());
				// Note : should not use PhysicalNode.giveYourSubstanceInstance(), because never returns null
				if(ss!=null){
					ss.multiplyQuantityAndConcentrationBy(q[i]);
				}
			}
			// 7.c) in the NEW neighbors
			for (Iterator<PhysicalNode> iter = newNeighbors.iterator(); iter.hasNext();) {
				PhysicalNode nn = iter.next();
				Substance ss = nn.getExtracellularSubstances().get(s.getId());
				if(ss!=null){
					ss.multiplyQuantityAndConcentrationBy(q[i]);
				}
			}
		}

	}

	//----------------------------------------------------------------------
	// MASS CONSERVATION WHEN A NEW POINT IS ADDED :
	// Before the point is added, we compute the concentrations at its future location
	// (if it is inside a tetrahedron, otherwise we don't add any substances and the 
	// concentration will be zero !!!!). After the node is added, we know its neighbors.
	// We then compute the summed quantity of the neighbors before volume update, we
	// then update quantities in everyones and compute the sum again (this time in 
	// including the new node). The concentration in the ex-neighbors is multiplied
	// by the ratio of the two sums.
	//----------------------------------------------------------------------

	public void nodeAboutToBeAdded(SpatialOrganizationNode<PhysicalNode> n, double[] planedPosition, Object[] vertices) {
		// (Reminder : vertices is an array of the userObject in the tetrahedron inside which the new point lies. If
		// no tetrahedron is found around the location, vertices is null).
		// 1) Computing the concentration for new inserted point
		PhysicalNode pn = n.getUserObject();
		if(vertices != null){
			PhysicalNode pnn = (PhysicalNode)vertices[0]; // a future neighbor of the PhysicalNode about to be inserted
			// (we have to rely on it to know the chemicals present )
			double[] barycentricCoord = PhysicalNode.getBarycentricCoordinates(planedPosition, vertices);

			for (Substance s : pnn.getExtracellularSubstances().values() ) {
				String name = s.getId();
				double newConcentration = 0;
				for (int j = 0; j < 4; j++) {
					newConcentration += ((PhysicalNode)vertices[j]).getExtracellularConcentration(name)*barycentricCoord[j];
				}
				Substance newSubstance = new Substance(s);
				newSubstance.setConcentration(newConcentration);
				pn.getExtracellularSubstances().put(name, newSubstance);
			}
		}
	}


	public void nodeAdded(SpatialOrganizationNode<PhysicalNode> n) {
		// 2) sum the quantity before update 
		PhysicalNode pn = n.getUserObject(); 
		// since there might be no substances yet in the point 
		// pn, we take a neighbor as furnishing the templates
		Iterable<PhysicalNode> neighbors = n.getPermanentListOfNeighbors();
		PhysicalNode pnn = neighbors.iterator().next(); 
		substancesInN = new Substance[pnn.getExtracellularSubstances().size()];
		q = new double[substancesInN.length];
		int i = 0;
		for (Substance s : pnn.getExtracellularSubstances().values() ) {
			q[i] = 0;
			substancesInN[i] = s;
			for (PhysicalNode nn : neighbors) {
				Substance ss = nn.getExtracellularSubstances().get(s.getId());
				if(ss!=null){
					q[i] += ss.getQuantity();
				}
			}
			i++;
		}


		// 3) update quantities in all nodes, and sum it :
		for (i = 0; i < substancesInN.length; i++) {	
			double quantityAfter = 0;
			// 3.a) in pn itself
			Substance s = substancesInN[i];
			Substance ss = pn.getExtracellularSubstances().get(s.getId());
			if(ss!=null){
				ss.updateQuantityBasedOnConcentration(n.getVolume());  
				quantityAfter += ss.getQuantity();
			}

			// 3.b) in the  neighbors
			for (PhysicalNode nn : n.getNeighbors()) {
				ss = nn.getExtracellularSubstances().get(s.getId());
				if(ss!=null){
					ss.updateQuantityBasedOnConcentration(nn.getSoNode().getVolume());  
					quantityAfter += ss.getQuantity();
				}
			}


			// 4) defining a ratio of quantity change (quantity before / quantity after) for the i-th substance
			if(quantityAfter<1.0E-14){
				q[i] = 0; //(avoid division by 0)
			}else{
				q[i] /= quantityAfter;  
//				q[i] = 1;     // De-comment this for DEACTIVATION !!!!!!!!!!!!!!!!!!!!!
			}


			// 5) changing the concentration of the i-th substance by its ratio
			// 5.a) in pn
			ss = pn.getExtracellularSubstances().get(s.getId());
			if(ss!=null){
				ss.multiplyQuantityAndConcentrationBy(q[i]);
			}
			// 5.b) in the neighbors
			for (PhysicalNode nn : n.getNeighbors()) {
				ss = nn.getExtracellularSubstances().get(s.getId());
				if(ss!=null){
					ss.multiplyQuantityAndConcentrationBy(q[i]);
				}
			}
		}
	}


	//----------------------------------------------------------------------
	// MASS CONSERVATION WHEN A POINT IS REMOVED :
	// We sum the total quantity in the node that we remove and
	// its neighbors before the movement. This is compared to the sum
	// of what the neighbors have after the removal (note that their volume
	// is then bigger). The concentration in the ex-neighbors is multiplied
	// by the ratio of the two sums.
	//----------------------------------------------------------------------

	public void nodeAboutToBeRemoved(SpatialOrganizationNode<PhysicalNode> n) {
		PhysicalNode pn = n.getUserObject();
		neighborsBefore = n.getPermanentListOfNeighbors();
		substancesInN = new Substance[pn.getExtracellularSubstances().size()];
		q = new double[substancesInN.length];
		// 1) Quantities summation in pn & pn's neighbors (before pn is removed):
		// (for all extracellularSubstances in the moving node)
		int i = 0;
		for (Substance s : pn.getExtracellularSubstances().values() ) {
			q[i] = s.getQuantity();
			substancesInN[i] = s;
			for (PhysicalNode nn : neighborsBefore) {
				Substance ss = nn.getExtracellularSubstances().get(s.getId());
				if(ss!=null){
					q[i] += ss.getQuantity();
				}
			}
			i++;
		}
		// (Note : since the point is removed, we don't have to interpolate anything...)
	}

	public void nodeRemoved(SpatialOrganizationNode<PhysicalNode> n) {
		// For all extracellularSubstances:		
		for (int i = 0; i < substancesInN.length; i++) {	

			// 2) Update the quantities in the old neighbors, and sum it
			Substance s = substancesInN[i];
			double quantityAfter = 0;
			for (PhysicalNode nn : neighborsBefore) {
				Substance ss = nn.getExtracellularSubstances().get(s.getId());
				if(ss!=null){
					ss.updateQuantityBasedOnConcentration(nn.getSoNode().getVolume());  
					quantityAfter += ss.getQuantity();
				}
			}

			// 3) defining a ratio of quantity change (quantity before / quantity after) for the i-th substance
			if(quantityAfter<1.0E-14){
				q[i] = 0; //(avoid division by 0)
			}else{
				q[i] /= quantityAfter;  
//				q[i] = 1;     // De-comment this for DEACTIVATION !!!!!!!!!!!!!!!!!!!!!
			}

			// 4) changing the concentration of the i-th substance by its ratio, in the old neighbors
			for (PhysicalNode nn : neighborsBefore) {
				Substance ss = nn.getExtracellularSubstances().get(s.getId());
				if(ss!=null){
					ss.multiplyQuantityAndConcentrationBy(q[i]);
				}
			}
		}
	}



}
