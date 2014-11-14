/*
 * @(#)classloader.c	1.11 00/04/12
 *
 * Copyright 1995-1999 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 * 
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 */

/*-
 *      Routines for loading and resolving class definitions.
 *      These routines should not depending up on the interpreter or
 *      garbage collector.
 *
 */

#include <string.h>
#include <sys/types.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <setjmp.h>

#include "oobj.h"
#include "path.h"
#include "tree.h"
#include "signature.h"

#include "sys_api.h"

char *stat_source(ClassClass *cb, struct stat *s, char *pathbuf, int maxlen);
extern ClassClass *allocClassClass();

void 
AddBinClass(ClassClass * cb)
{
    register int left, right, middle, result, i;
    char *name = cbName(cb);
    struct Hjava_lang_ClassLoader *loader = cbLoader(cb);
    
    BINCLASS_LOCK();
    left = 0;
    right = nbinclasses - 1;
    result = 1;
    while (left <= right) {
        ClassClass *cb1;
	middle = (left+right)/2;
	cb1 = binclasses[middle];
	result = strcmp(name, cbName(cb1));
	if (result == 0) {
	    if (loader < cbLoader(cb1)) {
	        result = -1;
	    } else if (loader > cbLoader(cb1)) {
	        result = 1;
	    } else {
	        result = 0;
	    }
	}
	if (result < 0) {
	    right = middle-1;
	} else if (result > 0) {
	    left = middle+1;
	} else {
	    break;
	}
    }
    if (result != 0) {
        if (nbinclasses >= sizebinclasses) {
	    if (binclasses == 0)
		binclasses = (ClassClass **)
		    sysMalloc(sizeof(ClassClass *) * (sizebinclasses = 50));
	    else
		binclasses = (ClassClass **)
		    sysRealloc(binclasses, sizeof(ClassClass *)
			    * (sizebinclasses = nbinclasses * 2));
	}
	if (binclasses == 0)
	    goto unlock;
	right++;
	for (i = nbinclasses; i > right; i--) {
	    binclasses[i] = binclasses[i-1];
	}
	binclasses[right] = cb;
	nbinclasses++;
    }

unlock:
    BINCLASS_UNLOCK();
}	
	
void
DelBinClass(ClassClass * cb)
{
    register int i, j;
    BINCLASS_LOCK();
    for (i = nbinclasses; --i >= 0; )
	if (binclasses[i] == cb) {
	    nbinclasses--;
	    for (j = i; j < nbinclasses; j++) {
	        binclasses[j] = binclasses[j+1];
	    }
	    break;
	}
    BINCLASS_UNLOCK();
}

void
MakeClassSticky(ClassClass *cb)
{
    /* monitorEnter(obj_monitor(cb));	*/
    CCSet(cb, Sticky);
    /* monitorExit(obj_monitor(cb));	*/
}

/* 
 * When the interpreter asks for a file, in the end it's looking
 * for a classblock structure to be created.  The only way it can
 * get one of those is by loading a compiled class.
 *
 * OpenCode tries to open a .class file first.  -2 means it failed
 * because it couldn't open the file.
 *
 * If OpenCode returns a valid fd, that means it is a .class file.
 * If SkipCheckSource is false (meaning we're checking source
 * files), and the source file as specified in the compiled .class
 * file is more recent than the .class file, then the loaded class
 * is thrown away, and OpenCode is called again, only this time,
 * with arguments which will cause it to recompile the source.  In
 * the case of java, it forks javac to do this.  When this happens,
 * it has to reload the resulting .class file.
 *
 * Returns the class when it succeeds, 0 when it fails. 
*/


/* Load a .class file normally from the local disk */
static ClassClass *
LoadClassFromFile(char *fn, char *dir, char *class_name)
{
    extern int OpenCode(char *, char *, char *, struct stat*);
    struct stat st;
    ClassClass *cb = 0;
    int codefd = -1;
    int	retryCount = 0;
    unsigned char *external_class;
    char *detail;

    codefd = OpenCode(fn, NULL, dir, &st);

    if (codefd < 0)		/* open failed */
	return 0;

    /* Snarf the file into memory. */
    external_class = (unsigned char *)sysMalloc(st.st_size);
    if (external_class == 0)
        goto failed;
    if (sysRead(codefd, external_class, st.st_size) != st.st_size)
        goto failed;
    sysClose(codefd);
    codefd = -1;

    /* Create the internal class */
    cb = allocClassClass();
    if (cb == NULL || 
	!createInternalClass(external_class, external_class + st.st_size, 
			     cb, NULL, class_name, &detail)) {
	sysFree(external_class);
	goto failed;
    }
    sysFree(external_class);

    if (verbose)
	jio_fprintf(stderr, "[Loaded %s]\n", fn);
    return cb;
failed:
    if (codefd >= 0)
	sysClose(codefd);
    if (cb != 0)
        FreeClass(cb);
    return 0;
}

/*
 * Find a class file that's somewhere local, and not from a classloader.  
 * We still need to search for it using the classpath.
 */

ClassClass *LoadClassLocally(char *name)
{
	extern char *extra_class_path;
    ClassClass *cb = 0;
    cpe_t **cpp;

    if (name[0] == DIR_SEPARATOR || name[0] == SIGNATURE_ARRAY) 
	return 0;

    for (cpp = sysGetClassPath(); cpp && *cpp != 0; cpp++) {
	cpe_t *cpe = *cpp;
	char path[255];
	if (cpe->type == CPE_DIR) {
	    if (jio_snprintf(path, sizeof(path), 
			     "%s%c%s." JAVAOBJEXT, cpe->u.dir,
			     LOCAL_DIR_SEPARATOR, name) == -1) {
		return 0;
	    }
	    if (cb = LoadClassFromFile(sysNativePath(path), cpe->u.dir, name)) {
		return cb;
	    }
	}
	
    }
	if (extra_class_path) {
		char path[255];
	    if (jio_snprintf(path, sizeof(path), 
			     "%s%c%s." JAVAOBJEXT, extra_class_path,
			     LOCAL_DIR_SEPARATOR, name) == -1) {
			return 0;
		}
	    if (cb = LoadClassFromFile(sysNativePath(path), extra_class_path, name)) {
			return cb;
		}
	}
    return cb;
}

