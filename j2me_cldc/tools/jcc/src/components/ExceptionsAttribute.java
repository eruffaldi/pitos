/*
 *	ExceptionsAttribute.java	1.7	00/02/19
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
import java.io.DataOutput;
import java.io.DataInput;
import java.io.IOException;
import util.*;

/*
 * A class to represent the Exceptions table Attribute
 * of a method
 */

public
class ExceptionsAttribute extends Attribute
{
    public ClassConstant	data[];

    public
    ExceptionsAttribute( UnicodeConstant name, int l, ClassConstant d[] ){
	super( name, l );
	data = d;
    }

    public void 
    countConstantReferences( boolean isRelocatable ){
	super.countConstantReferences( isRelocatable );
	int n = data.length;
	for ( int i = 0; i < n; i++ ){
	    data[i].incReference();
	}
    }

    protected int
    writeData( DataOutput o ) throws IOException{
	int n = data.length;
	o.writeShort( n );
	for ( int i = 0; i < n; i++ ){
	    o.writeShort( data[i].index );
	    // debug
	    if ( data[i].index <= 0 ){
		System.err.println(Localizer.getString("exceptionsattribute.exceptions_table_references_negative_subscript", data[i]));
	    }
	    // end debug
	}
	return 2 + 2*n;
    }


    public static Attribute
    readAttribute( DataInput i, ConstantObject locals[], ConstantObject globals[] ) throws IOException{
	UnicodeConstant name;

	name = (UnicodeConstant)globals[i.readUnsignedShort()];
	return finishReadAttribute( i, name, locals );
    }

    //
    // for those cases where we already read the name index
    // and know that its not something requiring special handling.
    //
    public static Attribute
    finishReadAttribute( DataInput in, UnicodeConstant name, ConstantObject t[] ) throws IOException {
	int l;
	int n;
	ClassConstant d[];

	l  = in.readInt();
	n  = in.readUnsignedShort();
	d = new ClassConstant[ n ];
	for ( int i = 0; i < n; i++ ){
	    int index = in.readUnsignedShort();
	    d[i] = (ClassConstant)t[ index ];
	}
	return new ExceptionsAttribute( name, l, d );
    }

}
