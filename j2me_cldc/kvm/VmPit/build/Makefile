# Makefile for the VM on Pit OS:
# * use my STDCLIBRARY!!
# * compile building with the integrated loader
# * don't use main.c but kmain.c
include $(ROOT)/config.mk

TOP=../../..
JKVM=$(ROOT)/examples/jkvm
GCC=true
AS=/bin/as-old
LD=ld-new
INCLUDE_ALL_CLASSES=false
ifeq ($(DEBUG), true)
   g = _g
else
   g =
endif

j =
# NO loaderFile.c jar.
SRCFILES  = cache.c class.c fields.c frame.c garbage.c global.c interpret.c \
            loader.c native.c pool.c thread.c  \
            nativeCore.c  \
	    nativeFunctionTablePit.c simpleLoaderFile.c runtime_md.c \
	    fakeStaticMemory.c hashtable.c profiling.c \
	    StartJVM.c verifier.c log.c  \
	    property.c 
	    
ifeq ($(INCLUDE_ALL_CLASSES), false)
   OTHER_FLAGS += -DINCLUDE_ALL_CLASSES=0
else
   SRCFILES += nativeSpotlet.c nativeGraphics.c \
               events.c storage.c storagePrim.c \
               network.c networkPrim.c resource.c
endif

ifeq ($(USE_JAM), true)
   OTHER_FLAGS += -DUSE_JAM=1
   SRCFILES += jam.c jamParse.c jamHttp.c jamStorage.c
   j = _j
else

endif

ifeq ($(ROMIZING), false) 
   ROMFLAGS = -DROMIZING=0
else
   SRCFILES += ROMjavaPit.c
endif

all: $(BINDIR)/libkvm.a


OBJFILES = $(patsubst %.c,obj$(j)$(g)/%.o,$(SRCFILES)) 


vpath %.c  $(TOP)/kvm/VmCommon/src/ $(TOP)/kvm/VmPit/src/ \
           $(TOP)/kvm/VmExtra/src/ $(TOP)/tools/jcc/ $(TOP)/jam/src
           
vpath %.cpp  $(TOP)/kvm/VmCommon/src/ $(TOP)/kvm/VmPit/src/ \
           $(TOP)/kvm/VmExtra/src/ $(TOP)/tools/jcc/ $(TOP)/jam/src


CPPFLAGS =  -DMAXIMUMHEAPSIZE=150000 \
	   -I$(TOP)/kvm/VmCommon/h -I$(TOP)/kvm/VmPit/h \
	   -I$(TOP)/kvm/VmExtra/h -I$(TOP)/jam/h

CC = gcc
CFLAGS = $(C_OPT) -Wall $(OTHER_FLAGS) $(CPPFLAGS) $(ROMFLAGS)
DEBUG_FLAG = -g 
OPTIMIZE_FLAG = -O2   

DEBUG_FLAG += -DINCLUDEDEBUGCODE=1 

$(TOP)/tools/jcc/ROMjavaPit.c $(TOP)/tools/jcc/nativeFunctionTablePit.c: jccPit

.PHONY:  jccPit
jccPit: 
	@(cd $(TOP)/tools/jcc; $(MAKE) pit)       

$(BINDIR)/libkvm.a: obj$j$g/ $(OBJFILES)
	@$(AR) -r $@ $(OBJFILES)

kvm$(j)$(g): obj$j$g/ $(OBJFILES) $(LIBDIR)/libsys.a
	@echo "Linking ... $@"
	@$(LD) $(LINKOPT) $(OBJFILES) -o $@
	@cp $@ $(BINDIR)


clean: 
	rm -rf core kvm* .noincludexpm* obj*

obj$(j)/%.o: %.c
		@echo "... $@"
		@$(CC) $(EXTRACFLAGS) $(CFLAGS) $(XWINFLAGS) $(OPTIMIZE_FLAG) -c -S -o tmp $< 
		$(AS) tmp -o $@ 

obj$(j)/%.o: %.cpp
		@echo "... $@"
		@$(CC) $(EXTRACFLAGS) $(CFLAGS) $(XWINFLAGS) $(OPTIMIZE_FLAG) -c -S -o tmp $< 
		$(AS) tmp -o $@ 


obj$(j)_g/%.o: %.c
		@echo "... $@"
		@$(CC) $(EXTRACFLAGS) $(CFLAGS) $(XWINFLAGS) $(DEBUG_FLAG) -c -S -o tmp $< 
		$(AS) tmp -o $@ 

obj$j$g/:
		mkdir -p $@

%.i: %.c .FORCE
	echo $(CC)
	$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -E -o $@ $<

%.s: %.c .FORCE
	@echo "... $@"
	@$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -S -o $@ $<


regression:
	CLASSPATH=../../../api/classes:../../../samples/classes kvm RegressionTest

