#
# Makefile for GNU make, adapted from the solaris version
# @(#)GNUmakefile	1.5 99/11/22
#

TOP=../../..

ifeq ($(DEBUG), true)
   g = _g
else
   g =
endif

j =



SRCFILES  = cache.c class.c fields.c frame.c garbage.c global.c interpret.c \
            loader.c main.c native.c pool.c thread.c log.c \
            nativeCore.c loaderFile.c \
	    nativeFunctionTableWin.c runtime_md.c runtime2_md.c  \
	    fakeStaticMemory.c hashtable.c profiling.c \
	    StartJVM.c verifier.c jar.c property.c
 
ifeq ($(INCLUDE_ALL_CLASSES), false)
   OTHER_FLAGS += -DINCLUDE_ALL_CLASSES=0
else
   SRCFILES += nativeSpotlet.c winGraphics.c nGraphics.c \
		events.c storage.c storagePrim.c network.c \
		networkPrim.c resource.c
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
   SRCFILES += ROMjavaWin.c
endif

all: kvm$(j)$(g).exe


OBJFILES = $(patsubst %.c,obj$(j)$(g)/%.o,$(SRCFILES))

vpath %.c  $(TOP)/kvm/VmCommon/src/ $(TOP)/kvm/VmWin/src/ \
	   $(TOP)/kvm/VmExtra/src/  $(TOP)/tools/jcc/ \
	   $(TOP)/jam/src/

LIBS= user32.lib gdi32.lib kernel32.lib wsock32.lib
CPPFLAGS = -DWIN32 -I$(TOP)/kvm/VmCommon/h -I$(TOP)/kvm/VmWin/h \
	   -I$(TOP)/kvm/VmExtra/h -I$(TOP)/jam/h /MT \
	   -DMAXIMUMHEAPSIZE=150000

CC= cl
LD= link
CFLAGS= -nologo $(CPPFLAGS) $(ROMFLAGS) $(OTHER_FLAGS)
DEBUG_FLAG= -Zi -Od
OPTIMIZE_FLAG= -Os -O1


$(TOP)/tools/jcc/ROMjavaWin.c $(TOP)/tools/jcc/nativeFunctionTableWin.c: jccWin

.PHONY:  jccWin
jccWin: 
	@(cd $(TOP)/tools/jcc; $(MAKE) windows)


kvm$(j)$(g).exe: obj$j$g/  $(OBJFILES)
	@echo "Linking ... $@"
	@$(LD) $(OBJFILES) -out:$@ $(LIBS)

clean: 
	rm -rf kvm* obj*
	
$(OBJFILES): $(TOP)/kvm/VmWin/h/machine_md.h

obj$(j)/%.o: %.c
		@echo "... $@"
		@$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -c -Fo$@ $<

obj$(j)_g/%.o: %.c
		@echo "... $@"
		@$(CC) $(CFLAGS) $(DEBUG_FLAG) -c -Fo$@ $<

obj$j$g/:
		mkdir -p $@

.includexpm$(j)$(g):
		@if [ ! -f .includexpm$(j)$(g) ]; then \
		    touch .includexpm$(j)$(g); \
		    rm -f .noincludexpm$(j)$(g); \
		    rm -rf obj$(j)$(g)/nativeGraphics.o; \
		fi

.noincludexpm$(j)$(g):
		@if [ ! -f .noincludexpm$(j)$(g) ]; then \
		    touch .noincludexpm$(j)$(g); \
		    rm -f .includexpm$(j)$(g); \
		    rm -rf obj$(j)$(g)/nativeGraphics.o; \
		fi
ifeq ($(GCC), true)

%.i: %.c .FORCE
	echo $(CC)
	$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -E -Fo$@ $<

else 

%.i: %.c .FORCE
	echo $(CC)
	$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -E $< > $@

endif

%.s: %.c .FORCE
	@echo "... $@"
	@$(CC) $(CFLAGS) $(OPTIMIZE_FLAG) -S -Fo$@ $<

.FORCE:
