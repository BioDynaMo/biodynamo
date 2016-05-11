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

import ini.cx3d.SimStateSerializationUtil;
import ini.cx3d.graphics.View;
import ini.cx3d.xml.XMLSerializable;

import java.awt.Color;
import java.util.concurrent.locks.ReadWriteLock;
import java.util.concurrent.locks.ReentrantReadWriteLock;

import org.w3c.dom.Node;

import static ini.cx3d.SimStateSerializationUtil.removeLastChar;

/**
 * Represents a diffusible or non-diffusible chemical, whether it is extra-cellular, membrane-bounded
 * or intra-cellular.
 * @author fredericzubler
 *
 */


public class Substance extends ini.cx3d.swig.simulation.Substance implements ini.cx3d.physics.interfaces.Substance {
	
	/* Name of the Substance.               */
	protected String id;
	/* How fast it is diffused by the methods in the PhysicalNode. */
	protected double diffusionConstant = 1000;
	/* How rapidly it is degraded by the PhysicalNode.*/
	protected double degradationConstant = 0.01;
	/* The color used to represent it if painted.*/
	protected Color color = Color.BLUE;

	/* The total amount present at a given PhysicalNode, or inside a PhysicalObject.*/
	protected double quantity = 0;
	/* Concentration = (quantity) / (volume of the PhysicalNode or PhysicalObject).*/
	protected double concentration = 0;
	
	/* used for synchronisation for multithreading. introduced by Haurian*/
	private ReadWriteLock rwLock = new ReentrantReadWriteLock();

	@Override
	public ini.cx3d.swig.NativeStringBuilder simStateToJson(ini.cx3d.swig.NativeStringBuilder sb) {
		sb.append("{");

		SimStateSerializationUtil.keyValue(sb, "id", id, true);
		SimStateSerializationUtil.keyValue(sb, "diffusionConstant", diffusionConstant);
		SimStateSerializationUtil.keyValue(sb, "degradationConstant", degradationConstant);
		SimStateSerializationUtil.keyValue(sb, "color", SimStateSerializationUtil.colorToHex(color), true);
		SimStateSerializationUtil.keyValue(sb, "quantity", quantity);
		SimStateSerializationUtil.keyValue(sb, "concentration", concentration);

		removeLastChar(sb);
		sb.append("}");
		return sb;
	}

	/**
	 * only needed for SWIG - forwards call to SWIG Base class
	 */
	protected Substance(long cPtr, boolean cMemoryOwn) {
		super(cPtr, cMemoryOwn);
	}

	public Substance(){
		registerJavaObject(this);
	}

	/**
	 * Copies the physical properties of the substance given as argument (id, diffusionConstant,
	 * degradationConstant and color), but : QUANTITY AND CONCENTRATION ARE NOT COPIED !!
	 * @param templateSubstance
	 */
	public Substance(Substance templateSubstance){
		registerJavaObject(this);
		getRwLock().writeLock().lock();
		this.id = templateSubstance.id;
		this.diffusionConstant = templateSubstance.diffusionConstant;
		this.degradationConstant = templateSubstance.degradationConstant;
		this.color = templateSubstance.color;
		this.concentration = 0;
		this.quantity = 0;
		getRwLock().writeLock().unlock();
	}

	public Substance(String id, double diffusionConstant, double degradationConstant){
		registerJavaObject(this);
		getRwLock().writeLock().lock();
		this.id = id;
		this.diffusionConstant = diffusionConstant;
		this.degradationConstant = degradationConstant;
		this.quantity = 0;
		this.concentration = 0;
		getRwLock().writeLock().unlock();
	}

	/**
	 * Especially used for the "artificial gradient" formation, in ECM.
	 * @param id
	 * @param color
	 */
	public Substance(String id, Color color){
		registerJavaObject(this);
		getRwLock().writeLock().lock();
		this.id = id;
		this.color = color;
		getRwLock().writeLock().unlock();
	}
	
