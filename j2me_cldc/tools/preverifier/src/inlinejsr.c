/*
 * @(#)inlinejsr.c	1.8 00/04/12
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oobj.h"
#include "opcodes.h"
#include "sys_api.h"
#include "tree.h"

/* Maximum byte code size is 64K. */
#define MAX_CODE_SIZE 65535

#define get_short_offset(pc)  ((((signed char *)(pc))[0] << 8) | (pc)[1])
#define get_long_offset(pc)  ((((signed char *)(pc))[0] << 24) | ((pc)[1]  << 16) \
			      | ((pc)[2] << 8) | ((pc)[3]))

/* subroutine context. uniquely identifies an inlined
 * subroutine instance.
 */
typedef struct SContext {
	int id;
    struct SContext *caller;            /* caller */
    struct SContext *next_alloced;      /* allow all to be freed */
    long *exception_handlers;           /* exception handlers for this 
										   subroutine instance */
    int sub_index;                      /* subroutine address */
    int ret_index;                      /* return address */
} SContext;

#define BLK_JSR_GOTO          -5
#define BLK_RET_GOTO          -4
#define BLK_EXCEPTION_HANDLER -2
#define BLK_METHOD_ENTRY      -3
#define BLK_GOTO              -1

/* a block of code. terminates at goto, jmp, return, etc. */
typedef struct Block {
    int kind;                           /* kind: what leads to the basic block:
                                           0-255:  byte code
                                           -1 - -4:special cases */ 
    struct Block *next;                 /* next pending */
    int old_index;                      /* original pc */
    int new_index;                      /* relocated pc */
    unsigned char* new_offset_addr;     /* address to patch */
    int new_offset_size;                /* size of address to patch, 2 or 4 */
    SContext *scontext;                 /* the SContext of the basic block */
} Block;

/* global context, keep track of info when a method is rewritten */
typedef struct {
    struct methodblock* mb;

    unsigned char *old_code;
    int old_size;
    int old_index;

    unsigned char *new_code;
    int new_size;
    int new_index;

    int *reverse_mapping;              /* new pc => old_pc (many to one) */
    SContext **scontext_mapping;       /* new_pc => scontext */

    SContext *scontext;                /* current scontext */
    SContext *alloced_scontexts;       /* all scontexts */
    SContext *root_scontext;           /* root scontext */

    Block *blocks;                     /* all pending basic blocks */

    struct CatchFrame *new_exception_table;
    int new_exception_table_length;

    char *mp_marks;                    /* whether the given pc is a merge point 
										  (jump target, or start of basic block) */
	char *jump_targets;
	int next_scontext_id;
} Context;

extern const short opcode_length[];
static int instruction_length(unsigned char *iptr);
static void do_block(Context *);

static void set_short_offset(short pc, unsigned char *code)
{
    code[0] = (unsigned char)(pc >> 8);
    code[1] = (unsigned char)pc;
}

static void set_long_offset(long pc, unsigned char *code)
{
    code[0] = (unsigned char)(pc >> 24);
    code[1] = (unsigned char)(pc >> 16);
    code[2] = (unsigned char)(pc >> 8);
    code[3] = (unsigned char)pc;
}

static Block* lookup_block(Context *context, Block *block)
{
	Block *blocks = context->blocks;
	while (blocks) {
		if (blocks->kind            == block->kind &&
			blocks->old_index       == block->old_index &&
			blocks->new_index       == block->new_index &&
			blocks->new_offset_addr == block->new_offset_addr &&
			blocks->new_offset_size == block->new_offset_size &&
			blocks->scontext        == block->scontext) {
			break;
		}
		blocks = blocks->next;
	}
	return blocks;
}