char *
stat_source(ClassClass *cb, struct stat *s, char *pathbuf, int maxlen)
{
#define NAMEBUFLEN 255
    char nm[NAMEBUFLEN];
    char *p, *q, *lp;
    cpe_t **cpp;

    /* don't bother searching if absolute */
    /* REMIND: only here for compatibility */
    if (sysIsAbsolute(cbSourceName(cb))) {
	if (sysStat(cbSourceName(cb), s) == 0) {
	    if (jio_snprintf(pathbuf, maxlen, "%s", cbSourceName(cb)) == -1) {
		return 0;
	    }
	    return pathbuf;
	} else {
	    return 0;
	}
    }

    /* parse the package name */
    p = cbName(cb);
    if (strlen(p) > NAMEBUFLEN - 1) {
	return 0;
    }
    for (q = lp = nm ; *p ; p++) {
	if (*p == DIR_SEPARATOR) {
	    *q++ = LOCAL_DIR_SEPARATOR;
	    lp = q;
	} else {
	    *q++ = *p;
	}
    }

    /* append the source file name */
    p = cbSourceName(cb);
    if (strlen(p) + (lp - nm) > NAMEBUFLEN - 1) {
	return 0;
    }
    for (; *p ; p++) {
	*lp++ = (*p == DIR_SEPARATOR ? LOCAL_DIR_SEPARATOR : *p);
    }
    *lp = '\0';

    /* search the class path */
    for (cpp = sysGetClassPath() ; cpp && *cpp != 0 ; cpp++) {
	cpe_t *cpe = *cpp;
	if (cpe->type == CPE_DIR) {
	    if (jio_snprintf(pathbuf, maxlen, "%s%c%s",
			 cpe->u.dir, LOCAL_DIR_SEPARATOR, nm) == -1) {
		return 0;
	    }
	    if (sysStat(pathbuf, s) == 0) {
		return pathbuf;
	    }
	}
    }
    return 0;
}

struct CICmallocs {
    struct CICmallocs *next;
    void * alignment_padding;

/* Whatever follows will be 8-byte aligned, if the structure itself is 
   8-byte aligned. */
};

typedef struct CICmallocs CICmallocs;

struct CICcontext {
    unsigned char *ptr;
    unsigned char *end_ptr;
    ClassClass *cb;
    jmp_buf jump_buffer;
    char **detail;

    int pass;         /* two passes, 1 or 2 */
    int malloc_size;  /* space needed for everything other than <clinit> */
    int clinit_size;  /* space needed for the <clinit> method */
    int in_clinit;    /* whether we are load the <clinit> method */

    struct {
        CICmallocs *mallocs; /* list of memory blocks used in the first pass */
    } pass1;
	  
    struct {
        char *malloc_buffer; /* used to hold everything other than <clinit> */ 
        char *malloc_ptr;    /* current point of allocation */

        char *clinit_buffer; /* used to hold the <clinit> method */
        char *clinit_ptr;    /* current point of allocation */
    } pass2;
};

typedef struct CICcontext CICcontext;

static char *getAsciz(CICcontext *);
static char *getAscizFromClass(CICcontext *, int i);

static unsigned char get1byte(CICcontext *);
static unsigned short get2bytes(CICcontext *);
static unsigned long get4bytes(CICcontext *);
static void getNbytes(CICcontext *, int count, char *buffer);
static void *allocNBytes(CICcontext *, int size);
static void freeBuffers(CICcontext *);

static void LoadConstantPool(CICcontext *);
static void ReadInCode(CICcontext *, struct methodblock *);
static void ReadLineTable(CICcontext *, struct methodblock *mb);
static void ReadExceptions(CICcontext *, struct methodblock *);
#ifdef JCOV
static void ReadCovTable(CICcontext *, struct methodblock *mb);
#endif
static void ReadLocalVars(CICcontext *, struct methodblock *mb);
static void InitializeStaticVar(struct fieldblock *fb, CICcontext *context);

static void
createInternalClass0(CICcontext *context, ClassClass *cb, 
		     struct Hjava_lang_ClassLoader *loader, char *name);

bool_t
createInternalClass1(unsigned char *ptr, unsigned char *end_ptr, 
		     ClassClass *cb, struct Hjava_lang_ClassLoader *loader, 
		     char *name, char **detail);

bool_t
createInternalClass(unsigned char *ptr, unsigned char *end_ptr, 
		    ClassClass *cb, struct Hjava_lang_ClassLoader *loader, 
		    char *name, char **detail)
{
    bool_t res;
    res = createInternalClass1(ptr, end_ptr, cb, loader, name, detail);

    return res;
}

void JAVA_ERROR(CICcontext *context, char *name) {
	printCurrentClassName();
    *(context->detail) = name;
    EE()->class_loading_msg = name;
	fprintf(stderr, "Class loading error: %s\n", name);
	exit(1);
}

/* Create an internal class file from the indicated data.  It should be
 * in a buffer for which the first byte is at *ptr and the last byte is just
 * before *end_ptr.
 * The class's classloader is indicated by the classloader argument.
 *
 * We go through the buffer twice. In the first pass we determine the amount
 * of storage needed by the class. We then allocate a single chunk of memory,
 * free the temporary storage, and load from the buffer for the second time.
 *
 * Since all storage needed by the class initialization method <clinit>
 * can be freed after the class is loaded, we count the <clinit> space needs
 * separately and store the <clinit> method in a separate chunk of memory.
 */

