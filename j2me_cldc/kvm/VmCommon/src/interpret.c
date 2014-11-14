/*
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
 *
 */

/*=========================================================================
 * SYSTEM:    KVM
 * SUBSYSTEM: Bytecode interpreter
 * FILE:      interpret.c
 * OVERVIEW:  This file defines the Java bytecode interpreter.
 * AUTHOR:    Antero Taivalsaari, Sun Labs
 *            Edited by Doug Simon 11/1998
 *            Added access checks for final static fields etc. by Sheng Liang
 *=======================================================================*/

/*=========================================================================
 * Local include files
 *=======================================================================*/

#include <global.h>

/*=========================================================================
 * Virtual machine global registers (see description in Interpreter.h)
 *=======================================================================*/


BYTE*         ip; /*  Instruction pointer (program counter) */
FRAME         fp; /*  Current frame pointer */
cell*         sp; /*  Execution stack pointer */
cell*         lp; /*  Local variable pointer */
CONSTANTPOOL  cp; /*  Constant pool pointer */


/*=========================================================================
 * Virtual machine debugging/profiling variables
 * These variables are initialized in InitializeMemory to ensure that they
 * are reset each execution of the VM.
 *=======================================================================*/


/*=========================================================================
 * Bytecode table for debugging purposes (can be
 * removed if executable size is a major concern).
 *=======================================================================*/


#if (TRACEBYTECODES || TRACEVERIFIER)
const char* const byteCodeNames[] = {
    "NOP",              /*  0x00 */
    "ACONST_NULL",      /*  0x01 */
    "ICONST_M1",        /*  0x02 */
    "ICONST_0",         /*  0x03 */
    "ICONST_1",         /*  0x04 */
    "ICONST_2",         /*  0x05 */
    "ICONST_3",         /*  0x06 */
    "ICONST_4",         /*  0x07 */
    "ICONST_5",         /*  0x08 */
    "LCONST_0",         /*  0x09 */
    "LCONST_1",         /*  0x0A */
    "FCONST_0",         /*  0x0B     */
    "FCONST_1",         /*  0x0C */
    "FCONST_2",         /*  0x0D */
    "DCONST_0",         /*  0x0E */
    "DCONST_1",         /*  0x0F */
    "BIPUSH",           /*  0x10 */
    "SIPUSH",           /*  0x11 */
    "LDC",              /*  0x12 */
    "LDC_W",            /*  0x13 */
    "LDC2_W",           /*  0x14 */
    "ILOAD",            /*  0x15 */
    "LLOAD",            /*  0x16 */
    "FLOAD",            /*  0x17 */
    "DLOAD",            /*  0x18 */
    "ALOAD",            /*  0x19 */
    "ILOAD_0",          /*  0x1A */
    "ILOAD_1",          /*  0x1B */
    "ILOAD_2",          /*  0x1C */
    "ILOAD_3",          /*  0x1D */
    "LLOAD_0",          /*  0x1E */
    "LLOAD_1",          /*  0x1F */
    "LLOAD_2",          /*  0x20 */
    "LLOAD_3",          /*  0x21 */
    "FLOAD_0",          /*  0x22 */
    "FLOAD_1",          /*  0x23 */
    "FLOAD_2",          /*  0x24 */
    "FLOAD_3",          /*  0x25 */
    "DLOAD_0",          /*  0x26 */
    "DLOAD_1",          /*  0x27 */
    "DLOAD_2",          /*  0x28 */
    "DLOAD_3",          /*  0x29 */
    "ALOAD_0",          /*  0x2A */
    "ALOAD_1",          /*  0x2B */
    "ALOAD_2",          /*  0x2C */
    "ALOAD_3",          /*  0x2D */
    "IALOAD",           /*  0x2E */
    "LALOAD",           /*  0x2F */
    "FALOAD",           /*  0x30 */
    "DALOAD",           /*  0x31 */
    "AALOAD",           /*  0x32 */
    "BALOAD",           /*  0x33 */
    "CALOAD",           /*  0x34 */
    "SALOAD",           /*  0x35 */
    "ISTORE",           /*  0x36 */
    "LSTORE",           /*  0x37 */
    "FSTORE",           /*  0x38 */
    "DSTORE",           /*  0x39 */
    "ASTORE",           /*  0x3A */
    "ISTORE_0",         /*  0x3B */
    "ISTORE_1",         /*  0x3C */
    "ISTORE_2",         /*  0x3D */
    "ISTORE_3",         /*  0x3E */
    "LSTORE_0",         /*  0x3F */
    "LSTORE_1",         /*  0x40 */
    "LSTORE_2",         /*  0x41 */
    "LSTORE_3",         /*  0x42 */
    "FSTORE_0",         /*  0x43 */
    "FSTORE_1",         /*  0x44 */
    "FSTORE_2",         /*  0x45 */
    "FSTORE_3",         /*  0x46 */
    "DSTORE_0",         /*  0x47 */
    "DSTORE_1",         /*  0x48 */
    "DSTORE_2",         /*  0x49 */
    "DSTORE_3",         /*  0x4A */
    "ASTORE_0",         /*  0x4B */
    "ASTORE_1",         /*  0x4C */
    "ASTORE_2",         /*  0x4D */
    "ASTORE_3",         /*  0x4E */
    "IASTORE",          /*  0x4F */
    "LASTORE",          /*  0x50 */
    "FASTORE",          /*  0x51 */
    "DASTORE",          /*  0x52 */
    "AASTORE",          /*  0x53 */
    "BASTORE",          /*  0x54 */
    "CASTORE",          /*  0x55 */
    "SASTORE",          /*  0x56 */
    "POP",              /*  0x57 */
    "POP2",             /*  0x58 */
    "DUP",              /*  0x59 */
    "DUP_X1",           /*  0x5A */
    "DUP_X2",           /*  0x5B */
    "DUP2",             /*  0x5C */
    "DUP2_X1",          /*  0x5D */
    "DUP2_X2",          /*  0x5E */
    "SWAP",             /*  0x5F */
    "IADD",             /*  0x60 */
    "LADD",             /*  0x61 */
    "FADD",             /*  0x62 */
    "DADD",             /*  0x63 */
    "ISUB",             /*  0x64 */
    "LSUB",             /*  0x65 */
    "FSUB",             /*  0x66 */
    "DSUB",             /*  0x67 */
    "IMUL",             /*  0x68 */
    "LMUL",             /*  0x69 */
    "FMUL",             /*  0x6A */
    "DMUL",             /*  0x6B */
    "IDIV",             /*  0x6C */
    "LDIV",             /*  0x6D */
    "FDIV",             /*  0x6E */
    "DDIV",             /*  0x6F */
    "IREM",             /*  0x70 */
    "LREM",             /*  0x71 */
    "FREM",             /*  0x72 */
    "DREM",             /*  0x73 */
    "INEG",             /*  0x74 */
    "LNEG",             /*  0x75 */
    "FNEG",             /*  0x76 */
    "DNEG",             /*  0x77 */
    "ISHL",             /*  0x78 */
    "LSHL",             /*  0x79 */
    "ISHR",             /*  0x7A */
    "LSHR",             /*  0x7B */
    "IUSHR",            /*  0x7C */
    "LUSHR",            /*  0x7D */
    "IAND",             /*  0x7E */
    "LAND",             /*  0x7F */
    "IOR",              /*  0x80 */
    "LOR",              /*  0x81 */
    "IXOR",             /*  0x82 */
    "LXOR",             /*  0x83 */
    "IINC",             /*  0x84 */
    "I2L",              /*  0x85 */
    "I2F",              /*  0x86 */
    "I2D",              /*  0x87 */
    "L2I",              /*  0x88 */
    "L2F",              /*  0x89 */
    "L2D",              /*  0x8A */
    "F2I",              /*  0x8B */
    "F2L",              /*  0x8C */
    "F2D",              /*  0x8D */
    "D2I",              /*  0x8E */
    "D2L",              /*  0x8F */
    "D2F",              /*  0x90 */
    "I2B",              /*  0x91 */
    "I2C",              /*  0x92 */
    "I2S",              /*  0x93 */
    "LCMP",             /*  0x94 */
    "FCMPL",            /*  0x95 */
    "FCMPG",            /*  0x96 */
    "DCMPL",            /*  0x97 */
    "DCMPG",            /*  0x98 */
    "IFEQ",             /*  0x99 */
    "IFNE",             /*  0x9A */
    "IFLT",             /*  0x9B */
    "IFGE",             /*  0x9C */
    "IFGT",             /*  0x9D */
    "IFLE",             /*  0x9E */
    "IF_ICMPEQ",        /*  0x9F */
    "IF_ICMPNE",        /*  0xA0 */
    "IF_ICMPLT",        /*  0xA1 */
    "IF_ICMPGE",        /*  0xA2 */
    "IF_ICMPGT",        /*  0xA3 */
    "IF_ICMPLE",        /*  0xA4 */
    "IF_ACMPEQ",        /*  0xA5 */
    "IF_ACMPNE",        /*  0xA6 */
    "GOTO",             /*  0xA7 */
    "JSR",              /*  0xA8 */
    "RET",              /*  0xA9 */
    "TABLESWITCH",      /*  0xAA */
    "LOOKUPSWITCH",     /*  0xAB */
    "IRETURN",          /*  0xAC */
    "LRETURN",          /*  0xAD */
    "FRETURN",          /*  0xAE */
    "DRETURN",          /*  0xAF */
    "ARETURN",          /*  0xB0 */
    "RETURN",           /*  0xB1 */
    "GETSTATIC",        /*  0xB2 */
    "PUTSTATIC",        /*  0xB3 */
    "GETFIELD",         /*  0xB4 */
    "PUTFIELD",         /*  0xB5 */
    "INVOKEVIRTUAL",    /*  0xB6 */
    "INVOKESPECIAL",    /*  0xB7 */
    "INVOKESTATIC",     /*  0xB8 */
    "INVOKEINTERFACE",  /*  0xB9 */
    "UNUSED",           /*  0xBA */
    "NEW",              /*  0xBB */
    "NEWARRAY",         /*  0xBC */
    "ANEWARRAY",        /*  0xBD */
    "ARRAYLENGTH",      /*  0xBE */
    "ATHROW",           /*  0xBF */
    "CHECKCAST",        /*  0xC0 */
    "INSTANCEOF",       /*  0xC1 */
    "MONITORENTER",     /*  0xC2 */
    "MONITOREXIT",      /*  0xC3 */
    "WIDE",             /*  0xC4 */
    "MULTIANEWARRAY",   /*  0xC5 */
    "IFNULL",           /*  0xC6 */
    "IFNONNULL",        /*  0xC7 */
    "GOTO_W",           /*  0xC8 */
    "JSR_W",            /*  0xC9 */
    "BREAKPOINT",       /*  0xCA */
    /*  Fast bytecodes: */
    "LDC_FAST",             /*  0xCB */
    "LDC_W_FAST",           /*  0xCC */
    "LDC2_W_FAST",          /*  0xCD */
    "GETFIELD_FAST",        /*  0xCE */
    "PUTFIELD_FAST",        /*  0xCF */
    "GETFIELD2_FAST",       /*  0xD0 */
    "PUTFIELD2_FAST",       /*  0xD1 */
    "GETSTATIC_FAST",       /*  0xD2 */
    "PUTSTATIC_FAST",       /*  0xD3 */
    "GETSTATIC2_FAST",      /*  0xD4 */
    "PUTSTATIC2_FAST",      /*  0xD5 */
    "INVOKEVIRTUAL_FAST",   /*  0xD6 */
    "INVOKESPECIAL_FAST",   /*  0xD7 */
    "INVOKESTATIC_FAST",    /*  0xD8 */
    "INVOKEINTERFACE_FAST", /*  0xD9 */
    "NEW_FAST",             /*  0xDA */
    "ANEWARRAY_FAST",       /*  0xDB */
    "MULTIANEWARRAY_FAST",  /*  0xDC */
    "CHECKCAST_FAST",       /*  0xDD */
    "INSTANCEOF_FAST",       /*  0xDE */
    "CUSTOMCODE"             /*  0xDF */
};
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * Auxiliary private operations for bytecode interpreter
 *=======================================================================*/

/*=========================================================================
 * Use the following macro to place null checks where a NullPointerException
 * should be thrown (e.g. invoking a virtual method on a null object).
 * As this macro executes a continue statement, it must only be used where
 * the continue will effect a jump to the start of the interpreter loop.
 *=======================================================================*/

#define checkNotNull(object) \
    if (object == NIL) {       \
        raiseException(NullPointerException);\
        continue; }

/*=========================================================================
 * FUNCTION:      getByteCodeName()
 * TYPE:          public debugging operation
 * OVERVIEW:      Given a bytecode value, get the mnemonic name of
 *                the bytecode.
 * INTERFACE:
 *   parameters:  bytecode as an integer
 *   returns:     constant string containing the name of the bytecode
 *=======================================================================*/

#if TRACEBYTECODES
static const
char* getByteCodeName(ByteCode token)
{
    if (token >= 0 && token <= LASTBYTECODE)
         return byteCodeNames[token];
    else return "<INVALID>";
}
#else
#  define getByteCodeName(token) ""
#endif

/*=========================================================================
 * FUNCTION:      printRegisterStatus()
 * TYPE:          public debugging operation
 * OVERVIEW:      Print all the VM registers of the currently executing
 *                thread for debugging purposes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void
printRegisterStatus()
{
    cell* i;
    fprintf(stdout,"VM status:\n");
    fprintf(stdout,"Instruction pointer.: %lx ", (long)ip);
    if (fp)
        fprintf(stdout, "(offset within invoking method: %d)",
                ip - fp->thisMethod->u.java.code);
    if (ip) {
        fprintf(stdout, "\nNext instruction....: 0x%x", *ip);
        if (TRACEBYTECODES) {
            fprintf(stdout, " (%s)", getByteCodeName(*ip));
        }
    }
    fprintf(stdout,"\nFrame pointer.......: %lx\n", (long)fp);
    fprintf(stdout,"Local pointer.......: %lx\n", (long)lp);

    {
        STACK stack;
        int size = 0;
        for (stack = CurrentThread->stack; stack != NULL; stack = stack->next) {
            size += stack->size;
        }

        fprintf(stdout,"Stack size..........: %d; sp: %lx; ranges: ",
                size, (long)sp);
        for (stack = CurrentThread->stack; stack != NULL; stack = stack->next) {
            fprintf(stdout, "%lx-%lx;", (long)stack->cells,
                    (long)(stack->cells + stack->size));
        }
        fprintf(stdout, "\n");
    }

    if (fp) {
        fprintf(stdout,"Contents of the current stack frame:\n");
        for (i = lp; i <= sp; i++) {
        fprintf(stdout,"    %lx: %lx", (long)i, (long)*i);
            if (i == lp) fprintf(stdout," (lp)");
            if (i == (cell*)fp) fprintf(stdout," (fp)");
            if (i == (cell*)fp + SIZEOF_FRAME - 1) {
                fprintf(stdout," (end of frame)");
            }
            if (i == sp) fprintf(stdout," (sp)");
            fprintf(stdout,"\n");
        }
    }
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * FUNCTION:      printVMstatus()
 * TYPE:          public debugging operation
 * OVERVIEW:      Print all the execution status of the VM
 *                for debugging purposes.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *=======================================================================*/

#if INCLUDEDEBUGCODE
void printVMstatus()
{
    printStackTrace();
    printRegisterStatus();
    printExecutionStack();
    printProfileInfo();
}
#endif /*  INCLUDEDEBUGCODE */

/*=========================================================================
 * FUNCTION:      checkVMStatus()
 * TYPE:          public debugging operation (consistency checking)
 * OVERVIEW:      Ensure that VM registers are pointing to meaningful
 *                locations and that stacks have not over/underflown.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 * NOTE:          This operation is only called when profiling
 *                is enabled.
 *=======================================================================*/

#if INCLUDEDEBUGCODE && ENABLEPROFILING
static  void
checkVMstatus()
{
    STACK stack;
    int valid;

    if (CurrentThread == NIL) {
        return;
    }

    /*  Catch stack over/underflows */
    for (valid = 0, stack = CurrentThread->stack; stack; stack = stack->next) {
        if (STACK_CONTAINS(stack, sp)) {
            valid = 1;
            break;
        }
    }
    if (!valid) {
        fatalVMError("bad stack pointer");
    }

    /*  Frame pointer should always be within stack range */
    for (valid = 0, stack = CurrentThread->stack; stack; stack = stack->next) {
        if (STACK_CONTAINS(stack, (cell*)fp) && (cell*)fp <= sp) {
            valid = 1;
            break;
        }
    }
    if (!valid) {
        fatalVMError("Frame pointer corrupted\n");
    }

    /*  Locals pointer should always be within stack range */
    for (valid = 0, stack = CurrentThread->stack; stack; stack = stack->next) {
        if (STACK_CONTAINS(stack, lp) && lp <= sp) {
            valid = 1;
            break;
        }
    }
    if (!valid) {
        fatalVMError("Locals pointer corrupted\n");
    }

    /*  Check the number of active threads; this should always be > 1 */
    if (ActiveThreadCount < 1) {
        fatalVMError("Active thread count corrupted\n");
   }
}
#else

#define checkVMstatus()

#endif /*  INCLUDEDEBUGCODE && ENABLEPROFILING */


static void
fatalSlotError(CONSTANTPOOL constantPool, int cpIndex) {
    CONSTANTPOOL_ENTRY thisEntry = &constantPool->entries[cpIndex];
    unsigned char thisTag   = CONSTANTPOOL_TAG(constantPool, cpIndex);
    START_TEMPORARY_ROOTS
        if (thisTag & CP_CACHEBIT) {
            if (thisTag == CONSTANT_Fieldref) {
                sprintf(str_buffer, "No such Field %s.%s",
                        fieldName((FIELD)(thisEntry->cache)),
                        fieldSignature((FIELD)thisEntry->cache));
            } else {
                sprintf(str_buffer, "No such Method %s.%s",
                        methodName((METHOD)thisEntry->cache),
                        methodSignature((METHOD)thisEntry->cache));
            }
        } else {
            int nameTypeIndex       = thisEntry->method.nameTypeIndex;
            NameTypeKey nameTypeKey = constantPool->entries[nameTypeIndex].nameTypeKey;
            NameKey nameKey         = nameTypeKey.nt.nameKey;
            MethodTypeKey typeKey   = nameTypeKey.nt.typeKey;

            if (thisTag == CONSTANT_Fieldref) {
                sprintf(str_buffer, "No such Field %s.%s",
                        change_Key_to_Name(nameKey, NULL),
                        change_Key_to_FieldSignature(typeKey));
            } else {
                sprintf(str_buffer, "No such Method %s%s",
                        change_Key_to_Name(nameKey, NULL),
                        change_Key_to_MethodSignature(typeKey));
            }
        }
    END_TEMPORARY_ROOTS
    fatalError(str_buffer);
}


#if TRACEMETHODCALLS 
static void
TraceMethodEntry(METHOD method, const char *what) {
    START_TEMPORARY_ROOTS
        const char *methodName = methodName(method);
        INSTANCE_CLASS thisClass = method->ofClass;
        Log->enterMethod(what, className(thisClass), methodName,
                         methodSignature(method));
    END_TEMPORARY_ROOTS
}

static void
TraceMethodExit(METHOD method) {
    INSTANCE_CLASS thisClass = method->ofClass;

    START_TEMPORARY_ROOTS
        Log->exitMethod(className(thisClass), methodName(method));
    END_TEMPORARY_ROOTS
}


#else
#define TraceMethodEntry(method, what)
#define TraceMethodExit(method)
#endif


/*=========================================================================
 * Bytecode interpreter
 *=======================================================================*/

/*=========================================================================
 * FUNCTION:      Interpret()
 * TYPE:          public global operation
 * OVERVIEW:      Execute bytecode.
 * INTERFACE:
 *   parameters:  <none>
 *   returns:     <nothing>
 *
 * NOTES:         Remember that the interpreter can be called from C/C++
 *                functions. When the bytecode execution is finished,
 *                the execution returns back to the calling function.
 *                In general, we have tried to keep the interpreter
 *                as re-entrant as possible.
 *
 *                Remember to initialize all virtual machine registers
 *                and the call stack before entering the interpreter.
 *=======================================================================*/