static void push_block(Context *context, int old_index, int new_index,
		unsigned char *new_offset_addr, int new_offset_size,
		SContext *scontext, int kind)
{
    int opcode;
    Block *new_block;

#ifdef MYDEBUG
    printf("pushd block o_idx=%d, n_idx=%d, sc=%d ", old_index, new_index, scontext->id);
#endif

    opcode = context->old_code[old_index];
    /* If the first instruction of a basic block is a goto, we short-circuit it */
    if (opcode == opc_goto) {
		int old_index2 = old_index + 
			get_short_offset(&context->old_code[old_index + 1]);
		if (old_index != old_index2) {
#ifdef MYDEBUG
			printf("goto short circuit\n");
#endif
			push_block(context, old_index2,
					   new_index, new_offset_addr, new_offset_size,
					   scontext, kind);
			return;
		}
    }
    /* we short-circuit ret as well */
    if (opcode == opc_ret) {
	int ret_index;
        if (scontext == context->root_scontext) {
	    panic("ret must be in a subroutine");
	}
#ifdef MYDEBUG
		printf("ret short circuit\n");
#endif
        ret_index = scontext->ret_index;
        scontext = scontext->caller;
        push_block(context, ret_index, 
	    new_index, new_offset_addr, new_offset_size, scontext, kind);
	return;
    }

    new_block = (Block *)malloc(sizeof(Block));
    new_block->kind = kind;
    new_block->next = context->blocks;
    new_block->old_index = old_index;
    new_block->new_index = new_index;
    new_block->new_offset_addr = new_offset_addr;
    new_block->new_offset_size = new_offset_size;

    new_block->scontext = scontext;
 
	if (lookup_block(context, new_block) == NULL) {
		context->blocks = new_block;
#ifdef MYDEBUG
		printf("inserted\n");
#endif
	} else {
		free(new_block);
#ifdef MYDEBUG
		printf("dup\n");
#endif
	}
}

static int has_block(Context *context)
{
    return context->blocks != NULL;
}

static Block * pop_block(Context *context)
{
    Block **blockP;
    Block **minBlockP;
    Block * block;
    int min_old_index = MAX_CODE_SIZE;

    /* pick the block that has minimum original pc, experience shows
     * that leads to smaller resulting byte code size.
     */
    for (blockP = &(context->blocks); *blockP; blockP = &((*blockP)->next)) {
	if ((*blockP)->old_index < min_old_index) {
	    min_old_index = (*blockP)->old_index;
	    minBlockP = blockP;
	}
    }
    
    block = *minBlockP;
    *minBlockP = block->next;

    return block;
}

static SContext * newSContext(Context *context, int ret_index) 
{
    int i;
    SContext *new_scontext = (SContext *)malloc(sizeof(SContext));

    new_scontext->exception_handlers = 
	(long*)malloc(context->mb->exception_table_length * sizeof(long));
    for (i = 0; i < (int)context->mb->exception_table_length; i++) {
        new_scontext->exception_handlers[i] = -1;
    }

    new_scontext->ret_index = ret_index;
    new_scontext->next_alloced = context->alloced_scontexts;
    context->alloced_scontexts = new_scontext;
    new_scontext->caller = context->scontext;
    new_scontext->id = context->next_scontext_id++;
    new_scontext->sub_index = -1;
    context->scontext = new_scontext;
    return new_scontext;
}

static void do_jsr(Context* context, int offset, int instr_size)
{
    int opcode;
    SContext *new_scontext = 
	newSContext(context, context->old_index + instr_size);

    context->old_index += offset;

    opcode = context->old_code[context->old_index];
    
    /* skip the first instruction of the subroutine. It must be an astore */
    switch (opcode) {
    case opc_astore:
	context->old_index += 2;
	break;
    case opc_astore_0:
    case opc_astore_1:
    case opc_astore_2:
    case opc_astore_3:
	context->old_index += 1;
	break;
    case opc_wide:
	opcode = context->old_code[context->old_index + 1];
	if (opcode == opc_astore) {
	    context->old_index += 4;
	    break;
	}
	/* fall through */
    default:
	panic("subroutine must start with astore");
    }

    {
	/* detect recursion.
	 * though subroutines themselves are never recursive, recursion
	 * may creep in because of exception handlers.
	 * See bug 4309639.
	 */
	SContext *sc = context->scontext;
	while (sc) {
	    if (sc->sub_index == context->old_index) {
		context->scontext = context->scontext->caller;
		context->new_code[context->new_index] = opc_goto;
		push_block(context,
			   context->old_index,
			   context->new_index, 
			   &context->new_code[context->new_index + 1],
			   2, context->scontext,
			   BLK_JSR_GOTO);
		context->new_index += 3;
		return;
	    }
	    sc = sc->caller;
	}
    }

    new_scontext->sub_index = context->old_index;

    do_block(context);
}