bool_t
createInternalClass1(unsigned char *ptr, unsigned char *end_ptr, 
		     ClassClass *cb, struct Hjava_lang_ClassLoader *loader, 
		     char *name, char **detail)
{
    struct CICcontext context_block;
    struct CICcontext *context = &context_block;
    struct Classjava_lang_Class *ucb = unhand(cb);

    /* Set up the context */
    context->ptr = ptr;
    context->end_ptr = end_ptr;
    context->cb = cb;
    context->detail = detail;

    if (setjmp(context->jump_buffer)) {
	/* We've gotten an error of some sort */
        /* See comments below about zeroing these two fields before
	 * freeing the temporary buffer.
         */
        cbConstantPool(cb) = NULL;
        cbFields(cb) = NULL;
	/* Zero out the method out so that freeClass will not try
	   to free the clinit method */
	cbMethodsCount(cb) = 0;

	freeBuffers(context);
	return FALSE;
    }

    context->in_clinit = 0;
    context->pass1.mallocs = 0;
    context->malloc_size = 0;
    context->clinit_size = 0;

    /* The first pass allows us to uncover any class format errors
       and find out the size of the buffer needed. */
    context->pass = 1;
    createInternalClass0(context, cb, loader, name);

    /* We must set the following two fields to zero before we free
     * the temporary buffers, because markClassClass may scan a
     * partially constructed class block in the second pass.
     * If these two fields are set to zero, markClassClass will
     * not scan the constant pool and field blocks, which may
     * point to freed memory.
     */
    cbConstantPool(cb) = NULL;
    cbFields(cb) = NULL;
    /* Zero out the method out so that freeClass will not try
       to free the clinit method */
    cbMethodsCount(cb) = 0;
    freeBuffers(context);

    context->ptr = ptr;   /* rewind the raw class data */

    context->pass2.malloc_buffer = sysCalloc(1, context->malloc_size);
    if (context->pass2.malloc_buffer == 0)
        JAVA_ERROR(context, "out of memory");

    if (context->clinit_size) {
        context->pass2.clinit_buffer = sysCalloc(1, context->clinit_size);
	if (context->pass2.clinit_buffer == 0) {
            sysFree(context->pass2.malloc_buffer);
	    JAVA_ERROR(context, "out of memory");
	}
    }

    context->pass2.malloc_ptr = context->pass2.malloc_buffer;
    context->pass2.clinit_ptr = context->pass2.clinit_buffer;

    /* The second pass accomplishes the real task. */
    context->pass = 2;
    createInternalClass0(context, cb, loader, name);

    /* Valid class - let's put it in the class table. */
    AddBinClass(cb);
    return TRUE;
}

static void
createInternalClass0(CICcontext *context, ClassClass *cb, 
		     struct Hjava_lang_ClassLoader *loader, char *name)
{
    int i, j;
    union cp_item_type *constant_pool;
    unsigned char *type_table;
    int attribute_count;
    unsigned fields_count;
    struct methodblock *mb;
    struct fieldblock *fb;
    struct Classjava_lang_Class *ucb = unhand(cb);
    
    if (get4bytes(context) != JAVA_CLASSFILE_MAGIC) 
	JAVA_ERROR(context, "Bad magic number");

    ucb->minor_version = get2bytes(context);
    ucb->major_version = get2bytes(context);
    ucb->loader = loader;
    if (ucb->major_version != JAVA_VERSION) 
	JAVA_ERROR(context, "Bad major version number");

    LoadConstantPool(context);
    constant_pool = ucb->constantpool;
    type_table = constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type;

    ucb->access = get2bytes(context) & ACC_WRITTEN_FLAGS;

	/* Work around javac bug */
	if (ucb->access & ACC_INTERFACE) {
		ucb->access |= ACC_ABSTRACT;
	}


    /* Get the name of the class */
    i = get2bytes(context);	/* index in constant pool of class */
    ucb->name = getAscizFromClass(context, i);
    if (name != NULL && strcmp(ucb->name, name) != 0)
	JAVA_ERROR(context, "Wrong name");
    constant_pool[i].clazz = cb;
    CONSTANT_POOL_TYPE_TABLE_SET_RESOLVED(type_table, i);

    if (loader) { 
	/* We don't trust a classloader to do the right thing. . . */
	ClassClass **pcb, **end_pcb;
	char *name = ucb->name;
	if (name == NULL || !IsLegalClassname(name, FALSE)) {
	    JAVA_ERROR(context, "Bad name");
	}
	BINCLASS_LOCK();
	for (pcb = binclasses, end_pcb = pcb + nbinclasses; 
	         pcb < end_pcb; pcb++) { 
	    ClassClass *cb = *pcb;
	    if ((cbLoader(cb) == loader) && (strcmp(name, cbName(cb)) == 0))
	        break;
	}
	BINCLASS_UNLOCK();
	if (pcb < end_pcb)
	    /* There's already a class with the same name and loader */
	    JAVA_ERROR(context, "Duplicate name");
    }

    /* Get the super class name. */
    i = get2bytes(context);	/* index in constant pool of class */
    if (i > 0) {
	ucb->super_name = getAscizFromClass(context, i);
	if (!IsLegalClassname(ucb->super_name, FALSE)) { 
	    JAVA_ERROR(context, "Bad superclass name");
	}

    } 

    i = ucb->implements_count = get2bytes(context);
    if (i > 0) {
	int j;
	ucb->implements = allocNBytes(context, i * sizeof(short));
	for (j = 0; j < i; j++) {
	    ucb->implements[j] = get2bytes(context);
	}
    }

    fields_count = ucb->fields_count = get2bytes(context);
    if (fields_count > 0) 
        ucb->fields = (struct fieldblock *)
	    allocNBytes(context, ucb->fields_count * sizeof(struct fieldblock));
    for (i = fields_count, fb = ucb->fields; --i >= 0; fb++) {
	fieldclass(fb) = cb;
	fb->access = get2bytes(context) & ACC_WRITTEN_FLAGS;
	fb->name = getAsciz(context);
	fb->signature = getAsciz(context);
	attribute_count = get2bytes(context);
	for (j = 0; j < (int)attribute_count; j++) {
	    char *name = getAsciz(context);
	    int length = get4bytes(context);
	    if ((fb->access & ACC_STATIC) 
		 && strcmp(name, "ConstantValue") == 0) {
		if (length != 2) 
		    JAVA_ERROR(context, "Wrong size for VALUE attribute");
		fb->access |= ACC_VALKNOWN;
		/* we'll change this below */
		fb->u.offset = get2bytes(context); 
	    } else {
		getNbytes(context, length, NULL);
	    }
	}
	/*
	if (fb->access & ACC_STATIC) {
	    InitializeStaticVar(fb, context);
	}
	*/
    }

