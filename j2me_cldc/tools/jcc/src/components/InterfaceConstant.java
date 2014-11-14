/*
 *	InterfaceConstant.java	1.2	97/05/27 SMI
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

/*
 * Represents CONSTANT_InterfaceMethodref
 * There is very, very little difference between one of these
 * and a plain Method reference.
 */

public
class InterfaceConstant extends MethodConstant
{
    InterfaceConstant( int t ){
	//tag = t;
	super( t );
    }

    public static ConstantObject
    read( int t, DataInput in ) throws IOException {
	FMIrefConstant mc = new InterfaceConstant( t );
	mc.read( in );
	return mc;
    }

}
