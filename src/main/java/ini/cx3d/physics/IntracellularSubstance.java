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

import ini.cx3d.Param;

import org.w3c.dom.Node;

/** 
 * Instances of this class represent the intracellular and surface (membrane bound) 
 * Substances. The visibility from outside (fact that they are expressed on the surface)
 * is specified by the appropriate getter and setter. 
 * 
 * @author fredericzubler
 *
 */
public class IntracellularSubstance extends Substance{
	
	/* If true, the Substance can be detected from outside of the PhysicalObject
	 * (equivalent to an membrane bound substance).*/
	private boolean visibleFromOutside = false;
	/* If true, the volume is taken into account for computing the concentration,
	 * otherwise quantity and volume are considered as equivalent (effective volume = 1).*/
	private boolean volumeDependant = false;
	
	/* For degradation we need to know the concentration that was present at the previous time step! */
	
	/* Degree of asymmetric distribution during cell division. 
	 * 0 represents complete symmetrical division, 1 represents complete asymmetric division. */
	protected double asymmetryConstant = 0;
	
	public IntracellularSubstance(){
	}
	
	public IntracellularSubstance(String id, double diffusionConstant, double degradationConstant){
		super(id, diffusionConstant, degradationConstant);
	}
	
	public IntracellularSubstance(String id, double diffusionConstant, double degradationConstant, double asymmetryConstant){
		super(id, diffusionConstant, degradationConstant);
		getRwLock().writeLock().lock();
		this.asymmetryConstant = asymmetryConstant;
		getRwLock().writeLock().unlock();
	}
	
	
	/**
	 * Copies the physical properties of the IntracellularSubstance given as argument 
	 * (id, diffusionConstant, degradationConstant, color, visibleFromOutside, volumeDependant), 
	 * but : QUANTITY AND CONCENTRATION ARE NOT COPIED !!
	 * @param templateSubstance
	 */
	public IntracellularSubstance(IntracellularSubstance templateIntracellularSubstance){
		// CAUTION : only the physical properties are copied. Quantity and concentration are not
		super(templateIntracellularSubstance);
		getRwLock().writeLock().lock();
		this.visibleFromOutside = templateIntracellularSubstance.isVisibleFromOutside();
		this.volumeDependant = templateIntracellularSubstance.isVolumeDependant();
		this.asymmetryConstant = templateIntracellularSubstance.getAsymmetryConstant();
		getRwLock().writeLock().unlock();
	}
	
	/**
	 * Distribute IntracellularSubstance concentration at division and update quantity.
	 * @param newIS
	 */
	public void distributeConcentrationOnDivision(IntracellularSubstance newIS){
		
		
		getRwLock().writeLock().lock();
		
		double oldConcentration = concentration; // fred debugg
//		System.out.println("con: "+getConcentration()+" type "+this);
		double p = asymmetryConstant*(1-concentration)*concentration;
		newIS.setConcentration(this.concentration * (1+p));
		this.concentration = this.concentration * (1-p);
//	 	System.out.println("con2: "+this.concentration);
		
		
		getRwLock().writeLock().unlock();
	}
	
	
	
	/**
	 * Degradation of the <code>IntracellularSubstance</code>.
	 * @param newIS
	 */
	public void degrade(){
		getRwLock().writeLock().lock();
		this.concentration = this.concentration * (1 - this.degradationConstant*Param.SIMULATION_TIME_STEP);
		getRwLock().writeLock().unlock();
	}
	
	
	/** Returns the degree of asymmetric distribution during cell division. 
	 * 0 represents complete symmetrical division, 1 represents complete asymmetric division. */
	public double getAsymmetryConstant(){
		try
		{
			getRwLock().readLock().lock();
			return asymmetryConstant;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	/** Sets the degree of asymmetric distribution during cell division. 
	 * 0 represents complete symmetrical division, 1 represents complete asymmetric division.
	 * The sign + or - is used to distinguish between one daughter (mother cell) and the other
	 * (new cell). */
	public void setAsymmetryConstant(double asymmetryConstant){
		try
		{
			getRwLock().readLock().lock();
			this.asymmetryConstant = asymmetryConstant;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	
//	/**
//	 * Computes the quantity represented when the current concentration is maintained
//	 * but the volume changes. Important when PhysicalNodes are moved.
//	 * @param volume
//	 */
//	public void updateQuantityBasedOnConcentration(double volume){
//		getRwLock().writeLock().lock();
//		quantity = concentration*volume;
//		getRwLock().writeLock().unlock();
//	}
//	
//	/**
//	 * Computes the new concentration if the current quantity is distributed in a given volume.
//	 *  Important when PhysicalNodes are moved.
//	 * @param volume
//	 */
//	public void updateConcentrationBasedOnQuantity(double volume){
//		getRwLock().writeLock().lock();
//		setConcentration(quantity/volume);
//		getRwLock().writeLock().unlock();
//	}
	
	/** If true, the Substance can be detected from outside of the PhysicalObject
	 * (equivalent to an membrane bound substance).*/
	public boolean isVisibleFromOutside() {
		try
		{
			getRwLock().readLock().lock();
			return visibleFromOutside;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	
	/** If true, the Substance can be detected from outside of the PhysicalObject
	 * (equivalent to an membrane bound substance).*/
	public void setVisibleFromOutside(boolean visibleFromOutside) {
		try
		{
			getRwLock().readLock().lock();
			this.visibleFromOutside = visibleFromOutside;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	/** If true, the volume is taken into account for computing the concentration,
	 * otherwise a virtual volume corresponding to the length of the physical object
	 * (with virtual radius 1) is used.*/
	public boolean isVolumeDependant() {
		try
		{
			getRwLock().readLock().lock();
			return volumeDependant;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}
	/** If true, the volume is taken into account for computing the concentration,
	 * otherwise a virtual volume corresponding to the length of the physical object
	 * (with virtual radius 1) is used.*/
	public void setVolumeDependant(boolean volumeDependant) {
		try
		{
			getRwLock().readLock().lock();
			this.volumeDependant = volumeDependant;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
	}

	public StringBuilder createXMLAttrubutes()
	{
		StringBuilder temp  = super.createXMLAttrubutes();
		try{
			getRwLock().readLock().lock();
			temp.append("visibleFromOutside=\"").append(this.isVisibleFromOutside()).append("\" ");
			temp.append("volumeDependent=\"").append(this.isVolumeDependant()).append("\" ");
			temp.append("asymmetryconstant=\"").append(this.asymmetryConstant).append("\" ");
			return temp;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}
	
	public void readOutAttributes(Node attr)
	{
		super.readOutAttributes(attr);
		if(attr.getNodeName().equals("visibleFromOutside"))
		{
			getRwLock().writeLock().lock();
			this.visibleFromOutside = Boolean.parseBoolean(attr.getNodeValue());
			getRwLock().writeLock().unlock();
		}
		if(attr.getNodeName().equals("volumeDependent"))
		{
			getRwLock().writeLock().lock();
			this.volumeDependant = Boolean.parseBoolean(attr.getNodeValue());
			getRwLock().writeLock().unlock();
		}
		if(attr.getNodeName().equals("asymmetryconstant"))
		{
			getRwLock().writeLock().lock();
			this.asymmetryConstant = Double.parseDouble(attr.getNodeValue());
			getRwLock().writeLock().unlock();
		}
		
	} 

	public Substance getCopy() { 
		// TODO Auto-generated method stub
		return new IntracellularSubstance(this);
	}
	
	
}
