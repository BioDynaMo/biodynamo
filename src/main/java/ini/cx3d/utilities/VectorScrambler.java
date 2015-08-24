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

package ini.cx3d.utilities;

import java.util.Arrays;
import java.util.Hashtable;
import java.util.Random;
import java.util.Vector;


/** An instance of this class is used to get a random permutation of the elements of a vector
 * (<code>java.util.Vector</code>). The vector given as argument to the method <code>scrambleVector</code>
 * is left unchanged, and a new one with the same elements in a different order is returned.
 * <p>
 * <B>Example: </B>
 * <p>
 * 
 * <dir>
 * We want to permute <code>allMyObjects</code>, a <code>Vector</code> of instances of the class </code>myObject<code>:<br>
 * <p>
 * 
 * <code>
 * VectorScrambler&tmyObject&gt s = new VectorScrambler&ltmyObject&gt() ; <br> 
 * allMyObjects = s.scrambleVector(allMyObjects);<br> 
 * <code>
 * </dir>
 *
 * @author fredericzubler
 *
 */
public class VectorScrambler<T> {
	/**
	 * Random permutation of elements.
	 * @param v the input vector
	 * @return a vector with the same elements but in a random order
	 */
	public Vector<T> scrambleVector(Vector<T> v){
		Hashtable<Double, T> table = new Hashtable<Double, T>(v.size());
		double[] doubles = new double[v.size()];
		Random r = new Random();
		for (int i = 0; i < doubles.length; i++) {
			double d = r.nextDouble();
			doubles[i] = d;
			table.put(d, v.get(i));
		}


		Arrays.sort(doubles, 0, doubles.length-1);
//		java.util.Arrays.sort(doubles);
		Vector<T> vScrabled = new Vector<T>(doubles.length);
		for (int i = 0; i < doubles.length; i++) {
			vScrabled.add(table.get(doubles[i]));
		}
		return vScrabled;
	}
	
}
