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
 * SUBSYSTEM: Internal runtime structures
 * FILE:      verifier.c
 * OVERVIEW:  byte code verifier
 * AUTHOR:    Sheng Liang
 *=======================================================================*/

/*=========================================================================
 * Include files
 *=======================================================================*/

#include <global.h>
#include <stddef.h>     

/*=========================================================================
 * Global variables and definitions.
 *=======================================================================*/

#if TRACEVERIFIER
/* For debugging only. */
extern const char* const byteCodeNames[];
/* Forward declaration */
static void printStackMap(METHOD thisMethod, unsigned short ip);
#endif

/* Used to encode the type of 2-byte entities: long and double */
struct DoubleWordItem {
    unsigned short fst;
    unsigned short snd;
};

/* Global variables holding the type key of well-known classes */
unsigned short booleanArrayClassKey;
unsigned short byteArrayClassKey;
unsigned short charArrayClassKey;
unsigned short shortArrayClassKey;
unsigned short intArrayClassKey;
unsigned short longArrayClassKey;

#if IMPLEMENTS_FLOAT
unsigned short floatArrayClassKey;
unsigned short doubleArrayClassKey;
#endif

unsigned short objectClassKey;
unsigned short stringClassKey;
unsigned short throwableClassKey;
unsigned short objectArrayClassKey;

/* Global variables used by the verifier. Obviously the verifier is
 * not re-entrant.
 */
unsigned short* vStack;
unsigned short* vLocals;
unsigned short vMaxStack;
unsigned short vFrameSize;
unsigned short vSP;
unsigned short lastStackPop;
struct DoubleWordItem lastStackPop2;
unsigned short lastLocalGet;

/* Flags used to control how to match two stack maps.
 * One of the stack maps is derived as part of the type checking process.
 * The other stack map is recorded in the class file.
 */
#define SM_CHECK 1           /* Check if derived types match recorded types. */
#define SM_MERGE 2           /* Merge recorded types into derived types. */
#define SM_EXIST 4           /* A recorded type attribute must exist. */

/* Error codes 
 * We use error code instead of text messages to reduce memory usage.
 */

#define VE_STACK_OVERFLOW          1
#define VE_STACK_UNDERFLOW         2
#define VE_STACK_EXPECT_CAT1       3
#define VE_STACK_BAD_TYPE          4
#define VE_LOCALS_OVERFLOW         5
#define VE_LOCALS_BAD_TYPE         6
#define VE_LOCALS_UNDERFLOW        7
#define VE_TARGET_BAD_TYPE         8
#define VE_BACK_BRANCH_UNINIT      9
#define VE_SEQ_BAD_TYPE           10
#define VE_EXPECT_CLASS           11
#define VE_EXPECT_THROWABLE       12
#define VE_BAD_LOOKUPSWITCH       13
#define VE_BAD_LDC                14
#define VE_BALOAD_BAD_TYPE        15
#define VE_AALOAD_BAD_TYPE        16
#define VE_BASTORE_BAD_TYPE       17
#define VE_EXPECT_FIELDREF        18
#define VE_FIELD_BAD_TYPE         19
#define VE_EXPECT_METHODREF       20
#define VE_ARGS_NOT_ENOUGH        21
#define VE_ARGS_BAD_TYPE          22
#define VE_EXPECT_INVOKESPECIAL   23
#define VE_EXPECT_NEW             24
#define VE_EXPECT_UNINIT          25
#define VE_BAD_INSTR              26
#define VE_EXPECT_ARRAY           27
#define VE_MULTIANEWARRAY         28
#define VE_EXPECT_NO_RETVAL       29
#define VE_RETVAL_BAD_TYPE        30
#define VE_EXPECT_RETVAL          31
#define VE_RETURN_UNINIT_THIS     32
#define VE_BAD_STACKMAP           33
#define VE_FALL_THROUGH           34
#define VE_EXPECT_ZERO            35
#define VE_NARGS_MISMATCH         36
#define VE_INVOKESPECIAL          37

#define CONSTANTPOOL_MASKED_TAG(cp, index) \
    (CONSTANTPOOL_TAG(cp, index) & CP_CACHEMASK)

#define GET_CLASS_CONSTANT_KEY(cp, index) (cp->entries[index].clazz->key)

/* Used to check stackmaps for exception handlers. */
#define CHECK_TARGET(this_ip, target_ip) \
    if (!matchStackMap(thisMethod, (this_ip), (unsigned short)(target_ip), SM_CHECK | SM_EXIST)) { \
        VERIFIER_ERROR(VE_TARGET_BAD_TYPE); \
    } else

/* Used to check stackmaps for jump instructions. Make sure that there are
 * no uninitialized instances on backward branch.
 */
#define CHECK_JUMP_TARGET(this_ip, target_ip) \
    if (TRACEVERIFIER) \
        fprintf(stdout, "Check jump target at %ld\n", (long)target_ip); \
    CHECK_TARGET(this_ip, target_ip); \
    if (!checkNewObject(this_ip, (unsigned short)(target_ip))) { \
        VERIFIER_ERROR(VE_BACK_BRANCH_UNINIT); \
    } else

/* Use goto statement to handle error to eliminate code duplication. */
#define BAIL_OUT() { goto err;}

/* Set result code and bail out. */
#define VERIFIER_ERROR(n) \
    result = n; \
    BAIL_OUT();

/* The following macros call a function and then check error code. We factor out
 * code in to functions to minimize code size.
 */
#define PUSH_STACK(t) \
    result = vPushStack(t); \
    if (result) BAIL_OUT();

#define POP_STACK(t) \
    result = vPopStack(t); \
    if (result) BAIL_OUT();

#define GET_LOCAL(i, v) \
    result = vGetLocal(i, v); \
    if (result) BAIL_OUT();

#define SET_LOCAL(i, v) \
    result = vSetLocal(i, v); \
    if (result) BAIL_OUT();

#define CHECK_VALID_CP_INDEX(cp, i) \
    if (i >= CONSTANTPOOL_LENGTH(cp)) { \
        result = 120; \
        BAIL_OUT(); \
    } else

/*=========================================================================
 * FUNCTION:      initializeVerifier
 * TYPE:          public global operation
 * OVERVIEW:      Initialize the bytecode verifier
 *
 * INTERFACE:
 *   parameters:  <nothing>
 *   returns:     <nothing>
 *=======================================================================*/

void
InitializeVerifier(void)
{
    booleanArrayClassKey    = change_FieldSignature_to_Key("[Z", 2);
    byteArrayClassKey       = change_FieldSignature_to_Key("[B", 2);
    charArrayClassKey       = change_FieldSignature_to_Key("[C", 2);
    shortArrayClassKey      = change_FieldSignature_to_Key("[S", 2);
    intArrayClassKey        = change_FieldSignature_to_Key("[I", 2);
    longArrayClassKey       = change_FieldSignature_to_Key("[J", 2);

#if IMPLEMENTS_FLOAT
    floatArrayClassKey      = change_FieldSignature_to_Key("[F", 2);
    doubleArrayClassKey     = change_FieldSignature_to_Key("[D", 2);
#endif
    
    /* Note that java.lang.Object and java.lang.String classes have been
     * created, but Throwable and Object[] classes have not.
     */
    objectClassKey      = JavaLangObject->class.key;
    stringClassKey      = JavaLangString->class.key;
    /* java.lang.Throwable class is loaded lazily. */
    throwableClassKey   = change_FieldSignature_to_Key("Ljava/lang/Throwable;", 21);
    /* Object[] class is not yet loaded. */
    objectArrayClassKey = change_FieldSignature_to_Key("[Ljava/lang/Object;", 19);
}

/*=========================================================================
 * FUNCTION:      vIsAssignable
 * TYPE:          private operation on type keys
 * OVERVIEW:      Check if a value of one type key can be converted to a value of
 *                another type key.
 *
 * INTERFACE:
 *   parameters:  fromKey: from type
 *                toKey: to type
 *                mergedKeyP: pointer to merged type
 *   returns:     TRUE: can convert.
 *                FALSE: cannot convert.
 *=======================================================================*/

