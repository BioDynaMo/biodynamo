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


import ini.cx3d.physics.PhysicalCylinder;
import ini.cx3d.physics.PhysicalObject;
import ini.cx3d.physics.PhysicalSphere;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;

import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JFrame;
import javax.swing.JTextArea;
import javax.swing.JMenuBar;

public class ShowInfo extends JFrame {

	private static final long serialVersionUID = 1L;
	private JPanel jContentPane = null;
	private JTextArea Properties = null;

	/**
	 * This is the default constructor
	 */
	public ShowInfo() {
		super();
		initialize();
	}

	/**
	 * This method initializes this
	 * 
	 * @return void
	 */
	private void initialize() {
		this.setSize(300, 600);
		this.setJMenuBar(getJJMenuBar());
		this.setContentPane(getJContentPane());
		this.setTitle("JFrame");
	}

	/**
	 * This method initializes jContentPane
	 * 
	 * @return javax.swing.JPanel
	 */
	private JPanel getJContentPane() {
		if (jContentPane == null) {
			jContentPane = new JPanel();
			jContentPane.setLayout(new BorderLayout());
			jContentPane.add(getProperties(), BorderLayout.NORTH);
		}
		return jContentPane;
	}

	/**
	 * This method initializes Properties	
	 * 	
	 * @return javax.swing.JTextArea	
	 */
	private JTextArea getProperties() {
		if (Properties == null) {
			Properties = new JTextArea();
			Properties.setEditable(false);
			Properties.setEnabled(false);
		}
		return Properties;
	}
	private PhysicalObject physo;
	private JMenuBar jJMenuBar = null; 
	private int depth;
	public void gatherInfo(PhysicalObject o,int  depth)
	{
		InfoVisiter info = new InfoVisiter();
		String s = info.VisitALL(o).toString(depth, "");
		physo = o;
		this.depth = depth;
		getProperties().setText(s);
	}

	/**
	 * This method initializes jJMenuBar	
	 * 	
	 * @return javax.swing.JMenuBar	
	 */
	private JMenuBar getJJMenuBar() {
//		if (jJMenuBar == null) {
//			jJMenuBar = new JMenuBar();
//		}
//		JMenu machins = new JMenu("Insert machines");
//		for (final String s : Genome.getInstance().GetAllMachineNames()) {
//			JMenuItem o = new JMenuItem(s);
//			o.addActionListener(new ActionListener(){
//
//				public void actionPerformed(ActionEvent e) {
//					if(physo.isAPhysicalCylinder())
//					{
//						Machine m = Genome.getInstance().getMachineInstance(s, new HashMap<String, String>());
//						((PhysicalCylinder)physo).getNeuriteElement().addLocalBiologyModule(m);
//						
//					}
//					else if(physo.isAPhysicalSphere())
//					{
//						Machine m = Genome.getInstance().getMachineInstance(s, new HashMap<String, String>());
//						System.out.println(m);
//						((PhysicalSphere)physo).getSomaElement().addLocalBiologyModule(m);
//					
//					}
//					gatherInfo(physo, depth);
//					
//				}
//				
//			});
//			machins.add(o);
//		}
//		jJMenuBar.add(machins);
//		return jJMenuBar;
		return new JMenuBar();
	}
}
