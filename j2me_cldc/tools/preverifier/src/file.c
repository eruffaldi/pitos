/*
 * @(#)file.c	1.6 00/04/12
 *
 * Copyright 1995-1998 by Sun Microsystems, Inc.,
 * 901 San Antonio Road, Palo Alto, California, 94303, U.S.A.
 * All rights reserved.
 * 
 * This software is the confidential and proprietary information
 * of Sun Microsystems, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Sun.
 */

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "oobj.h"
#include "tree.h"
#include "sys_api.h"

#ifdef WIN32
#include <direct.h>
#endif

/* initialize opnames[256]; */
#include "opcodes.init"

extern char *progname;

char *current_class_name = NULL;

bool_t stack_map_on = TRUE;

static char *PrintableClassname(char *classname);

char *output_dir = "output";
unsigned char *class_buf;
int class_buf_size = 0;
int class_index;
int last_not_utf8_index;
int n_new_class_entries;
int n_new_const_entries;

#define INIT_CLASS_BUF_SIZE 256

void ensure_capacity(int sz)
{
    if (class_index + sz >= class_buf_size) {
        while (class_index + sz >= class_buf_size) {
	    class_buf_size *= 2;
	}
    }
    class_buf = (unsigned char *)realloc(class_buf, class_buf_size);
}

void write_u1(unsigned long u1)
{
    ensure_capacity(1);
    class_buf[class_index++] = (unsigned char)u1;
}

void write_u2(unsigned long u2)
{
    ensure_capacity(2);
    class_buf[class_index++] = (unsigned char)(u2 >> 8);
    class_buf[class_index++] = (unsigned char)u2;
}

void write_u4(unsigned long u4)
{
    ensure_capacity(4);
    class_buf[class_index++] = (unsigned char)(u4 >> 24);
    class_buf[class_index++] = (unsigned char)(u4 >> 16);
    class_buf[class_index++] = (unsigned char)(u4 >> 8);
    class_buf[class_index++] = (unsigned char)u4;
}

int new_utf8_index(int index)
{
	if (index > last_not_utf8_index) {
		return index + n_new_class_entries;
	} else {
		return index;
	}
}

unsigned long lookup_utf8(ClassClass *cb, char *utf8)
{
    int i;
    cp_item_type *constant_pool = cbConstantPool(cb);
    unsigned char *type_table = 
        constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type;
    for (i = 0; i < cbConstantPoolCount(cb); i++) {
		if (type_table[i] == (CONSTANT_Utf8 | CONSTANT_POOL_ENTRY_RESOLVED) &&
			strcmp(utf8, constant_pool[i].cp) == 0) {
			return new_utf8_index(i);
		}
    }
    panic("utf8 expected to be in constant pool: %s", utf8);
    return 0;
}

unsigned long lookup_class(ClassClass *cb, char *name)
{
    int i;
    cp_item_type *constant_pool = cbConstantPool(cb);
    unsigned char *type_table = 
        constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type;
    for (i = 0; i < cbConstantPoolCount(cb); i++) {
	if (type_table[i] == (CONSTANT_Class | CONSTANT_POOL_ENTRY_RESOLVED) &&
	    strcmp(name, cbName(constant_pool[i].clazz)) == 0) {
	    return i;
	}
	if (type_table[i] == CONSTANT_Class &&
	    strcmp(name, constant_pool[constant_pool[i].i].cp) == 0) {
	    return i;
	}

    }
    panic("class expected to be in constant pool: %s", name);
    return 0;
}

int get_last_not_utf8_index(ClassClass *cb)
{
    int i;
	int result;
    cp_item_type *constant_pool = cbConstantPool(cb);
    unsigned char *type_table = 
        constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type;
	for (i = 0, result = -1; i < cbConstantPoolCount(cb); i++) {
		if (type_table[i] != (CONSTANT_Utf8 | CONSTANT_POOL_ENTRY_RESOLVED)) {
			result = i;
		}
	}
	return result;
}