static void do_ret(Context* context)
{
    int ret_index;
    if (context->scontext == context->root_scontext) {
	panic("ret must be in a subroutine");
    }
    /* turn ret into a goto */
    ret_index = context->scontext->ret_index;
    context->scontext = context->scontext->caller;
    context->new_code[context->new_index] = opc_goto;
    push_block(context, ret_index, context->new_index, 
	&context->new_code[context->new_index + 1], 2, context->scontext, BLK_RET_GOTO);
    context->new_index += 3;
    context->old_index = ret_index;
}

static void copy_instr(Context *context, int instr_size)
{
    context->reverse_mapping[context->new_index] = context->old_index;
    context->scontext_mapping[context->new_index] = context->scontext;
    for (; instr_size > 0; instr_size--) {
	context->new_code[context->new_index++] = context->old_code[context->old_index++];
    }
}

static void do_block(Context* context)
{
    int opcode;
    int target;

#ifdef MYDEBUG
    printf("doing block o_idx=%d, n_idx=%d, sc=%d\n", 
		   context->old_index, context->new_index, context->scontext->id);
#endif

    while (1) {
        if (context->old_index >= context->old_size) {
	    panic("passed the end of byte code");
	}
        if (context->new_index + 
	    instruction_length(&context->old_code[context->old_index]) > MAX_CODE_SIZE) {
    	    panic("method size exceeded %d bytes", MAX_CODE_SIZE);
	}

	/* is there an exception handler for the current bytecode? If so,
	 * add a new basic block 
	 */
	{
	    int i;
	    int old_pc = context->old_index;
	    for (i = 0; i < (int)context->mb->exception_table_length; i++) {
    		if (context->mb->exception_table[i].start_pc <= old_pc &&
		    context->mb->exception_table[i].end_pc > old_pc) {
	            push_block(context, context->mb->exception_table[i].handler_pc, 0,
	                       (unsigned char*)&context->scontext->exception_handlers[i],
						   4, 
		               context->scontext, BLK_EXCEPTION_HANDLER);
		}
	    }
	}

        opcode = context->old_code[context->old_index];
#ifdef MYDEBUG
	printf("%d\t%d\t%d\t%s\n", context->old_index, context->new_index,
		   context->scontext->id, opnames[opcode]);
#endif
        switch (opcode) {
        case opc_jsr: 
	    do_jsr(context, get_short_offset(&context->old_code[context->old_index + 1]), 3);
	    return;

        case opc_jsr_w:
	    do_jsr(context, get_long_offset(&context->old_code[context->old_index + 1]), 5);
	    return;

        case opc_ret:
	    do_ret(context);
	    return;

        case opc_ifeq :
        case opc_ifne :
        case opc_iflt :
        case opc_ifge :
        case opc_ifgt :
        case opc_ifle :
        case opc_if_icmpeq :
        case opc_if_icmpne :
        case opc_if_icmplt :
        case opc_if_icmpge :
        case opc_if_icmpgt :
        case opc_if_icmple :
        case opc_if_acmpeq :
        case opc_if_acmpne :
        case opc_ifnull :
        case opc_ifnonnull :
	    target = context->old_index + 
		get_short_offset(&context->old_code[context->old_index + 1]);
	    push_block(context, target, context->new_index,
		&context->new_code[context->new_index + 1], 2, context->scontext, opcode);
	    copy_instr(context, 3);
	    break;

	case opc_goto:
	    target = context->old_index + 
		get_short_offset(&context->old_code[context->old_index + 1]);
	    if (target == context->old_index + 3) {
		context->old_index += 3;
		break;
	    }
	    push_block(context, target, context->new_index,
		&context->new_code[context->new_index + 1], 2, context->scontext, opcode);
	    copy_instr(context, 3);
	    if (context->old_index < context->old_size &&
			context->jump_targets[context->old_index]) {
		push_block(context, context->old_index, -1, NULL, 0, context->scontext, BLK_GOTO);
	    }
	    return;

        case opc_goto_w:
	    target = context->old_index + 
		get_long_offset(&context->old_code[context->old_index + 1]);
	    push_block(context, target, context->new_index,
		&context->new_code[context->new_index + 1], 4, context->scontext, opcode);
	    copy_instr(context, 5);
	    return;

        case opc_tableswitch: 
	case opc_lookupswitch: 
	{
    	    long *lpc = (long *) UCALIGN(&context->old_code[context->old_index + 1]);
	    int keys;
	    int k;
	    int old_instr_index = context->old_index;
	    int new_instr_index = context->new_index;

            context->reverse_mapping[context->new_index] = context->old_index;
            context->scontext_mapping[context->new_index] = context->scontext;

	    context->new_code[context->new_index] = opcode;

	    context->new_index++;
	    context->old_index++;

	    /* clear the alignment bytes */
	    context->new_code[context->new_index] = 0;
	    context->new_code[context->new_index + 1] = 1;
	    context->new_code[context->new_index + 2] = 2;

	    /* 4-byte alignment */
	    context->new_index = (context->new_index + 3) & ~3;
	    context->old_index = (context->old_index + 3) & ~3;

	    push_block(context, ntohl(lpc[0]) + old_instr_index,
		new_instr_index, &context->new_code[context->new_index], 4, context->scontext, opcode);
	    if (opcode == opc_tableswitch) {
		keys = ntohl(lpc[2]) -  ntohl(lpc[1]) + 1;
		memcpy(&context->new_code[context->new_index], lpc, 12);
		context->new_index += 12;
		context->old_index += 12;
		memcpy(&context->new_code[context->new_index], lpc + 3, keys * 4);
		for (k = 0; k < keys; k++) {
		    push_block(context, ntohl(lpc[3 + k]) + old_instr_index,
			new_instr_index, &context->new_code[context->new_index + k * 4], 4,
			context->scontext, opcode);
		}
		context->new_index += k * 4;
		context->old_index += k * 4;
	    } else { 
		keys = ntohl(lpc[1]); /* number of pairs */
		memcpy(&context->new_code[context->new_index], lpc, 8);
		context->new_index += 8;
		context->old_index += 8;
		memcpy(&context->new_code[context->new_index], lpc + 2, keys * 8);
		for (k = 0; k < keys; k++) {
		    push_block(context, ntohl(lpc[3 + k * 2]) + old_instr_index, 
			new_instr_index, &context->new_code[context->new_index + 4 + k * 8], 4,
			context->scontext, opcode);
		}
		context->new_index += k * 8;
		context->old_index += k * 8;
	    }
	    return;
	}
	
        case opc_nop :
	    context->old_index++;
	    break;
        case opc_fconst_0 :
        case opc_fconst_1 :
        case opc_fconst_2 :
        case opc_dconst_0 :
        case opc_dconst_1 :
        case opc_fload :
        case opc_dload :
        case opc_fload_0 :
        case opc_fload_1 :
        case opc_fload_2 :
        case opc_fload_3 :
        case opc_dload_0 :
        case opc_dload_1 :
        case opc_dload_2 :
        case opc_dload_3 :
        case opc_faload :
        case opc_daload :
        case opc_fstore_0 :
        case opc_fstore_1 :
        case opc_fstore_2 :
        case opc_fstore_3 :
        case opc_dstore_0 :
        case opc_dstore_1 :
        case opc_dstore_2 :
        case opc_dstore_3 :
        case opc_fstore :
        case opc_dstore :
        case opc_fastore :
        case opc_dastore :
        case opc_fadd :
        case opc_dadd :
        case opc_fsub :
        case opc_dsub :
        case opc_fmul :
        case opc_dmul :
        case opc_fdiv :
        case opc_ddiv :
        case opc_frem :
        case opc_drem :
        case opc_fneg :
        case opc_dneg :
        case opc_i2f :
        case opc_i2d :
        case opc_l2f :
        case opc_l2d :
        case opc_f2i :
        case opc_f2l :
        case opc_f2d :
        case opc_d2i :
        case opc_d2l :
        case opc_d2f :
        case opc_fcmpl :
        case opc_fcmpg :
        case opc_dcmpl :
        case opc_dcmpg :
	    if (check_on) {
		panic("floating-point instructions should not appear");
	    }
	    /* fall through */
        case opc_aconst_null :
        case opc_iconst_m1 :
        case opc_iconst_0 :
        case opc_iconst_1 :
        case opc_iconst_2 :
        case opc_iconst_3 :
        case opc_iconst_4 :
        case opc_iconst_5 :
        case opc_lconst_0 :
        case opc_lconst_1 :
        case opc_bipush :
        case opc_sipush :
        case opc_ldc :
        case opc_ldc_w :
        case opc_ldc2_w :
        case opc_iload :
        case opc_lload :
        case opc_aload :
        case opc_iload_0 :
        case opc_iload_1 :
        case opc_iload_2 :
        case opc_iload_3 :
        case opc_lload_0 :
        case opc_lload_1 :
        case opc_lload_2 :
        case opc_lload_3 :
        case opc_aload_0 :
        case opc_aload_1 :
        case opc_aload_2 :
        case opc_aload_3 :
        case opc_iaload :
        case opc_laload :
        case opc_aaload :
        case opc_baload :
        case opc_caload :
        case opc_saload :
        case opc_istore :
        case opc_lstore :
        case opc_astore :
        case opc_istore_0 :
        case opc_istore_1 :
        case opc_istore_2 :
        case opc_istore_3 :
        case opc_lstore_0 :
        case opc_lstore_1 :
        case opc_lstore_2 :
        case opc_lstore_3 :
        case opc_astore_0 :
        case opc_astore_1 :
        case opc_astore_2 :
        case opc_astore_3 :
        case opc_iastore :
        case opc_lastore :
        case opc_aastore :
        case opc_bastore :
        case opc_castore :
        case opc_sastore :
        case opc_pop :
        case opc_pop2 :
        case opc_dup :
        case opc_dup_x1 :
        case opc_dup_x2 :
        case opc_dup2 :
        case opc_dup2_x1 :
        case opc_dup2_x2 :
        case opc_swap :
        case opc_iadd :
        case opc_ladd :
        case opc_isub :
        case opc_lsub :
        case opc_imul :
        case opc_lmul :
        case opc_idiv :
        case opc_ldiv :
        case opc_irem :
        case opc_lrem :
        case opc_ineg :
        case opc_lneg :
        case opc_ishl :
        case opc_lshl :
        case opc_ishr :
        case opc_lshr :
        case opc_iushr :
        case opc_lushr :
        case opc_iand :
        case opc_land :
        case opc_ior :
        case opc_lor :
        case opc_ixor :
        case opc_lxor :
        case opc_iinc :
        case opc_i2l :
        case opc_l2i :
        case opc_i2b :
        case opc_i2c :
        case opc_i2s :
        case opc_lcmp :

        case opc_getstatic :
        case opc_putstatic :
        case opc_getfield :
        case opc_putfield :
        case opc_invokevirtual :
        case opc_invokespecial :
        case opc_invokestatic :
        case opc_invokeinterface :

        case opc_new :
        case opc_newarray :
        case opc_anewarray :
        case opc_arraylength :
        case opc_checkcast :
        case opc_instanceof :
        case opc_monitorenter :
        case opc_monitorexit :

        case opc_multianewarray :

    	    copy_instr(context, opcode_length[opcode]);
	    break;

        case opc_freturn :
        case opc_dreturn :
	    if (check_on) {
		panic("floating-point instructions should not appear");
	    }
        case opc_ireturn :
        case opc_lreturn :
        case opc_areturn :
        case opc_return :
        case opc_athrow :
    	    copy_instr(context, opcode_length[opcode]);
	    return;

	case opc_wide:
	    opcode = context->old_code[context->old_index + 1];
	    if (opcode == opc_iinc) {
		copy_instr(context, 6);
		break;
	    }
	    if (opcode == opc_ret) {
		do_ret(context);
		return;
	    }
	    if (opcode == opc_iload ||
		opcode == opc_fload ||
		opcode == opc_aload ||
		opcode == opc_lload ||
		opcode == opc_dload ||
    		opcode == opc_istore ||
    		opcode == opc_fstore ||
    		opcode == opc_astore ||
    		opcode == opc_lstore ||
		opcode == opc_dstore) {
    		copy_instr(context, 4);
    		break;
	    }
       	    panic ("bad wide bytecode %d", opcode);
        default:
    	    panic ("bad bytecode %d", opcode);
	    break;
	}
    }   
}