static bool_t
vIsAssignable(unsigned short fromKey, unsigned short toKey, unsigned short *mergedKeyP)
{
    if (mergedKeyP) {
        /* Most of the time merged type is toKey */
        *mergedKeyP = toKey;
    }
    if (fromKey == toKey)     /* trivial case */
        return TRUE;
    if (toKey == ITEM_Bogus)
        return TRUE;
    if (toKey == ITEM_Reference) {
        return fromKey == ITEM_Null ||
            fromKey > 255 ||
            fromKey == ITEM_InitObject ||
            (fromKey & ITEM_NewObject_Flag);
    }
    if ((toKey & ITEM_NewObject_Flag) || (fromKey & ITEM_NewObject_Flag)) {
        return FALSE;
    }
    if (fromKey == ITEM_Null && toKey > 255) {
        return TRUE;
    }
    if (fromKey > 255 && toKey > 255) {
        CLASS fromClass = change_Key_to_CLASS(fromKey);
        CLASS toClass = change_Key_to_CLASS(toKey);
        bool_t res = isAssignableTo(fromClass, toClass);
        /* Interfaces are treated like java.lang.Object in the verifier. */
        if (toClass->accessFlags & ACC_INTERFACE) {
            /* Set mergedKey to fromKey for interfaces */
            if (mergedKeyP) {
                *mergedKeyP = objectClassKey;
            }
            return TRUE;
        }
        return res;
    }
    return FALSE;
}

/*=========================================================================
 * FUNCTION:      vPushStack
 * TYPE:          private operation on the virtual stack
 * OVERVIEW:      Push a type key onto the virtual stack maintained by
 *                the verifier. Performs stack overflow check.
 *
 * INTERFACE:
 *   parameters:  typeKey: the type to be pushed.
 *   returns:     0 on success, or an error code.
 *=======================================================================*/

static int
vPushStack(unsigned short typeKey)
{
    if (vSP >= vMaxStack) {
        return VE_STACK_OVERFLOW;
    }
    vStack[vSP++] = typeKey;
    return 0;
}

/*=========================================================================
 * FUNCTION:      vPopStack
 * TYPE:          private operation on the virtual stack
 * OVERVIEW:      Pop a type key from the virtual stack maintained by
 *                the verifier. Performs stack overflow check and type
 *                check.
 *
 * INTERFACE:
 *   parameters:  typeKey: the type expected to be poped.
 *   returns:     0 on success, or an error code.
 *=======================================================================*/

static int
vPopStack(unsigned short typeKey)
{
    if (typeKey == ITEM_DoubleWord || typeKey == ITEM_Category2) {
        if (vSP <= 1) {
            return VE_STACK_UNDERFLOW;
        }
        lastStackPop2.snd = vStack[vSP - 1];
        lastStackPop2.fst = vStack[vSP - 2];
        vSP -= 2;
        return 0;
    }
    if (vSP == 0) { /* vSP is unsigned, See bug 4323211 */
        return VE_STACK_UNDERFLOW;
    }
    lastStackPop = vStack[vSP - 1];
    vSP--;
    if (typeKey == ITEM_Category1) {
        if (lastStackPop == ITEM_Integer ||
#if IMPLEMENTS_FLOAT
            lastStackPop == ITEM_Float ||
#endif
            lastStackPop == ITEM_Null ||
            lastStackPop > 255 ||
            lastStackPop == ITEM_InitObject ||
            (lastStackPop & ITEM_NewObject_Flag)) {
            return 0;
        } else {
            return VE_STACK_EXPECT_CAT1;
        }
    }
    if (!vIsAssignable(lastStackPop, typeKey, NULL)) {
        return VE_STACK_BAD_TYPE;
    }
    return 0;
}

/*=========================================================================
 * FUNCTION:      vGetLocal
 * TYPE:          private operation on the virtual local frame
 * OVERVIEW:      Get a type key from the virtual local frame maintained by
 *                the verifier. Performs stack overflow check and type
 *                check.
 *
 * INTERFACE:
 *   parameters:  index: local index.
 *                typeKey: the type expected.
 *   returns:     0 on success, or an error code.
 *=======================================================================*/

static int
vGetLocal(unsigned int index, unsigned short typeKey)
{
    if (index >= vFrameSize) {
        return VE_LOCALS_OVERFLOW;
    }
    lastLocalGet = vLocals[index];
    if (!vIsAssignable(lastLocalGet, typeKey, NULL)) {
        return VE_LOCALS_BAD_TYPE;
    }
    return 0;
}

/*=========================================================================
 * FUNCTION:      vSetLocal
 * TYPE:          private operation on the virtual local frame
 * OVERVIEW:      Set a type key in the virtual local frame maintained by
 *                the verifier. Performs stack overflow check.
 *
 * INTERFACE:
 *   parameters:  index: local index.
 *                typeKey: the supplied type.
 *   returns:     0 on success, or an error code.
 *=======================================================================*/

static int
vSetLocal(unsigned int index, unsigned short typeKey)
{
    if (index >= vFrameSize) {
        return VE_LOCALS_OVERFLOW;
    }
    if (vLocals[index] == ITEM_Long_2
#if IMPLEMENTS_FLOAT
        || vLocals[index] == ITEM_Double_2
#endif
        ) {
        if (index < 1) {
            return VE_LOCALS_UNDERFLOW;
        }
        vLocals[index - 1] = ITEM_Bogus;
    }
    if (vLocals[index] == ITEM_Long
#if IMPLEMENTS_FLOAT
        || vLocals[index] == ITEM_Double
#endif
        ) {
        if (index >= (unsigned int)vFrameSize - 1) {
            return VE_LOCALS_OVERFLOW;
        }
        vLocals[index + 1] = ITEM_Bogus;
    }
    vLocals[index] = typeKey;
    return 0;
}

/*=========================================================================
 * FUNCTION:      getObjectArrayElementKey
 * TYPE:          private operation on type keys
 * OVERVIEW:      Obtain the element type key of a given object array type.
 *
 * INTERFACE:
 *   parameters:  typeKey: object array type key.
 *   returns:     element type key.
 *
 * DANGER:
 *   Before calling this function, make sure that typeKey in fact is an
 *   object array.
 *=======================================================================*/

static unsigned short
getObjectArrayElementKey(unsigned short typeKey)
{
    int n = (typeKey >> FIELD_KEY_ARRAY_SHIFT);
    if (n < MAX_FIELD_KEY_ARRAY_DEPTH) {
        return ((n - 1) << FIELD_KEY_ARRAY_SHIFT) + (typeKey & 0x1FFF);
    } else {
        CLASS arrayClass = change_Key_to_CLASS(typeKey);
        UString baseName = getUStringX(arrayClass->baseName->string + 1,
                                       arrayClass->baseName->length - 1);
        UString packageName = arrayClass->packageName;
        GCT_ObjectType objectType = 
            baseName->string[0] == '[' ? GCT_ARRAY_CLASS : GCT_INSTANCE_CLASS;
        CLASS elemClass = change_Name_to_CLASS(baseName, packageName, objectType);
        return elemClass->key;
    }
}

/*=========================================================================
 * FUNCTION:      makeObjectArrayElementKey
 * TYPE:          private operation on type keys
 * OVERVIEW:      Obtain the object array type key whose element type is the
 *                given element type key.
 *
 * INTERFACE:
 *   parameters:  typeKey: element type key.
 *   returns:     object array type key.
 *
 * DANGER:
 *   Before calling this function, make sure that typeKey in fact is an
 *   object type.
 *=======================================================================*/

static unsigned short
makeObjectArrayClassKey(unsigned short typeKey)
{
    int n = (typeKey >> FIELD_KEY_ARRAY_SHIFT);
    if (n < MAX_FIELD_KEY_ARRAY_DEPTH - 1) {
        return (1 << FIELD_KEY_ARRAY_SHIFT) + typeKey;
    } else {
        CLASS elemClass = change_Key_to_CLASS(typeKey);
        ARRAY_CLASS arrayClass = getObjectArrayClass(elemClass);
        return arrayClass->class.key;
    }
}

/*=========================================================================
 * FUNCTION:      isArrayClassKey
 * TYPE:          private operation on type keys
 * OVERVIEW:      Check if the given type key denotes an array type of the
 *                given dimension.
 *
 * INTERFACE:
 *   parameters:  typeKey: a type key.
 *                dim: dimension.
 *   returns:     TRUE if the type key is an array of dim dimension, FALSE
 *                otherwise.
 *=======================================================================*/

