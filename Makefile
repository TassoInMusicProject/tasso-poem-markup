##
## Programmer:    Craig Stuart Sapp <craig@ccrma.stanford.edu>
## Creation Date: Thu Nov  2 11:50:37 PDT 2023
## Last Modified: Thu Nov  2 11:50:41 PDT 2023
## Syntax:        GNU Makefile
## URL:           https://github.com/TassoInMusicProject/tasso-poem-markup/Makefile
## vim:           ts=3
##
## Description:   Makefile to convert ATON markup data into TEI.
##

DATADIR = data

all:
	@echo "make update	== Typical use of this makefile."
	@echo "make download	== Download ATON files containing poetic markup."
	@echo "make tei		== Convert ATON files into TEI files."
	@echo "make index		== Create file list data/index.txt for use on TiMP website markup pages."
	@echo "make clean	== Delete ATON and TEI files containing poetic markup."

update: download tei index


tei:
	for aton in data/aton/*.aton;                                               \
	do                                                                          \
	   cat $$aton | bin/makeMarkupTei > data/tei/$$(basename $$aton .aton).tei; \
	done


download: clean
	bin/downloadAtonFiles -d data/aton


index:
	ls data/tei/* | sed "s/\.tei$$//; s/data\/tei\///" > data/index.txt


check: check-rhyme
rhyme: check-rhyme
check-rhyme:
	bin/checkRhyme data/aton/*.aton


clean:
	-rm $(DATADIR)/tei/*.tei
	-rm $(DATADIR)/aton/*.aton



