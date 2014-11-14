/*
 *	InterfaceVector.java	1.4	99/04/30 SMI
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

package vm;
import components.ClassInfo;
/*
 * A class's InterfaceMethodTable is an array of InterfaceVector,
 * giving the correspondence between the methods of the interface and the
 * methods of the containing class.
 * This could be implemented as an array-of-short, except that we
 * want to do sharing at runtime, so must tag each such array with an "owner",
 * for naming purposes.
 */

public class
InterfaceVector {
    public ClassClass	parent;
    public ClassInfo	intf;
    public short	v[];
    public boolean	generated; // for use of output writer.
    public int		offset;    // for use of output writer.

    public InterfaceVector( ClassClass p, ClassInfo i, short vec[] ){
	parent = p;
	intf = i;
	v = vec;
	generated = false;
	offset = -1; // clearly a bogus value.
    }
}