/* get the exception handler entry for a given new pc and exception type */
static long get_new_handler(Context *context, int new_pc, unsigned short catchType)
{
    int i;
    int old_pc = context->reverse_mapping[new_pc];
    SContext *scontext = context->scontext_mapping[new_pc];

    if (old_pc == -1) {
	return -1;
    }
    for (i = 0; i < (int)context->mb->exception_table_length; i++) {
	if (context->mb->exception_table[i].start_pc <= old_pc &&
	    context->mb->exception_table[i].end_pc > old_pc &&
	    context->mb->exception_table[i].catchType == catchType) {
	    return scontext->exception_handlers[i];
	}
    }
    return -1;
}

/* Given a pointer to an instruction, return its length.
 */
static int instruction_length(unsigned char *iptr)
{
    int instruction = *iptr;
    switch (instruction) {
        case opc_tableswitch: {
	    long *lpc = (long *) UCALIGN(iptr + 1);
	    int low = ntohl(lpc[1]);
	    int high = ntohl(lpc[2]);
	    if ((low > high) || (low + 65535 < high)) 
		panic("bad tableswitch");	/* illegal */
	    else 
		return (unsigned char *)(&lpc[(high - low + 1) + 3]) - iptr;
	}
	    
	case opc_lookupswitch: {
	    long *lpc = (long *) UCALIGN(iptr + 1);
	    int npairs = ntohl(lpc[1]);
	    return (unsigned char *)(&lpc[2 * (npairs + 1)]) - iptr;
	}

	case opc_wide: 
	    switch(iptr[1]) {
	        case opc_ret:
	        case opc_iload: case opc_istore: 
	        case opc_fload: case opc_fstore:
	        case opc_aload: case opc_astore:
	        case opc_lload: case opc_lstore:
	        case opc_dload: case opc_dstore: 
		    return 4;
		case opc_iinc:
		    return 6;
		default: 
		    panic("bad opc_wide");
	    }

	default: {
	    /* A length of 0 indicates an error. */
	    int length = opcode_length[instruction];
	    if (length < 0) {
		panic("bad opcode");
	    }
	    return length;
	}
    }
}