    if ((ucb->methods_count = get2bytes(context)) > 0) 
	ucb->methods = (struct methodblock *)
	  allocNBytes(context, ucb->methods_count * sizeof(struct methodblock));
    for (i = cbMethodsCount(cb), mb = cbMethods(cb); --i >= 0; mb++) {
	fieldclass(&mb->fb) = cb;
	mb->fb.access = get2bytes(context) & ACC_WRITTEN_FLAGS;
	mb->fb.name = getAsciz(context);
	mb->fb.signature = getAsciz(context);

	if (strcmp(mb->fb.name, "<clinit>") == 0 &&
	    strcmp(mb->fb.signature, "()V") == 0)
	    context->in_clinit = TRUE;

	mb->args_size = Signature2ArgsSize(mb->fb.signature) 
	                + ((mb->fb.access & ACC_STATIC) ? 0 : 1);
	if (mb->args_size > 255)
	    JAVA_ERROR(context, "Too many arguments");
	attribute_count = get2bytes(context);
	for (j = 0; j < attribute_count; j++) {
	    char *attr_name = getAsciz(context);
            if ((strcmp(attr_name, "Code") == 0) 
		       && ((mb->fb.access & (ACC_NATIVE | ACC_ABSTRACT))==0)) {
		ReadInCode(context, mb);
	    } else if (strcmp(attr_name, "Exceptions") == 0) {
		ReadExceptions(context, mb);
	    } else {
		int length = get4bytes(context);
		getNbytes(context, length, NULL);
	    }
	}
	context->in_clinit = FALSE;
    }

    /* See if there are class attributes */
    attribute_count = get2bytes(context); 
    for (j = 0; j < attribute_count; j++) {
	char *name = getAsciz(context);
	int length = get4bytes(context);
	if (strcmp(name, "SourceFile") == 0) {
	    if (length != 2) {
		JAVA_ERROR(context, "Wrong size for VALUE attribute");
	    }
	    ucb->source_name = getAsciz(context);
#ifdef JCOV
	} else if (strcmp(name, "AbsoluteSourcePath") == 0) {
	    if (length == 2) {
	    	ucb->absolute_source_name = getAsciz(context);
	    } else
		getNbytes(context, length, NULL);
	} else if (strcmp(name, "TimeStamp") == 0) {
	    unsigned high;
	    unsigned low;
	    int64_t  value;
	    Java8    t1;

	    if (length == 8) {
		high = get4bytes(context);
		low  = get4bytes(context);
	    	value = ll_add(ll_shl(uint2ll(high), 32), uint2ll(low));
	    	SET_INT64(t1, &ucb->timestamp, value);
	    } else 
	    	getNbytes(context, length, NULL);
#endif
	} else {
	    getNbytes(context, length, NULL);
	}
    }
}

/* Create a fake array class that has the specified fields */

ClassClass *
createFakeArrayClass(char *name,           /* name */
		     int base_type,        /* base_type */
		     int depth,            /* array dimension */
		     ClassClass *inner_cb, /* base type if T_CLASS */
		     struct Hjava_lang_ClassLoader *loader)
{ 
    ClassClass *cb = allocClassClass();
    Classjava_lang_Class *ucb = unhand(cb);
    cp_item_type *constant_pool = 
	sysCalloc(CONSTANT_POOL_ARRAY_LENGTH, 
		  (sizeof(cp_item_type) + sizeof(unsigned char)));
    unsigned char *type_table = (unsigned char *)
        (constant_pool + CONSTANT_POOL_ARRAY_LENGTH);
    sysAssert(name[0] == SIGNATURE_ARRAY);
    ucb->major_version = JAVA_VERSION;
    ucb->minor_version = JAVA_MINOR_VERSION;
    ucb->name = strdup(name);
    ucb->super_name = JAVAPKG "Object";
    ucb->constantpool = constant_pool;
    ucb->constantpool_count = CONSTANT_POOL_ARRAY_LENGTH;
    ucb->loader = loader;

    constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type = type_table;
    constant_pool[CONSTANT_POOL_ARRAY_DEPTH_INDEX].i = depth;     
    constant_pool[CONSTANT_POOL_ARRAY_TYPE_INDEX].i = base_type;
    type_table[CONSTANT_POOL_ARRAY_DEPTH_INDEX] = 
	CONSTANT_Integer | CONSTANT_POOL_ENTRY_RESOLVED;
    type_table[CONSTANT_POOL_ARRAY_TYPE_INDEX] = 
	CONSTANT_Integer | CONSTANT_POOL_ENTRY_RESOLVED;
	        
    if (base_type == T_CLASS) {
	/* Initialize the appropriate fields of the constant pool */
	constant_pool[CONSTANT_POOL_ARRAY_CLASS_INDEX].clazz = inner_cb;
	type_table[CONSTANT_POOL_ARRAY_CLASS_INDEX] = 
	    CONSTANT_Class | CONSTANT_POOL_ENTRY_RESOLVED;
	/* The class is public iff its base class is public */
	ucb->access = ACC_FINAL | ACC_ABSTRACT | (cbAccess(inner_cb) & ACC_PUBLIC);
    } else {
	/* Set the class field to something innocuous */
	type_table[CONSTANT_POOL_ARRAY_CLASS_INDEX] = 
	    CONSTANT_Integer | CONSTANT_POOL_ENTRY_RESOLVED;
	ucb->access = ACC_FINAL | ACC_ABSTRACT | ACC_PUBLIC;
    }
    AddBinClass(cb);
    return cb;
}

