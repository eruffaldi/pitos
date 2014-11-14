/*
 *	DoubleValueConstant.java	1.2	97/05/27 SMI
 *
 * Copyright (c) 1997 Sun Microsystems, Inc. All Rights Reserved.
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
 */

/*
 * Represents a CONSTANT_Double or CONSTANT_Long
 * Should be read or written one whole word after another.
 */
package components;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import Const;

public
class DoubleValueConstant extends ConstantObject
{
    // The two words of the constant
    public int highVal;
    public int lowVal;

    private DoubleValueConstant( int t, int h, int l ){
	tag = t;
	highVal = h;
	lowVal = l;
	nSlots = 2;
    }

    public static ConstantObject read( int t, DataInput i ) throws IOException{
	return new DoubleValueConstant( t, i.readInt(), i.readInt() );
    }

    public void write( DataOutput o ) throws IOException{
	o.writeByte( tag );
	o.writeInt( highVal );
	o.writeInt( lowVal );
    }

    public String toString(){
	String t = (tag == Const.CONSTANT_DOUBLE)?"Double [ " : "Long [ ";
	return t + Integer.toHexString( highVal ) + " " + Integer.toHexString( lowVal ) +" ]";
    }

    public int hashCode() {
	return tag + highVal + lowVal;
    }


    public boolean equals(Object o) {
	if (o instanceof DoubleValueConstant) {
	    DoubleValueConstant d = (DoubleValueConstant) o;
	    return tag == d.tag && highVal == d.highVal && lowVal == d.lowVal;
	} else {
	    return false;
	}
    }
    
    public boolean isResolved(){ return true; }
}
