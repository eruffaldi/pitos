Build PITOS
Use a gcc+binutils configured for i386-linux-coff. If you are a cygwin user like me you have to
make a crosscompiler because of some (lot) bugs that affect the cygwin PE version of gcc+binutils.
See config.mk for tools configuration.

First setup the env:
	ROOT=<unpack directory>

Components:
1) main os library
2) libc libm
3) jkvm
	make -f j2me_cldc/build/pit/Makefile
4) examples
	actually the only working is jkvm
			

Solution to bug:
1) genero file pe
2) modifico il file pe attraverso bfd o altra libreria
