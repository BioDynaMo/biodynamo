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

import ini.cx3d.simulations.ECM;

import java.awt.Color;

public class SomeColors {
	
	
	public static Color LILAC = new Color(191,179,254);
	public static Color MAUVE = new Color(220,176,254);
	public static Color LIGHT_PINK = new Color(254,176,254);
	public static Color LIGHT_YELLOW = new Color(254,254,134);
	public static Color LIGHT_GREEN = new Color(142,252,134);
	
	public static Color GREY_1 = new Color(221,225,217);
	public static Color GREY_2 = new Color(176,179,172);
	public static Color GREY_3 = new Color(136,138,133);
	
	public static Color RED_1 = new Color(255,180,203);
	public static Color RED_2 = new Color(255,128,167);
	
	public static Color YELLOW_1 = new Color(255,255,107);
	
	public static Color GREEN_1 = new Color(177,255,177);
	
	
	public static Color getRandomColor(){
		return new Color((float) ECM.getRandomDouble(),(float)ECM.getRandomDouble(),(float) ECM.getRandomDouble(),0.1f);
	}
	
	
	
	
}
