##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Wed Nov  1 15:26:42 PDT 2023
## Last Modified: Wed Nov  1 15:26:46 PDT 2023
## Syntax:        GNU Makefile
## Filename:      humlib/Makefile
## vim:           ts=3
##
## Description: Makefile to run tasks for humlib library, particularly compiling code.
##
## The converter program relies (loosely) on humlib (https://github.org/craigsapp/humlib).
## Install humlib first, and then update the location of humlib in the following
## variables by typing "make setup".   Or you can type "make" to setup the symbolic
## links to humlib and compile the code in one step, or type "make compile" to compile
## without doing the setup stage (after the setup stage has been done once).
##

HUMLIB = /Users/craig/git-cloud/humdrum/humlib
INCDIR = $(HUMLIB)/include
LIBDIR = $(HUMLIB)/lib

all: setup compile install


setup: clean
	ln -s $(INCDIR) include
	ln -s $(LIBDIR) lib


compile:
	g++ -std=c++17 -O3 -I$(INCDIR) makeMarkupTei.cpp -L$(LIBDIR) -lpugixml -lhumlib -o makeMarkupTei


clean:
	-rm include
	-rm lib


install:
	cp makeMarkupTei ../bin
	strip ../bin/makeMarkupTei


test:
	cat test.aton | ./makeMarkupTei > test.tei



