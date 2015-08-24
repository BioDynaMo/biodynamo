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

import ini.cx3d.physics.Substance;
import ini.cx3d.simulations.ECM;
import ini.cx3d.utilities.export.Exporter;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.print.PrinterException;
import java.awt.print.PrinterJob;
import java.util.HashMap;

import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JCheckBoxMenuItem;
import javax.swing.JFrame;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JToolBar;





public class ECM_GUI_Creator implements ActionListener{

	
	private static int operation_on_close= JFrame.EXIT_ON_CLOSE;
	private JFrame window1;
	ECM ecm = ECM.getInstance();
	public View view;
	JToolBar toolBar;
	JPanel choiceRepresentationPanel;
	JPanel permanentPanel;
	JPanel projectionPanel;
	JPanel slicePanel;
	public Color transluGreen = new Color(0f, 1f, 0f, 0.7f );
	boolean continuousRotation = false;
	private int pictureNumber = 0;
	boolean takeSnapshotAtEachMenuBarClick = false;
	private JMenu saveChemical = new JMenu("Save chemical concentration");
	private JMenu chemicals = new JMenu("Chemicals"); 

	final JCheckBox pauseButton = new JCheckBox("Pause", true);
	final JCheckBox paintButton = new JCheckBox("Paint", true);
	final JCheckBox pauseButton2 = new JCheckBox("Pause",true);

	public View createPrincipalGUIWindow(){
		JFrame window1 = new JFrame("Just to get the toolkit");
		Toolkit leKit = window1.getToolkit();		
		Dimension wndSize = leKit.getScreenSize();
		return createPrincipalGUIWindow(wndSize.width/10,wndSize.height/20,
				(int)Math.ceil(wndSize.width*0.7),(int)Math.ceil(wndSize.height*0.7));
	}

	/**
	 * Sets what happens if the window is closed using the x
	 */
	public static void operationToTakeOnClose(int operation)
	{
	 	operation_on_close = operation; 
	}
	