static bool_t
isArrayClassKey(unsigned short typeKey, int dim)
{
    int n;
    if (typeKey == ITEM_Null) {
        return TRUE;
    }
    n = (typeKey >> FIELD_KEY_ARRAY_SHIFT);
    if (n == 0) {
        return FALSE;
    }
    if (n >= dim) {
        return TRUE;
    } else {
        CLASS arrayClass = change_Key_to_CLASS(typeKey);
        char *baseName = arrayClass->baseName->string;
        while (*baseName++ == '[') {
            dim--;
        }
        return dim >= 0;
    }
}

/*=========================================================================
 * FUNCTION:      change_Field_to_StackType
 * TYPE:          private operation on type keys
 * OVERVIEW:      Change an individual field type to a stack type
 *
 * INTERFACE:
 *   parameters:  fieldType: field type
 *                stackTypeP: pointer for placing the corresponding stack type(s)
 *   returns:     number of words occupied by the resulting type.
 *=======================================================================*/

static int
change_Field_to_StackType(unsigned short fieldType, unsigned short* stackTypeP)
{
    switch (fieldType) {
        case 'I':
        case 'B':
        case 'Z':
        case 'C':
        case 'S':
            *stackTypeP++ = ITEM_Integer;
            return 1;
#if IMPLEMENTS_FLOAT
        case 'F':
            *stackTypeP++ = ITEM_Float;
            return 1;
        case 'D':
            *stackTypeP++ = ITEM_Double;
            *stackTypeP++ = ITEM_Double_2;
            return 2;
#endif
        case 'J':
            *stackTypeP++ = ITEM_Long;
            *stackTypeP++ = ITEM_Long_2;
            return 2;
        default:
            *stackTypeP++ = fieldType;
            return 1;
    }
}

/*=========================================================================
 * FUNCTION:      change_Arg_to_StackType
 * TYPE:          private operation on type keys
 * OVERVIEW:      Change an individual method argument type to a stack type
 *
 * INTERFACE:
 *   parameters:  sigP: pointer to method signature.
 *                type: pointer for placing the corresponding stack type(s)
 *   returns:     number of words occupied by the resulting type.
 *=======================================================================*/

static int
change_Arg_to_StackType(unsigned char** sigP, unsigned short* typeP)
{
    unsigned char *sig = *sigP;
    unsigned short hi, lo;

    hi = *sig++;

    if (hi == 'L') {
        hi = *sig++;
        lo = *sig++;
        *sigP = sig;
        return change_Field_to_StackType((unsigned short)((hi << 8) + lo), typeP);
    }
    if (hi < 'A' || hi > 'Z') {
        lo = *sig++;
        *sigP = sig;
        return change_Field_to_StackType((unsigned short)((hi << 8) + lo), typeP);
    }
    *sigP = sig;
    return change_Field_to_StackType(hi, typeP);
}

/*=========================================================================
 * FUNCTION:      getStackType
 * TYPE:          private operation on type keys
 * OVERVIEW:      Get the recorded stack map of a given target ip.
 *
 * INTERFACE:
 *   parameters:  thisMethod: method being verified.
 *                this_ip: current ip (unused for now).
 *                target_ip: target ip (to look for a recored stack map).
 *   returns:     a stack map
 *=======================================================================*/

static unsigned short *
getStackMap(METHOD thisMethod, unsigned short this_ip, unsigned short target_ip)
{
    STACKMAP stackMaps = thisMethod->u.java.stackMaps;
    unsigned short i;
    if (stackMaps == NULL) {
        return NULL;
    }
    for (i = 0; i < stackMaps->nEntries; i++) {
        if (target_ip == stackMaps->entries[i].offset) {
            unsigned short stackMapKey = stackMaps->entries[i].stackMapKey;
            return (unsigned short *)change_Key_to_Name(stackMapKey, NULL);
        }
    }
    return NULL;
}

/*=========================================================================
 * FUNCTION:      matchStackMap
 * TYPE:          private operation on type keys
 * OVERVIEW:      Match two stack maps.
 *
 * INTERFACE:
 *   parameters:  thisMethod: method being verified.
 *                this_ip: current ip (unused for now).
 *                target_ip: target ip (to look for a recored stack map).
 *                flags: bit-wise or of the SM_* flags.
 *   returns:     TRUE if match, FALSE otherwise.
 *=======================================================================*/

static bool_t 
matchStackMap(METHOD thisMethod, unsigned short this_ip,
              unsigned short target_ip, int flags)
{
    unsigned short nstack;
    unsigned short nlocals;
    unsigned short i;
   
    unsigned short* stackMap = getStackMap(thisMethod, this_ip, target_ip);

#if TRACEVERIFIER
	fprintf(stdout, "Match stack maps (this=%d, target=%d)%s%s%s\n",
		this_ip, target_ip,
		(flags & SM_CHECK) ? " CHECK" : "",
		(flags & SM_MERGE) ? " MERGE" : "",
		(flags & SM_EXIST) ? " EXIST" : "");
#endif

    if (stackMap == NULL) {
#if TRACEVERIFIER
	fprintf(stdout, "No recorded stack map at %d\n", target_ip);
#endif
        return !(flags & SM_EXIST);
    }

#if TRACEVERIFIER
    printStackMap(thisMethod, target_ip);
#endif

    nlocals = *stackMap++;
    for (i = 0; i < nlocals; i++) {
        unsigned short ty = *stackMap++;
        unsigned short mergedType = ty;
        if ((SM_CHECK & flags) && !vIsAssignable(vLocals[i], ty, &mergedType)) {
            return FALSE;
        }
        if (SM_MERGE & flags) {
            vLocals[i] = mergedType;
        }
    }
    if (SM_MERGE & flags) {
	for (i = nlocals; i < vFrameSize; i++) {
	    vLocals[i] = ITEM_Bogus;
	}
    }

    nstack = *stackMap++;
    if ((SM_CHECK & flags) && nstack != vSP) {
        return FALSE;
    }
    if (SM_MERGE & flags) {
        vSP = nstack;
    }
    for (i = 0; i < nstack; i++) {
        unsigned short ty = *stackMap++;
        unsigned short mergedType = ty;
        if ((SM_CHECK & flags) && !vIsAssignable(vStack[i], ty, &mergedType)) {
            return FALSE;
        }
        if (SM_MERGE & flags) {
            vStack[i] = mergedType;
        }
    }
    return TRUE;
}

/*=========================================================================
 * FUNCTION:      checkNewObject
 * TYPE:          private operation on type keys
 * OVERVIEW:      Check if uninitialized objects exist on backward branches.
 *
 * INTERFACE:
 *   parameters:  this_ip: current ip
 *                target_ip: branch target ip
 *   returns:     TRUE if no uninitialized objects exist, FALSE otherwise.
 *=======================================================================*/

static bool_t 
checkNewObject(unsigned short this_ip, unsigned short target_ip)
{
    if (target_ip < this_ip) {
        int i;
        for (i = 0; i < vFrameSize; i++) {
            if (vLocals[i] & ITEM_NewObject_Flag) {
                return FALSE;
            }
        }
        for (i = 0; i < vMaxStack; i++) {
            if (vStack[i] & ITEM_NewObject_Flag) {
                return FALSE;
            }
        }
    }
    return TRUE;
}

/*=========================================================================
 * FUNCTION:      verifyMethod
 * TYPE:          private operation on methods.
 * OVERVIEW:      Perform byte-code verification of a given method.
 *
 * INTERFACE:
 *   parameters:  thisMethod: method to be verified.
 *   returns:     0 if verification succeeds, error code if verification
 *                fails.
 *=======================================================================*/

