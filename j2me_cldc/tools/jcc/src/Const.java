/*
 *	Const.java	1.22%	00/02/19 SMI
 *
 * Copyright (c) 1997, 1999 Sun Microsystems, Inc. All Rights Reserved.
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

/*
 * Constants used by the Java linker which are not exported in any
 * direct way from the Java runtime or compiler.
 * These include the opcode values of the quick bytecodes, the instruction
 * lengths of same, and the size of certain data structures in the .class
 * file format.
 * These have been split to more appropriate places, but are collected here
 * for the convenience of old code that doesn't know which parts are which.
 * util.ClassFileConst has constants from the Red book, such as assess flags,
 *	type numbers and signature characters.
 * vm.VMConst has VM-specific things, such as internal flag bits used in
 *	romizer output
 * OpcodeConst is a file generated from the current opcode list. This is
 *	pretty stable in JVM, very unstable in EVM.
 */

public interface Const extends util.ClassFileConst, vm.VMConst, OpcodeConst
{
}
