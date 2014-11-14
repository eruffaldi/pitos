/*
 *	SingleValueConstant.java	1.2	97/05/27 SMI
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

// Represents a CONSTANT_Integer or CONSTANT_Float
package components;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import Const;

public
class SingleValueConstant extends ConstantObject
{
    public int value;

    private SingleValueConstant( int t, int v ){
	tag = t;
	value = v;
	nSlots = 1;
    }

    public SingleValueConstant( int ival ){
	this( Const.CONSTANT_INTEGER, ival );
    }

    public static ConstantObject read( int t, DataInput i ) throws IOException{
	return new SingleValueConstant( t, i.readInt() );
    }

    public void write( DataOutput o ) throws IOException{
	o.writeByte( tag );
	o.writeInt( value );
    }

    public String toString(){
	return (tag==Const.CONSTANT_FLOAT)? ("Float: "+Float.intBitsToFloat(value) ):
		("Int: "+Integer.toHexString( value ));
    }

    public int hashCode() {
	return tag + value;
    }


    public boolean equals(Object o) {
	if (o instanceof SingleValueConstant) {
	    SingleValueConstant s = (SingleValueConstant) o;
	    return tag == s.tag && value == s.value;
	} else {
	    return false;
	}
    }

    public boolean isResolved(){ return true; }
}
