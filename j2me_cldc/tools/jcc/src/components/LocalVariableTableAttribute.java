/*
 *	LocalVariableTableAttribute.java	1.4	00/02/19
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
 * A class to represent the LocalVariable table Attribute
 * of a method's code.
 */

public
class LocalVariableTableAttribute extends Attribute
{
    public LocalVariableTableEntry	data[];

    public
    LocalVariableTableAttribute( UnicodeConstant name, int l, LocalVariableTableEntry d[] ){
	super( name, l );
	this.data = d;
    }

    public void
    externalize( ConstantPool p ){
	super.externalize( p );
 	for (int i = 0; i < data.length; i++) {
	    if (data[i].name != null) {
		data[i].name = (UnicodeConstant)p.add( data[i].name );
	    }
	    if (data[i].sig != null) {
		data[i].sig = (UnicodeConstant)p.add( data[i].sig );
	    }
	}
    }

    protected int
    writeData( DataOutput o ) throws IOException{
	int n = data.length;
	o.writeShort( n );
	for ( int i = 0; i < n; i++ ){
	    data[i].write( o );
	}
	return 2+LocalVariableTableEntry.size*n;
    }

    public void
    countConstantReferences( boolean isRelocatable ){
	super.countConstantReferences( isRelocatable );
	for (int i = 0; i < data.length; i++) {
	    data[i].name.incReference();
	    data[i].sig.incReference();
	}
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
    finishReadAttribute( DataInput in, UnicodeConstant name, ConstantObject globals[] ) throws IOException {
	int l;
	int n;
	LocalVariableTableEntry d[];

	l  = in.readInt();
	n  = in.readUnsignedShort();
	d = new LocalVariableTableEntry[ n ];
	for ( int i = 0; i < n; i++ ){
	    d[i] = new
		LocalVariableTableEntry( in.readUnsignedShort(),
					 in.readUnsignedShort(),
					 (UnicodeConstant)
					 globals[in.readUnsignedShort()],
					 (UnicodeConstant)
					 globals[in.readUnsignedShort()],
					 in.readUnsignedShort()
					 );
	}
	return new LocalVariableTableAttribute( name, l, d );
    }
}