/*
 * Create a classblock to represent a primitive type.
 * NOTE: We don't add it to the builtin class table, so this should be
 * called only once per primitive type.
 * See FindPrimitiveClass().
 */
ClassClass *
createPrimitiveClass(char *name, char sig, unsigned char typecode,
    unsigned char slotsize, unsigned char elementsize)
{
    ClassClass *cb = allocClassClass();
    Classjava_lang_Class *ucb = unhand(cb);

    ucb->major_version = JAVA_VERSION;
    ucb->minor_version = JAVA_MINOR_VERSION;
    ucb->name = strdup(name);
    ucb->super_name = JAVAPKG "Object";
    ucb->constantpool = NULL;
    ucb->constantpool_count = 0;
    ucb->loader = NULL;
    ucb->access = ACC_FINAL | ACC_ABSTRACT | ACC_PUBLIC;

    CCSet(cb, Primitive);
    cbTypeSig(cb) = sig;
    cbTypeCode(cb) = typecode;
    cbSlotSize(cb) = slotsize;
    cbElementSize(cb) = elementsize;
    MakeClassSticky(cb);

    return cb;
}

/* Initialize the static variables in a class structure */

static void InitializeStaticVar(struct fieldblock *fb, CICcontext *context) {
    Java8 t1, t2;
    char isig = fb->signature[0];
    t1.x[0] = t2.x[0] = 0; /* shut off "not used" warning */
    if (fb->access & ACC_VALKNOWN) { 
	ClassClass *cb = context->cb;
	int index = fb->u.offset; /* set by "VALUE" attribute */
	union cp_item_type *constant_pool = cbConstantPool(cb);
	unsigned char *type_table = 
	      constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type;

	if (index <= 0 || index >= cbConstantPoolCount(cb)) 
	    JAVA_ERROR(context, "Bad initial value");
	switch (isig) { 
	    case SIGNATURE_DOUBLE: 
		if (type_table[index] != 
		    (CONSTANT_Double | CONSTANT_POOL_ENTRY_RESOLVED))
		    JAVA_ERROR(context, "Bad index into constant pool");
		twoword_static_address(fb) = 
		    allocNBytes(context, 2 * sizeof(long));
		SET_DOUBLE(t1, twoword_static_address(fb),
			   GET_DOUBLE(t2, &constant_pool[index]));
	    break;

	    case SIGNATURE_LONG: 
		if (type_table[index] != 
		    (CONSTANT_Long | CONSTANT_POOL_ENTRY_RESOLVED))
		    JAVA_ERROR(context, "Bad index into constant pool");
		twoword_static_address(fb) = 
		    allocNBytes(context, 2 * sizeof(long));
		SET_INT64(t1, twoword_static_address(fb),
			  GET_INT64(t2, &constant_pool[index]));
		break;

	    case SIGNATURE_CLASS:
		break;

	    case SIGNATURE_BYTE: 
		if (type_table[index] != 
		    (CONSTANT_Integer | CONSTANT_POOL_ENTRY_RESOLVED))
		    JAVA_ERROR(context, "Bad index into constant pool");
		*(int *)normal_static_address(fb) = 
		        (signed char)(constant_pool[index].i);
		break;

	    case SIGNATURE_CHAR:
		if (type_table[index] != 
		    (CONSTANT_Integer | CONSTANT_POOL_ENTRY_RESOLVED))
		    JAVA_ERROR(context, "Bad index into constant pool");
		*(int *)normal_static_address(fb) = 
		        (unsigned short)(constant_pool[index].i);
		break;

	    case SIGNATURE_SHORT:
		if (type_table[index] != 
		    (CONSTANT_Integer | CONSTANT_POOL_ENTRY_RESOLVED))
		    JAVA_ERROR(context, "Bad index into constant pool");
		*(int *)normal_static_address(fb) = 
		        (signed short)(constant_pool[index].i);
		break;
	    case SIGNATURE_BOOLEAN:
		if (type_table[index] != 
		    (CONSTANT_Integer | CONSTANT_POOL_ENTRY_RESOLVED))
		    JAVA_ERROR(context, "Bad index into constant pool");
		*(int *)normal_static_address(fb) = 
		       (constant_pool[index].i != 0);
		break;

	    case SIGNATURE_INT:  
		if (type_table[index] != 
		    (CONSTANT_Integer | CONSTANT_POOL_ENTRY_RESOLVED))
		    JAVA_ERROR(context, "Bad index into constant pool");
		*(int *)normal_static_address(fb) = constant_pool[index].i;
		break;
		
	    case SIGNATURE_FLOAT:
		if (type_table[index] != 
		        (CONSTANT_Float | CONSTANT_POOL_ENTRY_RESOLVED))
		    JAVA_ERROR(context, "Bad index into constant pool");
		*(float *)normal_static_address(fb) = constant_pool[index].f;
		break;
			
	    default: 
		JAVA_ERROR(context, "Unable to set initial value");
	}
    } else { 
	switch (isig) { 
	    default:
		*(int *)normal_static_address(fb) = 0;
		break;

	    case SIGNATURE_FLOAT:
		*(float *)normal_static_address(fb) = 0.0;
		break;

	    case SIGNATURE_LONG: 
		twoword_static_address(fb) = 
		    allocNBytes(context, 2 * sizeof(long));
		SET_INT64(t1, twoword_static_address(fb), ll_zero_const);
		break;

	    case SIGNATURE_DOUBLE: 
		twoword_static_address(fb) = 
		    allocNBytes(context, 2 * sizeof(long));
		SET_DOUBLE(t1, twoword_static_address(fb), 0.0);
		break;

	    case SIGNATURE_CLASS:
		*normal_static_address(fb) = 0;
		break;
	}
    }
}