void write_constant_pool(ClassClass *cb)
{
    int i, j;
    cp_item_type *constant_pool = cbConstantPool(cb);
    unsigned char *type_table = 
        constant_pool[CONSTANT_POOL_TYPE_TABLE_INDEX].type;
	
	last_not_utf8_index = get_last_not_utf8_index(cb);
	if (stack_map_on && unhand(cb)->has_stack_maps) {
		n_new_class_entries = unhand(cb)->n_extra_utf8_entries - 1;
		n_new_const_entries = n_new_class_entries * 2 + 1;
	} else {
		n_new_class_entries = 0;
		n_new_const_entries = 0;
	}

    write_u2(cbConstantPoolCount(cb) + n_new_const_entries);

    for (i = 1; i < cbConstantPoolCount(cb); i++) {
        write_u1(type_table[i] & CONSTANT_POOL_ENTRY_TYPEMASK);
	switch(type_table[i]) {
	  case CONSTANT_Utf8 | CONSTANT_POOL_ENTRY_RESOLVED:
	      write_u2(strlen(constant_pool[i].cp));
	      for (j = 0; j < (int)strlen(constant_pool[i].cp); j++) {
	          write_u1(constant_pool[i].cp[j]);
	      }
	      break;
	
	  case CONSTANT_Class:
	  case CONSTANT_String:
	      write_u2(new_utf8_index(constant_pool[i].i));
	      break;

	  case CONSTANT_Class | CONSTANT_POOL_ENTRY_RESOLVED:
	      write_u2(lookup_utf8(cb, cbName(constant_pool[i].clazz)));
	      break;

	  case CONSTANT_Fieldref:
	  case CONSTANT_Methodref:
	  case CONSTANT_InterfaceMethodref:
	      write_u2(constant_pool[i].i >> 16);
		  write_u2(constant_pool[i].i & 0xFFFF);
	      break;

	  case CONSTANT_NameAndType:
	      write_u2(new_utf8_index(constant_pool[i].i >> 16));
		  write_u2(new_utf8_index(constant_pool[i].i & 0xFFFF));
	      break;
	    
	  case CONSTANT_Integer | CONSTANT_POOL_ENTRY_RESOLVED:
	  case CONSTANT_Float | CONSTANT_POOL_ENTRY_RESOLVED:
	      write_u4(constant_pool[i].i);
	      break;

	  case CONSTANT_Long | CONSTANT_POOL_ENTRY_RESOLVED:
	  case CONSTANT_Double | CONSTANT_POOL_ENTRY_RESOLVED:
#ifdef SOLARIS2
	      write_u4(constant_pool[i].i);
	      write_u4(constant_pool[i + 1].i);
#endif
#ifdef WIN32
	      write_u4(constant_pool[i + 1].i);
      	      write_u4(constant_pool[i].i);
#endif
	      i++;
	      break;

	  default:
	      panic("bad constant pool entry type: %d", type_table[i]);
	}
	if (i == last_not_utf8_index) {
	    for (j = 0; j < n_new_class_entries; j++) {
		write_u1(CONSTANT_Class);
		write_u2(cbConstantPoolCount(cb) +
			 n_new_class_entries + 1 + j);
	    }
	}
    }
    if (stack_map_on && unhand(cb)->has_stack_maps) {
	for (i = 0; i < unhand(cb)->n_extra_utf8_entries; i++) {
            write_u1(CONSTANT_Utf8);
	    write_u2(strlen(unhand(cb)->extra_utf8_entries[i]));
	    for (j = 0; j < (int)strlen(unhand(cb)->extra_utf8_entries[i]);
		 j++) {
	        write_u1(unhand(cb)->extra_utf8_entries[i][j]);
	    }
	}
    }
}

void write_interfaces(ClassClass *cb)
{
    int i;
    write_u2(cbImplementsCount(cb));
    for (i = 0; i < cbImplementsCount(cb); i++) {
	write_u2(cbImplements(cb)[i]);
    }
}

void write_fields(ClassClass *cb)
{
    int i;
    write_u2(cbFieldsCount(cb));
    for (i = 0; i < cbFieldsCount(cb); i++) {
	struct fieldblock *fb = &cbFields(cb)[i];
	write_u2(fb->access & ACC_WRITTEN_FLAGS);
	write_u2(lookup_utf8(cb, fb->name));
	write_u2(lookup_utf8(cb, fb->signature));
	if (fb->access & ACC_VALKNOWN) {
	    write_u2(1);
	    write_u2(lookup_utf8(cb, "ConstantValue"));
	    write_u4(2);
	    write_u2(fb->u.offset);
	} else {
	    write_u2(0);
	}
    }
}

void write_stack_map(int nentries, struct map_entry *entries)
{
    int i;
    for (i = 0; i < nentries; i++) {
		struct map_entry *entry = entries + i;
		write_u1(entry->type);
		if (entry->type == CF_ITEM_NewObject) {
			write_u2(entry->info);
		}
		if (entry->type == CF_ITEM_Object) {
			if (entry->info >= 0) {
				write_u2(entry->info);
			} else {
				write_u2(last_not_utf8_index - entry->info);
			}
		}
    }
}

int stack_map_size(int nentries, struct map_entry *entries)
{
    int i;
    int result = 0;
    for (i = 0; i < nentries; i++) {
	struct map_entry *entry = entries + i;
        result += 1;
	if (entry->type == CF_ITEM_Object || entry->type == CF_ITEM_NewObject) {
	    result += 2;
	}
    }
    return result;
}

