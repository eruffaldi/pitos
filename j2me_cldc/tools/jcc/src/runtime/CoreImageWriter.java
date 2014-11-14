/*
 *	CoreImageWriter.java	1.13	00/02/19 SMI
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

package runtime;
import components.ConstantPool;
import util.ClassnameFilterList;

/*
 * This is the interface that a machine-independent
 * (ROM) image writer must implement in order to be called
 * by the JavaCodeCompact driver.
 * Additionally, the image writer must have a public constructor
 * of no parameters.
 */
public
interface CoreImageWriter {

    void    init( boolean uselinenumbers, ClassnameFilterList nativeTypes, boolean verbose, int maxSegmentSize);
	/*
	 * Finish initialization. Parameters:
	 *	uselinenumbers: produce data structures giving correspondence
	 *		between bytecode PC and source line numbers
	 *	nativeTypes: classifies native method interface for a class
	 *	verbose: print informative messages on System.out.
	 */

    boolean setAttribute( String attributeValue );
	/*
	 * Process target-specific flags from the command line.
	 * Return value: false: flag unrecognized or malformed
	 *		 true:  flag recognized and well-formed.
	 */

    boolean open( String filename );
	/* 
	 * Open the named file for writing output.
	 * Return value: false: could not open. See printError.
	 *		 true: otherwise.
	 */

    boolean writeClasses( ConstantPool consts );
	/*
	 * Write class and other data structures to output file.
	 * Parameter:
	 *	consts: shared constant and string pool
	 * Return value: false: any error was encountered. See printError.
	 *		 true: otherwise.
	 */

    boolean writeClasses( ConstantPool consts, ConstantPool sharedconsts );
        /*
         * Write class and other data structures to output file.
         * Parameter:
         *      consts: string pool 
         *      sharedconsts: shared constant
         * Return value: false: any error was encountered. See printError.
         *               true: otherwise.   
         */

    void    printSpaceStats( java.io.PrintStream log );
	/*
	 * Print informative message upon normal completion.
	 * This is usually a set of numbers, such as the amount
	 * or read-only and read-write memory that will be consumed by
	 * this image.
	 * Parameter:
	 *	log: PrintStream to use for output
	 */

    void    close();
	/*
	 * Close output file.
	 */

    void    printError( java.io.PrintStream o );
	/*
	 * Print information about any error encountered during processing.
	 * To be called if either open or writeClasses returns false.
	 * Often, this is the contents of an exception thrown and caught
	 * internally.
	 * Parameter:
	 *	o: PrintStream to use for output
	 */

}
