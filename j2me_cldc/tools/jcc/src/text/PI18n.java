/*
 * @(#)PI18n.java	1.2 98/06/23
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

package text;

import java.util.*;

/**
 ** This class is to get the resource file with the current locale
 ** set by JDK. Use ResourceBundle objects to isolate locale-sensitive
 ** date, such as translatable text. The second argument passed to
 ** getBundle method identify which properties file we want to access.
 ** The first argument refers to the actual properties files.
 ** When the locale was created, the language code and country code
 ** were passed to its constructor. The property files are named 
 ** followed by the language and country code.   
 **/
public class PI18n {
    public ResourceBundle bundle = null;
    public String propertyName = null;
    
    public PI18n(String str) {
	propertyName = new String(str);
    }

    public String getString(String key){
       Locale currentLocale = java.util.Locale.getDefault();

       if (bundle == null) {
           try{
               bundle = ResourceBundle.getBundle(propertyName, currentLocale);
           } catch(java.util.MissingResourceException e){
               System.out.println("Could not load Resources");
               System.exit(0);
           }
        }
        String value = new String("");
        try{
            value = bundle.getString(key);
        } catch (java.util.MissingResourceException e){
            System.out.println("Could not find " + key);}
        return value;
    }
}
