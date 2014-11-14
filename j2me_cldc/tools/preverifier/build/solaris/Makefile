SRC_DIR = ../../src

OBJS = \
	check_class.o \
	main.o \
	utf.o \
	check_code.o \
	util.o \
	classloader.o \
	file.o \
	classresolver.o \
	stubs.o \
	inlinejsr.o \
	sys_support.o

CFLAGS = \
	-I$(SRC_DIR) \
	-DSOLARIS2 \
	-DJAVAVERIFY \
	-DTRIMMED \
	-Dsparc $(EXTRACFLAGS)

all: preverify

preverify: $(OBJS)
	cc -o $@ $(OBJS)

%.o: $(SRC_DIR)/%.c
	cc -c $(CFLAGS) $<

clean: 
	rm -f core *.o preverify
