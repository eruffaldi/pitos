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

public class Average extends Random {

    short[] options;
    	
    public boolean otherSideOK(int row, int col, int side) {
	switch (side & 0x3) {
	case TOP:
	    if (--row <0) return true;
	    break;
	case LEFT:
	    if (--col <0) return true;
	    break;
	case BOTTOM:
	    if (++row >= rows) return true;
	    break;
	case RIGHT:
	    if (++col >= cols) return true;
	    break;
	}
	byte boxCode = box[row][col];
	return (boxCode >> 4) != 2;
    }	
    
    /**
     * 
     */
    public short myTurn() {
	if (options == null) options = new short[rows * cols * 4];
	int numOptions = 0;
	for (int row = 0; row < rows; row++) {
	    for (int col = 0; col < cols; col++) {
		int boxCode = box[row][col];
		switch (boxCode >> 4) {
		case 1: // last side
                    for (int side = 0; side <= 3; side++) {
                        if (((1 << side) & boxCode) == 0) {
                            return getPlayCode(row, col, side);
                        }
                    }
		case 2: // don't move here
		    break;
		default:
                    for (int side = 0; side <= 3; side++) {
                        if (((1 << side) & boxCode) == 0 && 
			    otherSideOK(row, col, side)) {
			    options[numOptions++] =
				getPlayCode(row, col, side);
                        }
                    }
                }
            }
        }
        if (numOptions != 0) {
	    int n = (random.nextInt() >>> 1) % numOptions;
            return options[n];
        }
	return super.myTurn();
    }
}