	/** 
	 * Creates a GUI window: returns a <code>View</code> (a subclass of <code>JComponent</code>) whose 
	 * paint method will display the elements of the simulation stored in <code>ECM</code>. Creates also 
	 * a menu bar and a tool bar for control parameters of the simulation and the display.
	 * @return the GUI window
	 */
	public View createPrincipalGUIWindow(int x, int y, int width, int height) {

		// ---------------------------------------------------------------------------
		// MAIN WINDOW
		// ---------------------------------------------------------------------------
		window1 = new JFrame("CX 3D");
		window1.setDefaultCloseOperation(operation_on_close);
		Toolkit leKit = window1.getToolkit();		
		Dimension wndSize = leKit.getScreenSize();
		window1.setBounds(wndSize.width/10,wndSize.height/20,
				(int)Math.ceil(wndSize.width*0.7),(int)Math.ceil(wndSize.height*0.7));
		window1.setBounds(x,y,width,height);
		final View view = new View();
		view.setDoubleBuffered(true);
		window1.getContentPane().add(view);
		this.view = view;


		// ---------------------------------------------------------------------------
		// MOUSELISTENER
		// ---------------------------------------------------------------------------
		MouseActionState.mouseModeNav.EnableState(view);
		// ---------------------------------------------------------------------------
		// MENUBAR
		// ---------------------------------------------------------------------------
		JMenuBar menuBar = new JMenuBar();
		window1.setJMenuBar(menuBar);
		// Save Images or values
		JMenu file = new JMenu("File"); 
		menuBar.add(file);
		file.add(createAndLabelAJMenuItem("Select PhysicalSphere"));
		file.addSeparator();
		//file.add(createAndLabelAJMenuItem("Select ROI"));
		file.add(createAndLabelAJMenuItem("Save snapshot"));
		file.add(createAndLabelAJCheckBoxMenuItem("Snapshot at each time step", false));
		file.add(createAndLabelAJCheckBoxMenuItem("Snapshot each 100th time step", false));
		file.add(createAndLabelAJCheckBoxMenuItem("Snapshot at each MenuBar operation",false));
		file.addSeparator();
		file.add(createAndLabelAJMenuItem("Print"));
		file.addSeparator();
		file.add(createAndLabelAJMenuItem("Save cx3d.cells positions"));
		 
		file.add(getSaveChemical());
//		getSaveChemical().add(createAndLabelAJMenuItem("Ventricular Zone"));
//		getSaveChemical().add(createAndLabelAJMenuItem("Layer 1"));
//		getSaveChemical().add(createAndLabelAJMenuItem("Layer 2/3"));
//		getSaveChemical().add(createAndLabelAJMenuItem("Layer 4"));
//		getSaveChemical().add(createAndLabelAJMenuItem("Layer 5"));
//		getSaveChemical().add(createAndLabelAJMenuItem("Layer 6"));
//		getSaveChemical().add(createAndLabelAJMenuItem("A"));
		file.addSeparator();
		file.add(createAndLabelAJMenuItem("Concentration x 1"));
		file.add(createAndLabelAJMenuItem("Concentration x 10"));
		file.add(createAndLabelAJMenuItem("Concentration x 30"));

		// Choice of chemicals displayed
		
		menuBar.add(getChemicals());
		getChemicals().add(createAndLabelAJMenuItem("None"));
		getChemicals().add(createAndLabelAJCheckBoxMenuItem("Show Numbers",false));
//		getChemicals().addSeparator();
//		getChemicals().add(createAndLabelAJMenuItem("Ventricular Zone"));
//		getChemicals().add(createAndLabelAJMenuItem("Layer 1"));
//		getChemicals().add(createAndLabelAJMenuItem("Layer 2/3"));
//		getChemicals().add(createAndLabelAJMenuItem("Layer 4"));
//		getChemicals().add(createAndLabelAJMenuItem("Layer 5"));
//		getChemicals().add(createAndLabelAJMenuItem("Layer 6"));
		getChemicals().addSeparator();
		JMenu chemicalContrast = new JMenu("Enhance chemical Contrast");
		getChemicals().add(chemicalContrast);
		chemicalContrast.add(createAndLabelAJMenuItem("Concentration x 1"));
		chemicalContrast.add(createAndLabelAJMenuItem("Concentration x 2"));
		chemicalContrast.add(createAndLabelAJMenuItem("Concentration x 4"));
		chemicalContrast.add(createAndLabelAJMenuItem("Concentration x 10"));
		chemicalContrast.add(createAndLabelAJMenuItem("Concentration x 100"));
		chemicalContrast.add(createAndLabelAJMenuItem("Concentration x 1000"));
		getChemicals().addSeparator();

		// Display Delaunay neighbors, Neurite point masses etc
		JMenu display = new JMenu("Display"); 
		//display.add(createAndLabelAJMenuItem("Mouse 3D navigation"));
		menuBar.add(display);
		//display.addSeparator();
		display.add(createAndLabelAJCheckBoxMenuItem("Triangulation neighbors", false));
		display.add(createAndLabelAJCheckBoxMenuItem("Triangulation vertices", false));
		display.add(createAndLabelAJCheckBoxMenuItem("Total Force", false));
		display.add(createAndLabelAJCheckBoxMenuItem("Scale Bar", true));
		display.addSeparator();
		display.add(createAndLabelAJCheckBoxMenuItem("Neurites point masses", view.drawPointMass));
		display.add(createAndLabelAJCheckBoxMenuItem("Physical Bonds", false));
		display.add(createAndLabelAJCheckBoxMenuItem("Spines", false));
		display.addSeparator();
		display.add(createAndLabelAJCheckBoxMenuItem("Pale Cells", true));
		display.addSeparator();
		display.add(createAndLabelAJCheckBoxMenuItem("Rotation", false));
		display.add(createAndLabelAJMenuItem("Show XY-plane"));
		display.add(createAndLabelAJMenuItem("Show XZ-plane"));
		display.add(createAndLabelAJMenuItem("Show YZ-plane"));
		display.add(createAndLabelAJMenuItem("Restore original view"));
		
		display.addSeparator();
		JMenu changeWindowCentration = new JMenu("Change window centration"); 
		display.add(changeWindowCentration);
		changeWindowCentration.add(createAndLabelAJMenuItem("Up"));
		changeWindowCentration.add(createAndLabelAJMenuItem("Down"));
		changeWindowCentration.add(createAndLabelAJMenuItem("Left"));
		changeWindowCentration.add(createAndLabelAJMenuItem("Right"));
		display.addSeparator();
//		display.add(new JSlider(JSlider.HORIZONTAL,40, 200, 100));

		
		//Sabina View lineage tree
		display.add(createAndLabelAJMenuItem("View lineage tree"));

		

		JMenu export = new JMenu("Export"); 
		export.add(createAndLabelAJMenuItem("XML Export"));
		export.add(createAndLabelAJMenuItem("Matlab format Export"));
		menuBar.add(export);

		

		
		////////////////////////////////////////////////////////////
		// END NEW TOBY	
		////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
		// NEW ROMAN: MOUSEMODE Menu
		// Mouse mode allows clicking on a particular node to get information on volume, coordinates and concentration
		////////////////////////////////////////////////////////////
		
//
		JMenu mouseMode = new JMenu("Mouse Mode"); 
		JMenuItem menuitem = createAndLabelAJMenuItem("3D Navigate");
		menuitem.addActionListener(new ActionListener(){

			public void actionPerformed(ActionEvent e) {
				MouseActionState.mouseModeNav.EnableState(view);
			}			
		}
		);
		mouseMode.add(menuitem);
		
	    menuitem = createAndLabelAJMenuItem("Select ROI");
		menuitem.addActionListener(new ActionListener(){

			public void actionPerformed(ActionEvent e) {
				MouseActionState.mouseSmallerWindow.EnableState(view);
			}			
		}
		);
		mouseMode.add(menuitem);
		
		menuitem = createAndLabelAJMenuItem("Select Object");
		menuitem.addActionListener(new ActionListener(){
				public void actionPerformed(ActionEvent e) {
					MouseActionState.selectionState.EnableState(view);
				}			
			}
			);
		mouseMode.add(menuitem);
			
		
		
		mouseMode.add(createAndLabelAJMenuItem("Mouse Mode: Select Node"));
		mouseMode.add(createAndLabelAJMenuItem("Mouse Mode: Select Sphere"));
		
		menuBar.add(mouseMode);

		
		// ---------------------------------------------------------------------------
		// TOOLBAR
		// ---------------------------------------------------------------------------
		toolBar = new JToolBar("myToolBar");
		window1.getContentPane().add(toolBar,BorderLayout.NORTH);
		choiceRepresentationPanel = new JPanel ();
		permanentPanel = new JPanel ();
		projectionPanel = new JPanel ();
		slicePanel = new JPanel ();
		toolBar.add(choiceRepresentationPanel);
		toolBar.add(permanentPanel);
		toolBar.add(projectionPanel);
		toolBar.add(slicePanel);
		choiceRepresentationPanel.setLayout (new GridLayout (2, 1));
		permanentPanel.setLayout (new BoxLayout(permanentPanel, BoxLayout.X_AXIS ));
		projectionPanel.setLayout (new GridLayout (1, 3));
		slicePanel.setLayout (new GridLayout (1, 2));
		// choix de la repr�sentation : projection ou slice
		choiceRepresentationPanel.setLayout (new GridLayout (2, 1));
		JButton projection = new JButton("Projection");
		choiceRepresentationPanel.add(projection);
		JButton slice = new JButton("Slice");
		choiceRepresentationPanel.add(slice);
		slice.addActionListener(this);
		slice.setActionCommand("Slice");
		projection.addActionListener(this);
		projection.setActionCommand("Projection");
		// draw or not the elemnts of the triangulation (neighbors and Delaunay Vertices)
		JPanel delaunay = new JPanel();
		permanentPanel.add(delaunay);
		delaunay.setLayout (new BoxLayout(delaunay, BoxLayout.Y_AXIS ));

		JCheckBox delaunayNeighborsButton = new JCheckBox("Triangulation neighbors", view.drawDelaunayNeighbors);
		delaunay.add(delaunayNeighborsButton);
		delaunayNeighborsButton.setActionCommand("Triangulation neighbors");
		delaunayNeighborsButton.addActionListener(this);
		JCheckBox delaunayVerticesButton = new JCheckBox("Neurites point masses", view.drawPointMass);
		delaunay.add(delaunayVerticesButton);
		delaunayVerticesButton.setActionCommand("Neurites point masses");
		delaunayVerticesButton.addActionListener(this);


		// To make a pause (infinite loop)
		//Pause buton created above
		permanentPanel.add(pauseButton);
		permanentPanel.add(paintButton);
		pauseButton.setActionCommand("pause");
		pauseButton.addActionListener(this);
		paintButton.setActionCommand("paint");
		paintButton.addActionListener(this);

		// To change the scale of the view
		JPanel scalePanel = new JPanel();
		scalePanel.setLayout (new GridLayout (2, 1));
		JButton increaseScalingFactorButton = new JButton("Scale +");
		scalePanel.add(increaseScalingFactorButton);
		increaseScalingFactorButton.setActionCommand("increaseScalingFactor");
		increaseScalingFactorButton.addActionListener(this);
		JButton decreaseScalingFactorButton = new JButton("Scale -");
		scalePanel.add(decreaseScalingFactorButton);
		decreaseScalingFactorButton.setActionCommand("decreaseScalingFactor");
		decreaseScalingFactorButton.addActionListener(this);
		permanentPanel.add(scalePanel);

		// To make a continuous rotation around z axis && To activate /supresss the perspective view
		JPanel rotationAndPerspective = new JPanel();
		permanentPanel.add(rotationAndPerspective);
		rotationAndPerspective.setLayout (new BoxLayout(rotationAndPerspective, BoxLayout.Y_AXIS ));

		JCheckBox continuousRotationButton = new JCheckBox("Rotation", continuousRotation);
		projectionPanel.add(continuousRotationButton);
		continuousRotationButton.setActionCommand("continuousRotation");
		continuousRotationButton.addActionListener(this);

		JCheckBox perspectiveButton = new JCheckBox("Perspective", view.perspective);
		projectionPanel.add(perspectiveButton);
		perspectiveButton.setActionCommand("perspective");
		perspectiveButton.addActionListener(this);


		// To rotate left or Right view (around axis y)
		JPanel manualLRRotationPane = new JPanel();
		manualLRRotationPane.setLayout(new GridLayout (1, 2));
		JButton rotateLeftButton = new JButton("L");
		manualLRRotationPane.add(rotateLeftButton);
		rotateLeftButton.setActionCommand("rotateLeft");
		rotateLeftButton.addActionListener(this);
		JButton rotateRightButton = new JButton("R");
		manualLRRotationPane.add(rotateRightButton);
		rotateRightButton.setActionCommand("rotateRight");
		rotateRightButton.addActionListener(this);
		projectionPanel.add(manualLRRotationPane);

		// To rotate Uo or Down view (around axis x)
		JPanel manualUDRotationPane = new JPanel();
		manualUDRotationPane.setLayout(new GridLayout (2, 1));
		JButton rotateUpButton = new JButton("U");
		manualUDRotationPane.add(rotateUpButton);
		rotateUpButton.setActionCommand("rotateUp");
		rotateUpButton.addActionListener(this);
		JButton rotateDownButton = new JButton("D");
		manualUDRotationPane.add(rotateDownButton);
		rotateDownButton.setActionCommand("rotateDown");
		rotateDownButton.addActionListener(this);
		projectionPanel.add(manualUDRotationPane);


		// To choose between thick (OM) and thin (EM) slices
		JPanel thickOrThinPanel = new JPanel();
		thickOrThinPanel.setLayout(new GridLayout (2, 1));
		JButton thickerSlice = new JButton("Thick Slice");
		thickOrThinPanel.add(thickerSlice);
		thickerSlice.setActionCommand("Thick Slice");
		thickerSlice.addActionListener(this);
		JButton thinnerSlice = new JButton("Thin Slice");
		thickOrThinPanel.add(thinnerSlice);
		thinnerSlice.setActionCommand("Thin Slice");
		thinnerSlice.addActionListener(this);
		slicePanel.add(thickOrThinPanel);

		//  To go back and forth during slice view (along axis x)
		JPanel backOrForthSlicePanel = new JPanel();
		backOrForthSlicePanel.setLayout(new GridLayout (2, 1));
		JButton forwardSliceButton = new JButton("Next Slice");
		backOrForthSlicePanel.add(forwardSliceButton);
		forwardSliceButton.setActionCommand("Next Slice");
		forwardSliceButton.addActionListener(this);
		JButton backwardSliceButton = new JButton("Previous Slice");
		backOrForthSlicePanel.add(backwardSliceButton);
		backwardSliceButton.setActionCommand("Previous Slice");
		backwardSliceButton.addActionListener(this);
		slicePanel.add(backOrForthSlicePanel);

		window1.setVisible(true);
		if(view.representationType == view.OM_SLICE_TYPE || view.representationType == view.EM_SLICE_TYPE){
//			projectionPanel.setVisible(false);
			slicePanel.setVisible(true);
		}else if(view.representationType == view.PROJECTION_TYPE){
			projectionPanel.setVisible(true);
			slicePanel.setVisible(false);
		}


		this.view = view;
		return view; 
	}
	public boolean pause = true;
	public void togglePauseSim()
	{
		//pauseButton.setSelected(!pauseButton.isSelected());
		
		try {
			if(!pause)
			{
				pause = true;
				ecm.getInstance().canRun.acquire();
				//pausedBymenu = true;
			}
			else
			{
				pause = false;
				ecm.getInstance().canRun.release();
				//pausedBymenu = false;
				
			}
			pauseButton.setSelected(pause);
			pauseButton2.setSelected(pause);
		} catch (InterruptedException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		} //setSimulationOnPause(switcher(ecm.isSimulationOnPause()));
		
		
	}

