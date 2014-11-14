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
 
package dots;

import com.sun.kjava.*;

/**
 * This is a very simple Dots game-playing class. However, it will
 * beat the Random algorithm every time! This algorithm scans
 * line segments from left-to-right, top-to-bottom and draws the
 * first one that it finds open.
 */
public class TopDown extends DotGame {

    // My instance variables. These indicate the next play that I will
    // try. Except for the first time, this is the last play that I made.
    byte row = 0;   // start at row 0
    byte col = 0;   // start at column 0
    byte side = 0;  // start at side 0 (TOP)
    
    // It is my turn to play
    public short myTurn() {
	short playCode;     // temporary storage for a playCode
	
	// Get the playCode for my next possible play. If the
	// play is legal, then exit the loop and make the play.
	while ((playCode = getPlayCode(row, col, side)) == -1) {
	    // Illegal play, move to the next play
	    if (++col == cols) {        // move to the next column
		col = 0;                // start over
    	        if (++side > RIGHT) {   // move to the next side
    	            side = TOP;         // start over
		    ++row;              // move to the next row
    	        }
	    }
	}
	return playCode;    // I will make this play
    }
}
