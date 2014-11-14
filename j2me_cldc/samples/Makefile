ifndef PLATFORM
PLATFORM=solaris
endif

PREVERIFY = ../tools/preverifier/build/$(PLATFORM)/preverify
JAVAC     = javac

all: tools

JAVAFILES =  $(wildcard src/*.java src/missiles/*.java src/dots/*.java)

CLASSFILES = $(subst src,classes,$(JAVAFILES:java=class))

# $< is dependency
# $@ is target

$(CLASSFILES): classes/%.class : src/%.java
	@echo $< >> .filelist

eraselists:
	@rm -f .filelist

compilefiles:
	@if [ '!' -d tmpclasses ]; then rm -rf tmpclasses; mkdir tmpclasses; fi;
	@if [ -f .filelist ]; then \
		echo $(JAVAC) -g:none -d tmpclasses \
		      -classpath tmpclasses:../api/classes \
	              -bootclasspath ../api/classes `cat .filelist`; \
		$(JAVAC) -g:none -d tmpclasses \
                      -classpath tmpclasses:../api/classes \
	              -bootclasspath ../api/classes \
                      `cat .filelist` || exit 1; \
		echo $(PREVERIFY) -d classes -classpath ../api/classes \
                      tmpclasses; \
		$(PREVERIFY) -d classes -classpath ../api/classes \
                      tmpclasses || exit 1; \
		fi

tools: eraselists $(CLASSFILES) compilefiles

	
classes.zip: tools
	@rm -rf classes.zip
	@(cd classes; zip -qr ../classes.zip .)
	@(cd ../api/classes; zip -qr ../../samples/classes.zip .)

clean:
	rm -rf *.zip
	rm -rf .filelist
	rm -rf classes
	rm -rf tmpclasses
	rm -rf *~ */*~ */*/*~
	rm -rf *# */*# */*/*#