void write_methods(ClassClass *cb)
{
    int i;
    write_u2(cbMethodsCount(cb));
    for (i = 0; i < cbMethodsCount(cb); i++) {
	struct methodblock *mb = &cbMethods(cb)[i];
	int n_attrs = 0;
	write_u2(mb->fb.access & ACC_WRITTEN_FLAGS);
	write_u2(lookup_utf8(cb, mb->fb.name));
	write_u2(lookup_utf8(cb, mb->fb.signature));
	if (mb->code_length > 0) {
	    n_attrs++;
	}
	if (mb->nexceptions > 0) {
	    n_attrs++;
	}

	write_u2(n_attrs);
	if (mb->code_length > 0) {
	    int j;
	    int stack_map_attr_length = 0;

	    if (stack_map_on && mb->n_stack_maps > 0) {
		stack_map_attr_length = 8;
		for (j = 0; j < mb->n_stack_maps; j++) {
		    stack_map_attr_length += 6 + 
		        stack_map_size(mb->stack_maps[j].nlocals, mb->stack_maps[j].locals) +
		        stack_map_size(mb->stack_maps[j].nstacks, mb->stack_maps[j].stacks);
		}
	    }

	    write_u2(lookup_utf8(cb, "Code"));
	    write_u4(12 + mb->code_length + mb->exception_table_length * 8 + stack_map_attr_length);
	    write_u2(mb->maxstack);
	    write_u2(mb->nlocals);
	    write_u4(mb->code_length);
	    for (j = 0; j < (int)mb->code_length; j++) {
		write_u1(mb->code[j]);
	    }
	    write_u2(mb->exception_table_length);
	    for (j = 0; j < (int)mb->exception_table_length; j++) {
		write_u2(mb->exception_table[j].start_pc);
		write_u2(mb->exception_table[j].end_pc);
		write_u2(mb->exception_table[j].handler_pc);
		write_u2(mb->exception_table[j].catchType);
	    }
	    if (stack_map_on && mb->n_stack_maps > 0) {
	        write_u2(1);
    		write_u2(cbConstantPoolCount(cb) + n_new_class_entries);
			write_u4(stack_map_attr_length - 6);
	        write_u2(mb->n_stack_maps);
	        for (j = 0; j < mb->n_stack_maps; j++) {
		    write_u2(mb->stack_maps[j].offset);
		    write_u2(mb->stack_maps[j].nlocals);
		    write_stack_map(mb->stack_maps[j].nlocals, mb->stack_maps[j].locals);
		    write_u2(mb->stack_maps[j].nstacks);
		    write_stack_map(mb->stack_maps[j].nstacks, mb->stack_maps[j].stacks);
		}
	    } else {
		write_u2(0);
	    }
	}
	if (mb->nexceptions > 0) {
	    int j;
	    write_u2(lookup_utf8(cb, "Exceptions"));
	    write_u4(2 + mb->nexceptions * 2);
	    write_u2(mb->nexceptions);
	    for (j = 0; j < mb->nexceptions; j++) {
    		write_u2(mb->exceptions[j]);
	    }
	}
    }
}

void ensure_dir_exists(char *dir)
{
    struct stat stat_buf;
    char *parent;
    char *q;
    if (dir[0] == 0) {
	return;
    }
    parent = strdup(dir);
    q = strrchr(parent, '/');
    if (q) {
	*q = 0;
	ensure_dir_exists(parent);
    }
    if (stat(dir, &stat_buf) < 0) {
#ifdef WIN32
        mkdir(dir);
#endif
#ifdef SOLARIS2
        mkdir(dir, 0755);
#endif
    }
    free(parent);
}

void
WriteClass(ClassClass *cb)
{
    int fd;
    char fname[1024];
    
    class_buf = (unsigned char*)malloc(INIT_CLASS_BUF_SIZE);
    if (class_buf == NULL) {
	panic("out of memory");
    }
    class_buf_size = INIT_CLASS_BUF_SIZE;

    class_index = 0;

    write_u4(0xcafebabe);
    write_u2(3);
    write_u2(45);
    
    write_constant_pool(cb);

    write_u2(cbAccess(cb) & ACC_WRITTEN_FLAGS);

    write_u2(lookup_class(cb, cbName(cb)));
    write_u2(cbSuperclass(cb) ? lookup_class(cb,  cbName(cbSuperclass(cb))) : 0);

    write_interfaces(cb);
    
    write_fields(cb);
    
    write_methods(cb);
    
    write_u2(0);

    sprintf(fname, "%s/%s.class", output_dir, cbName(cb));

    {
	char *dir = strdup(fname);
	char *q;

	q = strrchr(dir, '/');
	if (q) {
	    *q = 0;
	    ensure_dir_exists(dir);
	}
	free(dir);
    }

#ifdef SOLARIS2
    fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC , 0644);
#endif
#ifdef WIN32
    fd = open(fname, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0644);
#endif

    if (fd < 0) {
	panic("failed to open %s", fname);
    }

    write(fd, class_buf, class_index);
    close(fd);
    free(class_buf);
    class_buf_size = 0;
}

void
VerifyFile(char *fn)
{
    current_class_name = fn;
    {
	ClassClass *cb = FindClass(0, fn, TRUE);
	char *class_name = PrintableClassname(fn);
	
	if (cb == NULL) {
	    jio_fprintf(stderr, "Error loading class %s\n", class_name);
	} else {
	    WriteClass(cb);
	}
    }
    current_class_name = NULL;
}

char *PrintableClassname(char *class_name)
{
    char *p;
    static char class_copy[257];
    strncpy(class_copy, class_name, 256);

    /* Convert all slashes in the classname to periods */
    for (p = class_copy; ((p = strchr(p, '/')) != 0); *p++ = '.');
    return class_copy;
}

void printCurrentClassName(void)
{
	if (current_class_name)
		fprintf(stderr, "Error preverifying class %s\n    ",
				PrintableClassname(current_class_name));
}