void Interpret()
{
#if NEED_LONG_ALIGNMENT || NEED_DOUBLE_ALIGNMENT
    Java8 tdub;
#endif
    ByteCode token;
    METHOD   thisMethod;
    OBJECT   thisObject;

    while (TRUE) {

        VALIDATE_WRITE_ADDRESS(sp);
        VALIDATE_WRITE_ADDRESS(ip);

#if ENABLEPROFILING
        if (INCLUDEDEBUGCODE) {
            /*  Check that the VM is doing ok */
            checkVMstatus();
        }
        /*  Increment the instruction counter */
        /*  for profiling purposes */
        InstructionCounter++;
#endif

        /* Test to see if it is time to switch threads, and do so if it is */
        if (isTimeToReschedule()) {
            reschedule();
        }

    /*  Get the next bytecode */
    token = *ip++;

    if (TRACEBYTECODES && INCLUDEDEBUGCODE) {
        START_TEMPORARY_ROOTS
            /* mySP points to just the first stack word. */
            cell *mySP = (cell *)((char *)fp + sizeof(struct frameStruct));
            fprintf(stdout, "STACK (size %d): ", (sp - mySP) + 1);
            for ( ; mySP <= sp; mySP++) {
                fprintf(stdout, "%lx, ", *mySP);
            }
            fprintf(stdout, "\n");
            if (TERSE_MESSAGES) {
                fprintf(stdout,"    %d: %s\n",
                        (ip - 1) - fp->thisMethod->u.java.code,
                        getByteCodeName(token));
            } else {
                fprintf(stdout,"    %d: %s (in %s::%s)\n",
                        (ip - 1) - fp->thisMethod->u.java.code,
                        getByteCodeName(token),
                        className(fp->thisMethod->ofClass),
                        methodName(fp->thisMethod));

            }
        END_TEMPORARY_ROOTS
    }

    switch (token) {

/*=========================================================================
 * Java bytecodes are defined below
 *=======================================================================*/

case NOP:               /*  0x00 */
        /*  No operation */
        break;

case ACONST_NULL:       /*  0x01 */
        /*  Push null reference onto the operand stack */
        pushStack(NIL);
        break;

case ICONST_M1:         /*  0x02 */
        /*  Push integer constant -1 onto the operand stack */
case ICONST_0:          /*  0x03 */
        /*  Push integer constant 0 onto the operand stack */
case ICONST_1:          /*  0x04 */
        /*  Push integer constant 1 onto the operand stack */
case ICONST_2:          /*  0x05 */
        /*  Push integer constant 2 onto the operand stack */
case ICONST_3:          /*  0x06 */
        /*  Push integer constant 3 onto the operand stack */
case ICONST_4:          /*  0x07 */
        /*  Push integer constant 4 onto the operand stack */
case ICONST_5:          /*  0x08 */
        /*  Push integer constant 5 onto the operand stack */
        pushStack(token - ICONST_0);
        break;


case LCONST_0:          /*  0x09 */
        /*  Push long integer constant 0 onto the operand stack */
case LCONST_1:          /*  0x0A */
        /*  Push long integer constant 1 onto the operand stack */
        oneMore;
#if BIG_ENDIAN
        ((long *)sp)[0] = 0;
        ((long *)sp)[1] = (token - LCONST_0);
#elif LITTLE_ENDIAN || !COMPILER_SUPPORTS_LONG
        ((long *)sp)[0] = (token - LCONST_0);
        ((long *)sp)[1] = 0;
#else
        SET_LONG(sp, (ulong64)(token - LCONST_0));
#endif
        oneMore;
        break;

#if IMPLEMENTS_FLOAT
case FCONST_0:          /*  0x0B */
        /*  Push float constant 0 onto the operand stack */
case FCONST_1:          /*  0x0C */
        /*  Push float constant 1 onto the operand stack */
case FCONST_2:          /*  0x0D */
        /*  Push float constant 2 onto the operand stack */
        oneMore;
        *(float*)sp = (float)(token - FCONST_0);
        break;
#endif

#if IMPLEMENTS_FLOAT
case DCONST_0:          /*  0x0E */
        /*  Push double float constant 0 onto the operand stack */
case DCONST_1:          /*  0x0F */
        /*  Push double float constant 1 onto the operand stack */
        {
            double value = (double)(token - DCONST_0);
            oneMore;
            SET_DOUBLE(sp, value);
            oneMore;
            break;
        }
#endif

case BIPUSH:            /*  0x10 */
        /*  Push byte constant onto the operand stack */
        {
	    /*  Extends sign for negative numbers */
            int i = *(signed char*)ip++; 
            pushStack(i);
        }
        break;


case SIPUSH:            /*  0x11 */
        /*  Push short constant onto the operand stack */
        {
            short s = getShort(ip);
            ip += 2;
            pushStack((int)s); /*  Extends sign for negative numbers */
        }
        break;


case LDC:               /*  0x12 */
        /*  Push item from constant pool onto the operand stack */
        {
            unsigned int cpIndex = *ip++;
            CONSTANTPOOL_ENTRY thisEntry = &cp->entries[cpIndex];
            pushStack(thisEntry->integer);
            break;
        }
        break;


case LDC_W:             /*  0x13 */
        /*  Push item from constant pool (wide index) */
        {
            unsigned int cpIndex = getUShort(ip);
            CONSTANTPOOL_ENTRY thisEntry = &cp->entries[cpIndex];
            ip += 2;
            pushStack(thisEntry->integer);
        }
        break;


case LDC2_W:            /*  0x14 */
        /*  Push double or long from constant pool (wide index) */
        {
            unsigned int cpIndex = getUShort(ip);
            CONSTANTPOOL_ENTRY thisEntry = &cp->entries[cpIndex];
            unsigned long hiBytes = (unsigned long)(thisEntry[0].integer);
            unsigned long loBytes = (unsigned long)(thisEntry[1].integer);
            ip += 2;
            oneMore;
            SET_LONG_FROM_HALVES(sp, hiBytes, loBytes);
            oneMore;
            break;
        }
        break;


case ILOAD:             /*  0x15 */
        /*  Load integer from local variable */
#if IMPLEMENTS_FLOAT
case FLOAD:             /*  0x17 */
        /*  Load float from local variable */
#endif
case ALOAD:             /*  0x19 */
        /*  Load address from local variable */
        {
            unsigned int index = *ip++;
            pushStack(lp[index]);
        }
        break;


case LLOAD:             /*  0x16 */
        /*  Load long from local variable */
#if IMPLEMENTS_FLOAT
case DLOAD:             /*  0x18 */
        /*  Load double from local variable */
#endif
        {
            unsigned int index = *ip++;
            pushStack(lp[index]);
            pushStack(lp[index+1]);
        }
        break;


case ILOAD_0:           /*  0x1A */
        /*  Load integer from first (zeroeth) local variable */
case ILOAD_1:           /*  0x1B */
        /*  Load integer from second local variable */
case ILOAD_2:           /*  0x1C */
        /*  Load integer from third local variable */
case ILOAD_3:           /*  0x1D */
        /*  Load integer from fourth local variable */
        pushStack(lp[token - ILOAD_0]);
        break;


case LLOAD_0:           /*  0x1E */
        /*  Load long from first (zeroeth) local variable */
case LLOAD_1:           /*  0x1F */
        /*  Load long from second local variable */
case LLOAD_2:           /*  0x20 */
        /*  Load long from third local variable */
case LLOAD_3:           /*  0x21 */
        /*  Load long from fourth local variable */
        pushStack(lp[token - LLOAD_0]);
        pushStack(lp[token + 1 - LLOAD_0]);
        break;


#if IMPLEMENTS_FLOAT
case FLOAD_0:           /*  0x22 */
        /*  Load float from first (zeroeth) local variable */
case FLOAD_1:           /*  0x23 */
        /*  Load float from second local variable */
case FLOAD_2:           /*  0x24 */
        /*  Load float from third local variable */
case FLOAD_3:           /*  0x25 */
        /*  Load float from fourth local variable */
        pushStack(lp[token - FLOAD_0]);
        break;
#endif


#if IMPLEMENTS_FLOAT
case DLOAD_0:           /*  0x26 */
        /*  Load double from first (zeroeth) local variable */
case DLOAD_1:           /*  0x27 */
        /*  Load double from second local variable */
case DLOAD_2:           /*  0x28 */
        /*  Load double from third local variable */
case DLOAD_3:           /*  0x29 */
        /*  Load double from fourth local variable */
        pushStack(lp[token - DLOAD_0]);
        pushStack(lp[token + 1 - DLOAD_0]);
        break;
#endif


case ALOAD_0:           /*  0x2A */
        /*  Load address from first (zeroeth) local variable */
case ALOAD_1:           /*  0x2B */
        /*  Load address from second local variable */
case ALOAD_2:           /*  0x2C */
        /*  Load address from third local variable */
case ALOAD_3:           /*  0x2D */
        /*  Load address from fourth local variable */
        pushStack(lp[token - ALOAD_0]);
        break;


case IALOAD:            /*  0x2E */
        /*  Load integer from array */
#if IMPLEMENTS_FLOAT
case FALOAD:            /*  0x30 */
        /*  Load float from array */
#endif
case AALOAD:            /*  0x32 */
        /*  Load address from array */
        {
            int index = popStack();
            ARRAY thisArray = topStackAsType(ARRAY);

            /*  Remember to check that the array exists */
            if (thisArray) {
                /*  Check that the given index is within array boundaries */
                if (index >= 0 && index < (long)thisArray->length) {
                    topStack = thisArray->data[index].cell;
                } else raiseException(ArrayIndexOutOfBoundsException);
            } else raiseException(NullPointerException);
        }
        break;


case LALOAD:            /*  0x2F */
        /*  Load long from array */
#if IMPLEMENTS_FLOAT
case DALOAD:            /*  0x31 */
        /*  Load double from array */
#endif
       {
            int index = topStack;
            ARRAY thisArray = secondStackAsType(ARRAY);

            /*  Remember to check that the array exists */
            if (thisArray) {
                /*  Check that the given index is within array boundaries */
                if (index >= 0 && index < (long)thisArray->length) {
                    secondStack = thisArray->data[index * 2].cell;
                    topStack    = thisArray->data[index * 2 + 1].cell;
                } else raiseException(ArrayIndexOutOfBoundsException);
            } else raiseException(NullPointerException);
        }
        break;

case BALOAD:            /*  0x33 */
        /*  Load byte from array */
        {
            int index = popStack();
            BYTEARRAY thisArray = topStackAsType(BYTEARRAY);

            /*  Remember to check that the array exists */
            if (thisArray) {
                /*  Check that the given index is within array boundaries */
                if (index >= 0 && index < (long)thisArray->length) {
                    /*  Note: Extends sign for negative numbers */
                    topStack = (long)thisArray->bdata[index];
                } else raiseException(ArrayIndexOutOfBoundsException);
            } else raiseException(NullPointerException);
        }
        break;


case CALOAD:            /*  0x34 */
        /*  Load UNICODE character (16 bits) from array */
case SALOAD:            /*  0x35 */
        /*  Load short from array */
        {
            int index = popStack();
            SHORTARRAY thisArray = topStackAsType(SHORTARRAY);

            /*  Remember to check that the array exists */
            if (thisArray) {
                /*  Check that the given index is within array boundaries */
                if (index >= 0 && index < (long)thisArray->length) {
                    long value = thisArray->sdata[index];
                    if (token == CALOAD)
                        value &= 0xFFFF;
                    topStack = (cell)value;
                } else raiseException(ArrayIndexOutOfBoundsException);
            } else raiseException(NullPointerException);
        }
        break;

case ISTORE:            /*  0x36 */
        /*  Store integer into local variable */
#if IMPLEMENTS_FLOAT
case FSTORE:            /*  0x38 */
        /*  Store float into local variable */
#endif
case ASTORE:            /*  0x3A */
        /*  Store address into local variable */
        {
            unsigned int index = *ip++;
            VALIDATE_WRITE_ADDRESS(&lp[index]);
            lp[index] = popStack();
        }
        break;


case LSTORE:            /*  0x37 */
        /*  Store long into local variable */
#if IMPLEMENTS_FLOAT
case DSTORE:            /*  0x39 */
        /*  Store double into local variable */
#endif
        {
            unsigned int index = *ip++;
            lp[index+1] = popStack();
            lp[index]   = popStack();
        }
        break;


case ISTORE_0:          /*  0x3B */
        /*  Store integer into first (zeroeth) local variable */
case ISTORE_1:          /*  0x3C */
        /*  Store integer into second local variable */
case ISTORE_2:          /*  0x3D */
        /*  Store integer into third local variable */
case ISTORE_3:          /*  0x3E */
        /*  Store integer into fourth local variable */
        VALIDATE_WRITE_ADDRESS(&lp[token - ISTORE_0]);
        lp[token - ISTORE_0] = popStack();
        break;

case LSTORE_0:          /*  0x3F */
        /*  Store long into first (zeroeth) local variable */
case LSTORE_1:          /*  0x40 */
        /*  Store long into second local variable */
case LSTORE_2:          /*  0x41 */
        /*  Store long into third local variable */
case LSTORE_3:          /*  0x42 */
        /*  Store long into fourth local variable */
        VALIDATE_WRITE_ADDRESS(&lp[token - LSTORE_0]);
        lp[token + 1 - LSTORE_0] = popStack();
        lp[token - LSTORE_0] = popStack();
        break;


#if IMPLEMENTS_FLOAT
case FSTORE_0:          /*  0x43 */
        /*  Store float into first (zeroeth) local variable */
case FSTORE_1:          /*  0x44 */
        /*  Store float into second local variable */
case FSTORE_2:          /*  0x45 */
        /*  Store float into third local variable */
case FSTORE_3:          /*  0x46 */
        /*  Store float into fourth local variable */
        VALIDATE_WRITE_ADDRESS(&lp[token - FSTORE_0]);
        lp[token - FSTORE_0] = popStack();
        break;
#endif

#if IMPLEMENTS_FLOAT
case DSTORE_0:          /*  0x47 */
        /*  Store double into first (zeroeth) local variable */
case DSTORE_1:          /*  0x48 */
        /*  Store double into second local variable */
case DSTORE_2:          /*  0x49 */
        /*  Store double into third local variable */
case DSTORE_3:          /*  0x4A */
        /*  Store double into fourth local variable */
        VALIDATE_WRITE_ADDRESS(&lp[token - DSTORE_0]);
        lp[token + 1 - DSTORE_0] = popStack();
        lp[token - DSTORE_0] = popStack();
        break;
#endif

case ASTORE_0:          /*  0x4B */
        /*  Store address into first (zeroeth) local variable */
case ASTORE_1:          /*  0x4C */
        /*  Store address into second local variable */
case ASTORE_2:          /*  0x4D */
        /*  Store address into third local variable */
case ASTORE_3:          /*  0x4E */
        /*  Store address into fourth local variable */
        VALIDATE_WRITE_ADDRESS(&lp[token - ASTORE_0]);
        lp[token - ASTORE_0] = popStack();
        break;




case IASTORE:           /*  0x4F */
        /*  Store into integer array */
#if IMPLEMENTS_FLOAT
case FASTORE:           /*  0x51 */
        /*  Store into float array */
#endif
        {
            long value = popStack();
            int index = popStack();
            ARRAY thisArray = popStackAsType(ARRAY);

            /*  Remember to check that the array exists */
            if (thisArray) {
                /*  Check that the given index is within array boundaries */
                if (index >= 0 && index < (long)thisArray->length) {
                    thisArray->data[index].cell = value;
                } else raiseException(ArrayIndexOutOfBoundsException);
            } else raiseException(NullPointerException);
        }
        break;


case LASTORE:           /*  0x50 */
        /*  Store into long array */
#if IMPLEMENTS_FLOAT
case DASTORE:           /*  0x52 */
        /*  Store into double array */
#endif
        {
            cell  hiValue   = popStack();
            cell  loValue   = popStack();
            int   index     = popStack();
            ARRAY thisArray = popStackAsType(ARRAY);

            /*  Remember to check that the array exists */
            if (thisArray) {
                /*  Check that the given index is within array boundaries */
                if (index >= 0 && index < (long)thisArray->length) {
                    thisArray->data[index * 2].cell = loValue;
                    thisArray->data[index * 2 + 1].cell = hiValue;
                } else raiseException(ArrayIndexOutOfBoundsException);
            } else raiseException(NullPointerException);
        }
        break;


case AASTORE:           /*  0x53 */
        /*  Store into address array */
        {
            OBJECT value = popStackAsType(OBJECT);
            int    index = popStack();
            ARRAY  thisArray = popStackAsType(ARRAY);

            /*  Remember to check that the array exists */
            if (thisArray) {
                /*  Check that the given index is within array boundaries */
                if (index >= 0 && index < (long)thisArray->length) {
                    if (isInstanceOf(value, thisArray->ofClass->u.elemClass)) {
                        thisArray->data[index].cellp = (cell*)value;
                    } else raiseException(ArrayStoreException);
                } else raiseException(ArrayIndexOutOfBoundsException);
            } else raiseException(NullPointerException);
        }
        break;


case BASTORE:           /*  0x54 */
        /*  Store into byte array */
        {
            int value = popStack();
            int index = popStack();
            BYTEARRAY thisArray = popStackAsType(BYTEARRAY);

            /*  Remember to check that the array exists */
            if (thisArray) {
                /*  Check that the given index is within array boundaries */
                if (index >= 0 && index < (long)thisArray->length) {
                    thisArray->bdata[index] = (char)value;
                } else raiseException(ArrayIndexOutOfBoundsException);
            } else raiseException(NullPointerException);
        }
        break;


case CASTORE:           /*  0x55 */
        /*  Store into UNICODE character (16-bit) array */
case SASTORE:           /*  0x56 */
        /*  Store into short array */

        {
            int value = popStack();
            int index = popStack();
            SHORTARRAY thisArray = popStackAsType(SHORTARRAY);

            /*  Remember to check that the array exists */
            if (thisArray) {
                /*  Check that the given index is within array boundaries */
                if (index >= 0 && index < (long)thisArray->length) {
                    thisArray->sdata[index] = (short)value;
                } else raiseException(ArrayIndexOutOfBoundsException);
            } else raiseException(NullPointerException);
        }
        break;

case POP:               /*  0x57 */
        /*  Pop top operand stack word */
        oneLess;
        break;


case POP2:              /*  0x58 */
        /*  Pop two top operand stack words */
        lessStack(2);
        break;


case DUP:               /*  0x59 */
        /*  Duplicate top operand stack word */
        {
            long temp = topStack;
            pushStack(temp);
        }
        break;


case DUP_X1:            /*  0x5A */
        /*  Duplicate top operand stack word and put two down */
        /*  (this is the same as 'tuck' in Forth) */
        {
            long a = topStack;
            long b = secondStack;

            secondStack = a;
            topStack = b;
            pushStack(a);
        }
        break;


case DUP_X2:            /*  0x5B */
        /*  Duplicate top operand stack word and put three down */
        {
            long a = topStack;
            long b = secondStack;
            long c = thirdStack;

            thirdStack = a;
            secondStack = c;
            topStack = b;
            pushStack(a);
        }
        break;


case DUP2:              /*  0x5C */
        /*  Duplicate top two operand stack words */
        {
            long a = topStack;
            long b = secondStack;

            pushStack(b);
            pushStack(a);
        }
        break;


case DUP2_X1:           /*  0x5D */
        /*  Duplicate top two operand stack words and put three down */
        {
            long a = topStack;
            long b = secondStack;
            long c = thirdStack;

            thirdStack = b;
            secondStack = a;
            topStack = c;

            pushStack(b);
            pushStack(a);
        }
        break;


case DUP2_X2:           /*  0x5E */
        /*  Duplicate top two operand stack words and put four down */
        {
            long a = topStack;
            long b = secondStack;
            long c = thirdStack;
            long d = fourthStack;

            fourthStack = b;
            thirdStack = a;
            secondStack = d;
            topStack = c;

            pushStack(b);
            pushStack(a);
        }
        break;


case SWAP:              /*  0x5F */
        /*  Swap top two operand stack words */
        {
            long a = topStack;
            long b = secondStack;

            topStack = b;
            secondStack = a;
        }
        break;


case IADD:              /*  0x60 */
        /*  Add integer */
        {
            long temp = popStack();
            *(long*)sp += temp;
        }
        break;


case LADD:              /*  0x61 */
        /*  Add long */
        {
            long64 lvalue = GET_LONG(sp - 3);
            long64 rvalue = GET_LONG(sp - 1);
            ll_inc(lvalue, rvalue);
            SET_LONG(sp - 3, lvalue);
        }
        lessStack(2);
        break;

#if IMPLEMENTS_FLOAT
case FADD:              /*  0x62 */
        /*  Add float */
        {
            float temp = *(float*)sp--;
            *(float*)sp += temp;
        }
        break;
#endif


#if IMPLEMENTS_FLOAT
case DADD:              /*  0x63 */
        /*  Add double */
        {
            double rvalue = GET_DOUBLE(sp-1);
            double lvalue = GET_DOUBLE(sp-3);
            SET_DOUBLE(sp-3, lvalue + rvalue);
            lessStack(2);
        }
        break;
#endif

case ISUB:              /*  0x64 */
        /*  Subtract integer */
        {
            long temp = popStack();
            *(long*)sp -= temp;
        }
        break;

case LSUB:              /*  0x65 */
        /*  Subtract long */
        {
            long64 lvalue = GET_LONG(sp - 3);
            long64 rvalue = GET_LONG(sp - 1);
            ll_dec(lvalue, rvalue);
            SET_LONG(sp - 3, lvalue);
        }
        lessStack(2);
        break;


#if IMPLEMENTS_FLOAT
case FSUB:              /*  0x66 */
        /*  Subtract float */
        {
            float temp = *(float*)sp--;
            *(float*)sp -= temp;
        }
        break;
#endif


#if IMPLEMENTS_FLOAT
case DSUB:              /*  0x67 */
        /*  Subtract double */
        {
            double rvalue = GET_DOUBLE(sp-1);
            double lvalue = GET_DOUBLE(sp-3);
            SET_DOUBLE(sp-3, lvalue - rvalue);
            lessStack(2);
        }
        break;
#endif


case IMUL:              /*  0x68 */
        /*  Multiply integer */
        {
            long temp = popStack();
            *(long*)sp *= temp;
        }
        break;


case LMUL:              /*  0x69 */
        /*  Multiply long */
        {
            long64 rvalue = GET_LONG(sp - 1);
            long64 lvalue = GET_LONG(sp - 3);
            SET_LONG(sp - 3, ll_mul(lvalue, rvalue));
            lessStack(2);
        }
        break;

#if IMPLEMENTS_FLOAT
case FMUL:              /*  0x6A */
        /*  Multiply float */
        {
            float temp = *(float*)sp--;
            *(float*)sp *= temp;
        }
        break;
#endif


#if IMPLEMENTS_FLOAT
case DMUL:              /*  0x6B */
        /*  Multiply double */
        {
            double rvalue = GET_DOUBLE(sp-1);
            double lvalue = GET_DOUBLE(sp-3);
            SET_DOUBLE(sp-3,  lvalue * rvalue);
            lessStack(2);
        }
        break;
#endif


case IDIV:              /*  0x6C */
        /*  Divide integer */
        {
            long rValue = popStack();
            long lValue = topStack;

            if (rValue == 0) {
                raiseException(ArithmeticException);
            } else if (lValue == 0x80000000 && rValue == -1) {
                topStack = lValue;
            } else {
                topStack = lValue / rValue;
            }
        }
        break;


case LDIV:              /*  0x6D */
        /*  Divide long */
        {
            long64 rvalue = GET_LONG(sp - 1);
            long64 lvalue = GET_LONG(sp - 3);
            if (ll_zero_eq(rvalue)) {
                raiseException(ArithmeticException);
            } else {
                SET_LONG(sp - 3, ll_div(lvalue, rvalue));
                lessStack(2);
            }
        }
        break;

#if IMPLEMENTS_FLOAT
case FDIV:              /*  0x6E */
        /*  Divide float */
        {
            float temp = *(float*)sp--;
            *(float*)sp /= temp;
        }
        break;
#endif


#if IMPLEMENTS_FLOAT
case DDIV:              /*  0x6F */
        /*  Divide double */
        {
            double rvalue = GET_DOUBLE(sp-1);
            double lvalue = GET_DOUBLE(sp-3);

            SET_DOUBLE(sp-3,  lvalue / rvalue);
            lessStack(2);
        }
        break;
#endif


case IREM:              /*  0x70 */
        /*  Calculate integer remainder */
        {
            long rValue = popStack();
            long lValue = topStack;

            if (rValue == 0) {
                raiseException(ArithmeticException);
            } else if (lValue == 0x80000000 && rValue == -1) {
                topStack = lValue % 1;
            } else {
                topStack = lValue % rValue;
            }
        }
        break;


case LREM:              /*  0x71 */
        /*  Calculate long remainder */
        {
            long64 rvalue = GET_LONG(sp - 1);
            long64 lvalue = GET_LONG(sp - 3);
            if (ll_zero_eq(rvalue)) {
                raiseException(ArithmeticException);
            } else {
                SET_LONG(sp - 3, ll_rem(lvalue, rvalue));
                lessStack(2);
            }
        }
        break;


#if IMPLEMENTS_FLOAT
case FREM:              /*  0x72 */
            /*  Float point remainder */
        {
            float rvalue = *(float*)sp--;
            float lvalue = *(float*)sp;
            double result = DOUBLE_REMAINDER(lvalue, rvalue);
            *(float *)sp = (float)result;
        }
 break;
#endif


#if IMPLEMENTS_FLOAT
case DREM:              /*  0x73 */
        {
            double rvalue = GET_DOUBLE(sp-1);
            double lvalue = GET_DOUBLE(sp-3);
            double result = DOUBLE_REMAINDER(lvalue, rvalue);
            SET_DOUBLE(sp-3,  result);
            lessStack(2);
        }
        break;
#endif


case INEG:              /*  0x74 */
        /*  Negate integer */
        *(long*)sp = 0 - *(long*)sp;
        break;


case LNEG:              /*  0x75 */
        {
            long64 lvalue = GET_LONG(sp - 1);
            ll_negate(lvalue);
            SET_LONG(sp - 1, lvalue);
        }
        break;


#if IMPLEMENTS_FLOAT
case FNEG:              /*  0x76 */
        /*  Negate float */
        *(float*)sp = 0.0f - *(float*)sp;
        break;
#endif


#if IMPLEMENTS_FLOAT
case DNEG:              /*  0x77 */
        /*  Negate double */
        {
            double value = - GET_DOUBLE(sp-1);
            SET_DOUBLE(sp-1,  value);
        }
        break;
#endif

case ISHL:              /*  0x78 */
        /*  Arithmetic shift left integer */
        /*  Note: shift range is limited to 31 (0x1F) */
        {
            long s = popStack() & 0x0000001F;
            *(long*)sp <<= s;
        }
        break;


case LSHL:              /*  0x79 */
        /*  Arithmetic shift left long */
        /*  Note: shift range is limited to 63 (0x3F) */
        {
            long s = popStack() & 0x0000003F;
            long64 value = GET_LONG(sp - 1);
            SET_LONG(sp - 1, ll_shl(value, s));
        }
        break;


case ISHR:              /*  0x7A */
        /*  Arithmetic shift right integer */
        /*  Note: shift range is limited to 31 (0x1F) */
        {
            long s = popStack() & 0x0000001F;
            *(long*)sp >>= s; /*  Extends sign for negative numbers */
        }
        break;


case LSHR:              /*  0x7B */
        /*  Arithmetic shift right long */
        /*  Note: shift range is limited to 63 (0x3F) */
        {
            long s = popStack() & 0x0000003F;
            long64 value = GET_LONG(sp - 1);
            SET_LONG(sp - 1, ll_shr(value, s));
        }
        break;


case IUSHR:             /*  0x7C */
        /*  Logical shift right integer */
        /*  Note: shift range is limited to 31 (0x1F) */
        {
            long s = popStack() & 0x0000001F;
            *(unsigned long*)sp >>= s; /*  No sign extension */
        }
        break;


case LUSHR:             /*  0x7D */
        /*  Logical shift right long */
        /*  Note: shift range is limited to 63 (0x3F) */
        {
            long s = popStack() & 0x0000003F;
            long64 value = GET_LONG(sp - 1);
            SET_LONG(sp - 1, ll_ushr(value, s));
        }
        break;


case IAND:              /*  0x7E */
        /*  Boolean integer AND */
        {
            long temp = popStack();
            topStack &= temp;
        }
        break;


case LAND:              /*  0x7F */
        /*  Boolean long AND */
        {
            thirdStack  &= topStack;
            fourthStack &= secondStack;
            lessStack(2);
        }
        break;


case IOR:               /*  0x80 */
        /*  Boolean integer OR */
        {
            long temp = popStack();
            topStack |= temp;
        }
        break;


case LOR:               /*  0x81 */
        /*  Boolean long OR */
        {
            thirdStack  |= topStack;
            fourthStack |= secondStack;
            lessStack(2);
        }
        break;


case IXOR:              /*  0x82 */
        /*  Boolean integer XOR */
        {
            long temp = popStack();
            topStack ^= temp;
        }
        break;


case LXOR:              /*  0x83 */
        /*  Boolean long XOR */
        {
            thirdStack  ^= topStack;
            fourthStack ^= secondStack;
            lessStack(2);
        }
        break;


case IINC:              /*  0x84 */
        /*  Increment local variable */
        {
            unsigned long index = *ip++;
            long value = *(signed char*)ip++;
            VALIDATE_WRITE_ADDRESS(&lp[index]);
            lp[index] = ((long)(lp[index]) + value);
        }
        break;


case I2L:               /*  0x85 */
        /*  Convert integer to long */
        {
            long value = *(long *)sp;
#if BIG_ENDIAN
            ((long *)sp)[1] = value;
            ((long *)sp)[0] = value >> 31;
#elif LITTLE_ENDIAN || !COMPILER_SUPPORTS_LONG
            ((long *)sp)[1] = value >> 31;
#else
            SET_LONG(sp, value);
#endif
        }
        oneMore;
        break;

case I2F:               /*  0x86 */
        /*  Convert integer to float */
        *(float*)sp = (float)*(long*)sp;
        break;


case I2D:               /*  0x87 */
        /*  Convert integer to double */
        SET_DOUBLE(sp, (double)*(long*)sp);
        oneMore;
        break;


case L2I:               /*  0x88 */
        /*  Convert long to integer */
        oneLess;
#if BIG_ENDIAN
        sp[0] = sp[1];
#elif LITTLE_ENDIAN || !COMPILER_SUPPORTS_LONG
        /* do nothing */
#else
        sp[0] = (long)(GET_LONG(sp));
#endif
        break;


#if IMPLEMENTS_FLOAT
case L2F:               /*  0x89 */
        /*  Convert long to float */
        oneLess;
        *(float*)sp = ll2float(GET_LONG(sp));
        break;
#endif


#if IMPLEMENTS_FLOAT
case L2D:               /*  0x8A */
        /*  Convert long to double */
        {
          double value = ll2double(GET_LONG(sp - 1));
          SET_DOUBLE(sp-1, value);
        }
        break;
#endif

#if IMPLEMENTS_FLOAT
case F2I:               /*  0x8B */
        /*  Convert float to integer */
        *(long*)sp = (long)*(float*)sp;
        break;
#endif


#if IMPLEMENTS_FLOAT
case F2L:               /*  0x8C */
        /*  Convert float to long */
        SET_LONG(sp, float2ll(*(float*)sp));
        oneMore;
        break;
#endif

#if IMPLEMENTS_FLOAT
case F2D:               /*  0x8D */
        /*  Convert float to double */
        SET_DOUBLE(sp, (double)*(float*)sp);
        oneMore;
        break;
#endif


#if IMPLEMENTS_FLOAT
case D2I:               /*  0x8E */
        /*  Convert double to integer */
        oneLess;
        *(long*)sp = (long)GET_DOUBLE(sp);
        break;
#endif


#if IMPLEMENTS_FLOAT
case D2L:               /*  0x8F */
        /*  Convert double to long */
        {
           long64 value = double2ll(GET_DOUBLE( sp - 1));
           SET_LONG(sp - 1, value);
        }
        break;
#endif


#if IMPLEMENTS_FLOAT
case D2F:               /*  0x90 */
        /*  Convert double to float */
        oneLess;
        *(float*)sp = (float)GET_DOUBLE(sp);
        break;
#endif

case I2B:               /*  0x91 */
        /*  Convert integer to byte (with sign extension) */
        *(long*)sp = (char)*(long*)sp;
        break;


case I2C:               /*  0x92 */
        /*  Convert integer to UNICODE character */
        *(long*)sp = (unsigned short)*(long*)sp;
        break;


case I2S:               /*  0x93 */
        /*  Convert integer to short (with sign extension) */
        *(long*)sp = (short)*(long*)sp;
        break;


case LCMP:              /*  0x94 */
        /*  Compare long */
        {
            long result;
#if COMPILER_SUPPORTS_LONG
            long64 rvalue = GET_LONG(sp-1);
            long64 lvalue = GET_LONG(sp-3);
            result = lvalue < rvalue ? -1 : lvalue == rvalue ? 0 : 1;
#else
            long64 *rvalue = (long64*)(sp-1);
            long64 *lvalue = (long64*)(sp-3);

            signed long highLeft =  (signed long)(lvalue->high);
            signed long highRight = (signed long)(rvalue->high);
            if (highLeft == highRight) {
                unsigned long lowLeft =  (unsigned long)(lvalue->low);
                unsigned long lowRight = (unsigned long)(rvalue->low);
                result = (lowLeft < lowRight ? -1 : lowLeft > lowRight ? 1 : 0);
            } else {
                result = highLeft < highRight ? -1 : 1;
            }
#endif
            lessStack(3);
            *(long *)sp = result;
        }
        break;

#if IMPLEMENTS_FLOAT
        {
            double rvalue;
            double lvalue;
case FCMPL:             /*  0x95 */
case FCMPG:             /*  0x96 */
        /*  Compare float */
            rvalue = *(float*)sp;
            lvalue = *(float*)(sp-1);
            oneLess;
            goto floatCompare;

case DCMPL:             /*  0x97 */
case DCMPG:             /*  0x98 */
        /*  Compare double */
            rvalue = GET_DOUBLE(sp-1);
            lvalue = GET_DOUBLE(sp-3);
            lessStack(3);
        floatCompare:
            *(long*)sp = (lvalue < rvalue) ? -1 :
                        (lvalue == rvalue) ? 0 :
                        (lvalue > rvalue) ?  1 :
                        (token & 01) ? -1 : 1;
            break;
        }
#endif


case IFEQ:              /*  0x99 */
        /*  Branch if equal to zero */
        if (popStack()) ip += 2;     /*  Continue execution */
        else ip += getShort(ip) - 1; /*  Branch */
        break;


case IFNE:              /*  0x9A */
        /*  Branch if different than zero */
        if (popStack())
             ip += getShort(ip) - 1; /*  Branch */
        else ip += 2;                /*  Continue */
        break;


case IFLT:              /*  0x9B */
        /*  Branch if less than zero */
        if ((long)popStack() < 0)
             ip += getShort(ip) - 1; /*  Branch */
        else ip += 2;                /*  Continue */
        break;


case IFGE:              /*  0x9C */
        /*  Branch if greater or equal to zero */
        if ((long)popStack() >= 0)
             ip += getShort(ip) - 1;
        else ip += 2;
        break;


case IFGT:              /*  0x9D */
        /*  Branch if greater than zero */
        if ((long)popStack() > 0)
              ip += getShort(ip) - 1;
        else ip += 2;
        break;


case IFLE:              /*  0x9E */
        /*  Branch if less or equal to zero */
        if ((long)popStack() <= 0)
             ip += getShort(ip) - 1;
        else ip += 2;
        break;


case IF_ICMPEQ:         /*  0x9F */
        /*  Branch if equal */
        {
            long b = popStack();
            long a = popStack();

            if (a == b)
                 ip += getShort(ip) - 1;
            else ip += 2;
        }
        break;


case IF_ICMPNE:         /*  0xA0 */
        /*  Branch if not equal */
        {
            long b = popStack();
            long a = popStack();

            if (a != b)
                 ip += getShort(ip) - 1;
            else ip += 2;
        }
        break;


case IF_ICMPLT:         /*  0xA1 */
        /*  Branch if less than */
        {
            long b = popStack();
            long a = popStack();

            if (a < b)
                 ip += getShort(ip) - 1;
            else ip += 2;
        }
        break;


case IF_ICMPGE:         /*  0xA2 */
        /*  Branch if greater or equal */
        {
            long b = popStack();
            long a = popStack();

            if (a >= b)
                 ip += getShort(ip) - 1;
            else ip += 2;
        }
        break;


case IF_ICMPGT:         /*  0xA3 */
        /*  Branch if greater */
        {
            long b = popStack();
            long a = popStack();

            if (a > b)
                 ip += getShort(ip) - 1;
            else ip += 2;
        }
        break;


case IF_ICMPLE:         /*  0xA4 */
        /*  Branch if less or equal */
        {
            long b = popStack();
            long a = popStack();

            if (a <= b)
                 ip += getShort(ip) - 1;
            else ip += 2;
        }
        break;


case IF_ACMPEQ:         /*  0xA5 */
        /*  Branch if references are equal */
        {
            cell b = popStack();
            cell a = popStack();

            if (a == b)
                 ip += getShort(ip) - 1;
            else ip += 2;
        }
        break;


case IF_ACMPNE:         /*  0xA6 */
        /*  Branch if references are not equal */
        {
            cell b = popStack();
            cell a = popStack();

            if (a != b)
                 ip += getShort(ip) - 1;
            else ip += 2;
        }
        break;


case GOTO:              /*  0xA7 */
        /*  Branch unconditionally */
        ip += getShort(ip) - 1;
        break;


case JSR:               /*  0xA8 */
        /*  Jump subroutine */
        pushStackAsType(BYTE*, ip + 2);
        ip += getShort(ip) - 1;
        break;


case RET:               /*  0xA9 */
        /*  Return from subroutine */
        {
            unsigned int index = *ip++;
            ip = *(BYTE**)&lp[index];
        }
        break;

case TABLESWITCH:       /*  0xAA */
        /*  Access jump table by index and jump */
        {
            long index = popStack();
            BYTE* opcodeAddress = ip - 1;
            long defaultValue, lowValue, highValue;

            /*  Align instruction pointer to the next 32-bit word boundary */
            ip = (unsigned char *) (((long)ip + 3) & ~3);

            defaultValue = getCell(ip);
            lowValue     = getCell(ip+4);
            highValue    = getCell(ip+8);

            if ((index < lowValue) || (index > highValue))
                 ip = opcodeAddress + defaultValue;
            else ip = opcodeAddress + getCell(ip+CELL*(index-lowValue+3));
        }
        break;


case LOOKUPSWITCH:      /*  0xAB */
        /*  Access jump table by key match and jump */
        {
            long key = popStack();
            BYTE* opcodeAddress = ip - 1;
            long defaultValue, numberOfPairs, i;

            /*  Align instruction pointer to the next 32-bit word boundary */
            ip = (unsigned char *) (((long)ip + 3) & ~3);

            defaultValue  = getCell(ip);
            numberOfPairs = getCell(ip+4);

            /*  Just a linear search now; it might be a good idea */
            /*  to use a binary search with larger lookupswitches */
            for (i = 0; i < numberOfPairs; i++) {
                if (getCell(ip+8 + CELL*2*i) == key) {
                    ip = opcodeAddress + getCell(ip+12 + CELL*2*i);
                    goto out;
                }
            }

            /*  If no match, choose default offset */
            ip = opcodeAddress + defaultValue;
        out: ;
        }
        break;


case IRETURN:           /*  0xAC */
        /*  Return integer from method */
case LRETURN:           /*  0xAD */
        /*  Return long from method */
#if IMPLEMENTS_FLOAT
case FRETURN:           /*  0xAE */
        /*  Return float from method */
case DRETURN:           /*  0xAF */
        /*  Return float from method */
#endif
case ARETURN:           /*  0xB0 */
        /*  Return address from method */
case RETURN:           /*  0xB1 */
        /*  Return no value from method */
        {
            BYTE*  previousIp  = fp->previousIp;
            OBJECT synchronized  = fp->syncObject;

            TraceMethodExit(fp->thisMethod);

            if (synchronized != NULL) {
                /* following prevents recursion if monitorExit fails */
                fp->syncObject = NULL;
                if (monitorExit(synchronized) == MonitorStatusError) {
                    /* the error handling has already modified the stack */
                    break;
                }
            }

            /*  Pop the stack frame and return data if we are not */
            /*  about to kill a thread */
            if (previousIp != KILLTHREAD) {
                if ((token & 1) == 0) {
                    /* The even ones are all the return of a single value */
                    cell data = topStack;
                    popFrame();
                    pushStack(data);
                } else if (token == RETURN) {
                    popFrame();
                } else {
                    /* We don't care whether it's little or big endian. . . */
                    long t2 = sp[0];
                    long t1 = sp[-1];
                    popFrame();
                    pushStack(t1);
                    pushStack(t2);
                }
            } else {
                stopThread();
            }
        }
        break;


case GETSTATIC:         /*  0xB2 */
        /*  Get static field from class */
        {
            /*  Get the CONSTANT_Fieldref index */
            unsigned int cpIndex;
            FIELD field;

            /*  Get the constant pool index */
            cpIndex = getUShort(ip);
            ip += 2;

            /*  Resolve constant pool reference */
            field = resolveFieldReference(cp, cpIndex, TRUE, token);

            if (field && !CLASS_INITIALIZED(field->ofClass)) {
                ip -= 3;
                initializeClass(field->ofClass);
                break;
            }

            if (field) {
                void *location = field->u.staticAddress;
                if (ENABLEFASTBYTECODES) {
                    /* Create an inline cache entry. */
                    int icacheIndex =
                        createInlineCacheEntry((cell*)location, ip-3);
                    ip[-3] = (field->accessFlags & ACC_DOUBLE)
                                ? GETSTATIC2_FAST : GETSTATIC_FAST;
                    putShort(ip - 2, icacheIndex);
                }
                /*  Load either one or two cells depending on field type */
                if (field->accessFlags & ACC_DOUBLE) {
                    oneMore;
                    COPY_LONG(sp, location);
                    oneMore;
                } else {
                    pushStack(*(cell *)location);
                }
            } else {
                fatalSlotError(cp, cpIndex);
            }
        }
        break;


case PUTSTATIC:         /*  0xB3 */
        /*  Set static field in class */
        {
            /*  Get the CONSTANT_Fieldref index */
            unsigned int cpIndex;
            FIELD field;

            /*  Get the constant pool index */
            cpIndex = getUShort(ip);
            ip += 2;

            /*  Resolve constant pool reference */
            field = resolveFieldReference(cp, cpIndex, TRUE, token);

            if (field && !CLASS_INITIALIZED(field->ofClass)) {
                ip -= 3;
                initializeClass(field->ofClass);
                break;
            }

            if (field) {
                void *location = field->u.staticAddress;
                if (ENABLEFASTBYTECODES) {
                    /*  Create an inline cache entry. */
                    int icacheIndex =
                        createInlineCacheEntry((cell*)location, ip - 3);
                    ip[-3] = (field->accessFlags & ACC_DOUBLE)
                           ? PUTSTATIC2_FAST : PUTSTATIC_FAST;
                    putShort(ip-2, icacheIndex);
                }
                VALIDATE_WRITE_ADDRESS(location);

                /*  Store either one or two cells depending on field type */
                if (field->accessFlags & ACC_DOUBLE) {
                    oneLess;
                    COPY_LONG(location, sp);
                    oneLess;
                }
                else *(cell *)location = popStack();
            } else {
                fatalSlotError(cp, cpIndex);
                /* raiseException(NoSuchFieldError); */
            }
        }
        break;


case GETFIELD:          /*  0xB4 */
        /*  Get field (instance variable) from object */
        {
            /*  Get the CONSTANT_Fieldref index */
            unsigned int cpIndex;
            FIELD field;
            INSTANCE instance = popStackAsType(INSTANCE);
            checkNotNull(instance)

            /*  Get the constant pool index */
            cpIndex = getUShort(ip);
            ip += 2;

            /*  Resolve constant pool reference */
            field = resolveFieldReference(cp, cpIndex, FALSE, token);
            if (field) {
                /*  Get slot index */
                int offset = field->u.offset;

                if (ENABLEFASTBYTECODES) {
                    if (field->ofClass == fp->thisMethod->ofClass) { /* self*/
                        ip[-3] = (field->accessFlags & ACC_DOUBLE)
                               ? GETFIELD2_FAST : GETFIELD_FAST;
                        putShort(ip - 2, offset);
                    }
                }
                /*  Load either one or two cells depending on field type */
                if (field->accessFlags & ACC_DOUBLE) {
                    pushStack(instance->data[offset].cell);
                    pushStack(instance->data[offset + 1].cell);
                } else {
                    pushStack(instance->data[offset].cell);
                }
            } else {
                fatalSlotError(cp, cpIndex);
            }
        }
        break;


case PUTFIELD:          /*  0xB5 */
        /*  Set field in object */
        {
            /*  Get the CONSTANT_Fieldref index */
            unsigned int cpIndex;
            FIELD field;

            /*  Get the constant pool index */
            cpIndex = getUShort(ip);
            ip += 2;

            /*  Resolve constant pool reference */
            field = resolveFieldReference(cp, cpIndex, FALSE, token);
            if (field) {

                /*  Get slot index */
                int offset = field->u.offset;

                if (ENABLEFASTBYTECODES) {
                    /* If feasible, replace the current bytecode sequence
                     * with a faster one
                     */
                    if (field->ofClass == fp->thisMethod->ofClass) {
                        ip[-3] = (field->accessFlags & ACC_DOUBLE)
                            ? PUTFIELD2_FAST : PUTFIELD_FAST;
                        putShort(ip - 2, offset);
                    }
                }

                /*  Store either one or two cells depending on field type */
                if (field->accessFlags & ACC_DOUBLE) {
                    cell  first       = popStack();
                    cell  second      = popStack();
                    INSTANCE instance = popStackAsType(INSTANCE);
                    checkNotNull(instance);
                    VALIDATE_WRITE_ADDRESS(&instance->data[offset].cell);
                    instance->data[offset].cell = second;
                    instance->data[offset + 1].cell = first;
                }
                else {
                    cell data         = popStack();
                    INSTANCE instance = popStackAsType(INSTANCE);
                    checkNotNull(instance)
                    VALIDATE_WRITE_ADDRESS(&instance->data[offset].cell);
                    instance->data[offset].cell = data;
                }
            } else {
                fatalSlotError(cp, cpIndex);
            }
        }
        break;


case INVOKEVIRTUAL:     /*  0xB6 */
        /*  Invoke instance method; dispatch based on dynamic class */
        {
            /*  Get the CONSTANT_Methodref index */
            unsigned int cpIndex;

            /*  Get the constant pool index */
            cpIndex = getUShort(ip);
            ip += 2;

            /*  Resolve constant pool reference */
            thisMethod = resolveMethodReference(cp, cpIndex, FALSE);
            if (thisMethod) {
                CLASS dynamicClass;

                /*  Calculate the number of parameters  from signature */
                int argCount = thisMethod->argCount;

                /* Get this */
                thisObject = *(OBJECT*)(sp-argCount+1);
                checkNotNull(thisObject);

                /* Get the dynamic class of the object */
                dynamicClass = thisObject->ofClass;

                /* Find the actual method */
                thisMethod = lookupMethod(dynamicClass, thisMethod->nameTypeKey);
                if (thisMethod) {
                    if (ENABLEFASTBYTECODES) {
                        /* Replace the current bytecode sequence */
                        int iCacheIndex =
                            createInlineCacheEntry((cell*)thisMethod, ip - 3);
                        *(ip-3) = INVOKEVIRTUAL_FAST;
                        putShort(ip-2, iCacheIndex);
                    }
                    TraceMethodEntry(thisMethod, "Virtual");
		    goto callMethod;
                } else {
                    fatalSlotError(cp, cpIndex);
                }
            } else {
                fatalSlotError(cp, cpIndex);
            }
        }
        break;


case INVOKESPECIAL:     /*  0xB7 */
        /*  Invoke instance method; special handling for superclass, */
        /*  private and instance initialization method invocations */
        {
            /*  Get the CONSTANT_Methodref index */
            unsigned int cpIndex;

            /*  Get the constant pool index */
            cpIndex = getUShort(ip);
            ip += 2;

            /*  Resolve constant pool reference */
            thisMethod = resolveMethodReference(cp, cpIndex, FALSE);
            if (thisMethod) {
                int argCount = thisMethod->argCount;
                if (ENABLEFASTBYTECODES) {
                    /* Replace the current bytecode sequence with inline
                     * caches a la Deutch-Schiffman.
                     */
                    int iCacheIndex =
                        createInlineCacheEntry((cell*)thisMethod, ip-3);
                    *(ip-3) = INVOKESPECIAL_FAST;
                    putShort(ip-2, iCacheIndex);
                }
		thisObject = *(OBJECT*)(sp-argCount+1);
		checkNotNull(thisObject);
                TraceMethodEntry(thisMethod, "special");
		goto callMethod;
            } else {
                fatalSlotError(cp, cpIndex);
            }
        }
        break;


case INVOKESTATIC:      /*  0xB8 */
        /*  Invoke a class (static) method */
        {
            /*  Get the CONSTANT_Methodref index */
            unsigned int cpIndex;

            /*  Get the constant pool index */
            cpIndex = getUShort(ip);
            ip += 2;

            /*  Resolve constant pool reference */
            thisMethod = resolveMethodReference(cp, cpIndex, TRUE);

            if (thisMethod && !CLASS_INITIALIZED(thisMethod->ofClass)) {
                ip -= 3;
                initializeClass(thisMethod->ofClass);
                break;
            }

            if (thisMethod) {

                if (ENABLEFASTBYTECODES) {
                    /* Replace the current bytecode sequence with inline
                     * caches a la Deutch-Schiffman.
                     */
                    int iCacheIndex =
                        createInlineCacheEntry((cell*)thisMethod, ip - 3);
                    ip[-3] = INVOKESTATIC_FAST;
                    putShort(ip-2, iCacheIndex);
                }

                TraceMethodEntry(thisMethod, "static");
		/* Indicate the object that is locked on for a monitor */
		thisObject = (OBJECT)thisMethod->ofClass;
		goto callMethod;
            } else {
                fatalSlotError(cp, cpIndex);
            }
        }
        break;


case INVOKEINTERFACE:   /*  0xB9 */
        /*  Invoke interface method */
        {
            unsigned int cpIndex;
            unsigned int argCount;

            /*  Get the constant pool index */
            cpIndex = getUShort(ip);
            ip += 2;

            /*  Get the argument count (specific to INVOKEINTERFACE bytecode) */
            argCount = *ip;
            ip += 2;

            /*  Resolve constant pool reference */
            thisMethod = resolveMethodReference(cp, cpIndex, FALSE);
            if (thisMethod) {
                INSTANCE_CLASS dynamicClass;

                /* Get "this" */
                thisObject = *(OBJECT*)(sp-argCount+1);
                checkNotNull(thisObject);

                /*  Get method table entry based on dynamic class */
                /*  with given method name and signature */
                dynamicClass = ((INSTANCE)thisObject)->ofClass;
                thisMethod = lookupMethod((CLASS)dynamicClass,
                                          thisMethod->nameTypeKey);
                if (thisMethod) {
                    if (ENABLEFASTBYTECODES) {
                        /* Replace the current bytecode sequence */
                        int iCacheIndex =
                            createInlineCacheEntry((cell*)thisMethod, ip-5);
                        ip[-5] = INVOKEINTERFACE_FAST;
                        putShort(ip-4, iCacheIndex);
                    }
                    TraceMethodEntry(thisMethod, "interface");
		    goto callMethod;
                } else {
                    fatalSlotError(cp, cpIndex);
                }
            } else {
                fatalSlotError(cp, cpIndex);
            }
        }
        break;


case NEW:               /*  0xBB */
        /*  Create new object */
        {
            /*  Get the CONSTANT_Class index */
            unsigned int cpIndex = getUShort(ip);

            /*  Get the corresponding class pointer */
            INSTANCE_CLASS thisClass =
                (INSTANCE_CLASS)resolveClassReference(cp,
						      cpIndex,
						      getCurrentClass());

            INSTANCE newObject;

            if (!CLASS_INITIALIZED(thisClass)) {
                ip--;
                initializeClass(thisClass);
                break;
            }
            ip += 2;

            if (ENABLEFASTBYTECODES) {
                int iCacheIndex =
                    createInlineCacheEntry((cell*)thisClass, ip - 3);
                ip[-3] = NEW_FAST;
                putShort(ip - 2, iCacheIndex);
            }
            newObject = instantiate(thisClass);
            if (newObject != NULL) {
                pushStackAsType(INSTANCE, newObject);
            }
        }
        break;


case NEWARRAY:          /*  0xBC */
        /*  Create new array object */
        {
            unsigned int arrayType = *ip++;
            long arrayLength = topStack;
            ARRAY_CLASS type = getPrimitiveArrayClass(arrayType);
            ARRAY result = instantiateArray(type, arrayLength);
            if (result != NULL) {
                topStackAsType(ARRAY) = result;
            }
        }
        break;


case ANEWARRAY:         /*  0xBD */
        /*  Create new array of reference type */
        {
            /*  Get the CONSTANT_Class index */
            unsigned int cpIndex = getUShort(ip);
            int  arrayLength = topStack;
            /*  Get the corresponding class pointer */
            CLASS elemClass = cp->entries[cpIndex].clazz; /* may be raw  */
            ARRAY_CLASS thisClass = getObjectArrayClass(elemClass);
            ARRAY result;
            ip += 2;

            if (ENABLEFASTBYTECODES) {
                /* Note that instantiateArray may change the ip on an error */
                int iCacheIndex =
                    createInlineCacheEntry((cell*)thisClass, ip - 3);
                ip[-3] = ANEWARRAY_FAST;
                putShort(ip - 2, iCacheIndex);
            }
            result = instantiateArray(thisClass, arrayLength);
            if (result != NULL) {
                topStackAsType(ARRAY) = result;
            }
        }
        break;


case ARRAYLENGTH:       /*  0xBE */
        /*  Get length of array */
        {
            ARRAY thisArray = topStackAsType(ARRAY);
            checkNotNull(thisArray)
            topStack = thisArray->length;
        }
        break;


case ATHROW:            /*  0xBF */
        /*  Throw exception or error */
        {
            /*  Get the exception object from the stack */
            INSTANCE exception = popStackAsType(INSTANCE);
            checkNotNull(exception)

            throwException(exception);
        }
        break;


case CHECKCAST:         /*  0xC0 */
        /*  Check whether object is of given type */
        {
            /*  Get the CONSTANT_Class index */
            unsigned int cpIndex = getUShort(ip);

            /*  Get the corresponding class pointer */
            CLASS thisClass = resolveClassReference(cp,
						    cpIndex,
						    getCurrentClass());

            /* Get the object */
            OBJECT object = topStackAsType(OBJECT);
            ip += 2;

            if (ENABLEFASTBYTECODES) {
                int iCacheIndex =
                    createInlineCacheEntry((cell*)thisClass, ip - 3);
                ip[-3] = CHECKCAST_FAST;
                putShort(ip - 2, iCacheIndex);
            }

            if ((object != NULL) && !isInstanceOf(object, thisClass)) {
                raiseException(ClassCastException);
            }


        }
        break;


case INSTANCEOF:        /*  0xC1 */
        /*  Determine if object is of given type */
        {
            /*  Get the CONSTANT_Class index */
            unsigned int cpIndex = getUShort(ip);

            /*  Get the corresponding class pointer */
            CLASS thisClass = resolveClassReference(cp,
						    cpIndex,
						    getCurrentClass());
            OBJECT object = topStackAsType(OBJECT);
            ip += 2;

            if (ENABLEFASTBYTECODES) {
                int iCacheIndex =
                    createInlineCacheEntry((cell*)thisClass, ip - 3);
                ip[-3] = INSTANCEOF_FAST;
                putShort(ip - 2, iCacheIndex);
            }

            topStack = (object != NULL) && isInstanceOf(object, thisClass);
        }
        break;


case MONITORENTER:      /*  0xC2 */
        /*  Enter a monitor */
        {
            /*  Get the object pointer from stack */
            OBJECT object = popStackAsType(OBJECT);
            checkNotNull(object)
            monitorEnter(object);
        }
        break;


case MONITOREXIT:       /*  0xC3 */
        /*  Exit from a monitor */
        {
            /*  Get the object pointer from stack */
            OBJECT object = popStackAsType(OBJECT);
            checkNotNull(object)
            monitorExit(object);
        }
        break;


case WIDE:              /*  0xC4 */
        /*  Extend local variable index by additional bytes */
        {
        unsigned int index;

        token = *ip++;
        index = getUShort(ip); ip += 2;
        switch (token) {
            case ILOAD: case FLOAD: case ALOAD:
                pushStack(lp[index]);
                break;
            case LLOAD: case DLOAD:
                pushStack(lp[index]);
                pushStack(lp[index+1]);
                break;
            case ISTORE: case FSTORE: case ASTORE:
                VALIDATE_WRITE_ADDRESS(&lp[index]);
                lp[index] = popStack();
                break;
            case LSTORE: case DSTORE:
                VALIDATE_WRITE_ADDRESS(&lp[index]);
                lp[index+1] = popStack();
                lp[index]   = popStack();
                break;
            case RET:
                ip = *(BYTE**)&lp[index];
                break;
            case IINC:
                {
                    int value = (int)getShort(ip); ip += 2;
                    VALIDATE_WRITE_ADDRESS(&lp[index]);
                    lp[index] = ((long)(lp[index]) + value);
                }
                break;
            default:
                /* raiseException(VerifyError); */
                fatalError("WIDE: Illegal bytecode extension");

                break;
        }
        }
        break;


case MULTIANEWARRAY:    /*  0xC5 */
        /*  Create new multidimensional array */
        {
            unsigned int cpIndex;
            int dimensions;
            ARRAY_CLASS thisClass;
            ARRAY result;

            /*  Get the CONSTANT_Class index */
            cpIndex = getUShort(ip);
            ip += 2;

            /*  Get the requested array dimensionality */
            dimensions = *ip++;

            /*  Get the corresponding class pointer */
            thisClass = (ARRAY_CLASS)cp->entries[cpIndex].clazz;

            if (ENABLEFASTBYTECODES) {
                int iCacheIndex =
                    createInlineCacheEntry((cell*)thisClass, ip - 4);
                ip[-4] = MULTIANEWARRAY_FAST;
                putShort(ip - 3, iCacheIndex);
            }

            /*  Instantiate the array using a recursive algorithm */
            result = instantiateMultiArray(thisClass,
                                           (long*)(sp - dimensions + 1), dimensions);
            if (result != NULL) {
                sp -= dimensions;
                pushStackAsType(ARRAY, result);
            }

        }
        break;


case IFNULL:            /*  0xC6 */
        /*  Branch if reference is NULL */
        if (popStack())
             ip += 2;                /*  Continue execution */
        else ip += getShort(ip) - 1; /*  Branch */
        break;


case IFNONNULL:         /*  0xC7 */
        /*  Branch if reference is not NULL */
        if (popStack())
             ip += getShort(ip) - 1; /*  Branch */
        else ip += 2;                /*  Continue execution */
        break;


case GOTO_W:            /*  0xC8 */
        /*  Branch unconditionally (wide index) */
        ip += getCell(ip) - 1;
        break;


case JSR_W:             /*  0xC9 */
        /*  Jump subroutine (wide index) */
        pushStackAsType(BYTE*, (ip+4));
        ip += getCell(ip) - 1;
        break;


case BREAKPOINT:        /*  0xCA */
        fprintf(stdout,"Breakpoint\n");
        break;

/*=========================================================================
 * End of Java bytecodes; bytecodes below are used for internal optimization
 *=======================================================================*/

/*=========================================================================
 * If FAST bytecodes are enabled, the system replaces official Java
 * bytecodes with faster bytecodes at runtime whenever applicable.
 *
 * Note: most of the FAST bytecodes are applicable only when accessing
 * methods and variables of the currently executing class, i.e., they
 * cannot be used for accessing fields and methods of other classes.
 *=======================================================================*/

#if ENABLEFASTBYTECODES


case GETFIELD_FAST:     /*  0xCE */
        /*  Get single-word field (instance variable) from object (fast version) */
        {
            /*  Get the field index (resolved earlier) */
            /*  relative to the data area of instance */
            unsigned int index;
            INSTANCE instance;

            /*  Get the inline cache index */
            index = getUShort(ip);
            ip += 2;

            /*  Load one word onto the operand stack */
            instance = popStackAsType(INSTANCE);
            checkNotNull(instance);
            pushStack(instance->data[index].cell);
        }
        break;


case PUTFIELD_FAST:     /*  0xCF */
        /*  Set single-word field (fast version) */
        {
            /*  Get the field index (resolved earlier) */
            /*  relative to the data area of instance */
            unsigned int index;
            cell data;
            INSTANCE instance;

            index = getUShort(ip);
            ip += 2;

            /*  Store one word from the operand stack */
            data     = popStack();
            instance = popStackAsType(INSTANCE);
            checkNotNull(instance);
            VALIDATE_WRITE_ADDRESS(&instance->data[index].cell);
            instance->data[index].cell = data;
        }
        break;


case GETFIELD2_FAST:    /*  0xD0 */
        /*  Get double-word field (instance variable) from object (fast version) */
        {
            /*  Get the field index (resolved earlier) */
            /*  relative to the data area of instance */
            unsigned int index;
            INSTANCE instance;

            /*  Get the inline cache index */
            index = getUShort(ip);
            ip += 2;

            /*  Load two words onto the operand stack */
            instance = popStackAsType(INSTANCE);
            checkNotNull(instance);
            pushStack(instance->data[index].cell);
            pushStack(instance->data[index + 1].cell);
        }
        break;


case PUTFIELD2_FAST:    /*  0xD1 */
        /*  Set double-word field (fast version) */
        {
            /*  Get the field index (resolved earlier) */
            /*  relative to the data area of instance */
            unsigned int index;
            cell first;
            cell second;
            INSTANCE instance;

            /*  Get the inline cache index */
            index = getUShort(ip);
            ip += 2;

            /*  Store two words from the operand stack */
            first    = popStack();
            second   = popStack();
            instance = popStackAsType(INSTANCE);
            checkNotNull(instance);
            VALIDATE_WRITE_ADDRESS(&instance->data[index].cell);
            instance->data[index].cell = second;
            instance->data[index + 1].cell = first;
        }
        break;


case GETSTATIC_FAST:    /*  0xD2 */
        /*  Get single-word static field from class (fast version) */
        {
            unsigned int iCacheIndex;
            ICACHE iCache;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  The address of the field is stored in the inline cache */
            /*  Push contents of the field onto the operand stack */
            iCache = getInlineCache(iCacheIndex);
            pushStack(*iCache->contents);
        }
        break;


case PUTSTATIC_FAST:    /*  0xD3 */
        /*  Set single-word static field in class (fast version) */
        {
            unsigned int iCacheIndex;
            ICACHE iCache;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  The address of the field is stored in the inline cache */
            /*  Pop data from the operand stack into the field */
            iCache = getInlineCache(iCacheIndex);
            *iCache->contents = popStack();
        }
        break;


case GETSTATIC2_FAST:   /*  0xD4 */
        /*  Get double-word static field from class (fast version) */
        {
            unsigned int iCacheIndex;
            ICACHE iCache;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  The address of the field is stored in the inline cache */
            /*  Push the contents of the field onto the operand stack */
            iCache = getInlineCache(iCacheIndex);
            oneMore;
            COPY_LONG(sp, iCache->contents);
            oneMore;
        }
        break;


case PUTSTATIC2_FAST:   /*  0xD5 */
        /*  Set double-word static field in class (fast version) */
        {
            unsigned int iCacheIndex;
            ICACHE iCache;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  The address of the field is stored in the inline cache */
            /*  Pop data from the operand stack into the field */
            iCache = getInlineCache(iCacheIndex);
            oneLess;
            COPY_LONG(iCache->contents, sp);
            oneLess;
        }
        break;


case INVOKEVIRTUAL_FAST: /*  0xD6 */
        /*  Invoke instance method; dispatch based on dynamic class (fast version) */
        {
            /*  Get the inline cache index */
            unsigned int iCacheIndex;
            ICACHE   thisICache;
            INSTANCE_CLASS defaultClass;
            int      argCount;
            CLASS    dynamicClass;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  Get the inline cache entry */
            thisICache = getInlineCache(iCacheIndex);

            /*  Get the default method stored in cache */
            thisMethod = (METHOD)thisICache->contents;

            /*  Get the class of the default method */
            defaultClass = thisMethod->ofClass;

            /*  Get the object pointer ('this') from the operand stack */
            /*  (located below the method arguments in the stack) */
            argCount = thisMethod->argCount;
            thisObject = *(OBJECT*)(sp-argCount+1);
            checkNotNull(thisObject);

            /*  This may be different than the default class */
            dynamicClass = thisObject->ofClass;

            /* If the default class and dynamic class are the same, we can
             * just execute the method.  Otherwise a new lookup
             */
            if (dynamicClass != (CLASS)defaultClass) {
                thisMethod = lookupMethod(dynamicClass,
                                          thisMethod->nameTypeKey);
                /*  Update inline cache entry with the newly found method */
                thisICache->contents = (cell*)thisMethod;

#if ENABLEPROFILING
                InlineCacheMissCounter++;
#endif

                if (TRACEMETHODCALLS & 1) {
                    START_TEMPORARY_ROOTS
                        fprintf(stdout,
                                "Inline cache miss (%s::%s%s; default: %s)\n",
                                className((INSTANCE_CLASS)dynamicClass),
                                methodName(thisMethod),
                                methodSignature(thisMethod),
                                className(defaultClass));
                    END_TEMPORARY_ROOTS
                }
            }
#if ENABLEPROFILING
            else InlineCacheHitCounter++;
#endif

            if (thisMethod) {
                TraceMethodEntry(thisMethod, "fast virtual");

                /*  Check if the method is a native method */
		goto callMethod;
            } else {
                thisMethod = (METHOD)thisICache->contents;
                START_TEMPORARY_ROOTS
                    sprintf(str_buffer, "No such method %s.%s",
                            methodName(thisMethod),
                            methodSignature(thisMethod));
                    fatalError(str_buffer);
                END_TEMPORARY_ROOTS
            }
        }
        break;

case INVOKESPECIAL_FAST: /*  0xD7 */
        /*  Invoke instance method; special handling for superclass, */
        /*  private and instance initialization method invocations */
        /*  (fast version) */
        {
            /*  Get the inline cache index */
            unsigned int iCacheIndex;
            ICACHE thisICache;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  Get the inline cache entry */
            thisICache = getInlineCache(iCacheIndex);

            /*  Get the default method stored in cache */
            thisMethod = (METHOD)thisICache->contents;
	    thisObject = *(OBJECT*)(sp-thisMethod->argCount+1);
	    checkNotNull(thisObject)
            /*  Get the class of the default method */

            TraceMethodEntry(thisMethod, "fast special");

	    goto callMethod;
        }
        break;


case INVOKESTATIC_FAST: /*  0xD8 */
        /*  Invoke a class (static) method (fast version) */
        {
            /*  Get the inline cache index */
            unsigned int iCacheIndex;
            ICACHE thisICache;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  Get the inline cache entry */
            thisICache = getInlineCache(iCacheIndex);

            /*  Get the default method stored in cache */
            thisMethod = (METHOD)thisICache->contents;

            /*  Get the class of the currently executing method */
	    thisObject = (OBJECT)thisMethod->ofClass;

            TraceMethodEntry(thisMethod, "fast static");
	    goto callMethod;
        }
        break;


case INVOKEINTERFACE_FAST: /*  0xD9 */
        /*  Invoke interface method */
        {
            /*  Get the inline cache index */
            unsigned int iCacheIndex;
            unsigned int   argCount;
            ICACHE   thisICache;
            INSTANCE_CLASS defaultClass;
            CLASS    dynamicClass;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  Get the argument count (specific to INVOKEINTERFACE bytecode) */
            argCount = *ip;
            ip += 2;

            /*  Get the inline cache entry */
            thisICache = getInlineCache(iCacheIndex);

            /*  Get the default method stored in cache */
            thisMethod = (METHOD)thisICache->contents;

            /*  Get the class of the default method */
            defaultClass = thisMethod->ofClass;

            /*  Get the object pointer ('this') from the operand stack */
            thisObject = *(OBJECT*)(sp-argCount+1);
            checkNotNull(thisObject);

            /*  Get the runtime (dynamic) class of the object */
            dynamicClass = thisObject->ofClass;

            /*  If the default class and dynamic class are the same, done */
            if (dynamicClass != (CLASS)defaultClass) {
                /*  Get method table entry based on dynamic class */
                thisMethod = lookupMethod(dynamicClass, thisMethod->nameTypeKey);
                /*  Update inline cache entry with the newly found method */
                thisICache->contents = (cell*)thisMethod;

#if ENABLEPROFILING
                InlineCacheMissCounter++;
#endif

                if (TRACEMETHODCALLS & 1) {
                    START_TEMPORARY_ROOTS
                        fprintf(stdout,
                                "Inline cache miss (%s::%s%s; default: %s)\n",
                                className((INSTANCE_CLASS)dynamicClass),
                                methodName(thisMethod),
                                methodSignature(thisMethod),
                                className((INSTANCE_CLASS)defaultClass));
                    END_TEMPORARY_ROOTS
                }
            }
#if ENABLEPROFILING
            else InlineCacheHitCounter++;
#endif

            if (thisMethod) {
                TraceMethodEntry(thisMethod, "fast interface");
		goto callMethod;
            } else {
                thisMethod = (METHOD)thisICache->contents;
                START_TEMPORARY_ROOTS
                    sprintf(str_buffer, "No such method %s.%s",
                            methodName(thisMethod),
                            methodSignature(thisMethod));
                    fatalError(str_buffer);
                END_TEMPORARY_ROOTS
            }
        }
        break;

case NEW_FAST:          /*  0xDA */
        /*  Create new object (fast version) */
        {
            /*  Get the inline cache index */
            unsigned int iCacheIndex;
            ICACHE   thisICache;
            INSTANCE_CLASS    thisClass;
            INSTANCE newObject;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  Get the inline cache entry */
            thisICache = getInlineCache(iCacheIndex);

            /*  Get the class pointer stored in cache */
            thisClass = (INSTANCE_CLASS)thisICache->contents;

            /*  Instantiate */
            newObject = instantiate(thisClass);
            if (newObject != NULL) {
                pushStack((cell)newObject);
            }
        }
        break;


case ANEWARRAY_FAST:    /*  0xDB */
        /*  Create new array of reference type (fast version) */
        {
            /*  Get the inline cache index */
            unsigned int iCacheIndex = getUShort(ip);

            /*  Get the inline cache entry */
            ICACHE thisICache = getInlineCache(iCacheIndex);
            /*  Get the class pointer stored in cache */
            ARRAY_CLASS thisClass = (ARRAY_CLASS)thisICache->contents;
            long arrayLength = topStack;
            ARRAY result = instantiateArray(thisClass, arrayLength);
            if (result != NULL) {
                ip += 2;
                topStack = (cell)result;
            } else {
                /* Stack and ip have been set by error handler */
            }
        }
        break;


case MULTIANEWARRAY_FAST: /*  0xDC */
        /*  Create new multidimensional array (fast version) */
        {
            /*  Get the inline cache index */
            unsigned int iCacheIndex;
            int dimensions;
            ICACHE thisICache;
            ARRAY_CLASS thisClass;
            ARRAY result;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  Get the requested array dimensionality */
            dimensions = *ip++;

            /*  Get the inline cache entry */
            thisICache = getInlineCache(iCacheIndex);

            /*  Get the class pointer stored in cache */
            thisClass = (ARRAY_CLASS)thisICache->contents;


            /*  Instantiate the array using a recursive algorithm */
            result = instantiateMultiArray(thisClass,
                                           (long *)(sp - dimensions + 1), dimensions);
            if (result != NULL) {
                sp -= dimensions;
                pushStack((cell)result);
            }
        }
        break;


case CHECKCAST_FAST:    /*  0xDD */
        /*  Check whether object is of given type (fast version) */
        {
            /*  Get the inline cache index */
            unsigned int iCacheIndex;
            ICACHE   thisICache;
            CLASS    thisClass;
            OBJECT   object;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  Get the inline cache entry */
            thisICache = getInlineCache(iCacheIndex);

            /*  Get the class pointer stored in cache */
            thisClass = (CLASS)thisICache->contents;

            /*  If object reference is NULL, or if the object */
            /*  is a proper subclass of the cpIndexed class,  */
            /*  the operand stack remains unchanged; otherwise */
            /*  throw exception */
            object = (OBJECT)topStack;
            if ((object != NULL) && !isInstanceOf(object, thisClass)) {
                raiseException(ClassCastException);
            }
        }
        break;


case INSTANCEOF_FAST:   /*  0xDE */
        /*  Determine if object is of given type (fast version) */
        {
            /*  Get the inline cache index */
            unsigned int iCacheIndex;
            ICACHE   thisICache;
            CLASS    thisClass;
            OBJECT   object;

            /*  Get the inline cache index */
            iCacheIndex = getUShort(ip);
            ip += 2;

            /*  Get the inline cache entry */
            thisICache = getInlineCache(iCacheIndex);

            /*  Get the class pointer stored in cache */
            thisClass = (CLASS)thisICache->contents;
            object = (OBJECT)topStack;
            topStack = (object != NULL && isInstanceOf(object, thisClass));
        }
        break;

#endif /*  ENABLEFASTBYTECODES */

/*=========================================================================
 * End of FAST bytecodes
 *=======================================================================*/

case CUSTOMCODE:
        {   /* 0xDF */
            cell *stack = (cell*)(fp + 1);
            CustomCodeCallbackFunction func =
                   *(CustomCodeCallbackFunction *)stack;
            ip--;
            func(stack + 1, NULL);
            break;
        }

default:
    {
        char safe_buffer[30];
        sprintf(safe_buffer, "Illegal bytecode %ld", (long)token);

        fatalError(safe_buffer);
    }


  callMethod:
	/*  Check if the method is a native method */
	if (thisMethod->accessFlags & ACC_NATIVE) {
	    invokeNativeFunction(thisMethod);
	    TraceMethodExit(thisMethod);
	} else if (thisMethod->accessFlags & ACC_ABSTRACT) {
	    fatalError("abstract method invoked");
	} else {
	    /*  Create an execution frame for executing a Java method */
	    if (pushFrame(thisMethod)) { 
		if (thisMethod->accessFlags & ACC_SYNCHRONIZED) {
		    monitorEnter(thisObject);
		    fp->syncObject = thisObject;
		}
	    }
	}
	break;
}
}
}