	/** 
	 * Handles the <code>ActionEvent</code> produced by the menu and tool bar items of the main GUI window.
	 */
	public void actionPerformed(ActionEvent e) {
		String command = e.getActionCommand();
		if ("Projection".equals(e.getActionCommand())) {
			view.representationType = view.PROJECTION_TYPE;
			projectionPanel.setVisible(true);
			slicePanel.setVisible(false);
			toolBar.revalidate();
		} 
		else if ("Slice".equals(e.getActionCommand())) {
			view.representationType = view.OM_SLICE_TYPE;
//			projectionPanel.setVisible(false);
			slicePanel.setVisible(true);
			toolBar.revalidate();
		} 

		else if ("Triangulation neighbors".equals(e.getActionCommand())) {
			view.drawDelaunayNeighbors = switcher(view.drawDelaunayNeighbors);
		} 
		else if ("Triangulation vertices".equals(e.getActionCommand())) {
			view.drawDelaunayVertices = switcher(view.drawDelaunayVertices);
		} else if ("Total Force".equals(e.getActionCommand())) {
			view.drawForces = switcher(view.drawForces);
		} 
		else if ("Neurites point masses".equals(e.getActionCommand())) {
			view.drawPointMass = switcher(view.drawPointMass);
		} 
		else if ("pause".equals(e.getActionCommand())) {
//			ecm.setSimulationOnPause(switcher(ecm.isSimulationOnPause()));
			togglePauseSim();
		} 
		else if ("paint".equals(e.getActionCommand())) {
//			ecm.setSimulationOnPause(switcher(ecm.isSimulationOnPause()));
			view.setDrawIT(paintButton.isSelected());
		} 
		else if ("continuousRotation".equals(e.getActionCommand())) {
			ecm.setContinuouslyRotating(switcher(ecm.isContinuouslyRotating()));
		} 
		else if ("perspective".equals(e.getActionCommand())) {
			view.perspective = switcher(view.perspective);
		} 

		else if ("increaseScalingFactor".equals(e.getActionCommand())) {
			view.increaseScalingFactor();
		}  else if ("decreaseScalingFactor".equals(e.getActionCommand())) {
			view.decreaseScalingFactor();
		} 

		else if ("rotateLeft".equals(e.getActionCommand())) {
			view.rotateAroundZ(Math.PI/18.0);
		}  else if ("rotateRight".equals(e.getActionCommand())) {
			view.rotateAroundZ(-Math.PI/18.0);
		}

		else if ("rotateUp".equals(e.getActionCommand())) {
			view.rotateAroundX(Math.PI/18.0);
		}  else if ("rotateDown".equals(e.getActionCommand())) {
			view.rotateAroundX(-Math.PI/18.0);

		}

		else if ("Next Slice".equals(e.getActionCommand())) {

			if(view.representationType == View.OM_SLICE_TYPE)
				view.sliceYPosition += 5;
			if(view.representationType == View.EM_SLICE_TYPE)
				view.sliceYPosition += 0.5;
		}  else if ("Previous Slice".equals(e.getActionCommand())) {
			if(view.representationType == View.OM_SLICE_TYPE)
				view.sliceYPosition -= 5;
			if(view.representationType == View.EM_SLICE_TYPE)
				view.sliceYPosition -= 0.5;
		}

		else if ("Thick Slice".equals(e.getActionCommand())) {
			view.representationType = View.OM_SLICE_TYPE;
			view.sliceThickness = View.OM_SLICE_THICKNESS;
		}  else if ("Thin Slice".equals(e.getActionCommand())) {
			view.representationType = View.EM_SLICE_TYPE;
			view.sliceThickness = View.EM_SLICE_THICKNESS;
		}
		else if("Show Numbers".equals(e.getActionCommand()))
		{
			view.Shownumbers=!view.Shownumbers;
		}

		if (e.getSource() instanceof JMenuItem){  
			// events triggered by menus 
			String ChoixOption = e.getActionCommand(); 
			if (ChoixOption.equals("Select PhysicalSphere")){
				//vml.mouseOperation=vml.SELECT_A_PHYSICAL_SPHERE;
			}else if (ChoixOption.equals("Select ROI")){
				MouseActionState.mouseSmallerWindow.EnableState(view);
			}else if(ChoixOption.equals("Save snapshot")){
				ecm.dumpImage();
			}else if(ChoixOption.equals("Snapshot at each time step")){
				ecm.setTakingSnapshotAtEachTimeStep(switcher(ecm.isTakingSnapshotAtEachTimeStep()));
			}else if(ChoixOption.equals("Snapshot each 100th time step")){
				ecm.setTakingSnapshotEach100TimeSteps(switcher(ecm.isTakingSnapshotEach100TimeSteps()));
			}else if(ChoixOption.equals("Snapshot at each MenuBar operation")){
				takeSnapshotAtEachMenuBarClick = switcher(takeSnapshotAtEachMenuBarClick);

			}else if(ChoixOption.equals("Print")){
				PrinterJob job = PrinterJob.getPrinterJob();
		         job.setPrintable(new ViewPrinter(ecm.view));
		        
		         boolean ok = job.printDialog();
		         if (ok) {
		             try {
		                  job.print();
		             } catch (PrinterException ex) {
		              /* The job did not successfully complete */
		             }
		         }
			}

//			else if(ChoixOption.equals("Save cx3d.cells positions")){
//			ecm.savePositionsOfCells();
//			}

//			else if (ChoixOption.equals("A")){
//				view.drawChemicals = true;
//				view.drawA = switcher(view.drawA);
//			}else if (ChoixOption.equals("B")){
//				view.drawChemicals = true;
//				view.drawB = switcher(view.drawB);
//			}else if (ChoixOption.equals("C")){
//				view.drawChemicals = true;
//				view.drawC = switcher(view.drawC);
//			}else if (ChoixOption.equals("D")){
//				view.drawChemicals = true;
//				view.drawD = switcher(view.drawD);
//			}else if (ChoixOption.equals("E")){
//				view.drawChemicals = true;
//				view.drawE = switcher(view.drawE);
//			}else if (ChoixOption.equals("F")){
//				view.drawChemicals = true;
//				view.drawF = switcher(view.drawF);
//			}
			else if (ChoixOption.equals("Concentration x 1")){
				view.chemicalDrawFactor = 1;
			}else if (ChoixOption.equals("Concentration x 2")){
				view.chemicalDrawFactor = 2;
			}else if (ChoixOption.equals("Concentration x 4")){
				view.chemicalDrawFactor = 4;
			}else if (ChoixOption.equals("Concentration x 10")){
				view.chemicalDrawFactor = 10;
			}else if (ChoixOption.equals("Concentration x 100")){
				view.chemicalDrawFactor = 100;
			}else if (ChoixOption.equals("Concentration x 1000")){
				view.chemicalDrawFactor = 1000;
			}


			

			else if (ChoixOption.equals("Show XY-plane")){
				view.showXYPlan();
			}else if (ChoixOption.equals("Show XZ-plane")){
				view.showXZPlan();
			}else if (ChoixOption.equals("Show YZ-plane")){
				view.showYZPlan();
			}else if (ChoixOption.equals("Restore original view")){
				view.setMagnification(1);
				view.diameter = 10*view.getMagnification();
				view.displacementx = 0;
				view.displacementy = 0;
				view.representationType = view.PROJECTION_TYPE;
				view.sliceYPosition = 0.5;
				view.showXZPlan();

			}else if (ChoixOption.equals("Up")){
				view.displacementy += 300/view.getMagnification();
			}else if (ChoixOption.equals("Down")){
				view.displacementy -= 300/view.getMagnification();
			}else if (ChoixOption.equals("Left")){
				view.displacementx += 300/view.getMagnification();
			}else if (ChoixOption.equals("Right")){
				view.displacementx -= 300/view.getMagnification();
			}else if (ChoixOption.equals("Mouse 3D navigation")){
				//vml.mouseOperation = vml.THREE_D_NAVIGATION;
				view.setCursor(new Cursor(Cursor.HAND_CURSOR));
			}else if (ChoixOption.equals("Scale Bar")){
				view.drawScaleBar = switcher(view.drawScaleBar);
			}else if (ChoixOption.equals("Physical Bonds")){
				view.drawPhysicalBonds = switcher(view.drawPhysicalBonds);
			}else if (ChoixOption.equals("Spines")){
				view.drawSpines = switcher(view.drawSpines);
			}else if (ChoixOption.equals("Pale Cells")){
				view.drawPaleCells = switcher(view.drawPaleCells);
			}
			
//			//Sabina View lineage tree
//			else if (ChoixOption.equals("View lineage tree")){
//				(new Thread(new ViewTree())).start();
//			}
			

			else if (ChoixOption.equals("XML Export")){
				Exporter.saveExport();
				//MachineLabelExporter.saveExport();
//				ini.cx3d.simulations.roman.ExtendedMachineLabelExporter.saveExport();
			}
//			else if (ChoixOption.equals("Matlab format Export")){
//				System.out.println("exporting.....");
//				Cx3dCellToMatlab.toMatlabNeuron(ecm.getCellList().firstElement(),"matlabfiles/cx3d.exp");
//				MainGUI.InitDrawer(0);
//			}
			
			// Roman MouseMode
			else if (ChoixOption.equals("Mouse Mode: Select Node")){
				System.out.println("mouse mode: node");
				view.setCursor(new Cursor(Cursor.CROSSHAIR_CURSOR));
				//vml.mouseOperation=ViewMouseListener.SELECT_A_PHYSICAL_NODE;
			}
			else if (ChoixOption.equals("Mouse Mode: Select Sphere")){
				System.out.println("mouse mode: sphere");
				view.setCursor(new Cursor(Cursor.CROSSHAIR_CURSOR));
				//vml.mouseOperation=ViewMouseListener.SELECT_A_PHYSICAL_SPHERE;
			}


			if (takeSnapshotAtEachMenuBarClick){
			ecm.dumpImage();
			}
		}
		view.repaint();
	}

//	#########################################################################
//	Some methods for creating the menus and the tool bar
//	#########################################################################
	/* Creates a JButton with a label and the action command, and
	 * registers this object (<code>ECM_GUI_Creator</code>) as Listener.
	 * @param LabelAndActionCommand the label and action command
	 * @return the <code>JButton</code>.
	 */
	private JButton createAndLabelAJButton(String LabelAndActionCommand){
		JButton myButton = new JButton(LabelAndActionCommand);
		myButton.setActionCommand(LabelAndActionCommand);
		myButton.addActionListener(this);
		return myButton;
	}

