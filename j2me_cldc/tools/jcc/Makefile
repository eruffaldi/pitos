TOP=../..
INCLUDE_ALL_CLASSES=false
JAVA          = java
JAVAC         = javac

JAVAFILES =  $(shell find src -name "*.java"|grep -v SCCS)
 
CLASSFILES = $(subst src,classes,$(JAVAFILES:java=class))

# $< is dependency
# $@ is target


$(CLASSFILES): classes/%.class : src/%.java
	@echo $< >> .filelist

eraselists: 
	@rm -f .filelist 

compilefiles:
	@if [ '!' -d classes ]; then rm -rf classes; mkdir classes; fi;
	@if [ -f .filelist ]; then \
		echo javac `cat .filelist`; \
		javac -d classes -classpath classes:src \
	              `cat .filelist`; \
	fi

ifndef PLATFORM
PLATFORM=solaris
endif

ifeq ($(PLATFORM), solaris)
VMDIR = VmUnix
endif

ifeq ($(PLATFORM), win32)
CC= cl
VMDIR = VmPit
all:    windows
endif

ifeq ($(PLATFORM), pit) 
CC=GCC
VMDIR = VmPit
all:    pit
endif


ifeq ($(PLATFORM), solaris) 
all:    unix
endif


unix:	 nativeFunctionTableUnix.c ROMjavaUnix.c

pit:	 nativeFunctionTablePit.c ROMjavaPit.c


windows: nativeFunctionTableWin.c ROMjavaWin.c

tools: eraselists $(CLASSFILES) compilefiles 

ifneq ($(JCC_PASS_TWO), true)

ROMjava%.c: classes%.zip tools
	@cp -f src/*.properties classes
	@$(MAKE) $@ JCC_PASS_TWO=true

nativeFunctionTable%.c: classes%.zip tools
	@cp -f src/*.properties classes
	@$(MAKE) $@ JCC_PASS_TWO=true

nativeRelocation%.c: classes%.zip tools
	@cp -f src/*.properties classes
	@$(MAKE) $@ JCC_PASS_TWO=true

#Classes that aren't currently used for a particular platform
NON_Unix_CLASSES =

NON_Win_CLASSES =

NON_Pit_CLASSES = 'com/sun/kjava/*' \
	'com/sun/cldc/io/connections/*' \
	'com/sun/cldc/io/j2me/datagram/*' \
	'com/sun/cldc/io/j2me/serversocket/*' \
	'com/sun/cldc/io/j2me/socket/*' \
	'com/sun/cldc/io/j2me/storage/*' \
	'com/sun/cldc/io/j2me/testhttp/*' 







ifeq ($(INCLUDE_ALL_CLASSES), false)
CLASSES_TO_BE_EXCLUDED = \
		'com/sun/kjava/*' \
		'com/sun/cldc/io/DateParser.class' \
		'com/sun/cldc/io/HttpConnection.class' \
		'com/sun/cldc/io/NetworkConnectionBase.class' \
		'com/sun/cldc/io/j2se/*' \
		'com/sun/cldc/io/j2me/datagram/*' \
		'com/sun/cldc/io/j2me/events/*' \
		'com/sun/cldc/io/j2me/serversocket/*' \
		'com/sun/cldc/io/j2me/socket/*' \
		'com/sun/cldc/io/j2me/storage/*' \
		'com/sun/cldc/io/j2me/resource/*' \
		'com/sun/cldc/io/j2me/translate/*' \
		'com/sun/cldc/io/j2me/tckhttp/*' \
		'com/sun/cldc/io/j2me/testhttp/*'

NON_Unix_CLASSES += $(CLASSES_TO_BE_EXCLUDED)
NON_Win_CLASSES += $(CLASSES_TO_BE_EXCLUDED)
NON_Pit_CLASSES += $(CLASSES_TO_BE_EXCLUDED)
endif

classes%.zip: classes.zip
	@echo ... $@
	@cp classes.zip $@
	@echo gnumake should ignore zip error with emtpy args
	-zip -q $@ -d $($(patsubst classes%.zip,NON_%_CLASSES,$@))
	
classes.zip: $(TOP)/api/classes.zip
	@echo ... $@
	@cmp -s $< $@ || (echo cp $< $@ ; cp $< $@)


$(TOP)/api/classes.zip: .FORCE
	@(cd $(TOP)/api; $(MAKE) classes.zip)

else

# One of the possible architectures when creating ROMjava*.c
UnixArch = KVM
WinArch =  KVM
PitArch = KVM

ROMjava%.c: classes%.zip 
	@echo ... $@
	@$(JAVA) -classpath classes JavaCodeCompact \
	         -nq -arch $($(patsubst classes%.zip,%Arch,$<)) -o $@ $<

nativeFunctionTable%.c: classes%.zip
	@echo ... $@
	@cp -f src/*.properties classes
	@$(JAVA) -classpath classes JavaCodeCompact \
		 -nq -arch KVM_Native -o $@ $<

nativeRelocation%.c: classes%.zip
	@echo ... $@
	@$(JAVA) -classpath classes JavaCodeCompact \
	         -imageAttribute relocating \
	         -nq -arch $($(patsubst classes%.zip,%Arch,$<)) -o $@ $<

endif


clean:
	@rm -rf .filelist
	@rm -rf classes
	@rm -rf *.zip
	@rm -rf ROMjava*
	@rm -rf nativeFunctionTable*
	@rm -rf nativeRelocation*
	@rm -rf *~ */*~ */*/*~
	@rm -rf *# */*# */*/*#


# I have no idea why this is necessary, but otherwise gnumake deletes
# these as intermediate files
.SECONDARY:	classesUnix.zip classesWin.zip classesPit.zip

.FORCE:
