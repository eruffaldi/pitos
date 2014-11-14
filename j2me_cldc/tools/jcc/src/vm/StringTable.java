/*
 *	StringTable.java	1.6	00/02/19 SMI
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
import jcc.Str2ID;
import components.StringConstant;
import components.UnicodeConstant;
import java.util.Hashtable;
import java.util.Enumeration;

/**
 *
 * There are two string-like data types in today's JDK:
 * 1) zero-terminated, C-language, ASCII strings
 * 2) Java Strings.
 * 
 * The former arise from:
 * (a) UTF encoding of Java class, method, field names and type
 *     signatures, used for linkage
 * (b) UTF encoded forms of Java String constants, used as keys
 *     for the intern-ing of said constants.
 * See the class AsciiStrings where these are manipulated
 * to achieve some sharing of runtime data structures.
 *
 * In this, the StringTable class, we keep track of Java Strings, in
 * the form of StringConstant's.
 * We enter them in a Str2ID structure, which will be wanted
 * at runtime. And we assign layout of the runtime char[] data.
 * Much aliasing of data is possible, since this is read-only and
 * not usually zero-terminated. We won't do any of that, for now.
 * There is much potential here.
 */

public class StringTable {
    public Str2ID	stringHash = new Str2ID();
    public Hashtable	htable = new Hashtable();
    public StringBuffer data;
    private int		aggregateSize;
    private int		stringIndex = 0;

    public void intern( StringConstant s ){
	StringConstant t = (StringConstant)htable.get( s );
	if ( t == null ){
	    htable.put( s, s );
	    stringHash.getID( s.str , s );
	    aggregateSize += s.str.string.length();
	    s.unicodeIndex = stringIndex++;
	} else {
	    s.unicodeIndex = t.unicodeIndex;
	}
    }

    public Enumeration allStrings(){
	return htable.elements();
    }

    public int internedStringCount(){ return stringIndex; }

    /*
     * Arrange for the "data" buffer to hold all the string bodies
     * in some form.
     * Arrange for the unicodeOffset field of each StringConstant
     * to be set to the index of the beginning of the representation
     * of its data in this array.
     */
    public int arrangeStringData(){
	/*
	 * Our initial guess is simply to concatenate all the data.
	 * Later, we can try to be cleverer.
	 */
	data = new StringBuffer( aggregateSize );
	int curOffset = 0;
	Enumeration s = allStrings();
	while ( s.hasMoreElements() ){
	    StringConstant t = (StringConstant)s.nextElement();
	    t.unicodeOffset = curOffset;
	    data.append( t.str.string );
	    curOffset += t.str.string.length();
	}
	return curOffset;
    }
}
