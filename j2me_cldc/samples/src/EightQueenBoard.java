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

public class EightQueenBoard {

// Constants
    static final Graphics g = Graphics.getGraphics();
	static final int SQUARESIZE = 13;


// Variables

// Constructor

	public EightQueenBoard(int[] queenTable) {
		draw(queenTable);
	}


// Instance methods

	public void draw(int[] queenTable) {
		clearTable();
		drawTable();
		drawQueens(queenTable);
	}

	private void clearTable() {

		g.drawRectangle(26, 16, 134, 124, g.ERASE, 0);
	}

	private void drawTable() {
		int i;

		for (i = 0; i <= 8; i++)  {
			int y = i*SQUARESIZE + 18;
			g.drawLine(28, y, 132, y, g.PLAIN);
		}

		for (i = 0; i <= 8; i++) {
			int x = i*SQUARESIZE + 28;
			g.drawLine(x, 18, x, 122, g.PLAIN);
		}
	
	}

	private void drawQueens(int[] queenTable) {

		for (int i = 1; i <= 8; i++) {
			int q = queenTable[i];
			int x = (i-1)*SQUARESIZE + 30;
			int y = (q-1)*SQUARESIZE + 20;

			if (q != 0) g.drawRectangle(x, y, 10, 10, g.PLAIN, 5);
		}
	}
	
}
