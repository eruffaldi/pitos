/*
 *	ClassnameFilterList.java	1.2	99/06/22 SMI
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

import java.util.Vector;

/*
 * A ClassnameFilterList stores a list of ClassnameFilter objects.
 * It is used for mapping a classname into group, such as "JNI"
 */

public
class ClassnameFilterList {

    public ClassnameFilterList() {
	groups = new Vector();
	filters = new Vector();
    }

    public void addTypePatterns(String group, String patterns ) {
	boolean conditional;
	// If a pattern starts with '-', it matches only if there
	// were no earlier matches
	if (patterns.charAt(0) == '-') {
	    conditional = true;
	    patterns = patterns.substring(1);
	} else {
	    conditional = false;
	}
	ClassnameFilter f = new ClassnameFilter(conditional);
	parseClassList( patterns, f );
	groups.addElement(group.intern());
	filters.addElement(f);
    }

    public String[] getTypes(String classname) {
	Vector types = new Vector(1);
	int l = groups.size();
	for (int i = 0; i < l; ++i) {
	    String name = (String)groups.elementAt(i);
	    ClassnameFilter f = (ClassnameFilter)filters.elementAt(i);
	    // If a pattern is conditional, it matches only if there
	    // were no earlier matches
	    if (types.size() == 0 || !f.conditional) {
		if (f.accept(null, classname)) {
		    types.addElement(name);
		}
	    }
	}
	String[] strings = new String[types.size()];
	types.copyInto(strings);
	return strings;
    }

    public boolean isType(String classname, String type) {
	String[] types = getTypes(classname);
	for (int i = 0; i < types.length; ++i) {
	    if (types[i] == type.intern()) {
		return true;
	    }
	}
	return false;
    }

    // Parse the rest of the string as a list of classes.
    private void parseClassList(String val, ClassnameFilter filter) {
	java.util.StringTokenizer tkn =
	    new java.util.StringTokenizer(val, " ,", false );
	while ( tkn.hasMoreTokens() ){
	    String classname =
		util.LinkerUtil.sanitizeClassname( tkn.nextToken() );
	    filter.includeName(classname);
	}
    }

    private Vector groups;
    private Vector filters;

}
