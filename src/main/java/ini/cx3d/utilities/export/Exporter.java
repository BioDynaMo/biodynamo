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

package ini.cx3d.utilities.export;

import ini.cx3d.cells.Cell;
import ini.cx3d.localBiology.NeuriteElement;
import ini.cx3d.simulations.ECM;
import ini.cx3d.synapses.Excrescence;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.HashMap;
import java.util.Vector;

import javax.swing.JFileChooser;

public class Exporter {


	private static Vector<ConnectionHolder> synapses = new Vector<ConnectionHolder>();
	// two types of cells: excitatory (E) or inhibitory (I)
	private static PopulationHolder inhibitoryPopulation = new PopulationHolder("Inhibitory_cells", "inh_lif");
	private static PopulationHolder excitatoryPopulation = new PopulationHolder("Excitatory_cells", "exz_lif");
	private static HashMap<Cell, Integer> inhibID = new HashMap<Cell, Integer>();
	private static HashMap<Cell, Integer> excitID = new HashMap<Cell, Integer>();
	// four types of projections : EE,EI,IE,II
	private static ProjectionHolder projection_E_to_E = new ProjectionHolder("E_to_E", Cell.ExcitatoryCell, Cell.ExcitatoryCell, "ExzSyn");
	private static ProjectionHolder projection_E_to_I = new ProjectionHolder("E_to_I", Cell.ExcitatoryCell, Cell.InhibitoryCell, "ExzSyn");;
	private static ProjectionHolder projection_I_to_E = new ProjectionHolder("I_to_E", Cell.InhibitoryCell, Cell.ExcitatoryCell, "InhSyn");;
	private static ProjectionHolder projection_I_to_I = new ProjectionHolder("I_to_I", Cell.InhibitoryCell, Cell.InhibitoryCell, "InhSyn");;


	public static void saveExport() {
		// name of the file
		try {
			String nameOfTheFile = "WTAcirc";
			JFileChooser fc = new JFileChooser();
			int returnVal = fc.showSaveDialog(null);
			if (returnVal == JFileChooser.APPROVE_OPTION) {
				File file = fc.getSelectedFile();
				nameOfTheFile = file.getAbsolutePath();
				System.out.println(file);
			} else {
				System.out.println("Could not export NeuroMl description");
				return;
			}
			nameOfTheFile = nameOfTheFile +  ".xml";
			PrintWriter out;
			out = new PrintWriter(new BufferedWriter(
					new FileWriter( nameOfTheFile, false))); // false = overwrites
			out.println(neuroML_level3_export());
			out.close();
		} catch (IOException e) {
			e.printStackTrace();
		}  
	}

	/** 
	 * Finds all Synapses on a neurite
	 * @param chs
	 * @param ne
	 * @return
	 */
	public static Vector<ConnectionHolder> dispatchSynapsesOfThisNeuriteElement(Vector<ConnectionHolder> chs, NeuriteElement ne){
		// all excresc.
		Vector<Excrescence> exs = ne.getPhysicalCylinder().getExcrescences();
		for (int i = 0; i < exs.size(); i++) {
			Excrescence ex = exs.get(i);
			// Are they connected to something? Are they pre-synaptic?
			if(ex.getEx() != null && ex.getType() == Excrescence.BOUTON){  
				// the two cells involved
				Cell preCell = ne.getCell();
				Cell postCell = ex.getEx().getPo().getCellElement().getCell();
				// synapse connection holder between them
				ConnectionHolder ch;

				// add it to the special collections
				if(ne.getCell().getNeuroMLType() == Cell.ExcitatoryCell){
					if(postCell.getNeuroMLType() == Cell.ExcitatoryCell){
						// Ex -> Ex
						ch = new ConnectionHolder(excitID.get(preCell), excitID.get(postCell));
						projection_E_to_E.addConnectionHolder(ch);
					}else{
						// Ex -> In
						ch = new ConnectionHolder(excitID.get(preCell), inhibID.get(postCell));
						projection_E_to_I.addConnectionHolder(ch);
					}
				}else{
					System.out.println("Exporter.dispatchSynapsesOfThisNeuriteElement()");
					if(postCell.getNeuroMLType() == Cell.ExcitatoryCell){
						// In -> Ex
						ch = new ConnectionHolder(inhibID.get(preCell), excitID.get(postCell));
						projection_I_to_E.addConnectionHolder(ch);
					}else{
						// In -> In
						ch = new ConnectionHolder(inhibID.get(preCell), inhibID.get(postCell));
						projection_I_to_I.addConnectionHolder(ch);
					}
				}
				// add it to the Vector given as arg
				chs.add(ch);
			}

		}
		return chs;	
	}


