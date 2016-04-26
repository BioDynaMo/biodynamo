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

package ini.cx3d.localBiology;

import static ini.cx3d.SimStateSerializationUtil.keyValue;
import static ini.cx3d.SimStateSerializationUtil.removeLastChar;
import static ini.cx3d.utilities.Matrix.add;
import static ini.cx3d.utilities.Matrix.scalarMult;
import static ini.cx3d.utilities.Matrix.subtract;

import ini.cx3d.JavaUtil2;
import ini.cx3d.Param;
import ini.cx3d.localBiology.factory.NeuriteElementFactory;
import ini.cx3d.physics.interfaces.PhysicalCylinder;
import ini.cx3d.simulations.ECMFacade;
import ini.cx3d.simulations.interfaces.ECM;
import ini.cx3d.synapses.factory.PhysicalSomaticSpineFactory;
import ini.cx3d.synapses.interfaces.BiologicalSomaticSpine;
import ini.cx3d.synapses.PhysicalSomaticSpine;
import ini.cx3d.synapses.factory.BiologicalSomaticSpineFactory;

import java.util.AbstractSequentialList;
import java.util.Iterator;
import java.util.LinkedList;

/**
 * This class contains the description of the biological properties of a soma (if it contains
 * instances of <code>LocalBiologyModule</code>. It is asociated with a <code>PhysicalSphere</code>.
 * 
 * @author fredericzubler
 *
 */
public class SomaElement extends ini.cx3d.swig.simulation.SomaElement implements ini.cx3d.localBiology.interfaces.SomaElement {

	static {
		ini.cx3d.swig.simulation.CellElement.setECM(ECMFacade.getInstance());
	}

	/* The PhysicalSphere associated with this SomaElement.*/
	private ini.cx3d.physics.interfaces.PhysicalSphere physical = null ;

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		superSuperSimStateToJson(sb);

