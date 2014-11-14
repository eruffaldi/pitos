/*
 * Copyright (c) 1998 Sun Microsystems, Inc. All Rights Reserved.
 * 
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 * 
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 * 
 */

import com.sun.kjava.*;

public class EightQueens extends Spotlet {

// Constants
    static final Graphics g = Graphics.getGraphics();
	static final int NUMBEROFQUEENS = 8;
	// Note: this solver can be used for solving n queens problems
	// with any n value; however, currently the drawing algorithm
	// in class EightQueenBoard can draw solutions with n = 8 only.

// Instance variables

	// Solution array
	int[] table;

	// Current x coordinate of the solver
	int x;

	// Solution counter
	int solutions;

	// GUI widgets and buttons
	EightQueenBoard board;
	Button nextButton;
	Button prevButton;
	Button exitButton;


// Constructor

	public EightQueens() {
		table = new int[NUMBEROFQUEENS+1];
		x = 0;
		solutions = 0;

		// GUI widgets and buttons
		g.clearScreen();
		board = new EightQueenBoard(table);
		nextButton = new Button("Next", 1,   145);
		exitButton = new Button("Exit", 139, 145);

		nextButton.paint();
		exitButton.paint();
	}


// Instance methods

	public void initialize() {
		for (int i = 0; i <= NUMBEROFQUEENS; i++) table[i] = 0;
		x = 0;
		solutions = 0;
	}


	public int abs(int n) {
		if (n < 0) 
			 return -n;
		else return n;
	}

	
	private boolean validPosition() {

		for (int i = 1; i < x; i++) {
			if (table[x] == table[i]) return false;
			if (abs(table[x] - table[i]) == abs(x - i)) return false;
		}
		return true;
	}


	private boolean findPosition() {

		while (++table[x] <= NUMBEROFQUEENS) {
			if (validPosition()) return true;
		}

		return false;
	}


	public boolean nextSolution() {

		while (true) {

			if (findPosition()) {

				// Check if a solution was found
				if (x == NUMBEROFQUEENS) return true;

				// Otherwise try next column
				table[++x] = 0;

			} else {
				// No more solutions?
				if (x == 1) return false;

				// Otherwise backtrack to previous column
				--x;
			}
		}
	}


// Event handlers

	public void penDown(int x, int y) {

		if (nextButton.pressed(x, y)) {

			// Erase previous text
			g.drawRectangle(30, 146, 100, 12, g.ERASE, 0);
			g.drawString("...", 30, 146);

			if (nextSolution()) {
				// Draw next solution & solution counter
				board.draw(table);
				solutions++;
				g.drawString("Solution " + solutions, 30, 146);
			} else {
				// No more solutions
				g.drawString("Solution " + solutions + " (last)", 30, 146);

				// Reinitialize the solver
				initialize();
			}
		}

		else if (exitButton.pressed(x, y)) {
            System.exit(0);
		}
	}


// Main program

	public static void main(String[] args) {
		(new EightQueens()).register(NO_EVENT_OPTIONS);
	}

}
