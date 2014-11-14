# Standard library for X/COFF applications
# Makefile for GNU MAKE & GCC 2.8.0

# Standard library for X/COFF applications
# Makefile for GNU MAKE & GCC 2.8.0

include ..\..\config.mk

C_OPT += -D__GNU__
LINK_OPT = -Bstatic -Ttext 0x320000 -oformat coff-go32 -s -nostartfiles -nostdlib -L$(LIB_PATH)


COMMON_OBJS = stdlib.o \
	random.o \
	sprintf.o \
	sscanf.o \
	cons.o  \
	reboot.o

COMMON_OBJS = cons.o  \
	reboot.o

STRING_OBJS = strbase.o \
	string.o \
	strncat.o \
	strrchr.o \
	strstr.o

IO_OBJS = ecvt.o \
	fcvt.o \
	gcvt.o \
	sprintf.o \
	sscanf.o \
	ucvt.o

STD_OBJS = random.o \
	stdlib.o \
	strtod.o \
	strtoi.o \
	strtol.o \
	strtou.o \
	strtoul.o

GNU_S_OBJS = modf.o

GNU_C_OBJS =

OBJS = $(GNU_S_OBJS) $(GNU_C_OBJS)  $(STRING_OBJS) $(IO_OBJS) $(STD_OBJS)

vpath %.c string stdlib ioformat
#VPATH := $(subst $(space),:,$(SRCDIRS) $(MOSTLY_SRCDIRS))
	
all : $(LIBDIR)/libhc.a
libs : $(LIBDIR)/libhc.a

$(LIBDIR)/libhc.a: $(LIBDIR) $(OBJS)
	ar rs $@ $(OBJS)

$(LIBDIR):
	$(MKDIR) $(LIBDIR)
	
clean :
	$(RM) *.o
	$(RM) *.err
	$(RM) *~
	$(RM) libhc.a

allclean :
	$(RM) $(LIBDIR)\libhc.a