	/* Creates a JMenuItem with a label and the action command, and
	 * registers this object (<code>ECM_GUI_Creator</code>) as Listener.
	 * @param LabelAndActionCommand the label and action command
	 * @return the <code>JMenuItem</code>.
	 */
	private JMenuItem createAndLabelAJMenuItem(String LabelAndActionCommand){
		JMenuItem myItem = new JMenuItem(LabelAndActionCommand);
		myItem.setActionCommand(LabelAndActionCommand);
		myItem.addActionListener(this);
		return myItem;
	}

	/* Creates a JCheckBoxMenuItem with a label and the action command, and
	 * registers this object (<code>ECM_GUI_Creator</code>) as Listener.
	 * @param LabelAndActionCommand the label and action command
	 * @return the <code>JCheckBoxMenuItem</code>.
	 */
	private JCheckBoxMenuItem createAndLabelAJCheckBoxMenuItem(String LabelAndActionCommand, boolean state){
		JCheckBoxMenuItem myItem = new JCheckBoxMenuItem(LabelAndActionCommand, state);
		myItem.setActionCommand(LabelAndActionCommand);
		myItem.addActionListener(this);
		return myItem;
	}

	/* Returns false if argument is true & vice-versa*/
	private boolean switcher(boolean b){
		if(b == true){
			return false;
		}else{
			return true;
		}
	}
	

