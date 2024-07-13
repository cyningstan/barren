# ======================================================================
# SPKLIB
# A PC Speaker Sound Library
#
# Copyright (C) Damian Gareth Walker 2021.
#
# Project Makefile

# Don't prompt for deleting after errors.
.ERASE

# Set memory model
MODEL = ml

# Directories
BINDIR = bin
SRCDIR = src
INCDIR = inc
DOCDIR = doc
OBJDIR = obj
TGTDIR = spklib

# Keylib directories
!ifdef KEYLIB
KEYDIR = $(KEYLIB)\keylib
KEYINC = $(KEYLIB)\inc
KEYDEF = -dKEYLIB
KEYLNK = $(KEYDIR)\key-$(MODEL).lib
!endif

# Tool commands and their options
CC = wcc
LIB = wlib
LD = wcl
COPTS = -q -0 -W4 -e1 -we -I=$(INCDIR) -I=$(KEYINC) -$(MODEL) $(KEYDEF)
LOPTS = -q
CP = copy

# Whole project
all : &
	$(TGTDIR)\demo.exe &
	$(TGTDIR)\fxdemo.exe &
	$(TGTDIR)\maketune.exe &
	$(TGTDIR)\playback.exe &
	$(TGTDIR)\demo.tun &
	$(TGTDIR)\spk-$(MODEL).lib &
	$(TGTDIR)\speaker.h &
	$(TGTDIR)\tune.h &
	$(TGTDIR)\effect.h

# Executables
$(TGTDIR)\demo.exe : $(OBJDIR)\demo.o $(TGTDIR)\spk-$(MODEL).lib $(KEYLNK)
	$(LD) $(LOPTS) -fe=$@ $<
$(TGTDIR)\fxdemo.exe : $(OBJDIR)\fxdemo.o $(TGTDIR)\spk-$(MODEL).lib
	$(LD) $(LOPTS) -fe=$@ $<
$(TGTDIR)\maketune.exe : $(OBJDIR)\maketune.o &
	$(TGTDIR)\spk-$(MODEL).lib &
	$(KEYLNK)
	$(LD) $(LOPTS) -fe=$@ $<
$(TGTDIR)\playback.exe : $(OBJDIR)\playback.o &
	$(TGTDIR)\spk-$(MODEL).lib &
	$(KEYLNK)
	$(LD) $(LOPTS) -fe=$@ $<

# Libraries
$(TGTDIR)\spk-$(MODEL).lib : &
	$(OBJDIR)\speaker.o $(OBJDIR)\tune.o $(OBJDIR)\effect.o
	$(LIB) $(LIBOPTS) $@ &
	+-$(OBJDIR)\speaker.o &
	+-$(OBJDIR)\tune.o &
	+-$(OBJDIR)\effect.o

# Header files in target directory
$(TGTDIR)\speaker.h : $(INCDIR)\speaker.h
	$(CP) $< $@
$(TGTDIR)\tune.h : $(INCDIR)\tune.h
	$(CP) $< $@
$(TGTDIR)\effect.h : $(INCDIR)\effect.h
	$(CP) $< $@

# The demonstration tune
$(TGTDIR)\demo.tun : $(DOCDIR)\grieg.txt $(TGTDIR)\maketune.exe
	$(TGTDIR)\maketune.exe $[@ $@

# Object files for the demonstration
$(OBJDIR)\demo.o : $(SRCDIR)\demo.c $(INCDIR)\speaker.h $(INCDIR)\tune.h
	$(CC) $(COPTS) -fo=$@ $[@
$(OBJDIR)\fxdemo.o : $(SRCDIR)\fxdemo.c $(INCDIR)\speaker.h $(INCDIR)\effect.h
	$(CC) $(COPTS) -fo=$@ $[@

# Object file for the maketune utility
$(OBJDIR)\maketune.o : $(SRCDIR)\maketune.c &
	$(INCDIR)\speaker.h &
	$(INCDIR)\tune.h
	$(CC) $(COPTS) -fo=$@ $[@

# Object file for playback utility
$(OBJDIR)\playback.o : $(SRCDIR)\playback.c &
	$(INCDIR)\speaker.h &
	$(INCDIR)\tune.h
	$(CC) $(COPTS) -fo=$@ $[@

# Object files for the tune module
$(OBJDIR)\speaker.o : $(SRCDIR)\speaker.c $(INCDIR)\speaker.h $(INCDIR)\tune.h
	$(CC) $(COPTS) -fo=$@ $[@
$(OBJDIR)\tune.o : $(SRCDIR)\tune.c $(INCDIR)\speaker.h $(INCDIR)\tune.h
	$(CC) $(COPTS) -fo=$@ $[@
$(OBJDIR)\effect.o : $(SRCDIR)\effect.c $(INCDIR)\speaker.h $(INCDIR)\effect.h
	$(CC) $(COPTS) -fo=$@ $[@