	/**
	 * Increases or decreases the quantity. Makes sure the quantity is never negative.
	 * @param deltaQ
	 */
	@Override
	public void changeQuantityFrom(double deltaQ){
		getRwLock().writeLock().lock();
		quantity += deltaQ;
		if(quantity < 0){
			quantity = 0;
		}
		getRwLock().writeLock().unlock();
	}
	
	/** Well, as the name says, it multiplies the quantity and the concentration
	 * by a certain value. This method is mainly used for degradation .*/
	@Override
	public void multiplyQuantityAndConcentrationBy(double factor){
		getRwLock().writeLock().lock();
		quantity *= factor;
		setConcentration(concentration *= factor);
		getRwLock().writeLock().unlock();
	}
	
	/**
	 * Computes the quantity represented when the current concentration is maintained
	 * but the volume changes. Important when PhysicalNodes are moved.
	 * @param volume
	 */
	@Override
	public void updateQuantityBasedOnConcentration(double volume){
		getRwLock().writeLock().lock();
		quantity = concentration*volume;
		getRwLock().writeLock().unlock();
		
	}
	
	/**
	 * Computes the new concentration if the current quantity is distributed in a given volume.
	 *  Important when PhysicalNodes are moved.
	 * @param volume
	 */
	@Override
	public void updateConcentrationBasedOnQuantity(double volume){
		getRwLock().writeLock().lock();
		setConcentration(quantity/volume);
		getRwLock().writeLock().unlock();
	}
	
	/**
	 * Determines whether an other object is equal to this Substance. 
	 * <br>The result is <code>true</code> if and only if the argument 
	 * is not null and is a Substance object with the same id, color, 
	 * degradationConstant and diffusionConstant. The
	 * quantity and concentration are note taken into account.
	 */
	@Override
	public boolean equals(Object o){
		
		if (o instanceof Substance) {
			
			Substance s = (Substance) o;
			try{
				this.getRwLock().readLock().lock(); //no need to sort here cause its only read lock!
				s.getRwLock().readLock().lock();
				if(		s.id.equals(this.id) && 
						s.color.equals(this.color) &&
						Math.abs(s.degradationConstant - this.degradationConstant)<1E-10 &&
						Math.abs(s.diffusionConstant - this.diffusionConstant)<1E-10 
				){
					return true;
				}
			}
			finally{
				this.getRwLock().readLock().unlock(); 
				s.getRwLock().readLock().unlock();
			}
		}
		return false;
	}
	
	/**
	 * Returns the color scaled by the concentration. Useful for painting PhysicalObjects / PhysicalNode
	 * based on their Substance concentrations.
	 * @return scaled Color
	 */
	@Override
	public Color getConcentrationDependentColor(){
		try{
			getRwLock().readLock().lock();
			int alpha = (int)(255.0*concentration*View.chemicalDrawFactor);
			//System.out.println(alpha);
			if(alpha<0){
				alpha = 0;
			}else if(alpha>255){
				alpha = 255;
			}
			return new Color(color.getRed(), color.getGreen(), color.getBlue(),alpha );
		}
		finally
		{
		getRwLock().readLock().unlock();
		}
	}
	


	// --------- GETTERS & SETTERS--------------------------------------------------------
	

	@Override
	public String getId() {
		return id;
	}

