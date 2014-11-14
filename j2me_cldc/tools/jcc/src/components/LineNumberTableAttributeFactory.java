/*
 *	LineNumberTableAttributeFactory.java	1.1	97/06/23
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
import java.io.DataInput;
import java.io.IOException;

class LineNumberTableAttributeFactory extends AttributeFactory {

    public static AttributeFactory instance = new LineNumberTableAttributeFactory();

    Attribute finishReadAttribute(
	DataInput in, UnicodeConstant name, ConstantObject locals[], ConstantObject globals[] )
	throws IOException{
	    return LineNumberTableAttribute.finishReadAttribute( in, name );
    }
}
