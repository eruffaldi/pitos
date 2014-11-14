/*
 *	ClassComponent.java	1.2	97/05/27 SMI
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
import java.io.IOException;

/*
 * An abstract class for representing components of a class
 * This include any field, method, or "constant".
 */

public
abstract class ClassComponent
{
    // whether or not "resolved" has been called, 
    // which sometimes determines interpretation
    public boolean resolved = false;


    abstract public void write( DataOutput o ) throws IOException;


    public void resolve( ConstantObject table[] ){
	// by default, just note that we're resolved.
	resolved = true;
    }

    public void
    externalize( ConstantPool p ){
	// by default, there's nothing to do
    }

}
