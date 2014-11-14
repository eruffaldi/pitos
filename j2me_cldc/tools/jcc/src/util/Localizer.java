/*
 *	Localizer.java	1.3	98/08/12 SMI
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
import text.*;

import java.text.MessageFormat;

/*
 * The source of all localized messages for all the programs
 * and classes that used the JCCMessage bundle. Since there
 * are multiple programs involved, this facility is a separate class,
 * rather than part of any one of them. It is assumed that locale
 * is uniform over all classes of these programs, so does not need
 * to be determined per-class or per-object.
 */

public class Localizer {
    static      PI18n localizer = new PI18n("JCCMessage");

    public static String getString( String key ){
	return localizer.getString( key );
    }

    public static String getString (String key, Object[] values) { 
        return new MessageFormat(localizer.getString(key)).format(values);
    }

    public static String getString(String key, Object value) {
        return getString(key, new Object[]{value});
    }

    public static String getString(String key, Object value1, Object value2) {
        return getString(key, new Object[]{value1, value2});
    }

    public static String getString(String key, Object value1, 
				   Object value2, Object value3) {
        return getString(key, new Object[]{value1, value2, value3});
    }
}
