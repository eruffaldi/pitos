/*
 *  Copyright (c) 1999 Sun Microsystems, Inc., 901 San Antonio Road,
 *  Palo Alto, CA 94303, U.S.A.  All Rights Reserved.
 * 
 *  Sun Microsystems, Inc. has intellectual property rights relating
 *  to the technology embodied in this software.  In particular, and
 *  without limitation, these intellectual property rights may include
 *  one or more U.S. patents, foreign patents, or pending
 *  applications.  Sun, Sun Microsystems, the Sun logo, Java, KJava,
 *  and all Sun-based and Java-based marks are trademarks or
 *  registered trademarks of Sun Microsystems, Inc.  in the United
 *  States and other countries.
 *
 *  This software is distributed under licenses restricting its use,
 *  copying, distribution, and decompilation.  No part of this
 *  software may be reproduced in any form by any means without prior
 *  written authorization of Sun and its licensors, if any.
 *
 *  FEDERAL ACQUISITIONS:  Commercial Software -- Government Users
 *  Subject to Standard License Terms and Conditions
 */
 
import com.sun.kjava.*;

public class Dragon extends Spotlet {
	
	Graphics g;
	ValueSelector iterBox;
	ValueSelector segLenBox;

	Button exitButton, clearButton;

	public static void main(String[] args) {
		(new Dragon()).register(NO_EVENT_OPTIONS);
	}
	void clear() {

		g.drawLine(0,128,159,128,g.ERASE);
		for (int i = 128; i >= 1; i--)
			g.copyRegion(0,1,160,i,0,0,g.OVERWRITE);
	}

	void paint() {

		// display the frames
		g.drawBorder(1,130,158,29,g.PLAIN,g.SIMPLE);

		// display the buttons and value selectors
		iterBox.paint();
		segLenBox.paint();
		exitButton.paint();
		clearButton.paint();
	}

	public Dragon() {
		g = Graphics.getGraphics();

		// create the buttons
		iterBox = new ValueSelector("Iterations",1,14,10,5,132);
		segLenBox = new ValueSelector("Segment", 1, 5, 2,8,146);

		clearButton = new Button("Clear",111,145);
		exitButton = new Button("Exit",139,145);

		g.clearScreen();
		paint();
	}

	public void penDown(int x, int y) {
		// that only action that can be taken if there is a dragon running
		// is attempting to start another one
		if (0 <= x && x < 160 && 0 <= y && y <130) {
			// start a dragon at the chosen point
			(new OneDragon(iterBox.getValue(),segLenBox.getValue(),x,y)).start();
		}
		else if (exitButton.pressed(x,y)) {
                    clear();
		    System.exit(0);
		}
		else if (!OneDragon.noDragons())
			return;
		else if (clearButton.pressed(x,y))
			clear();
		else if (iterBox.pressed(x,y) || segLenBox.pressed(x,y))
			return;
	}
}