static void LoadConstantPool(CICcontext *context) 
{
    ClassClass *cb = context->cb;
    int nconstants = get2bytes(context);
    cp_item_type *constant_pool;
    unsigned char *type_table;
    int i;
    Java8 t1;
    
	if (nconstants > 16384) {
		JAVA_ERROR(context, "Preverifier only "
				   "handles constant pool size up to 16K");
	}

    t1.x[0] = 0; /* shut off warning */
    if (nconstants < CONSTANT_POOL_UNUSED_INDEX) {
	JAVA_ERROR(context, "Illegal constant pool size");
    }
    
    constant_pool = (cp_item_type *)
        allocNBytes(context, nconstants * sizeof(cp_item_type));
    type_table = allocNBytes(context, nconstants * sizeof(char));

    for (i = CONSTANT_POOL_UNUSED_INDEX; i < nconstants; i++) {
	int type = get1byte(context);
	CONSTANT_POOL_TYPE_TABLE_PUT(type_table, i, type);
	switch (type) {
	  case CONSTANT_Utf8: {
	      int length = get2bytes(context);
	      char *result = allocNBytes(context, length + 1);
	      getNbytes(context, length, result);
	      result[length] = '\0';
	      constant_pool[i].cp = result;
	      CONSTANT_POOL_TYPE_TABLE_SET_RESOLVED(type_table, i);
	      break;
	  }
	
	  case CONSTANT_Class:
	  case CONSTANT_String:
	    constant_pool[i].i = get2bytes(context);
	    break;

	  case CONSTANT_Fieldref:
	  case CONSTANT_Methodref:
	  case CONSTANT_InterfaceMethodref:
	  case CONSTANT_NameAndType:
	    constant_pool[i].i = get4bytes(context);
	    break;
	    
	  case CONSTANT_Float:
	      if (check_on) {
		  panic("floating-point constants should not appear");
	      }
	  case CONSTANT_Integer:
	    constant_pool[i].i = get4bytes(context);
	    CONSTANT_POOL_TYPE_TABLE_SET_RESOLVED(type_table, i);
	    break;

	  case CONSTANT_Double: 
	      if (check_on) {
		  panic("floating-point constants should not appear");
	      }
	  case CONSTANT_Long: {
	      unsigned high = get4bytes(context);
	      unsigned low = get4bytes(context);
	      int64_t value;
	      value = ll_add(ll_shl(uint2ll(high), 32), uint2ll(low));
	      SET_INT64(t1, &constant_pool[i], value);
	      CONSTANT_POOL_TYPE_TABLE_SET_RESOLVED(type_table, i);
	      i++;		/* increment i for loop, too */
	      if (i >= nconstants) {
		JAVA_ERROR(context, "illegal constant pool entry");
	      }
	      /* Indicate that the next object in the constant pool cannot
	       * be accessed independently.    */
	      CONSTANT_POOL_TYPE_TABLE_PUT(type_table, i, 0);
	      CONSTANT_POOL_TYPE_TABLE_SET_RESOLVED(type_table, i);
	      break;
	  }

	  default:
	      JAVA_ERROR(context, "Illegal constant pool type");
	}
    }
    /* It is important to only set these after everything is setup,
       so that the GC sees a consistent state.*/
    cbConstantPool(cb) = constant_pool;
    cbConstantPoolCount(cb) = nconstants;
    constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type = type_table;
}



static void ReadInCode(CICcontext *context, struct methodblock *mb)
{
    int attribute_length = get4bytes(context);
    unsigned char *end_ptr = context->ptr + attribute_length;
    int attribute_count;
    int code_length;
    int i;

    if (cbMinorVersion(context->cb) <= 2) { 
	mb->maxstack = get1byte(context);
	mb->nlocals = get1byte(context);
	code_length = mb->code_length = get2bytes(context);
    } else { 
	mb->maxstack = get2bytes(context);
	mb->nlocals = get2bytes(context);
	code_length = mb->code_length = get4bytes(context);
    }
    if (mb->nlocals < mb->args_size)
        JAVA_ERROR(context, "Arguments can't fit into locals");

    if (code_length > 65535) {
	JAVA_ERROR(context, "Byte code size exceeds 65535 bytes");
    }

    mb->code = allocNBytes(context, code_length);

    getNbytes(context, code_length, (char *)mb->code);
    if ((mb->exception_table_length = get2bytes(context)) > 0) {
	unsigned exception_table_size = mb->exception_table_length 
	                                  * sizeof(struct CatchFrame);
	mb->exception_table = allocNBytes(context, exception_table_size);
	for (i = 0; i < (int)mb->exception_table_length; i++) {
	    mb->exception_table[i].start_pc = get2bytes(context);
	    mb->exception_table[i].end_pc = get2bytes(context);		
	    mb->exception_table[i].handler_pc = get2bytes(context); 
	    mb->exception_table[i].catchType = get2bytes(context);
            mb->exception_table[i].compiled_CatchFrame = NULL;
	}
    }
    attribute_count = get2bytes(context);
    for (i = 0; i < attribute_count; i++) {
	char *name = getAsciz(context);
	if (strcmp(name, "LineNumberTable") == 0) {
	    ReadLineTable(context, mb);
#ifdef JCOV
	} else if (strcmp(name, "CoverageTable") == 0 && cov_file) {
	    ReadCovTable(context, mb); 
#endif
	} else if (strcmp(name, "LocalVariableTable") == 0) {
	    ReadLocalVars(context, mb);
	} else {
	    int length = get4bytes(context);
	    getNbytes(context, length, NULL);
	}
    }
    if (context->ptr != end_ptr) 
	JAVA_ERROR(context, "Code segment was wrong length");
}


static void ReadLineTable(CICcontext *context, struct methodblock *mb)
{
    int attribute_length = get4bytes(context);
    unsigned char *end_ptr = context->ptr  + attribute_length;
    int i;
    if ((mb->line_number_table_length = get2bytes(context)) > 0) {
	struct lineno *ln = 
	    allocNBytes(context, mb->line_number_table_length * 
			sizeof(struct lineno));
	mb->line_number_table = ln;
	for (i = mb->line_number_table_length; --i >= 0; ln++) {
	    ln->pc = get2bytes(context);
	    ln->line_number = get2bytes(context);
	}
    }
    if (context->ptr != end_ptr)
	JAVA_ERROR(context, "Line number table was wrong length?");
}	
	
