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

/**
 * This class contains some methods for measuring execution speed and memory.
 * @author fredericzubler
 *
 */
public abstract class SystemUtilities {
	
	/** Prints the total memory used (with Runtime.getRuntime.totalMemory().*/
	public static void totalMemory(){
		System.out.print("Total memory : ");
		System.out.print(Runtime.getRuntime().totalMemory() / 1000000d);
		System.out.println(" kb.");
	}
	// the time when the chronometer is started.
	private static long clicValue;
	
	/** Starts the chronometer.*/
	public static void tic(){
		clicValue = System.currentTimeMillis();
	}
	
	/** Prints the time lapse since the chronometer was last started.*/
	public static void tac(){
		long endTime = System.currentTimeMillis();
		long timeElapsed = endTime-clicValue;
		if(timeElapsed<1000){
			System.out.println(timeElapsed+" ms");
		}else{
			System.out.println(((double)timeElapsed)/1000.0+" s");
		}
	}
	
	/** Prints the time lapse since the chronometer was last started,
	 * the restarts it again.*/
	public static void tacAndTic(){
		long endTime = System.currentTimeMillis();
		System.out.println((endTime-clicValue)+" ms.");
		clicValue = System.currentTimeMillis();
	}
	
	/** Pause in the execution of the  current thread. */
	public static void freeze(int time){
		try {
			Thread.sleep(time);
		} catch (Exception e){}
	}

}
