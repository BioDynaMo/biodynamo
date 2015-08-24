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



import ini.cx3d.simulations.ECM;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.BufferedImage;

import javax.swing.*;
import java.awt.print.*;

/**
 * Prints onto a page the upper left part of ECM.view
 * @author fredericzubler
 *
 */
public class ViewPrinter implements Printable {

	private View view;
	
    public ViewPrinter(View view) {
		super();
		this.view = view;
	}

	public int print(Graphics g, PageFormat pf, int page) throws
                                                        PrinterException {

        if (page > 0) { /* We have only one page, and 'page' is zero-based */
            return NO_SUCH_PAGE;
        }

        /* User (0,0) is typically outside the imageable area, so we must
         * translate by the X and Y values in the PageFormat to avoid clipping
         */
        Graphics2D g2d = (Graphics2D)g;
        g2d.translate(pf.getImageableX(), pf.getImageableY());
//        int dimX = 1000;
//		int dimY = 1500;
//        BufferedImage im = new BufferedImage(dimX, dimY,BufferedImage.TYPE_INT_RGB);
//        g = im.getGraphics();
//		view.paint(g);
//		im = im.getSubimage(view.smallWindowRectangle.x, view.smallWindowRectangle.y,
//				view.smallWindowRectangle.width, view.smallWindowRectangle.height);
        
	    view.paint(g2d);

        /* tell the caller that this page is part of the printed document */
        return PAGE_EXISTS;
    }
}