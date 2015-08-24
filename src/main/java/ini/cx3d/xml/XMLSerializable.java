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

package ini.cx3d.xml;

import org.w3c.dom.Node;


/**
 * This interface will make the implementer able to serialize and deserialize itself from/to xml. 
 * Each class that implements this interface should have a constructer that generates and empty sceleton of object.
 * A factory such as <code>XMLGenRegNetworkFactory</code> should handle the deserialisation 
 * of the objects that implement this interface.
 * 
 * @author andreashauri
 *
 */

public interface XMLSerializable {
	/**
	 * The implementation of this method should return a complete xml representation of the Object.
	 * @param ident how much the current block of xml is to be idented. to make a nice readable structure.
	 * @return The xml string of the object.
	 */
	public StringBuilder toXML(String ident);
	/**
	 * The implementation of this method should take an xml representation of the object and fill an 
	 * empty sceleton of this object with the data contained in the xml
	 * @param xml the XML node that represents the objects content.
	 * @return the deserialized object.
	 */
	public XMLSerializable fromXML(Node xml);
}
