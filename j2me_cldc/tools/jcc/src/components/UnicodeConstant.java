/*
 *	UnicodeConstant.java	1.3	99/03/03 SMI
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

package components;
import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import Const;


// Class that represents a CONSTANT_UTF8 in a classfile's
// constant pool

public
class UnicodeConstant extends ConstantObject
{
    public String string;
    public String UTFstring;
    public int	  stringTableOffset;	// used by in-core output writers
    public boolean isSuffix = false;    // used by in-core output writers

    private UnicodeConstant( int t, String s ){
	tag = t;
	string = s;
	nSlots = 1;
    }

    public UnicodeConstant( String s ){
	this( Const.CONSTANT_UTF8, s );
    }

    public static ConstantObject read( int t, DataInput i ) throws IOException{
	return new UnicodeConstant( t, i.readUTF() );
    }

    public void write( DataOutput o ) throws IOException{
	o.writeByte( tag );
	o.writeUTF( string );
    }


    public int hashCode() {
	return string.hashCode();
    }


    public boolean equals(Object o) {
	if (o instanceof UnicodeConstant) {
	    UnicodeConstant a = (UnicodeConstant) o;
	    return string.equals(a.string);
	} else {
	    return false;
	}
    }

    public String toString() {
	return string;
    }

    public boolean isResolved(){ return true; }
}
