/*
 *	KVMStringTable.java	1.9	00/02/11
 *
 * Copyright (c) 1997,1999 Sun Microsystems, Inc. All Rights Reserved.
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
import java.util.Enumeration;
import Const;
import components.*;
import vm.*;

public class KVMStringTable extends vm.StringTable {
    static final public String charArrayName =  "stringCharArrayInternal";
    static final public String stringArrayName = "stringArrayInternal";
    
    StringHashTable stringHashTable = new StringHashTable();

    final int writeStrings(KVMWriter writer, String name){
	writeCharacterArray(writer);
	writeStringInfo(writer);
	stringHashTable.writeTable(writer.out, name);
	return internedStringCount();
    }

    private boolean writeCharacterArray(KVMWriter writer) {
	final CCodeWriter out = writer.out;
	int n = arrangeStringData();
	if ( n == 0 ) { 
	    return false; // nothing to do here.
	}
	final char v[] = new char[n];
	data.getChars( 0, n, v, 0 );

	// First, typedef the header we're about to create
	out.print("static CONST CHARARRAY_X(" + n + ") " 
		  + charArrayName + " = {\n");
	
	out.println("\tCHARARRAY_HEADER(" + v.length + "),");
	out.println("\t{");
	writer.writeArray(v.length, 10, "\t\t", 
			  new KVMWriter.ArrayPrinter() { 
                              public void print(int index) { 
				  char c = v[index];
				  if (c == '\\') {
					  out.print("'\\\\'");
				  } else if (c == '\'') {
					  out.print("'\\''");
				  } else if (c >= ' ' && c <= 0x7E) { 
				      out.print("'" + c + "' ");
				  } else { 
				      out.printHexInt(c);
				  }
			      }
	                  });
	out.println("\t}");
	out.println("};\n");
	return true;
    }


    private void writeStringInfo(KVMWriter writer) {
	CCodeWriter out = writer.out;
	int count = internedStringCount();

	StringConstant stringTable[] = new StringConstant[count];
	
	for (Enumeration e = allStrings(); e.hasMoreElements(); ) { 
	    StringConstant s = (StringConstant)e.nextElement();
	    stringTable[ s.unicodeIndex ] = s;
	}
		
	out.println("static CONST struct stringInstanceStruct " + 
		    stringArrayName + "[" + count + "] = {");
	for (int i = 0; i < count; i++) { 
	    StringConstant s = stringTable[i];
	    String string = s.str.string;
	    out.print("\tKVM_INIT_JAVA_STRING(" + s.unicodeOffset + ", "
		      + string.length() + "), ");
	    commentary(string, out);
	}
	out.println("};\n\n");
    }

    public void writeString(CCodeWriter out, StringConstant s) { 
	if (s.unicodeIndex < 0) { 
	    System.out.println("String not in stringtable: \"" + s.str.string + "\"");
	}
	out.write('&');
	out.print(stringArrayName);
	out.write('[');
	out.print(s.unicodeIndex);
	out.write(']');		
    }


	/* Overwrite intern in vm.StringTable */
    public void intern( StringConstant s ) { 
	super.intern(s);
	stringHashTable.addEntry(s);
    }

    private final static int maxCom = 20;

    private void commentary( String s, CCodeWriter out ) {
	out.print("/* ");
	if (s.length() > maxCom) { 
	    s = s.substring(0, maxCom - 3) + "...";
	} 
	out.printSafeString(s);
	out.print(" */\n");
    }

    class StringHashTable extends KVMHashtable { 
	
	StringHashTable() { 
	    super(32, StringConstant.class); 
	}
	
	long hash(Object x) { 
	    StringConstant sc = (StringConstant) x;
	    String str = sc.str.string;
	    return stringHash(str);
	}

	int chainCount = 0;
	
        public void chainPrefix(CCodeWriter out) { 
	    out.println("static CONST struct String_Hash_Entry StringHashLinksInternal[] = {");
	}

        public void chainSuffix(CCodeWriter out) { 
	    out.println("};\n");
	}


	Object tableChain(CCodeWriter out, int bucket, Object[] list) {
	    int length = list.length;
	    int startCount = chainCount;
	    out.print("\t/* Start of bucket " + bucket + " */\n");
	    for (int i = 0; i < length; i++) { 
		out.print("\tSTRING_LINK(");
		writeString(out, (StringConstant)(list[i]));
		out.print(", ");
		if (i == length - 1) { 
		    out.print("NULL");
		} else { 
		    out.print("StringHashLinksInternal");
		    out.print(" + ");
		    out.print(chainCount + 1);
		} 
		out.println("), /* " + chainCount + " */");
		chainCount++;
	    }
	    return length == 0 ? null : new Integer(startCount);
	}
	
	void tableEntry(CCodeWriter out, int bucket, Object token ) { 
	    if (token == null) { 
		out.print("NULL");
	    } else { 
		out.print("(StringHashLinksInternal + ");
		out.print((Integer)token);
		out.write(')');
	    }
	}
    }
}
