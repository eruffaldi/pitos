/*
 *	LinkerUtil.java	1.2	00/02/19 SMI
 *
 * Copyright (c) 1998 Sun Microsystems, Inc. All Rights Reserved.
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

package util;
/*
 * Some things I find convenient for parsing strings and
 * options containing signatures.
 */
public class LinkerUtil{

    /*
     * Read the named file.
     * Tokenize the input.
     * Returns an array of Strings which are the tokens.
     * Propagates FileNotFound if the named file cannot be opened.
     * In the input stream, the character '#' is understood
     * to start a comment which continues to end-of-line.
     */
    public static String[]
    parseOptionFile( String fname ) throws java.io.IOException {
	java.util.Vector v = new java.util.Vector();
	java.io.StreamTokenizer in;
	in = new java.io.StreamTokenizer(
	    new java.io.BufferedInputStream(
		new java.io.FileInputStream( fname ) ) );
	in.resetSyntax();
	in.eolIsSignificant( false );
	in.whitespaceChars( 0, 0x20 );
	in.wordChars( '!', '~' );
	in.commentChar('#');

	while ( in.nextToken() != java.io.StreamTokenizer.TT_EOF ){
	    v.addElement( in.sval );
	}

	int n = v.size();
	String olist[] = new String[ n ];
	v.copyInto( olist );
	return olist;
    }

    /*
     * A few constants we frequently use.
     */
    public final static String mainName = "main";
    public final static String mainSig  = "([Ljava/lang/String;)V";
    public final static String constructorName = "<init>";
    public final static String constructorSig  = "()V";
    public final static String staticInitializerName = "<clinit>";
    public final static String staticInitializerSig  = "()V";


    /*
     * Classes are often written with . as component separator.
     * But in classfiles, as internally in our programs, we use /.
     * So we often want to find all instances of . and change them
     * into /.
     */
    public static String 
    sanitizeClassname( String classname ){
	return classname.replace('.', '/').intern();
    }

    /*
     * When writing a fully-qualified method name,
     * the type signature starts with (. Exploit this
     * fact when looking for that signature.
     */
    public static int
    sigOff( String n ){
	return n.indexOf( '(' );
    }

    /*
     * When writing a fully-qualified method name, 
     * the name of the method is separated from the name
     * of the containing class by a ., or sometimes by a /
     * find the offset of that character.
     */
    public static int
    methodOff( String n ){
	int moff = n.lastIndexOf( '.' );
	if ( moff >= 0 ) return moff;
	// curses. Must work harder.
	// Cannot just say lastIndexOf('/'), as that
	// may get us into the signature.
	int ending = n.indexOf( '(' );
	if ( ending < 0 ) ending = n.length();
	moff = n.lastIndexOf( '/', ending );
	return moff;
    }


}