	@Override
	public void setId(String id) {
		try{
			getRwLock().readLock().lock();
			this.id = id;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
		
	}

	@Override
	public double getDiffusionConstant() {
		try{
			getRwLock().readLock().lock();
			return diffusionConstant;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}
		
	}

	@Override
	public void setDiffusionConstant(double diffusionConstant) {
		
		getRwLock().writeLock().lock();
		this.diffusionConstant = diffusionConstant;
		getRwLock().writeLock().unlock();
		
	}

	@Override
	public double getDegradationConstant() {
		try{
			getRwLock().readLock().lock();
			return degradationConstant;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}

	@Override
	public void setDegradationConstant(double degradationConstant) {
		getRwLock().writeLock().lock();
		this.degradationConstant = degradationConstant;
		getRwLock().writeLock().unlock();
	}

	@Override
	public Color getColor() {
		try{
			getRwLock().readLock().lock();
			return color;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}

	@Override
	public void setColor(Color color) {
		getRwLock().writeLock().lock();
		this.color = color;
		getRwLock().writeLock().unlock();
	}

	@Override
	public double getConcentration() {
		try{
			getRwLock().readLock().lock();
			return concentration;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
		
	}

	@Override
	public void setConcentration(double concentration) {
		getRwLock().writeLock().lock();
		if(concentration<0.0){
			this.concentration = 0.0; //fixme bug - was concentration = 0.0;
		}
		else{
			this.concentration = concentration;
		}
		getRwLock().writeLock().unlock();
	}

	@Override
	public double getQuantity() {
		try{
			getRwLock().readLock().lock();
			return quantity;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
			
	}

	@Override
	public void setQuantity(double quantity) {
		getRwLock().writeLock().lock();
		this.quantity = quantity;
		getRwLock().writeLock().unlock();
	}
	
	protected StringBuilder createXMLAttrubutes()
	{
		try{
			getRwLock().readLock().lock();
			StringBuilder temp= new StringBuilder();
			temp.append("name=\"").append(this.id).append("\" ");
			temp.append("concentration=\"").append(this.concentration).append("\" ");
			temp.append("color=\"").append(this.color.getRGB()).append("\" ");
			temp.append("degradationConstant=\"").append(this.degradationConstant).append("\" ");
			temp.append("diffusionConstant=\"").append(this.diffusionConstant).append("\" ");
			temp.append("quantity=\"").append(this.quantity).append("\" ");
			return temp;
		}
		finally
		{
			getRwLock().readLock().unlock();
		}	
	}
	
	protected void readOutAttributes(Node attr)
	{
		if(attr.getNodeName().equals("name"))
		{
			getRwLock().writeLock().lock();
			this.id = attr.getNodeValue();
			getRwLock().writeLock().unlock();
		}
		else if(attr.getNodeName().equals("concentration"))
		{
			getRwLock().writeLock().lock();
			this.concentration = Double.parseDouble(attr.getNodeValue());
			getRwLock().writeLock().unlock();
		}
		else if(attr.getNodeName().equals("color"))
		{
			getRwLock().writeLock().lock();
			this.color = new Color(Integer.parseInt(attr.getNodeValue()));
			getRwLock().writeLock().unlock();
		}
		else if(attr.getNodeName().equals("degradationConstant"))
		{
			getRwLock().writeLock().lock();
			this.degradationConstant =  Double.parseDouble(attr.getNodeValue());
			getRwLock().writeLock().unlock();
		}
		else if(attr.getNodeName().equals("diffusionConstant"))
		{
			getRwLock().writeLock().lock();
			this.diffusionConstant =  Double.parseDouble(attr.getNodeValue());
			getRwLock().writeLock().unlock();
		}
		else if(attr.getNodeName().equals("quantity"))
		{
			getRwLock().writeLock().lock();
			this.quantity =  Double.parseDouble(attr.getNodeValue());
			getRwLock().writeLock().unlock();
		}	
	}
	
//	@Override
//	public XMLSerializable fromXML(Node xml) {
//
//		for(int i=0;i<xml.getAttributes().getLength();i++)
//		{
//			Node attr =  xml.getAttributes().item(i);
//			readOutAttributes(attr);
//		}
//		return this;
//	}
//
//	@Override
//	public StringBuilder toXML(String ident) {
//
//		StringBuilder temp = new StringBuilder();
//		temp.append(ident).append("<substance ").append(createXMLAttrubutes());
//		temp.append("\" />\n");
//		return temp;
//	}


//	@Override
	public ReadWriteLock getRwLock() {
		return rwLock;
	}
	
	@Override
	public ini.cx3d.physics.interfaces.Substance getCopy() {
		// TODO Auto-generated method stub
		return new Substance(this);
	}

	@Override
	public String toString() {
		return "SubstanceID" + id;
	}
}
