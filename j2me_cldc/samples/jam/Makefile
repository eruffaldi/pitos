#
# @(#)Makefile	1.2 00/03/23
#
# Copyright (c) 1999 Sun Microsystems, Inc. All Rights Reserved.
# 
# This software is the confidential and proprietary information of Sun
# Microsystems, Inc. ("Confidential Information").  You shall not
# disclose such Confidential Information and shall use it only in
# accordance with the terms of the license agreement you entered into
# with Sun.
# 
# SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
# SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
# SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
# THIS SOFTWARE OR ITS DERIVATIVES.
# 

TOP=../..
OS=$(shell uname -s)

ifneq (,$(findstring SunOS,$(OS)))
export PLATFORM=solaris
PATHSEP=:
else
export PLATFORM=win32
PATHSEP=\;
endif

PREVERIFY = $(TOP)/tools/preverifier/build/$(PLATFORM)/preverify
JAVAC     = javac
JAR       = jar

BCP = $(TOP)/api/classes

all: HelloWorld.jam HelloWorld.jar

JARSIZE=$(shell wc -c $(subst jam,jar,$@) | cut -b1-8)

%.jam: %.jar
	sed "s/%%JarSize%%/$(JARSIZE)/" src/$@.template > $@

%.jar: src/%.java src/%.jam.template
	rm -rf tmpclasses1
	rm -rf tmpclasses2
	mkdir tmpclasses1
	mkdir tmpclasses2
	$(JAVAC) -d tmpclasses1 -bootclasspath $(BCP) src/$*.java
	$(PREVERIFY) -classpath $(BCP) -d tmpclasses2 tmpclasses1
	cd tmpclasses2; $(JAR) cvf ../$@ .; cd ..

clean:
	rm -rf tmpclasses1 tmpclasses2
	rm -f *.class
	rm -f *.jam
	rm -f *.jar

