/*
 *	ConstantValueAttribute.java	1.3	99/04/06
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
import util.DataFormatException;

/*
 * A class to represent the ConstantValue Attribute
 * of a field
 */

public
class ConstantValueAttribute extends Attribute
{
    public ConstantObject	data;

    public
    ConstantValueAttribute( UnicodeConstant name, int l, ConstantObject d ){
	super( name, l );
	data = d;
    }

    public void
    externalize( ConstantPool p ){
	super.externalize( p );
	data = p.dup( data );
    }

    protected int
    writeData( DataOutput o ) throws IOException{
	o.writeShort( data.index );
	return 2;
    }

    public void
    countConstantReferences( boolean isRelocatable ){
	super.countConstantReferences( isRelocatable );
	data.incReference();
    }


    public static Attribute
    readAttribute( DataInput i, ConstantObject globals[] ) throws IOException{
	UnicodeConstant name;

	name = (UnicodeConstant)globals[i.readUnsignedShort()];
	return finishReadAttribute( i, name, globals );
    }

    //
    // for those cases where we already read the name index
    // and know that its not something requiring special handling.
    //
    public static Attribute
    finishReadAttribute( DataInput i, UnicodeConstant name, ConstantObject globals[] ) throws IOException {
	int l;
	int n;
	ConstantObject d;

	l  = i.readInt();
	n  = i.readUnsignedShort();
	d  = globals[n];
	return new ConstantValueAttribute( name, l, d );
    }

}
