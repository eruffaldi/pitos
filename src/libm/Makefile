# Standard library for X/COFF kernel
# Makefile for GNU MAKE & GCC 2.8.0

#
# Standard path
#


include ../../config.mk

C_OPT += -Dlint -I.
ASM_OPT += -Dlint -I.

SRCDIRS = msun/src msun/i387 machine
space := $(empty) $(empty)


vpath %.s msun/i387
vpath %.c msun/src machine

# First find a list of every file that might possibly be a source file,
# so we only have to scan the source directories once.
FILES := $(foreach DIR,$(SRCDIRS),$(wildcard $(DIR)/*))


# C source files
CFILES := $(filter %.c,$(FILES))
SFILES := $(filter %.s,$(FILES))


# The generated object files have the same prefix names as the source files,
# except they live in the current (object) directory.
OBJFILES += $(patsubst %.s,%.o,$(notdir $(SFILES)))
OBJFILES += $(patsubst %.c,%.o,$(notdir $(CFILES)))

# This is to eliminate duplicate files,
# which might appear when files are being overridden.
OBJFILES := $(sort $(OBJFILES))

OBJS = $(STUB_OBJS) $(OBJFILES)

#
# Ok! Finally the dependency rules!
# We do not mess with automatic depencencies here!!
#

.PHONY : all clean info install


info :
	@echo "OSLib Makefile"
	@echo "Chose: all, install, clean"
	

all : $(LIBDIR)/libhm.a

install : libhm.a $(LIB_DIR)
	$(CP) libhm.a $(LIB_DIR)

$(LIBDIR) :
	$(MKDIR) $(LIBDIR)
clean :
	$(RM) *.o
	$(RM) *.err
	$(RM) *~
	$(RM) libhm.a

allclean : clean
	echo # Kernel Dependency file > deps
	$(RM) ..\lib\libhm.a

deps : $(OBJS:.o=.c)
	$(CC) $(C_OPT) $(VMINCL) -M $(OBJS:.o=.c) > deps

$(LIBDIR)/libhm.a : $(OBJS)
	$(AR) rs $@ $(OBJS)

deb:
	type $(VPATH)

ifeq (deps,$(wildcard deps))
include deps
endif