static int 
verifyMethod(METHOD thisMethod)
{
    int result = 0;

    START_TEMPORARY_ROOTS

    unsigned short ip = 0;     /* virtual ip */
    unsigned short stackMapIndex = 0; 
                               /* index into the StackMap table (corresponds
                                * to jump targets).
                                */

    unsigned char *code = thisMethod->u.java.code;
    unsigned short codeLength = thisMethod->u.java.codeLength;
    CONSTANTPOOL constPool = thisMethod->ofClass->constPool;

    unsigned char* returnSig;  /* holds the return signature */
    unsigned short index;
    unsigned short typeKey;
    unsigned char tag;
    bool_t noControlFlow;      /* set to TRUE where is no direct control
                                * flow from current instruction to the next
                                * instruction in sequence.
                                */

    vMaxStack = thisMethod->maxStack;
    vFrameSize = thisMethod->frameSize;
    vStack = (unsigned short*)
        callocObject(vMaxStack * sizeof(unsigned short), GCT_NOPOINTERS);
    MAKE_TEMPORARY_ROOT(vStack);
    vLocals = (unsigned short*)
        callocObject(vFrameSize * sizeof(unsigned short), GCT_NOPOINTERS);
    MAKE_TEMPORARY_ROOT(vLocals);

    vSP = 0;                   /* initial stack is empty. */

    if (TRACEVERIFIER) {
        fprintf(stdout, "Verifying method %s.", 
                getClassName(&(thisMethod->ofClass->class)));
        fprintf(stdout, "%s",
                change_Key_to_Name(thisMethod->nameTypeKey.nt.nameKey, NULL));
        fprintf(stdout, "%s\n",
                change_Key_to_MethodSignature(thisMethod->nameTypeKey.nt.typeKey));
    }

    {
        /* Fill in the initial derived stack map with argument types. */
        unsigned char *sig = (unsigned char*)
            change_Key_to_Name(thisMethod->nameTypeKey.nt.typeKey, NULL);
        int nargs = *sig++;
        int i;
        int n;

        n = 0;
        if (!(thisMethod->accessFlags & ACC_STATIC)) {
            /* add one extra argument for instance methods */
            n++;
            if (thisMethod->nameTypeKey.nt.nameKey == initNameAndType.nt.nameKey &&
                thisMethod->ofClass->class.key != objectClassKey) {
                vLocals[0] = ITEM_InitObject;
            } else {
                vLocals[0] = thisMethod->ofClass->class.key;
            }
        }
        for (i = 0; i < nargs; i++) {
            n += change_Arg_to_StackType(&sig, &vLocals[n]);
        }
        returnSig = sig;
    }

    noControlFlow = FALSE;

    while (ip < codeLength) {
        int opcode;

        START_TEMPORARY_ROOTS

#if TRACEVERIFIER
	fprintf(stdout, "Display derived/recorded stackmap at %d:\n", ip);
        printStackMap(thisMethod, ip);
        fprintf(stdout, "%3d %s\n", ip, byteCodeNames[code[ip]]);
#endif

        {
            /* Check that stackmaps are ordered according to offset, and
             * every offset in stackmaps point to the beginning to an
             * instruction.
             */
            STACKMAP stackMaps = thisMethod->u.java.stackMaps;
            if (stackMaps && stackMapIndex < stackMaps->nEntries) {
                if (stackMaps->entries[stackMapIndex].offset == ip) {
                    stackMapIndex++; /* This offset is good. */
                } else if (stackMaps->entries[stackMapIndex].offset < ip) {
                    /* ip should have met offset. */
                    VERIFIER_ERROR(VE_BAD_STACKMAP);
                } else {
                    /* Good so far. Nothing to do */
                }
            }
        }

        {
            /* Merge with the next instruction. Note how noControlFlow
             * is used here.
             */
            int flags = (noControlFlow ? 0 : SM_CHECK) | SM_MERGE;
	    if (TRACEVERIFIER) {
		fprintf(stdout, "Check instruction %d\n", ip);
	    }
            if (!matchStackMap(thisMethod, ip, ip, flags)) {
                VERIFIER_ERROR(VE_SEQ_BAD_TYPE);
            }
            noControlFlow = FALSE;
        }
        
        {
            /* Look for possible jump target in exception handlers. */
            HANDLERTABLE handlers = thisMethod->u.java.handlers;
            if (handlers) {
                int i;
                for (i = 0; i < handlers->length; i++) {
                    if (ip >= handlers->handlers[i].startPC &&
                        ip < handlers->handlers[i].endPC) {
                        unsigned short exceptionClassKey;
                        unsigned short vSP_bak;
                        unsigned short vStack0_bak;
                        index = handlers->handlers[i].exception;
                        if (index) {
                            CHECK_VALID_CP_INDEX(constPool, index);
                            tag = CONSTANTPOOL_MASKED_TAG(constPool, index);
                            if (tag != CONSTANT_Class) {
                                VERIFIER_ERROR(VE_EXPECT_CLASS);
                            }
                            exceptionClassKey =
                                GET_CLASS_CONSTANT_KEY(constPool, index);
                            if (!vIsAssignable(exceptionClassKey,
                                               throwableClassKey,
                                               NULL)) {
                                VERIFIER_ERROR(VE_EXPECT_THROWABLE);
                            }
                        } else {
                            exceptionClassKey = throwableClassKey;
                        }
                        /* Reset stack to only contain the exception object. */
                        vSP_bak = vSP;
                        vStack0_bak = vStack[0];
                        vSP = 0;
                        PUSH_STACK(exceptionClassKey);
#if TRACEVERIFIER
			fprintf(stdout, "Check exception handler at %d:\n",
				handlers->handlers[i].handlerPC);
#endif
                        CHECK_TARGET(ip, handlers->handlers[i].handlerPC);
                        /* Recover old stack. */
                        vStack[0] = vStack0_bak;
                        vSP = vSP_bak;
                    }
                }
            }
        }
        
        opcode = code[ip];
        switch (opcode) {
        case IF_ICMPEQ :
        case IF_ICMPNE :
        case IF_ICMPLT :
        case IF_ICMPGE :
        case IF_ICMPGT :
        case IF_ICMPLE :
            POP_STACK(ITEM_Integer);
            /* fall through */
        case IFEQ :
        case IFNE :
        case IFLT :
        case IFGE :
        case IFGT :
        case IFLE :
            POP_STACK(ITEM_Integer);
            CHECK_JUMP_TARGET(ip, ip + getShort(code + ip + 1));
            ip += 3;
            break;

        case IF_ACMPEQ :
        case IF_ACMPNE :
            POP_STACK(ITEM_Reference);
            /* fall through */
        case IFNULL :
        case IFNONNULL :
            POP_STACK(ITEM_Reference);
            CHECK_JUMP_TARGET(ip, ip + getShort(code + ip + 1));
            ip += 3;
            break;

        case GOTO:
            CHECK_JUMP_TARGET(ip, ip + getShort(code + ip + 1));
            ip += 3;
            noControlFlow = TRUE;
            break;

        case GOTO_W:
            CHECK_JUMP_TARGET(ip, ip + getCell(code + ip + 1));
            ip += 5;
            noControlFlow = TRUE;
            break;

        case TABLESWITCH:
        case LOOKUPSWITCH: 
            {
                long *lpc = (long *)(((long)(code + ip + 1) + 3) & ~3);
                long *lptr;
                int keys;
                int k, delta;
                POP_STACK(ITEM_Integer);
                if (opcode == TABLESWITCH) {
                    keys = getCell(&lpc[2]) -  getCell(&lpc[1]) + 1;
                    delta = 1;
                } else { 
                    keys = getCell(&lpc[1]);
                    delta = 2;
                    /* Make sure that the tableswitch items are sorted */
                    for (k = keys - 1, lptr = &lpc[2]; --k >= 0; lptr += 2) {
                        long this_key = getCell(&lptr[0]);
                        long next_key = getCell(&lptr[2]);
                        if (this_key >= next_key) { 
                            VERIFIER_ERROR(VE_BAD_LOOKUPSWITCH);
                        }
                    }
                }
                CHECK_JUMP_TARGET(ip, ip + getCell(&lpc[0]));
                for (k = keys, lptr = &lpc[3]; --k >= 0; lptr += delta) {
                    CHECK_JUMP_TARGET(ip, ip + getCell(&lptr[0]));
                }
                ip = (unsigned char*)(lptr - delta + 1) - code;
                noControlFlow = TRUE;
                break;
            }
    
        case NOP :
            ip++;
            break;
        case ACONST_NULL :
            PUSH_STACK(ITEM_Null);
            ip++;
            break;
        case ICONST_M1 :
        case ICONST_0 :
        case ICONST_1 :
        case ICONST_2 :
        case ICONST_3 :
        case ICONST_4 :
        case ICONST_5 :
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case LCONST_0 :
        case LCONST_1 :
            PUSH_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long_2);
            ip++;
            break;
#if IMPLEMENTS_FLOAT
        case FCONST_0 :
        case FCONST_1 :
        case FCONST_2 :
            PUSH_STACK(ITEM_Float);
            ip++;
            break;
        case DCONST_0 :
        case DCONST_1 :
            PUSH_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Double_2);
            ip++;
            break;
#endif
        case SIPUSH :
            ip++;
            /* fall through */
        case BIPUSH :
            PUSH_STACK(ITEM_Integer);
            ip += 2;
            break;
        case LDC :
            index = code[ip + 1];
            ip += 2;
            goto label_ldc;
        case LDC_W :
            /* fall through */
        case LDC2_W :
            index = getUShort(code + ip + 1);
            ip += 3;
        label_ldc:
            CHECK_VALID_CP_INDEX(constPool, index);
            tag = CONSTANTPOOL_MASKED_TAG(constPool, index);
            if (opcode == LDC || opcode == LDC_W) {
                if (tag == CONSTANT_String) {
                    PUSH_STACK(stringClassKey);
                    break;
                } else if (tag == CONSTANT_Integer) {
                    PUSH_STACK(ITEM_Integer);
                    break;
#if IMPLEMENTS_FLOAT
                } else if (tag == CONSTANT_Float) {
                    PUSH_STACK(ITEM_Float);
                    break;
#endif
                }
#if IMPLEMENTS_FLOAT
            } else if (tag == CONSTANT_Double) {
                PUSH_STACK(ITEM_Double);
                PUSH_STACK(ITEM_Double_2);
                break;
#endif
            } else if (tag == CONSTANT_Long) {
                PUSH_STACK(ITEM_Long);
                PUSH_STACK(ITEM_Long_2);
                break;
            }
            VERIFIER_ERROR(VE_BAD_LDC);
        case ILOAD :
            index = code[ip + 1];
            ip += 2;
            goto label_iload;
        case ILOAD_0 :
        case ILOAD_1 :
        case ILOAD_2 :
        case ILOAD_3 :
            index = opcode - ILOAD_0;
            ip++;
        label_iload:
            GET_LOCAL(index, ITEM_Integer);
            PUSH_STACK(ITEM_Integer);
            break;
        case LLOAD :
            index = code[ip + 1];
            ip += 2;
            goto label_lload;
        case LLOAD_0 :
        case LLOAD_1 :
        case LLOAD_2 :
        case LLOAD_3 :
            index = opcode - LLOAD_0;
            ip++;
        label_lload:
            GET_LOCAL(index, ITEM_Long);
            GET_LOCAL(index + 1, ITEM_Long_2);
            PUSH_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long_2);
            break;