		keyValue(sb, "physical", physical);
		//cell is a circular reference
		removeLastChar(sb);
		sb.append("}");
		return sb;
	}

	
	// *************************************************************************************
	//   Constructor and divide
	// *************************************************************************************
	public SomaElement(){
		super();
		ini.cx3d.swig.simulation.CellElement.registerJavaObject(this);
		ini.cx3d.swig.simulation.SomaElement.registerJavaObject(this);
		ECMFacade.getInstance().addSomaElement(this);
	}

	@Override
	public SomaElement divide(double volumeRatio, double phi, double theta){
		SomaElement newSoma = new SomaElement();
		ini.cx3d.physics.interfaces.PhysicalSphere pc = physical.divide(volumeRatio, phi, theta);
		newSoma.setPhysical(pc);   // this method also sets the callback

		// Copy of the local biological modules:
		for (LocalBiologyModule m : getLocalBiologyModulesList()) {
			if(m.isCopiedWhenSomaDivides()){
				LocalBiologyModule m2 = m.getCopy();
				newSoma.addLocalBiologyModule(m2);
			}
		}
		return newSoma;
	}


	// *************************************************************************************
	//   Run
	// *************************************************************************************

	@Override
	public void run(){
		runLocalBiologyModules();
	}
	

	// *************************************************************************************
	//   Extend neurites
	// *************************************************************************************

	
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite(){
		return extendNewNeurite(Param.NEURITE_DEFAULT_DIAMETER);
	}
	
	/**
	 * Extends a new neurite at a random place on the sphere
	 * @param diameter the diameter of the new neurite
	 * @return
	 */
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite(double diameter) {
		// find random point on sphere (based on : http://www.cs.cmu.edu/~mws/rpos.html)
//		
		
		
//		double R = physical.getDiameter()*0.5;
//		double z = - R + R*ecm.getRandomDouble();
//		double phi = Math.asin(z/R);
//		double theta = 6.28318531*ecm.getRandomDouble();
		
		//andreas thinks this gives a better distribution based on some friends of mine.
		double phi =(JavaUtil2.getRandomDouble()-0.5f)*2*Math.PI;
		double theta =Math.asin(JavaUtil2.getRandomDouble()*2-1) + Math.PI/2;

		return extendNewNeurite(diameter ,phi, theta);
	}

	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite(double[] directionInGlobalCoordinates){
		// we do this cause transform is for 2 points in space and not for a direction:
		double[] dir = add(directionInGlobalCoordinates, physical.getMassLocation());
		double[] angles = physical.transformCoordinatesGlobalToPolar(dir);
		return extendNewNeurite(Param.NEURITE_DEFAULT_DIAMETER, angles[1], angles[2]);
	}
	
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite(double diameter, double[] directionInGlobalCoordinates){
		// we do this cause transform is for 2 points in space and not for a direction:
		double[] dir = add(directionInGlobalCoordinates, physical.getMassLocation());
		double[] angles = physical.transformCoordinatesGlobalToPolar(dir);
		return extendNewNeurite(diameter, angles[1], angles[2]);
	}
	
	/**
	 * Extends a new neurites
	 * @param diameter the diameter of the new neurite
	 * @param phi the angle from the zAxis
	 * @param theta the angle from the xAxis around the zAxis
	 * @return
	 */
	@Override
	public ini.cx3d.localBiology.interfaces.NeuriteElement extendNewNeurite(double diameter, double phi, double theta) {
		// creating the new NeuriteElement and PhysicalCylinder, linking them
		double lengthOfNewCylinder = Param.NEURITE_DEFAULT_ACTUAL_LENGTH;
		ini.cx3d.localBiology.interfaces.NeuriteElement ne = NeuriteElementFactory.create();
		PhysicalCylinder pc = physical.addNewPhysicalCylinder(lengthOfNewCylinder, phi, theta);
		ne.setPhysical(pc);
		// setting diameter for new branch
		pc.setDiameter(diameter, true);
		// setting ref for Cell
		ne.setCell(getCell());
		// copy of the biological modules
		for (LocalBiologyModule module : getLocalBiologyModulesList()) {
			if(module.isCopiedWhenNeuriteExtendsFromSoma())
				ne.addLocalBiologyModule(module.getCopy());
		}
		// return the new neurite	
		return ne;
	}
	
	
	// Roman: Begin
	

	// *************************************************************************************
	//   Synapses
	// **
	
	/**
	 * Makes somatic spines (the physical and the biological part) dependent on some parameter (e.g. substance concentration) on this NeuriteElement.
	 */
	public void makeSomaticSpines(double p, double maxNr){
		
		// how many spines for this NeuriteElement ?
		double radius = physical.getDiameter()/2;
	
		// TODO : better way to define number (ex : if interval >> length -> no spine at all) 

		for (int i = 0; i < maxNr; i++) {
			
			if (physical.getExcrescences().size() > maxNr) {
				return;
			}
			
			if (Math.random()<p) {
				// create the physical part
				double[] coord = {radius, Math.PI*JavaUtil2.getRandomDouble(), 2*Math.PI*JavaUtil2.getRandomDouble()};
				ini.cx3d.synapses.interfaces.PhysicalSomaticSpine pSomSpine = PhysicalSomaticSpineFactory.create(physical, coord, 0.1);
				physical.addExcrescence(pSomSpine);
				System.out.println(physical.getID());
				// create the biological part and set call backs
				BiologicalSomaticSpine bSomSpine = BiologicalSomaticSpineFactory.create();
				pSomSpine.setBiologicalSomaticSpine(bSomSpine);
				bSomSpine.setPhysicalSomaticSpine(pSomSpine);
			}
		}
			
	}
	// Roman

	// *************************************************************************************
	//   Getters & Setters
	// *************************************************************************************

	@Override
	public ini.cx3d.physics.interfaces.PhysicalObject getPhysical() {
		return this.physical;
	}

	@Override
	public void setPhysical(ini.cx3d.physics.interfaces.PhysicalObject physical) {
		this.physical = (ini.cx3d.physics.interfaces.PhysicalSphere) physical;
		this.physical.setSomaElement(this); // callback
	}

	@Override
	public ini.cx3d.physics.interfaces.PhysicalSphere getPhysicalSphere(){
		return physical;
	}

	@Override
	public void setPhysicalSphere(ini.cx3d.physics.interfaces.PhysicalSphere physicalsphere){
		physical = physicalsphere;
		physical.setSomaElement(this);
	}

	@Override
	public AbstractSequentialList<ini.cx3d.localBiology.interfaces.NeuriteElement>  getNeuriteList() {
		AbstractSequentialList<ini.cx3d.localBiology.interfaces.NeuriteElement> neuriteList = new LinkedList<ini.cx3d.localBiology.interfaces.NeuriteElement>();
		AbstractSequentialList<ini.cx3d.physics.interfaces.PhysicalCylinder> pcList = physical.getDaughters();
		for (Iterator<ini.cx3d.physics.interfaces.PhysicalCylinder> element = pcList.iterator(); element.hasNext();) {
			ini.cx3d.physics.interfaces.PhysicalCylinder pc = element.next();
			neuriteList.add(pc.getNeuriteElement());
		}        
		return neuriteList ;
	}

	@Override
	public boolean isANeuriteElement() {
		return false;
	}
	
	/** Returns true, because this <code>CellElement</code> is a <code>SomaElement</code>.*/ 
	@Override
	public boolean isASomaElement(){
		return true;
	}


}