	public static void debug(){

	}

	public static StringBuilder neuroML_level3_export(){
		ECM ecm = ECM.getInstance();
		// Dispatch all cells in Excitatory or Inhibitory category
		int excitCounter = 0;
		int inhibCounter = 0;
		for (int i = 0; i < ecm.getCellList().size(); i++) {
			Cell c = ecm.getCellList().get(i);
			if(c.getNeuroMLType() == Cell.ExcitatoryCell){
				// put into the hash to retreive it's id
				excitID.put(c, new Integer(excitCounter));
				// put an instance holder
				InstanceHolder ih = new InstanceHolder(c, excitCounter);
				excitatoryPopulation.addInstanceHolder(ih);
				excitCounter++;
			}else if(c.getNeuroMLType() == Cell.InhibitoryCell){
				inhibID.put(c, new Integer(inhibCounter));
				InstanceHolder ih = new InstanceHolder(c, inhibCounter);
				inhibitoryPopulation.addInstanceHolder(ih);
				inhibCounter++;
			}
		}

		// Get all their Synapses (connections)
		for (Cell c : ecm.cellList) {
			// Find all neurites of the cell :
			Vector<NeuriteElement> nes = c.getNeuriteElements();
			Vector<ConnectionHolder> chs = new Vector<ConnectionHolder>(); // not really used, necessary for next method
			for (NeuriteElement neuriteElement : nes) {
				dispatchSynapsesOfThisNeuriteElement(chs, neuriteElement);
			}

		}

		String one = "   ";
		String two = "      ";
		String three = "         ";
		String four = "            ";
		StringBuilder sb = new StringBuilder();

		sb.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		sb.append("<neuroml xmlns      = \"http://morphml.org/neuroml/schema\"\n");
		sb.append(one+"xmlns:xsi  = \"http://www.w3.org/2001/XMLSchema-instance\"\n");
		sb.append(one+"xmlns:meta = \"http://morphml.org/metadata/schema\"\n");
		sb.append(one+"xsi:schemaLocation=\"http://morphml.org/neuroml/schema NeuroML_Level3_v1.7.1.xsd\"\n");
		sb.append(one+"lengthUnits=\"micron\"\n");
		sb.append(">\n");
		sb.append("\n");
		sb.append("\n");
		sb.append(one+"<meta:notes>\n");
		sb.append(one+"A NetworkML file for PCSIM\n");
		sb.append(one+"</meta:notes>\n");
		sb.append("\n");
		sb.append("\n");

		sb.append(one).append("<cells>\n");
		sb.append(two).append("<cell name=\"exz_lif\">\n");
		sb.append(three).append("<meta:properties>\n");
		sb.append(four).append("<meta:property tag=\"PointNeuronModel\" value=\"yes\"/>\n");
		sb.append(four).append("<meta:property tag=\"Class\"    value=\"CbLifNeuron\"/>\n");
		sb.append(four).append("<meta:property tag=\"Cm\"       value=\"2e-10\"/>\n");
		sb.append(four).append("<meta:property tag=\"Rm\"       value=\"1e8\"/>\n");
		sb.append(four).append("<meta:property tag=\"Vthresh\"  value=\"-50e-3\"/>\n");
		sb.append(four).append("<meta:property tag=\"Vresting\" value=\"-60e-3\"/>\n");
		sb.append(four).append("<meta:property tag=\"Vreset\"   value=\"-60e-3\"/>\n");
		sb.append(four).append("<meta:property tag=\"Trefract\" value=\"5e-3\"/>\n");
		sb.append(four).append("<meta:property tag=\"Vinit\"    value=\"-60e-3\"/>\n");
		sb.append(three).append("</meta:properties>\n");
		sb.append(two).append("</cell>\n");
		sb.append(two).append("<cell name=\"inh_lif\">\n");
		sb.append(three).append("<meta:properties>\n");
		sb.append(four).append("<meta:property tag=\"PointNeuronModel\" value=\"yes\"/>\n");
		sb.append(four).append("<meta:property tag=\"Class\"    value=\"CbLifNeuron\"/>\n");
		sb.append(four).append("<meta:property tag=\"Cm\"       value=\"5e-10\"/>\n");
		sb.append(four).append("<meta:property tag=\"Rm\"       value=\"1e8\"/>\n");
		sb.append(four).append("<meta:property tag=\"Vthresh\"  value=\"-50e-3\"/>\n");
		sb.append(four).append("<meta:property tag=\"Vresting\" value=\"-60e-3\"/>\n");
		sb.append(four).append("<meta:property tag=\"Vreset\"   value=\"-60e-3\"/>\n");
		sb.append(four).append("<meta:property tag=\"Trefract\" value=\"5e-3\"/>\n");
		sb.append(four).append("<meta:property tag=\"Vinit\"    value=\"-60e-3\"/>\n");
		sb.append(three).append("</meta:properties>\n");
		sb.append(two).append("</cell>\n");
		sb.append(one).append("</cells>\n");

		sb.append("\n");
		sb.append("\n");

		sb.append(one).append("<channels units=\"SI Units\">\n");
		sb.append(two).append("<synapse_type name=\"ExzSyn\"  xmlns=\"http://morphml.org/channelml/schema\">\n");
		sb.append(three).append("<doub_exp_syn max_conductance=\"4e-9\" rise_time=\"0\" decay_time=\"1e-3\" reversal_potential=\"0\"/>\n");
		sb.append(two).append("</synapse_type>\n");
		sb.append(two).append("<synapse_type name=\"InhSyn\"  xmlns=\"http://morphml.org/channelml/schema\">\n");
		sb.append(three).append("<doub_exp_syn max_conductance=\"81e-9\" rise_time=\"0\" decay_time=\"10e-3\" reversal_potential=\"-80e-3\"/>\n");
		sb.append(two).append("</synapse_type>\n");
		sb.append(one).append("</channels>\n");

		sb.append("\n");
		sb.append("\n");

		// POPULATIONS
		sb.append(one).append("<populations xmlns=\"http://morphml.org/networkml/schema\">");
		sb.append("\n");
		// Exctatory populations holder
		sb.append(excitatoryPopulation.toXML(two));
		// Inhibitory population holder
		sb.append(inhibitoryPopulation.toXML(two));
		sb.append(one).append("</populations>\n");
		sb.append("\n");
		sb.append("\n");

		// CONNECTIONS

		sb.append(one).append("<projections units=\"Physiological Units\" xmlns=\"http://morphml.org/networkml/schema\">");
		sb.append("\n");
		// E to E, E to I, etc 
		sb.append(projection_E_to_E.toXML(two));
		sb.append("\n");
		sb.append(projection_E_to_I.toXML(two));
		sb.append("\n");
		sb.append(projection_I_to_E.toXML(two));
		sb.append("\n");
		//sb.append(projection_I_to_I.toXML(two));
		//sb.append("\n");

		sb.append(one).append("</projections>\n");
		sb.append("\n");
		sb.append("\n");


		sb.append("</neuroml>");
		System.out.println(sb);
		return sb;
	}
}
