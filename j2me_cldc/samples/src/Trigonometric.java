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
 
/**
 * Fast integer trigonometric sin calculation
 */

public class Trigonometric {

/*==============================================================================
 * Constants
 *============================================================================*/

    /**
     * Tables for rapid sin calculation.
     */
    static int dividend[] = { 
	    0, 3050, 5582, 6159, 1627, 5440, 7578, 3703, 3659, 9461, 2993, 
    	4887, 226, 7813, 8902, 7469, 6260, 1054, 5473, 6862, 3701, 5349, 
    	4523, 978, 9455, 9604, 1604, 2008, 6059, 6048, 1, 3990, 6695, 
    	1275, 6679, 8431, 4456, 4841, 6329, 3241, 3302, 5690, 1901, 860, 
    	2796, 5741, 9414, 4187, 3469, 1923, 4309, 2870, 6152, 3863, 5473, 
    	5603, 6047, 3457, 1412, 5011, 2521, 5462, 3319, 1496, 3801, 4411, 
    	3244, 4157, 7398, 6297, 5625, 538, 5266, 8623, 8015, 652, 7317, 
    	9010, 5819, 374, 7325, 5375, 5393, 5859, 1997, 6283, 819, 2186, 
    	3281, 6565, 1}; 

    static int divider[] = {
    	1, 174761, 159945, 117682, 23324, 62417, 72497, 30385, 26291, 60479, 
    	17236, 25612, 1087, 34732, 36797, 28858, 22711, 3605, 17711, 21077, 
    	10821, 14926, 12074, 2503, 23246, 22725, 3659, 4423, 12906, 12475, 2, 
    	7747, 12634, 2341, 11944, 14699, 7581, 8044, 10280, 5150, 5137, 8673, 
    	2841, 1261, 4025, 8119, 13087, 5725, 4668, 2548, 5625, 3693, 7807, 4837, 
    	6765, 6840, 7294, 4122, 1665, 5846, 2911, 6245, 3759, 1679, 4229, 4867, 
    	3551, 4516, 7979, 6745, 5986, 569, 5537, 9017, 8338, 675, 7541, 9247, 
    	5949, 381, 7438, 5442, 5446, 5903, 2008, 6307, 821, 2189, 
    	3283, 6566, 1};

/*==============================================================================
 * Static methods
 *============================================================================*/

    private static int multiSin90(int multiplier, int sin)
    {
        return(multiplier * dividend[sin] / divider[sin]);
    }

    /**
     * Rapid sin and cos functions.  Note that you must supply 
     * multiplier in addition to the (decimal) angle parameter,
     * or otherwise the result is always 0 or 1.
     */
    public static int multiSin(int multiplier, int sin)
    {
        while (sin < 0) sin += 360;
    	sin %= 360;
	
    	if (sin <=  90) return multiSin90(multiplier, sin);
    	if (sin <= 180) return multiSin90(multiplier, 180-sin);
    	if (sin <= 270) return -multiSin90(multiplier, sin-180);
    	return -multiSin90(multiplier, 360-sin);
    }

    public static int multiCos(int multiplier, int cos)
    {
        return(multiSin(multiplier, cos+90));
    }

}