#if IMPLEMENTS_FLOAT
        case FLOAD :
            index = code[ip + 1];
            ip += 2;
            goto label_fload;
        case FLOAD_0 :
        case FLOAD_1 :
        case FLOAD_2 :
        case FLOAD_3 :
            index = opcode - FLOAD_0;
            ip++;
        label_fload:
            GET_LOCAL(index, ITEM_Float);
            PUSH_STACK(ITEM_Float);
            break;
        case DLOAD :
            index = code[ip + 1];
            ip += 2;
            goto label_dload;
        case DLOAD_0 :
        case DLOAD_1 :
        case DLOAD_2 :
        case DLOAD_3 :
            index = opcode - DLOAD_0;
            ip++;
        label_dload:
            GET_LOCAL(index, ITEM_Double);
            GET_LOCAL(index + 1, ITEM_Double_2);
            PUSH_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Double_2);
            break;
#endif
        case ALOAD :
            index = code[ip + 1];
            ip += 2;
            goto label_aload;
        case ALOAD_0 :
        case ALOAD_1 :
        case ALOAD_2 :
        case ALOAD_3 :
            index = opcode - ALOAD_0;
            ip++;
        label_aload:
            GET_LOCAL(index, ITEM_Reference);
            PUSH_STACK(lastLocalGet);
            break;
        case IALOAD :
            POP_STACK(ITEM_Integer);
            POP_STACK(intArrayClassKey);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case BALOAD :
            POP_STACK(ITEM_Integer);
            POP_STACK(objectClassKey);
            if (lastStackPop != byteArrayClassKey &&
                lastStackPop != booleanArrayClassKey) {
                VERIFIER_ERROR(VE_BALOAD_BAD_TYPE);
            }
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case CALOAD :
            POP_STACK(ITEM_Integer);
            POP_STACK(charArrayClassKey);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case SALOAD :
            POP_STACK(ITEM_Integer);
            POP_STACK(shortArrayClassKey);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case LALOAD :
            POP_STACK(ITEM_Integer);
            POP_STACK(longArrayClassKey);
            PUSH_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long_2);
            ip++;
            break;
#if IMPLEMENTS_FLOAT
        case FALOAD :
            POP_STACK(ITEM_Integer);
            POP_STACK(floatArrayClassKey);
            PUSH_STACK(ITEM_Float);
            ip++;
            break;
        case DALOAD :
            POP_STACK(ITEM_Integer);
            POP_STACK(doubleArrayClassKey);
            PUSH_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Double_2);
            ip++;
            break;
#endif
        case AALOAD :
            POP_STACK(ITEM_Integer);
            POP_STACK(objectClassKey);
            if (!vIsAssignable(lastStackPop, objectArrayClassKey, NULL)) {
                VERIFIER_ERROR(VE_AALOAD_BAD_TYPE);
            }
            typeKey = getObjectArrayElementKey(lastStackPop);
            PUSH_STACK(typeKey);
            ip++;
            break;
        case ISTORE :
            index = code[ip + 1];
            ip += 2;
            goto label_istore;
        case ISTORE_0 :
        case ISTORE_1 :
        case ISTORE_2 :
        case ISTORE_3 :
            index = opcode - ISTORE_0;
            ip++;
        label_istore:
            POP_STACK(ITEM_Integer);
            SET_LOCAL(index, ITEM_Integer);
            break;
        case LSTORE :
            index = code[ip + 1];
            ip += 2;
            goto label_lstore;
        case LSTORE_0 :
        case LSTORE_1 :
        case LSTORE_2 :
        case LSTORE_3 :
            index = opcode - LSTORE_0;
            ip++;
        label_lstore:
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            SET_LOCAL(index + 1, ITEM_Long_2);
            SET_LOCAL(index, ITEM_Long);
            break;
#if IMPLEMENTS_FLOAT
        case FSTORE :
            index = code[ip + 1];
            ip += 2;
            goto label_fstore;
        case FSTORE_0 :
        case FSTORE_1 :
        case FSTORE_2 :
        case FSTORE_3 :
            index = opcode - FSTORE_0;
            ip++;
        label_fstore:
            POP_STACK(ITEM_Float);
            SET_LOCAL(index, ITEM_Float);
            break;
        case DSTORE :
            index = code[ip + 1];
            ip += 2;
            goto label_dstore;
        case DSTORE_0 :
        case DSTORE_1 :
        case DSTORE_2 :
        case DSTORE_3 :
            index = opcode - DSTORE_0;
            ip++;
        label_dstore:
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            SET_LOCAL(index + 1, ITEM_Double_2);
            SET_LOCAL(index, ITEM_Double);
            break;
#endif
        case ASTORE :
            index = code[ip + 1];
            ip += 2;
            goto label_astore;
        case ASTORE_0 :
        case ASTORE_1 :
        case ASTORE_2 :
        case ASTORE_3 :
            index = opcode - ASTORE_0;
            ip++;
        label_astore:
            POP_STACK(ITEM_Reference);
            SET_LOCAL(index, lastStackPop);
            break;
        case IASTORE :
            POP_STACK(ITEM_Integer);
            POP_STACK(ITEM_Integer);
            POP_STACK(intArrayClassKey);
            ip++;
            break;
        case BASTORE :
            POP_STACK(ITEM_Integer);
            POP_STACK(ITEM_Integer);
            POP_STACK(objectClassKey);
            if (lastStackPop != byteArrayClassKey &&
                lastStackPop != booleanArrayClassKey) {
                VERIFIER_ERROR(VE_BASTORE_BAD_TYPE);
            }
            ip++;
            break;
        case CASTORE :
            POP_STACK(ITEM_Integer);
            POP_STACK(ITEM_Integer);
            POP_STACK(charArrayClassKey);
            ip++;
            break;
        case SASTORE :
            POP_STACK(ITEM_Integer);
            POP_STACK(ITEM_Integer);
            POP_STACK(shortArrayClassKey);
            ip++;
            break;
        case LASTORE :
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            POP_STACK(ITEM_Integer);
            POP_STACK(longArrayClassKey);
            ip++;
            break;
#if IMPLEMENTS_FLOAT
        case FASTORE :
            POP_STACK(ITEM_Float);
            POP_STACK(ITEM_Integer);
            POP_STACK(floatArrayClassKey);
            ip++;
            break;
        case DASTORE :
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            POP_STACK(ITEM_Integer);
            POP_STACK(doubleArrayClassKey);
            ip++;
            break;