/* fixup exception handlers */
static void do_exceptions(Context *context)
{
    int i,j;
    int new_exception_table_length = 0;
    int new_exception_table_limit = context->mb->exception_table_length * 5 + 10000;
    struct CatchFrame *new_exception_table = (struct CatchFrame *)
	malloc(new_exception_table_limit * sizeof(struct CatchFrame));

#if MYDEBUG
    for (j = 0; j < context->new_size;
	 j += instruction_length(&context->new_code[j])) {
	int old_pc = context->reverse_mapping[j];
	printf("%d\t=> %d\n", j, old_pc);
    }
#endif

    for (i = 0; i < (int)context->mb->exception_table_length; i++) {
	long *handlers = (long*)malloc(context->new_size * sizeof(long));
	int start, end;
	unsigned short catch_type = context->mb->exception_table[i].catchType;
#ifdef MYDEBUG
	printf("old %d, %d, (%d) => %d\n",
	    context->mb->exception_table[i].start_pc,
	    context->mb->exception_table[i].end_pc,
	    context->mb->exception_table[i].catchType,
	    context->mb->exception_table[i].handler_pc);
#endif
	for (j = 0; j < context->new_size;
	     j += instruction_length(&context->new_code[j])) {
	    int old_pc = context->reverse_mapping[j];
	    SContext *scontext = context->scontext_mapping[j];
	    if (context->mb->exception_table[i].start_pc <= old_pc &&
		context->mb->exception_table[i].end_pc > old_pc) {
		handlers[j] = scontext->exception_handlers[i];
	    } else {
		handlers[j] = -1;
	    }
	}
	for (start = 0; start < context->new_size;) {
	    if (handlers[start] == -1) {
	        start += instruction_length(&context->new_code[start]);
		continue;
	    }
	    for (end = start; end < context->new_size;
		 end += instruction_length(&context->new_code[end])) {
		if (handlers[end] != handlers[start]) {
		    break;
		}
	    }
	    if (end != start) {
		if (new_exception_table_length >= new_exception_table_limit) {
		    panic("exception table overflow");
		}
#ifdef MYDEBUG
		printf("new %d, %d, (%d) => %d\n", start, end, catch_type, handlers[start]);
#endif
		new_exception_table[new_exception_table_length].start_pc = start;
		new_exception_table[new_exception_table_length].end_pc = end;
		new_exception_table[new_exception_table_length].handler_pc = handlers[start];
		new_exception_table[new_exception_table_length].catchType = catch_type;

		new_exception_table_length++;

		start = end;
	    }
	}
	free(handlers);
    }

    context->new_exception_table = (struct CatchFrame *)
	realloc(new_exception_table, new_exception_table_length * sizeof(struct CatchFrame));
    context->new_exception_table_length = new_exception_table_length;

#ifdef MYDEBUG
    printf("exception table: old %d, new %d\n",
	context->mb->exception_table_length, new_exception_table_length);
#endif
}