	// **************************************************************************
	// Getters & Setters
	// **************************************************************************
	
	public void setBauseButtonTo(boolean pause){
		pauseButton.setSelected(true);
		pauseButton.doClick();
		System.out.println(pause);
	}
	
	/**
	 * Needed for dynamicly Add Chemicals
	 */
	public void setSaveChemical(JMenu saveChemical) {
		this.saveChemical = saveChemical;
	}
	/**
	 * Needed for dynamicly Add Chemicals
	 */
	public JMenu getSaveChemical() {
		return saveChemical;
	}

	private HashMap<String,JMenuItem> dynamicchemicals = new HashMap<String,JMenuItem>();
	public void addNewChemical(final Substance s) {
		
		JMenuItem chemical = new JMenuItem(s.getId());
		dynamicchemicals.put(s.getId()+"_saver", chemical);
		chemical.addActionListener(new ActionListener(){

			public void actionPerformed(ActionEvent arg0) {
				
			}}
		);
		this.saveChemical.add(chemical);
		JCheckBoxMenuItem chemi = new JCheckBoxMenuItem(s.getId(), false);
		chemi.addActionListener(new ActionListener(){

			public void actionPerformed(ActionEvent arg0) {
				if(!view.containsTobeDrawn(s))
				{
					view.addTobeDrawn(s);
				}
				else
				{
					view.removeTobeDrawn(s);
				}
				((JPanel)window1.getContentPane()).updateUI();
			}}
		);
		this.chemicals.add(chemi);
		dynamicchemicals.put(s.getId()+"_draw", chemi);
		
		
	}

	public void setChemicals(JMenu chemicals) {
		this.chemicals = chemicals;
	}

	public JMenu getChemicals() {
		return chemicals;
	}

	public void removeAllChemicalSubstances() {
		for (String i : dynamicchemicals.keySet()) {
			this.chemicals.remove(dynamicchemicals.get(i));
			this.saveChemical.remove(dynamicchemicals.get(i));
		}
		view.removeAllToBeDrawn();
		dynamicchemicals.clear();
	}
	public void togglePaint()
	{
		paintButton.setSelected(!paintButton.isSelected());
		view.setDrawIT(paintButton.isSelected());
	}
	
	public boolean isPainted()
	{
		return paintButton.isSelected();
	}

}