#endif
        case AASTORE :
            POP_STACK(objectClassKey);
            POP_STACK(ITEM_Integer);
            POP_STACK(objectArrayClassKey);
            ip++;
            break;
        case POP :
            POP_STACK(ITEM_Category1);
            ip++;
            break;
        case POP2 :
            POP_STACK(ITEM_Category2);
            ip++;
            break;
        case DUP : 
            POP_STACK(ITEM_Category1);
            PUSH_STACK(lastStackPop);
            PUSH_STACK(lastStackPop);
            ip++;
            break;
        case DUP_X1 :
            {
                unsigned short type1, type2;
                POP_STACK(ITEM_Category1);
                type1 = lastStackPop;
                POP_STACK(ITEM_Category1);
                type2 = lastStackPop;
                PUSH_STACK(type1);
                PUSH_STACK(type2);
                PUSH_STACK(type1);
                ip++;
                break;
            }
        case DUP_X2 :
            POP_STACK(ITEM_Category1);
            POP_STACK(ITEM_DoubleWord);
            PUSH_STACK(lastStackPop);
            PUSH_STACK(lastStackPop2.fst);
            PUSH_STACK(lastStackPop2.snd);
            PUSH_STACK(lastStackPop);
            ip++;
            break;
        case DUP2 :
            POP_STACK(ITEM_DoubleWord);
            PUSH_STACK(lastStackPop2.fst);
            PUSH_STACK(lastStackPop2.snd);
            PUSH_STACK(lastStackPop2.fst);
            PUSH_STACK(lastStackPop2.snd);
            ip++;
            break;
        case DUP2_X1 :
            POP_STACK(ITEM_DoubleWord);
            POP_STACK(ITEM_Category1);
            PUSH_STACK(lastStackPop2.fst);
            PUSH_STACK(lastStackPop2.snd);
            PUSH_STACK(lastStackPop);
            PUSH_STACK(lastStackPop2.fst);
            PUSH_STACK(lastStackPop2.snd);
            ip++;
            break;
        case DUP2_X2 :
            {
                struct DoubleWordItem item1;
                struct DoubleWordItem item2;
                POP_STACK(ITEM_DoubleWord);
                item1 = lastStackPop2;
                POP_STACK(ITEM_DoubleWord);
                item2 = lastStackPop2;
                PUSH_STACK(item1.fst);
                PUSH_STACK(item1.snd);
                PUSH_STACK(item2.fst);
                PUSH_STACK(item2.snd);
                PUSH_STACK(item1.fst);
                PUSH_STACK(item1.snd);
                ip++;
                break;
            }
        case SWAP :
            {
                unsigned short item1, item2;
                POP_STACK(ITEM_Category1);
                item1 = lastStackPop;
                POP_STACK(ITEM_Category1);
                item2 = lastStackPop;
                PUSH_STACK(item1);
                PUSH_STACK(item2);
                ip++;
                break;
            }
        case IADD :
        case ISUB :
        case IMUL :
        case IDIV :
        case IREM :
        case ISHL :
        case ISHR :
        case IUSHR :
        case IOR :
        case IXOR :
        case IAND :
            POP_STACK(ITEM_Integer);
            /* fall through */
        case INEG :
            POP_STACK(ITEM_Integer);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case LADD :
        case LSUB :
        case LMUL :
        case LDIV :
        case LREM :
        case LAND :
        case LOR :
        case LXOR :
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            /* fall through */
        case LNEG :
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long_2);
            ip++;
            break;
        case LSHL :
        case LSHR :
        case LUSHR :
            POP_STACK(ITEM_Integer);
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long_2);
            ip++;
            break;
#if IMPLEMENTS_FLOAT
        case FADD :
        case FSUB :
        case FMUL :
        case FDIV :
        case FREM :
            POP_STACK(ITEM_Float);
            /* fall through */
        case FNEG :
            POP_STACK(ITEM_Float);
            PUSH_STACK(ITEM_Float);
            ip++;
            break;
        case DADD :
        case DSUB :
        case DMUL :
        case DDIV :
        case DREM :
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            /* fall through */
        case DNEG :
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Double_2);
            ip++;
            break;
#endif
        case IINC : 
            index = code[ip + 1];
            ip += 3;
        label_iinc:
            GET_LOCAL(index, ITEM_Integer);
            SET_LOCAL(index, ITEM_Integer);
            break;
        case I2L :
            POP_STACK(ITEM_Integer);
            PUSH_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long_2);
            ip++;
            break;
        case L2I :
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
#if IMPLEMENTS_FLOAT
        case I2F :
            POP_STACK(ITEM_Integer);
            PUSH_STACK(ITEM_Float);
            ip++;
            break;
        case I2D :
            POP_STACK(ITEM_Integer);
            PUSH_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Double_2);
            ip++;
            break;
        case L2F :
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Float);
            ip++;
            break;
        case L2D :
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Double_2);
            ip++;
            break;
        case F2I :
            POP_STACK(ITEM_Float);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case F2L :
            POP_STACK(ITEM_Float);
            PUSH_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long_2);
            ip++;
            break;
        case F2D :
            POP_STACK(ITEM_Float);
            PUSH_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Double_2);
            ip++;
            break;
        case D2I :
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case D2L :
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Long_2);
            ip++;
            break;
        case D2F :
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Float);
            ip++;
            break;
#endif
        case I2B :
        case I2C :
        case I2S :
            POP_STACK(ITEM_Integer);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case LCMP :
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
#if IMPLEMENTS_FLOAT
        case FCMPL :
        case FCMPG :
            POP_STACK(ITEM_Float);
            POP_STACK(ITEM_Float);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
        case DCMPL :
        case DCMPG :
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            PUSH_STACK(ITEM_Integer);
            ip++;
            break;
