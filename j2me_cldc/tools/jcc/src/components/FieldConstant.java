/*
 *	FieldConstant.java	1.2	97/05/27 SMI
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

// Represents a CONSTANT_Field

public
class FieldConstant extends FMIrefConstant
{
    // Reference to the actual field (need it for index into the
    // fieldblock for the field reference).
    boolean	didLookup;
    FieldInfo	theField;

    FieldConstant( int t ){ tag = t; }

    public FieldConstant( ClassConstant c, NameAndTypeConstant s ){
        super( Const.CONSTANT_FIELD, c, s );
    }

    public static ConstantObject
    read( int t, DataInput in ) throws IOException {
	FieldConstant f = new FieldConstant( t );
	f.read( in );
	return f;
    }

    public FieldInfo find(){
	if ( ! didLookup ){
	    theField = (FieldInfo)super.find(false);
	    didLookup = true;
	}
	return theField;
    }

}