static void mark_jump_targets(Context *context)
{
	int offset = 0;
	while (offset < context->old_size) {
		int opcode = context->old_code[offset];
		int target;

        switch (opcode) {
        case opc_ifeq :
        case opc_ifne :
        case opc_iflt :
        case opc_ifge :
        case opc_ifgt :
        case opc_ifle :
        case opc_if_icmpeq :
        case opc_if_icmpne :
        case opc_if_icmplt :
        case opc_if_icmpge :
        case opc_if_icmpgt :
        case opc_if_icmple :
        case opc_if_acmpeq :
        case opc_if_acmpne :
        case opc_ifnull :
        case opc_ifnonnull :
			target = offset + 
				get_short_offset(&context->old_code[offset + 1]);
			context->jump_targets[target] = 1;
			break;

		case opc_goto:
			target = offset + 
				get_short_offset(&context->old_code[offset + 1]);
			context->jump_targets[target] = 1;
			break;
        case opc_goto_w:
			target = offset + 
				get_long_offset(&context->old_code[offset + 1]);
			context->jump_targets[target] = 1;
			break;
		default:
			break; /* do nothing */
		}

		offset += instruction_length(context->old_code + offset);
	}
}

static void inline_jsr_method(struct methodblock *mb)
{
    int i;
    Context context_buf;
    Context *context = &context_buf;
    unsigned short start_pc;

    memset(context, 0, sizeof(Context));

    context->mb = mb;
    context->old_code = mb->code;
    context->old_size = mb->code_length;
    context->old_index = 0;

    /* Method byte code sizes are bound by a 64K limit. We allocate
     * chunks of 64K memory to simplify memory management
     */
    context->new_code = (unsigned char*)malloc(MAX_CODE_SIZE * sizeof(unsigned char));
    context->new_size = 0;
    context->new_index = 0;

    context->reverse_mapping = (int*)malloc(MAX_CODE_SIZE * sizeof(int));
    context->scontext_mapping = (SContext**)malloc(MAX_CODE_SIZE * sizeof(SContext*));
    context->mp_marks = (char*)malloc(MAX_CODE_SIZE * sizeof(char));
    context->jump_targets = (char*)malloc(MAX_CODE_SIZE * sizeof(char));

    for (i = 0; i < MAX_CODE_SIZE; i++) {
	context->reverse_mapping[i] = -1;
	context->scontext_mapping[i] = (SContext*)0xFFFFFFFF;
	context->mp_marks[i] = 0;
	context->jump_targets[i] = 0;
    }

    context->root_scontext = newSContext(context, -1);

	mark_jump_targets(context);

    start_pc = 0;
    push_block(context, 0, 0, (unsigned char*)&start_pc, 2, context->root_scontext, BLK_METHOD_ENTRY);

    while (has_block(context)) {
	Block *block = pop_block(context);
        int new_index;

        context->old_index = block->old_index;
        context->scontext = block->scontext;

#ifdef MYDEBUG
    printf("poped block o_idx=%d, n_idx=%d, sc=%d\n", 
		   context->old_index, context->new_index, context->scontext->id);
#endif

	/* make sure that we have not yet generated code for exactly the same
	 * pc and the same scontext
	 */
	for (new_index = context->new_index - 1; new_index >= 0; new_index--) {
	    if (context->reverse_mapping[new_index] == context->old_index &&
	        context->scontext_mapping[new_index] == context->scontext) {
	        break;
	    }
	}
	if (new_index < 0) {
	    new_index = context->new_index;
	    do_block(context);
	}
	/* patch the instruction that led to the basic block */
	if (block->kind != BLK_GOTO) {
	    if (block->new_offset_size == 4) {
	        set_long_offset(new_index - block->new_index, block->new_offset_addr);
	    } else {
	        set_short_offset((short)(new_index - block->new_index), block->new_offset_addr);
	    }

	}
    /* mark jump targets */
	if (block->kind != BLK_METHOD_ENTRY) {
	    context->mp_marks[new_index] = 1;
	}
	free(block);
    }

    context->new_size = context->new_index;
    context->new_code = (unsigned char*)realloc(context->new_code, context->new_size);

    {
	/* the exception handler pcs are not yet in the right byte order */
	SContext *sc = context->alloced_scontexts;
	while (sc) {
	    for (i = 0; i < (int)(mb->exception_table_length); i++) {
	        sc->exception_handlers[i] = ntohl(sc->exception_handlers[i]);
	    }
	    sc = sc->next_alloced;
	}
    }

    do_exceptions(context);

#ifdef MYDEBUG
    {
	int mp_count = 0;
	for (i = 0; i < context->new_size; i++) {
	    mp_count += context->mp_marks[i];
	}
        printf("code %d=>%d, exc %d=>%d, mp %d\t%s.%s%s\n",
	    context->old_size, context->new_size,
	    context->mb->exception_table_length, context->new_exception_table_length,
	    mp_count,
	    cbName(context->mb->fb.clazz),
	    context->mb->fb.name,
	    context->mb->fb.signature);
    }
#endif

    context->mb->code = context->new_code;
    context->mb->code_length = context->new_size;
    context->mb->exception_table = context->new_exception_table;
    context->mb->exception_table_length = context->new_exception_table_length;

    context->mb->line_number_table = NULL;
    context->mb->localvar_table = NULL;
    context->mb->line_number_table_length = 0;
    context->mb->localvar_table_length = 0;

#ifdef VERIFIER_TOOL
    context->mp_marks = (char*)realloc(context->mp_marks, context->new_size);
    context->mb->mp_marks = context->mp_marks;
#else
    free(context->mp_marks);
#endif

	free(context->jump_targets);
    free(context->reverse_mapping);
    free(context->scontext_mapping);

    while (context->alloced_scontexts) {
	SContext *next = context->alloced_scontexts->next_alloced;
	free(context->alloced_scontexts->exception_handlers);
	free(context->alloced_scontexts);
	context->alloced_scontexts = next;
    }
}

void inline_jsr(ClassClass *cb)
{
    int i;
    struct methodblock *mb;
    for (i = cbMethodsCount(cb), mb = cbMethods(cb); --i >= 0; mb++) {
	if (mb->code_length > 0) {
	    inline_jsr_method(mb);
	}
    }
}