#endif
        case GETSTATIC :
        case PUTSTATIC :
        case GETFIELD :
        case PUTFIELD :
            {
                unsigned short nameTypeIndex;
                unsigned short fieldClassIndex;
                unsigned short targetClassKey = 0; /* keep compilers happy */

                index = getUShort(code + ip + 1);
                CHECK_VALID_CP_INDEX(constPool, index);
                tag = CONSTANTPOOL_TAG(constPool, index);
                if (tag != CONSTANT_Fieldref) {
                    VERIFIER_ERROR(VE_EXPECT_FIELDREF);
                }
                nameTypeIndex = constPool->entries[index].method.nameTypeIndex;
                fieldClassIndex = constPool->entries[index].method.classIndex;
                typeKey = constPool->entries[nameTypeIndex].nameTypeKey.nt.typeKey;

                {
                    unsigned short ty[2];
                    int n = change_Field_to_StackType(typeKey, ty);
                    int i;

                    if (opcode == GETFIELD) {
                        POP_STACK(objectClassKey);
                        targetClassKey = lastStackPop;
                        for (i = 0; i < n; i++) {
                            PUSH_STACK(ty[i]);
                        }
                    } else if (opcode == PUTFIELD) {
                        for (i = n - 1; i >= 0; i--) {
                            POP_STACK(ty[i]);
                        }
                        POP_STACK(objectClassKey);
                        targetClassKey = lastStackPop;
                    } else if (opcode == GETSTATIC) {
                        for (i = 0; i < n; i++) {
                            PUSH_STACK(ty[i]);
                        }
                    } else {
                        for (i = n - 1; i >= 0; i--) {
                            POP_STACK(ty[i]);
                        }
                    }
                }

                if (opcode == GETFIELD || opcode == PUTFIELD) {
                    unsigned short fieldClassKey = 
                        GET_CLASS_CONSTANT_KEY(constPool, fieldClassIndex);
                    if (!vIsAssignable(targetClassKey, fieldClassKey, NULL)) {
                        VERIFIER_ERROR(VE_FIELD_BAD_TYPE);
                    }
                }

                ip += 3;
                break;
            }

        case INVOKEVIRTUAL :
        case INVOKESPECIAL :
        case INVOKESTATIC :
        case INVOKEINTERFACE :
            {
                unsigned short nameTypeIndex;
                unsigned short methodClassIndex;
                unsigned short methodNameKey;
                unsigned char* sig;
                unsigned short i;
		unsigned short nargs;
		unsigned short nwords;

                index = getUShort(code + ip + 1);
                CHECK_VALID_CP_INDEX(constPool, index);
                tag = CONSTANTPOOL_TAG(constPool, index);
                if (tag != CONSTANT_Methodref && tag != CONSTANT_InterfaceMethodref) {
                    VERIFIER_ERROR(VE_EXPECT_METHODREF);
                }
                methodClassIndex = constPool->entries[index].method.classIndex;
                nameTypeIndex = constPool->entries[index].method.nameTypeIndex;

                typeKey = constPool->entries[nameTypeIndex].nameTypeKey.nt.typeKey;
                sig = (unsigned char*)change_Key_to_Name(typeKey, NULL);
		nargs = *sig++;

                {
                    unsigned char* sig_bak;
                    unsigned short ty[2];
                    int j,k;
                    unsigned short i;
                    
                    sig_bak = sig;
                    for (nwords = 0, i = 0; i < nargs; i++) {
                        nwords += change_Arg_to_StackType(&sig, ty);
                    }
                    if (vSP < nwords) {
                        VERIFIER_ERROR(VE_ARGS_NOT_ENOUGH);
                    }
                    vSP -= nwords;
                    sig = sig_bak;
                    for (i = 0, k = 0; i < nargs; i++) {
                        int n = change_Arg_to_StackType(&sig, ty);
                        for (j = 0; j < n; j++) {
                            unsigned short arg = vStack[vSP + k];
                            if (!vIsAssignable(arg, ty[j], NULL)) {
                                VERIFIER_ERROR(VE_ARGS_BAD_TYPE);
                            }
                            k++;
                        }
                    }
                }

		methodNameKey = 
		    constPool->entries[nameTypeIndex].nameTypeKey.nt.nameKey;
		{
		    char *methodName = change_Key_to_Name(methodNameKey, NULL);
		    if (methodName[0] == '<') {
                        if (opcode != INVOKESPECIAL ||
			    methodNameKey != initNameAndType.nt.nameKey) {
                            VERIFIER_ERROR(VE_EXPECT_INVOKESPECIAL);
                        }
		    }
		}
                if (opcode != INVOKESTATIC) {
                    unsigned short methodClassKey = 
                        GET_CLASS_CONSTANT_KEY(constPool, methodClassIndex);
                    unsigned short targetClassKey;
                    if (methodNameKey == initNameAndType.nt.nameKey) {
                        POP_STACK(ITEM_Category1);
                        typeKey = lastStackPop;
                        if (typeKey & ITEM_NewObject_Flag) {
                            index = DECODE_NEWOBJECT(typeKey);
                            if (index > codeLength - 3 || code[index] != NEW) {
                                VERIFIER_ERROR(VE_EXPECT_NEW);
                            }
                            index = getUShort(code + index + 1);
                            CHECK_VALID_CP_INDEX(constPool, index);
                            tag = CONSTANTPOOL_MASKED_TAG(constPool, index);
                            if (tag != CONSTANT_Class) {
                                VERIFIER_ERROR(VE_EXPECT_CLASS);
                            }
                            targetClassKey = GET_CLASS_CONSTANT_KEY(constPool, index);
                        } else if (typeKey == ITEM_InitObject) {
                            targetClassKey = thisMethod->ofClass->class.key;
                        } else {
                            VERIFIER_ERROR(VE_EXPECT_UNINIT);
                        }
                        for (i = 0; i < vSP; i++) {
                            if (vStack[i] == typeKey) {
                                vStack[i] = targetClassKey;
                            }
                        }
                        for (i = 0; i < thisMethod->frameSize; i++) {
                            if (vLocals[i] == typeKey) {
                                vLocals[i] = targetClassKey;
                            }
                        }
                        PUSH_STACK(targetClassKey);
                    } else {
			if (opcode == INVOKESPECIAL &&
			    methodClassKey != thisMethod->ofClass->class.key) {
			    INSTANCE_CLASS superClass = 
				thisMethod->ofClass->superClass;
			    while (superClass && 
				   superClass->class.key != methodClassKey) {
				superClass = superClass->superClass;
			    }
			    if (!superClass) {
				VERIFIER_ERROR(VE_INVOKESPECIAL);
			    }
			}
		    }
                    /* This ensures that targetClass is assignable to methodClass. */
                    POP_STACK(methodClassKey);
                }
                /* Push the result type. */
                if (*sig != 'V') {
                    unsigned short ty[2];
                    int n = change_Arg_to_StackType(&sig, ty);
                    for (i = 0 ; i < n; i++) {
                        PUSH_STACK(ty[i]);
                    }
                }
                if (opcode == INVOKEINTERFACE) {
		    if (code[ip + 3] != nwords + 1) {
			VERIFIER_ERROR(VE_NARGS_MISMATCH);
		    }
		    if (code[ip + 4] != 0) {
			VERIFIER_ERROR(VE_EXPECT_ZERO);
		    }
                    ip += 5;
                } else {
                    ip += 3;
                }
                break;
            }
        case NEW :
            index = getUShort(code + ip + 1);
            CHECK_VALID_CP_INDEX(constPool, index);
            tag = CONSTANTPOOL_MASKED_TAG(constPool, index);
            if (tag != CONSTANT_Class) {
                VERIFIER_ERROR(VE_EXPECT_CLASS);
            }
	    typeKey = GET_CLASS_CONSTANT_KEY(constPool, index);
	    if (isArrayClassKey(typeKey, 1)) {
                VERIFIER_ERROR(VE_EXPECT_CLASS);
	    }
            PUSH_STACK((unsigned short)(ENCODE_NEWOBJECT(ip)));
            ip += 3;
            break;
        case NEWARRAY :
            tag = code[ip + 1];
            switch (tag) {
            case T_BOOLEAN:
                typeKey = booleanArrayClassKey;
                break;
            case T_CHAR:
                typeKey = charArrayClassKey;
                break;
#if IMPLEMENTS_FLOAT
            case T_FLOAT:
                typeKey = floatArrayClassKey;
                break;
            case T_DOUBLE:
                typeKey = doubleArrayClassKey;
                break;
#endif
            case T_BYTE:
                typeKey = byteArrayClassKey;
                break;
            case T_SHORT:
                typeKey = shortArrayClassKey;
                break;
            case T_INT:
                typeKey = intArrayClassKey;
                break;
            case T_LONG:
                typeKey = longArrayClassKey;
                break;
            default:
                VERIFIER_ERROR(VE_BAD_INSTR);
            }
            POP_STACK(ITEM_Integer);
            PUSH_STACK(typeKey);
            ip += 2;
            break;

        case ANEWARRAY :
            index = getUShort(code + ip + 1);
            CHECK_VALID_CP_INDEX(constPool, index);
            tag = CONSTANTPOOL_MASKED_TAG(constPool, index);
            if (tag != CONSTANT_Class) {
                VERIFIER_ERROR(VE_EXPECT_CLASS);
            }
            typeKey = GET_CLASS_CONSTANT_KEY(constPool, index);
            typeKey = makeObjectArrayClassKey(typeKey);
            POP_STACK(ITEM_Integer);
            PUSH_STACK(typeKey);
            ip += 3;
            break;

        case ARRAYLENGTH :
            POP_STACK(objectClassKey);
            if (!isArrayClassKey(lastStackPop, 1)) {
                VERIFIER_ERROR(VE_EXPECT_ARRAY);
            }
            PUSH_STACK(ITEM_Integer);
            ip += 1;
            break;
        case CHECKCAST :
            index = getUShort(code + ip + 1);
            CHECK_VALID_CP_INDEX(constPool, index);
            tag = CONSTANTPOOL_MASKED_TAG(constPool, index);
            if (tag != CONSTANT_Class) {
                VERIFIER_ERROR(VE_EXPECT_CLASS);
            }
            typeKey = GET_CLASS_CONSTANT_KEY(constPool, index);
            POP_STACK(objectClassKey);
            PUSH_STACK(typeKey);
            ip += 3;
            break;
        case INSTANCEOF :
            index = getUShort(code + ip + 1);
            CHECK_VALID_CP_INDEX(constPool, index);
            tag = CONSTANTPOOL_MASKED_TAG(constPool, index);
            if (tag != CONSTANT_Class) {
                VERIFIER_ERROR(VE_EXPECT_CLASS);
            }
            POP_STACK(objectClassKey);
            PUSH_STACK(ITEM_Integer);
            ip += 3;
            break;
        case MONITORENTER :
        case MONITOREXIT :
            POP_STACK(objectClassKey);
            ip++;
            break;
        case MULTIANEWARRAY : {
            int dim, i;
            index = getUShort(code + ip + 1);
            CHECK_VALID_CP_INDEX(constPool, index);
            tag = CONSTANTPOOL_MASKED_TAG(constPool, index);
            if (tag != CONSTANT_Class) {
                VERIFIER_ERROR(VE_EXPECT_CLASS);
            }
            typeKey = GET_CLASS_CONSTANT_KEY(constPool, index);
            dim = code[ip + 3];
            if (dim == 0 || !isArrayClassKey(typeKey, dim)) {
                VERIFIER_ERROR(VE_MULTIANEWARRAY);
            }
            for (i = 0; i < dim; i++) {
                POP_STACK(ITEM_Integer);
            }
            PUSH_STACK(typeKey);
            ip += 4;
            break;
        }

        case IRETURN :
            POP_STACK(ITEM_Integer);
            goto label_return_value;
        case LRETURN :
            POP_STACK(ITEM_Long_2);
            POP_STACK(ITEM_Long);
            goto label_return_value;