#ifdef JCOV
static void ReadCovTable(CICcontext *context, struct methodblock *mb)
{
    int attribute_length = get4bytes(context);
    unsigned char *end_ptr = context->ptr  + attribute_length;
    int old_clinit, i;

    old_clinit = context->in_clinit;		
    context->in_clinit = FALSE;		/* Since all storage needed by the class initialization method  
					 * <clinit> can be freed after the class is loaded
					 * we should not place Jcov data into this memory
				         */

    if ((mb->coverage_table_length = get2bytes(context)) > 0) {
	int sz = mb->coverage_table_length * sizeof(struct covtable)
                         + mb->code_length * sizeof(long);
	struct covtable *cv = allocNBytes(context, sz);
        long *hash;
	mb->coverage_table = cv;
        hash = (long*)(mb->coverage_table + mb->coverage_table_length);
	memset(hash, 0, mb->code_length*sizeof(long)); 
	for (i = 0; i < mb->coverage_table_length; cv++) {
	    cv->pc = get2bytes(context);
            *(hash+cv->pc) = i + 1;
	    cv->type = get2bytes(context);
	    cv->where = get4bytes(context);
	    cv->count = 0;
            i++;
	}
    }
    if (context->ptr != end_ptr)
	mb->coverage_table = NULL;	/* Don't use a wrong coverage table */

    context->in_clinit = old_clinit;
}	
#endif 

static void ReadLocalVars(CICcontext *context, struct methodblock *mb)
{
    int attribute_length = get4bytes(context);
    unsigned char *end_ptr = context->ptr  + attribute_length;
    int i;
    if ((mb->localvar_table_length = get2bytes(context)) > 0) {
	struct localvar *lv = 
	  allocNBytes(context, mb->localvar_table_length * 
		      sizeof(struct localvar));
	mb->localvar_table = lv;
	for (i = mb->localvar_table_length; --i >= 0; lv++) {
	    lv->pc0 = get2bytes(context);
	    lv->length = get2bytes(context);
	    lv->nameoff = get2bytes(context);
	    lv->sigoff = get2bytes(context);
	    lv->slot = get2bytes(context);
	}
    }
    if (context->ptr != end_ptr)
	JAVA_ERROR(context, "Local variables table was wrong length?");
}	

static void
ReadExceptions(CICcontext *context, struct methodblock *mb)
{
    int attribute_length = get4bytes(context);
    unsigned char *end_ptr = context->ptr + attribute_length;
    unsigned short nexceptions = get2bytes(context);

    if ((mb->nexceptions = nexceptions) > 0) {
	unsigned short *ep, *exceptions =
	    allocNBytes(context, nexceptions * sizeof (unsigned short));
	mb->exceptions = ep = exceptions;
	while (nexceptions-- > 0) {
	    *ep++ = get2bytes(context);
	}
    }
    if (context->ptr != end_ptr)
	JAVA_ERROR(context, "Exceptions attribute has wrong length");
}

unsigned Signature2ArgsSize(char *method_signature)
{
    char *p;
    int args_size = 0;
    for (p = method_signature; *p != SIGNATURE_ENDFUNC; p++) {
	switch (*p) {
	  case SIGNATURE_FLOAT:
	      if (check_on) {
		  panic("floating-point descriptors should not appear");
	      }
	  case SIGNATURE_BOOLEAN:
	  case SIGNATURE_BYTE:
	  case SIGNATURE_CHAR:
	  case SIGNATURE_SHORT:
	  case SIGNATURE_INT:
	    args_size += 1;
	    break;
	  case SIGNATURE_CLASS:
	    args_size += 1;
	    while (*p != SIGNATURE_ENDCLASS) p++;
	    break;
	  case SIGNATURE_ARRAY:
	    args_size += 1;
	    while ((*p == SIGNATURE_ARRAY)) p++;
	    /* If an array of classes, skip over class name, too. */
	    if (*p == SIGNATURE_CLASS) { 
		while (*p != SIGNATURE_ENDCLASS) 
		  p++;
	    } 
	    break;
	  case SIGNATURE_DOUBLE:
	      if (check_on) {
		  panic("floating-point descriptors should not appear");
	      }
	  case SIGNATURE_LONG:
	    args_size += 2;
	    break;
	  case SIGNATURE_FUNC:	/* ignore initial (, if given */
	    break;
	  default:
	    /* Indicate an error. */
	    return 0;
	}
    }
    return args_size;
}

void free_clinit_memory(struct methodblock *mb)
{
    /* This function is somewhat a hack. It fixes the problem in 1.1.3
     * and before. sysFree may be called on the wrong memory block if
     * the exception attribute comes before the code attribute.
     */
    /* If there is no exceptions attribute, or if both has already
     * been freed. */
    if (mb->exceptions == NULL) {
        if (mb->code) {
	    sysFree(mb->code);
	    mb->code = NULL;
	}
	return;
    }

    /* If both attributes exist, free the one at the lower address */
    if ((char *)mb->code < (char *)mb->exceptions)
        sysFree(mb->code);
    else
        sysFree(mb->exceptions);

    mb->code = NULL;
    mb->exceptions = NULL;
}

void FreeClass(ClassClass *cb) 
{
    int i;
    struct methodblock *mb;

    for (i = cbMethodsCount(cb), mb = cbMethods(cb); --i >= 0; mb++) {
        if (strcmp(mb->fb.name, "<clinit>") == 0 &&
	    strcmp(mb->fb.signature, "()V") == 0 &&
	    mb->code_length /* not external */ )
	    free_clinit_memory(mb);
    }

    sysFree(cbConstantPool(cb));

    sysFree(cbMethodTableMem(cb));
    sysFree(cbSlotTable(cb));
    /* Interface method tables can be shared between child and super classes
     */
    if (cbImplementsCount(cb) != 0 || cbIsInterface(cb))
        sysFree(cbIntfMethodTable(cb));
}
	
