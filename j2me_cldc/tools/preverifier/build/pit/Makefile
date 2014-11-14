SRC_DIR = ../../src

OBJS = \
	check_class.obj \
	main.obj \
	utf.obj \
	check_code.obj \
	util.obj \
	classloader.obj \
	file.obj \
	classresolver.obj \
	stubs.obj \
	inlinejsr.obj \
	sys_support.obj

CFLAGS = \
	-I$(SRC_DIR) \
	-DWIN32 \
	-DJAVAVERIFY \
	-DTRIMMED \
        $(EXTRACFLAGS)

all: preverify.exe

preverify.exe: $(OBJS)
	link -out:$@ $(OBJS) wsock32.lib

%.obj: $(SRC_DIR)/%.c
	cl -c $(CFLAGS) $<

clean: 
	rm -f core *.obj preverify.exe