#if IMPLEMENTS_FLOAT
        case FRETURN :
            POP_STACK(ITEM_Float);
            goto label_return_value;
        case DRETURN :
            POP_STACK(ITEM_Double_2);
            POP_STACK(ITEM_Double);
            goto label_return_value;
#endif
        case ARETURN :
            POP_STACK(objectClassKey);
        label_return_value: 
            {
                unsigned short ty[2];
                unsigned char *sig = returnSig;
                if (sig[0] == 'V') {
                    VERIFIER_ERROR(VE_EXPECT_NO_RETVAL);
                }
                change_Arg_to_StackType(&sig, ty);
                if (!vIsAssignable(lastStackPop, ty[0], NULL)) {
                    VERIFIER_ERROR(VE_RETVAL_BAD_TYPE);
                }
                goto label_return;
            }
        case RETURN :
            if (returnSig[0] != 'V') {
                VERIFIER_ERROR(VE_EXPECT_RETVAL);
            }
        label_return:
            if (thisMethod->nameTypeKey.nt.nameKey == initNameAndType.nt.nameKey) {
                if (vLocals[0] == ITEM_InitObject) {
                    VERIFIER_ERROR(VE_RETURN_UNINIT_THIS);
                }
            }
            ip++;
            noControlFlow = TRUE;
            break;

        case ATHROW :
            POP_STACK(throwableClassKey);
            ip++;
            noControlFlow = TRUE;
            break;

        case WIDE:
            opcode = code[ip + 1];
            switch (opcode) {
            case IINC:
                index = getUShort(code + ip + 2);
                ip += 6;
                goto label_iinc;
            case ILOAD:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_iload;
            case ALOAD:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_aload;
            case LLOAD:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_lload;
            case ISTORE:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_istore;
            case ASTORE:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_astore;
            case LSTORE:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_lstore;
#if IMPLEMENTS_FLOAT
            case FLOAD:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_fload;
            case DLOAD:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_dload;
            case FSTORE:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_fstore;
            case DSTORE:
                index = getUShort(code + ip + 2);
                ip += 4;
                goto label_dstore;
#endif
            default:
                VERIFIER_ERROR(VE_BAD_INSTR);
            }
        default:
            VERIFIER_ERROR(VE_BAD_INSTR);
            break;
        }
        END_TEMPORARY_ROOTS
    }

    /* Make sure that control flow does not fall through the end of the method. */
    if (!noControlFlow) {
        VERIFIER_ERROR(VE_FALL_THROUGH);
    }

 err:
    END_TEMPORARY_ROOTS
    return result;
}

/*=========================================================================
 * FUNCTION:      verifyClass
 * TYPE:          public operation on classes.
 * OVERVIEW:      Perform byte-code verification of a given class. Iterate
 *                through all methods.
 *
 * INTERFACE:
 *   parameters:  thisClass: class to be verified.
 *   returns:     0 if verification succeeds, error code if verification
 *                fails.
 *=======================================================================*/

int
verifyClass(INSTANCE_CLASS thisClass)
{
    int i;
    if (thisClass->methodTable) {
        for (i = 0; i < thisClass->methodTable->length; i++) {
            METHOD thisMethod = &thisClass->methodTable->methods[i];
            int result;
            /* Skip special synthesized methods. */
            if (thisMethod == RunCustomCodeMethod) {
                continue;
            }
            /* Skip abstract and native methods. */
            if (thisMethod->accessFlags & (ACC_NATIVE | ACC_ABSTRACT)) {
                continue;
            }
            result = verifyMethod(thisMethod);
            if (result != 0) {
                return result;
            }
        }
    }
    thisClass->status = CLASS_VERIFIED;
    return 0;
}

/* Debugging support follows */

#if TRACEVERIFIER

/* Print a string, and add padding. */
static void 
print_str(char *str)
{
    int i;
    fprintf(stdout, "%s", str);
    i = 30 - strlen(str);
    while (i > 0) {
        fprintf(stdout, " ");
        i--;
    }
}

/* Print a type key */
static void 
printStackType(METHOD thisMethod, unsigned short key)
{
    char buf[128];
    switch (key) {
    case 0xFFF:
        print_str("-");
        return;
    case ITEM_Bogus:
        print_str("*");
        return;
    case ITEM_Integer:
        print_str("I");
        return;
#if IMPLEMENTS_FLOAT
    case ITEM_Float:
        print_str("F");
        return;
    case ITEM_Double:
        print_str("D");
        return;
#endif
    case ITEM_Long:
        print_str("J");
        return;
    case ITEM_Null:
        print_str("null");
        return;
    case ITEM_InitObject:
        print_str("this");
        return;
    case ITEM_Long_2:
        print_str("J2");
        return;
#if IMPLEMENTS_FLOAT
    case ITEM_Double_2:
        print_str("D2");
        return;
#endif
    default: /* ITEM_Object and ITEM_NewObject */
        if (key & ITEM_NewObject_Flag) {
            sprintf(buf, "new %d", DECODE_NEWOBJECT(key));
            print_str(buf);
        } else {
            print_str(change_Key_to_FieldSignature(key));
        }
        return;
    }
}

#define MAX(a,b) ((a) > (b) ? (a) : (b))

/* Print the derived stackmap and recorded stackmap (if it exists) at
 * a given ip.
 */
static void 
printStackMap(METHOD thisMethod, unsigned short ip)
{
    unsigned short* stackMap = getStackMap(thisMethod, 65535, ip);
    int nlocals, nstack, i;
    int lTop;

    for (lTop = vFrameSize; lTop > 0; lTop--) {
        if (vLocals[lTop - 1] != ITEM_Bogus) {
            break;
        }
    }

    fprintf(stdout, "__SLOT__DERIVED_______________________");
    if (stackMap) {
        fprintf(stdout, "RECORDED______________________\n");
    } else {
        fprintf(stdout, "\n");
    }

    nlocals = stackMap ? (*stackMap++) : 0;

    for (i = 0; i < MAX(lTop, nlocals); i++) {
        unsigned short ty;
        if (i < 100) {
            fprintf(stdout, " ");
            if (i < 10) {
                fprintf(stdout, " ");
            }
        }
        fprintf(stdout, "L[%d]  ", i);
        if (i < lTop) {
            ty = vLocals[i];
        } else {
            ty = 0xFFF;
        }
        printStackType(thisMethod, ty);
        if (stackMap) {
            if (i < nlocals) {
                ty = *stackMap++;
            } else {
                ty = 0xFFF;
            }
            printStackType(thisMethod, ty);
        }
        fprintf(stdout, "\n");
    }

    nstack = stackMap ? (*stackMap++) : 0;

    for (i = 0; i < MAX(vSP, nstack); i++) {
        unsigned short ty;
        if (i < 100) {
            fprintf(stdout, " ");
            if (i < 10) {
                fprintf(stdout, " ");
            }
        }
        fprintf(stdout, "S[%d]  ", i);
        if (i < vSP) {
            ty = vStack[i];
        } else {
            ty = 0xFFF;
        }
        printStackType(thisMethod, ty);

        if (stackMap) {
            if (i < nstack) {
                ty = *stackMap++;
            } else {
                ty = 0xFFF;
            }
            printStackType(thisMethod, ty);
        }
        fprintf(stdout, "\n");
    }
}

#endif
