/*
 *	StringConstant.java	1.3	99/04/06 SMI
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
import util.DataFormatException;

// Represents a CONSTANT_String.  These constants are somewhat
// complicated in that we must fake string bodies for them.

public
class StringConstant extends ConstantObject
{
    // Filled in by Clas.resolveConstants()
    public UnicodeConstant str;

    public int strIndex;	// unresolved index of UnicodeConstant

    // used by core-image writer.
    public int unicodeIndex = -1;
    public int unicodeOffset= 0;
    public boolean dataWritten = false;
    public boolean handleWritten = false;


    private StringConstant( int t, int i ){
	tag = t;
	strIndex = i;
	nSlots = 1;
    }

    public static ConstantObject read( int t, DataInput i ) throws IOException{
	return new StringConstant( t, i.readUnsignedShort() );
    }

    public void resolve( ConstantObject table[] ){
	if (resolved) return;
	str = (UnicodeConstant)table[strIndex];
	resolved = true;
    }

    public void externalize( ConstantPool p ){
	str = (UnicodeConstant)p.add( str );
    }

    public void write( DataOutput o ) throws IOException{
	o.writeByte( tag );
	if ( resolved ){
	    o.writeShort( str.index );
	} else {
	    throw new DataFormatException("unresolved StringConstant");
	    //o.writeShort( strIndex );
	}
    }

    public String toString(){
	return "String: "+ ((resolved)?str.string:("[ "+strIndex+" ]"));
    }

    public void incReference() {
	references++;
	str.incReference();
    }

    public void decReference() {
	references--;
	str.decReference();
    }

    public int hashCode() {
	return tag + str.string.hashCode();
    }


    public boolean equals(Object o) {
	if (o instanceof StringConstant) {
	    StringConstant s = (StringConstant) o;
	    return str.string.equals(s.str.string);
	} else {
	    return false;
	}
    }

    public boolean isResolved(){ return true; }
}