static unsigned char get1byte(CICcontext *context)
{
    unsigned char *ptr = context->ptr;
    if (context->end_ptr - ptr < 1) {
	JAVA_ERROR(context, "Truncated class file");
	return 0;
    } else {
	unsigned char *ptr = context->ptr;
	unsigned char value = ptr[0];
	(context->ptr) += 1;
	return value;
    }
}

static unsigned short get2bytes(CICcontext *context)
{
    unsigned char *ptr = context->ptr;
    if (context->end_ptr - ptr < 2) {
	JAVA_ERROR(context, "Truncated class file");
	return 0;
    } else {
	unsigned short value = (ptr[0] << 8) + ptr[1];
	(context->ptr) += 2;
	return value;
    }
}


static unsigned long get4bytes(CICcontext *context)
{
    unsigned char *ptr = context->ptr;
    if (context->end_ptr - ptr < 4) {
	JAVA_ERROR(context, "Truncated class file");
	return 0;
    } else {
	unsigned long value = (ptr[0] << 24) + (ptr[1] << 16) + 
	                             (ptr[2] << 8) + ptr[3];
	(context->ptr) += 4;
	return value;
    }
}

static void getNbytes(CICcontext *context, int count, char *buffer) 
{
    unsigned char *ptr = context->ptr;
    if (context->end_ptr - ptr < count)
	JAVA_ERROR(context, "Truncated class file");
    if (buffer != NULL) 
	memcpy(buffer, ptr, count);
    (context->ptr) += count;
}


static char *getAsciz(CICcontext *context) 
{
    ClassClass *cb = context->cb;
    union cp_item_type *constant_pool = cbConstantPool(cb);
    int nconstants = cbConstantPoolCount(cb);
    unsigned char *type_table = 
	constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type;

    int value = get2bytes(context);
    if ((value == 0) || (value >= nconstants) || 
	   type_table[value] != (CONSTANT_Utf8 | CONSTANT_POOL_ENTRY_RESOLVED))
	JAVA_ERROR(context, "Illegal constant pool index");
    return constant_pool[value].cp;
}

static char *getAscizFromClass(CICcontext *context, int value) 
{
    ClassClass *cb = context->cb;
    union cp_item_type *constant_pool = cbConstantPool(cb);
    int nconstants = cbConstantPoolCount(cb);
    unsigned char *type_table = 
	constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type;
    if ((value > 0) && (value < nconstants)) {
	if (type_table[value] == CONSTANT_Class) {
	    value = constant_pool[value].i;
	    if ((value <= 0) || (value >= nconstants) ||
		(type_table[value] != (CONSTANT_Utf8 |
				       CONSTANT_POOL_ENTRY_RESOLVED)))
		JAVA_ERROR(context, "Illegal constant pool index");
	    return constant_pool[value].cp;
	} else if (type_table[value] == (CONSTANT_Class |
		                         CONSTANT_POOL_ENTRY_RESOLVED)) {
	    ClassClass *cb = constant_pool[value].clazz;
	    return cbName(cb);
	} else {
	    JAVA_ERROR(context, "Illegal constant pool index");
	}
    } else {
	JAVA_ERROR(context, "Illegal constant pool index");
    }
    return NULL; /* not reached */
}

/* To be avoid possible alignment errors, round up all sizes to multiples
   of eight.
*/
#define ROUNDUP_SIZE(s) while ((s) % 8 != 0) (s)++

static void *allocNBytes(CICcontext *context, int size)
{
    void *result;
    if (context->pass == 1) {
        /* The first pass
	 * A more sophisticated scheme could reduce the number of mallocs.
	 */
        CICmallocs *mallocs = 
	  (CICmallocs *)sysCalloc(1, sizeof(CICmallocs) + size);
	if (mallocs == 0)
	    JAVA_ERROR(context, "out of memory");
	result = (void *)(mallocs + 1);
	mallocs->next = context->pass1.mallocs;
	ROUNDUP_SIZE(size);
	if (context->in_clinit)
	    context->clinit_size += size;
	else
	    context->malloc_size += size;
	context->pass1.mallocs = mallocs;
    } else {
        /* The second pass */

#define ALLOC_BLOCK(ptr,buf,sizelimit) \
	result = (ptr); \
	ROUNDUP_SIZE(size); \
	(ptr) += (size); \
	sysAssert((ptr) <= (buf) + (sizelimit))

        if (context->in_clinit) {
            ALLOC_BLOCK(context->pass2.clinit_ptr,
			context->pass2.clinit_buffer,
			context->clinit_size);
	} else {
            ALLOC_BLOCK(context->pass2.malloc_ptr,
			context->pass2.malloc_buffer,
			context->malloc_size);
	}
    }
    return result;
}

static void freeBuffers(CICcontext * context)
{
    if (context->pass == 1) {
        CICmallocs *mallocs = context->pass1.mallocs;
	while (mallocs) {
	    CICmallocs *tmp = mallocs;
	    mallocs = mallocs->next;
	    sysFree(tmp);
	}
	context->pass1.mallocs = 0;
    } else {
        sysFree(context->pass2.malloc_buffer);
        context->pass2.malloc_buffer = 0;
        sysFree(context->pass2.clinit_buffer);
        context->pass2.clinit_buffer = 0;
    }
}

/* In certain cases, we only need a class's name.  We don't want to resolve
 * the class reference if it isn't, already. 
 */

char *GetClassConstantClassName(cp_item_type *constant_pool, int index)
{
    unsigned char *type_table = 
	constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type;
    switch(type_table[index]) {
        case CONSTANT_Class | CONSTANT_POOL_ENTRY_RESOLVED: {
	    ClassClass *cb = constant_pool[index].clazz;
	    return cbName(cb);
	}

        case CONSTANT_Class: {
	    int name_index = constant_pool[index].i;
	    return constant_pool[name_index].cp;
	}
	    
	default:
	    return (char *)0;
    }
}


