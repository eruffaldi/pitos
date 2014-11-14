/*
 *	InterfaceMethodFactory.java	1.3	97/05/27 SMI
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
/*
 * Provide a way to call back to make an InterfaceMethodTable
 * Annoying artifact of Java.
 * See InterfaceMethodTable and its target-specific subclasses, such
 * as coreimage.SPARCInterfaceMethodTable
 */
public interface InterfaceMethodFactory {
    public  InterfaceMethodTable newInterfaceMethodTable( ClassClass c, String n, InterfaceVector v[] );
}
