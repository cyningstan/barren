# ======================================================================
# KeyLib
# A keyboard handler for DOS games.
#
# Copyright (C) Damian Gareth Walker 2021.
#
# Makefile.

# Memory model
MODEL = ml

# Directories
BINDIR = bin
SRCDIR = src
INCDIR = inc
DOCDIR = doc
OBJDIR = obj
TGTDIR = keylib

# Tool commands and their options
CC = wcc
LIB = wlib
LD = wcl
COPTS = -q -0 -W4 -I=$(INCDIR) -$(MODEL)
LOPTS = -q
!ifdef __LINUX__
CP = cp
!else
CP = copy
!endif

# Whole project
all : &
	$(TGTDIR)/demo.exe &
	$(TGTDIR)/key-$(MODEL).lib &
	$(TGTDIR)/keylib.h &
	$(TGTDIR)/keylib.txt

# Executables
$(TGTDIR)/demo.exe : $(OBJDIR)/demo.o $(TGTDIR)/key-$(MODEL).lib
	$(LD) $(LOPTS) -fe=$@ $<

# Libraries
$(TGTDIR)/key-$(MODEL).lib : &
	$(OBJDIR)/keylib.o
	$(LIB) $(LIBOPTS) $@ +-$(OBJDIR)/keylib.o

# Header files in target directory
$(TGTDIR)/keylib.h : $(INCDIR)/keylib.h
	$(CP) $< $@

# Documentation in target directory
$(TGTDIR)/keylib.txt : $(DOCDIR)/keylib.txt
	$(CP) $< $@

# Object files for the demonstration
$(OBJDIR)/demo.o : $(SRCDIR)/demo.c $(INCDIR)/keylib.h
	$(CC) $(COPTS) -fo=$@ $[@

# Object files for the library module
$(OBJDIR)/keylib.o : $(SRCDIR)/keylib.c $(INCDIR)/keylib.h
	$(CC) $(COPTS) -ml -fo=$@ $[@
