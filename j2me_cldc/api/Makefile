ifndef PLATFORM
PLATFORM=solaris
endif

JAVAC     = javac
PREVERIFY = ../tools/preverifier/build/$(PLATFORM)/preverify

all: tools

JAVAFILES =  $(shell find src -name "*.java"|grep -v SCCS|grep -v j2se)
 
CLASSFILES = $(subst src,classes,$(JAVAFILES:java=class))

# $< is dependency
# $@ is target
#
$(CLASSFILES): classes/%.class : src/%.java
	@echo $< >> .filelist

eraselists:
	@rm -f .filelist

compilefiles:
	@if [ '!' -d tmpclasses ]; then rm -rf tmpclasses; mkdir tmpclasses; fi;
	@if [ -f .filelist ]; then \
		echo $(JAVAC) -g:none -d tmpclasses -bootclasspath classes \
		      -classpath classes `cat .filelist`; \
		$(JAVAC) -g:none -d tmpclasses -bootclasspath classes \
		      -classpath classes `cat .filelist` || exit 1; \
		echo $(PREVERIFY) -d classes tmpclasses; \
		$(PREVERIFY) -d classes tmpclasses || exit 1; \
		fi

tools: eraselists $(CLASSFILES) compilefiles

classes.zip: tools
	@rm -rf classes.zip
	@(cd classes; zip -qr ../classes.zip .)
clean:
	rm -rf *.zip
	rm -rf .filelist
	rm -rf classes
	rm -rf tmpclasses
	rm -rf *~ */*~ */*/*~
	rm -rf *# */*# */*/*#

