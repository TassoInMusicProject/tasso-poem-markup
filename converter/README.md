This directory contains the converter from basic markup encoding in ATON format.
(done by humans) to TEI via automatic conversion by the makeMarkupTei program.

Instructions 
=============

(1) Install humlib

(2) Set the HUMLIB variable to the location of humlib in the makefile:

```make
HUMLIB = /Users/craig/git-cloud/humdrum/humlib
```

(3) Type "make"


Files in this directory
=======================


| File | Description | 
| ---- | ----------- |
| Makefile | Makefile to compile and manage converter |
| README.md | This file |
| include | humlib include directory (symbolic link after running "make setup") |
| lib | humlib lib directory (symbolic link after running "make setup") |
| makeMarkupTei | Executable program after compiling |
| makeMarkupTei.cpp | Source code for program |
| template.xml | Template TEI file from Raffaele |
| test.aton | Test input file |
| test.tei | Test output file (after "make test" is run) |



