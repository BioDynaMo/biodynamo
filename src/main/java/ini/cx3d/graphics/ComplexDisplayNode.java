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

package ini.cx3d.graphics;

import java.util.HashMap;

public class ComplexDisplayNode extends DisplayNode{
	
	private HashMap<String, DisplayNode> values = new HashMap<String, DisplayNode>();
	
	public void addSimpleDisplayNode(String name,String value)
	{
		values.put(name, new SimpleDisplayNode(value));
	}
	
	public void addCompelxDisplayNode(String name,ComplexDisplayNode value)
	{
		values.put(name, value);
	}
	
	public String toString(int depth, String indent)
	{
		if(depth==0) return "";
		String temp = "\n";
		for (String n : values.keySet()) {
			temp+=indent+n+": "+values.get(n).toString(depth-1, indent+ "  ")+"\n";
		}
		return temp;
	}
}
