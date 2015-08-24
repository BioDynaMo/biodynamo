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

package ini.cx3d.spatialOrganization;

/**
 * An exception which is thrown every time when a node is either being inserted or
 * moved to a position which is already occupied by another node. 
 * @author Dennis Goehlsdorf
 *
 */
public class PositionNotAllowedException extends Exception {
	
	/**
	 * An alternative position where the concerned node could be moved to.
	 */
	private double[] proposedPosition;
	
	/**
	 * Identifies the version of this Exception.
	 */
	static final long serialVersionUID = 1;
	
	/**
	 * Creates a new exception with a given alternative position.
	 * @param proposedPosition The alternative position that should be 
	 * 		proposed to the process that tried to move a node to a forbidden position.
	 */
	public PositionNotAllowedException(double[] proposedPosition) {
		super();
		setProposedPosition(proposedPosition);
	}

	/**
	 * Creates a new exception with a given alternative position and a message that can be
	 * passed to the calling function.
	 * @param message The text message that explains this exception.
	 * @param proposedPosition The alternative position that should be 
	 * 		proposed to the process that tried to move a node to a forbidden position.
	 */
	public PositionNotAllowedException(String message, double[] proposedPosition) {
		super(message);
		setProposedPosition(proposedPosition);
	}


	/**
	 * Creates a new exception with a given alternative position and a link to a {@link Throwable} that was the reason
	 * for this function to be called.
	 * @param cause The cause of this exception.
	 * @param proposedPosition The alternative position that should be 
	 * 		proposed to the process that tried to move a node to a forbidden position.
	 */
	public PositionNotAllowedException(Throwable cause, double[] proposedPosition) {
		super(cause);
		setProposedPosition(proposedPosition);
	}

	/**
	 * Creates a new exception with a given alternative position, a message that can be
	 * passed to the calling function and a link to a {@link Throwable} that was the reason
	 * for this function to be called.
	 * @param message The text message that explains this exception.
	 * @param cause The cause of this exception.
	 * @param proposedPosition The alternative position that should be 
	 * 		proposed to the process that tried to move a node to a forbidden position.
	 */
	public PositionNotAllowedException(String message, Throwable cause, double[] proposedPosition) {
		super(message, cause);
		setProposedPosition(proposedPosition);
	}
	
	/**
	 * Changes the new position that is recommended for the node which caused
	 * this exception. 
	 * @param proposedPosition The alternative position that should be 
	 * 		proposed to the process that tried to move a node to a forbidden position.
	 */
	private void setProposedPosition(double[] proposedPosition) {
		this.proposedPosition = proposedPosition;
	}
	
	/**
	 * @return The position proposed for the node insertion or movement.
	 */
	public double[] getProposedPosition() {
		return proposedPosition;
	}

}
