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

package ini.cx3d;

import java.awt.Color;
import java.util.Random;

import ini.cx3d.physics.Substance;

/**
 * This abstract class contains some reference values, parameters etc...
 * Units : time is measured in hours , space is in micrometers (microns).
 * @author fredericzubler
 *
 */
public abstract class Param {
	
	// Discretization (time and neurite segment length)
	/** Time between two simulation step, in hours.*/
	public static double SIMULATION_TIME_STEP = 0.01; 
	/** Maximum jump that a point mass can do in one time step. Useful to stabilize the simulation*/
	public static final double SIMULATION_MAXIMAL_DISPLACEMENT = 3.0;

	/** Maximum length of a discrete segment before it is cut into two parts.*/
	public static double NEURITE_MAX_LENGTH = 15.0 ;  	// usual value : 20
	/** Minimum length of a discrete segment before. If smaller it will try to fuse with the proximal one*/
	public static final double NEURITE_MIN_LENGTH = 2.0 ;	 	// usual value : 10

	// Diffusion (saving time by not running diffusion for too small differences)
	
	/** If concentration of a substance smaller than this, it is not diffused */
	public static final double MINIMAL_CONCENTRATION_FOR_EXTRACELLULAR_DIFFUSION = 1e-5;
	/** If absolute concentration difference is smaller than that, there is no diffusion*/
	public static final double MINIMAL_DIFFERENCE_CONCENTRATION_FOR_EXTRACELLULAR_DIFFUSION = 1e-5;
	/** If ration (absolute concentration difference)/concentration is smaller than that, no diffusion.*/
	public static final double MINIMAL_DC_OVER_C_FOR_EXTRACELLULAR_DIFFUSION = 1e-3;
	
	/** If concentration of a substance smaller than this, it is not diffused */
	public static final double MINIMAL_CONCENTRATION_FOR_INTRACELLULAR_DIFFUSION = 1e-10;
	/** If absolute concentration difference is smaller than that, there is no diffusion*/
	public static final double MINIMAL_DIFFERENCE_CONCENTRATION_FOR_INTRACELLULAR_DIFFUSION = 1e-7;
	/** If ration (absolute concentration difference)/concentration is smaller than that, no diffusion.*/
	public static final double MINIMAL_DC_OVER_C_FOR_INTRACELLULAR_DIFFUSION = 1e-4;
	
	
	// Neurites
	/** Initial value of the restingLength before any specification.*/
	public static final double NEURITE_DEFAULT_ACTUAL_LENGTH = 1.0;
	/** Diameter of an unspecified (= axon/dendrite) neurite when extends from the somaElement */
	public static final double NEURITE_DEFAULT_DIAMETER = 1.0;
	public static final double NEURITE_MINIMAL_LENGTH_FOR_BIFURCATION = 0; 
	/** Spring constant*/
	public static final double NEURITE_DEFAULT_SPRING_CONSTANT = 10; // 10;
	/** Threshold the force acting on a neurite has to reach before a move is made ( = static friction).*/
	public static final double NEURITE_DEFAULT_ADHERENCE = 0.1; 
	/** Rest to the movement ( = kinetic friction).*/
	public static final double NEURITE_DEFAULT_MASS = 1;		
	
	public static final double NEURITE_DEFAULT_TENSION = 0.0;
	

	// Somata
    /** CAUTION: not the radius but the diameter*/
    public static final double SPHERE_DEFAULT_DIAMETER = 20;
	/** Threshold the force acting on a somaElement has to reach before a move is made ( = static friction).*/
	public static final double SPHERE_DEFAULT_ADHERENCE = 0.4; 
	/** Restistance to the movement ( = kinetic friction).*/
	public static final double SPHERE_DEFAULT_MASS = 1;	

	
	
	
	// some colors, that we define, because we find them.. well beautiful.
	public static final Color YELLOW = new Color(1f, .83f, .12f, 0.7f ); 	// L1
	public static final Color X_LIGHT_YELLOW = new Color(1f, .83f, .12f, 0.3f ); 	
	public static final Color X_SOLID_YELLOW = new Color(1f, .83f, .12f, 1f ); 	
	
	public static final Color GREEN = new Color(.16f, 1f, 0.15f, 0.7f ); 	// L 2/3
	public static final Color X_LIGHT_GREEN = new Color(.16f, 1f, 0.15f, .03f ); 
	public static final Color SEA_GREEN = new Color(0.2f, 0.55f, 0.35f, 0.7f );
	
	public static final Color PINK = new Color(0f, 0.08f, 0.58f, 0.7f );
	public static final Color X_SOLID_PINK = new Color(0f, 0.08f, 0.58f, 1f );
	
	public static final Color RED = new Color(1f, 0.02f, 0.32f, 0.7f ); 		// L4
	public static final Color X_LIGHT_RED = new Color(1f, 0.02f, 0.32f, 0.3f ); 		
	public static final Color X_SOLID_RED = new Color(1f, 0.02f, 0.32f, 1f ); 	
	
	public static final Color ORANGE = new Color(1f, 0.55f, 0.0f, 0.7f ); 	
	public static final Color X_SOLID_ORANGE = new Color(1f, 0.55f, 0.0f, 1f );
	
	public static final Color VIOLET = new Color(.73f, 0f, 0.97f, .7f ); 	// L 5
	public static final Color X_LIGHT_VIOLET = new Color(.73f, 0f, 0.97f, .3f ); 	
	public static final Color X_SOLID_VIOLET = new Color(.73f, 0f, 0.97f, 1f ); 
	
	public static final Color PURPLE = new Color(.5f, 0f, 0.5f, .7f );
	public static final Color X_SOLID_PURPLE = new Color(.5f, 0f, 0.5f, 1f );
	
	public static final Color BLUE = new Color(.0f, 0f, 0.75f, .7f ); 		// L6 6
	public static final Color X_LIGHT_BLUE = new Color(.0f, 0f, 0.75f, 0.3f ); 
	public static final Color X_SOLID_BLUE = new Color(.2f, 0.2f, 0.75f, 1f );
	
	public static final Color GRAY = new Color(0.5f, 0.5f, 0.5f, 0.7f ); 	// preplate
	public static final Color X_LIGHT_GRAY = new Color(0.5f, 0.5f, 0.5f, 0.3f ); 
	public static final Color X_DARK_GRAY = new Color(0.2f, 0.2f, 0.2f, 0.9f );
	public static final Color X_SOLID_GRAY = new Color(0.6f, 0.6f, 0.6f, 1f ); 
	
	public static final Color CYAN = new Color(0f, 1f, 1f, 0.7f ); 
	public static final Color DARK_CYAN = new Color(.0f, 0.55f, 0.55f, 0.7f );
	public static final Color X_LIGHT_CYAN = new Color(0f, 1f, 1f, 0.3f );

	
    
	
}
