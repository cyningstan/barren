# ======================================================================
# Barren Planet
# A turn-based strategy game.
#
# Copyright (C) Damian Gareth Walker 2020. Released under the GNU GPL.
# Created: 29-Dec-2020.
#
# Makefile for building using OpenWatcom C.
#

# Don't prompt for deleting after errors.
.ERASE

# Paths (this project)
SRCDIR = src
INCDIR = inc
ASSDIR = assets
CAMDIR = campaign
DOCDIR = doc
OBJDIR = obj
LIBDIR = lib
BINDIR = bin
TGTDIR = barren

# Paths (CGALIB)
CGADIR = cgalib
CGALIB = $(CGADIR)\cgalib
CGAINC = $(CGADIR)\inc

# Paths (CWG)
CWGDIR = cwg
CWGLIB = $(CWGDIR)\lib
CWGINC = $(CWGDIR)\inc

# Paths (KEYLIB)
KEYDIR = keylib
KEYLIB = keylib\keylib
KEYINC = keylib\inc

# Paths (SPKLIB)
SPKDIR = spklib
SPKLIB = spklib\spklib
SPKINC = spklib\inc

# Paths (LOGLIB)
LOGDIR = loglib
LOGLIB = loglib\lib
LOGINC = loglib\inc

# Tools
CC = wcc
LD = wcl
AR = wlib
CP = copy
TOUCH = $(BINDIR)\touch.exe
MKASSETS = $(BINDIR)\mkassets
MKCAMP = $(CAMDIR)\mkcamp $(CAMDIR)\barren
MKTUNE = $(SPKLIB)\maketune
RM = del

# Compiler flags
CCOPTS = -q -0 -w4 -ml -e1 -we -wcd=303 &
	-i=$(INCDIR) -i=$(CGAINC) -i=$(CWGINC) -i=$(KEYINC) &
	-i=$(SPKINC) -i=$(LOGINC)
LDOPTS = -q

#
# Main Targets
#

# Everything for the game
GAME : $(TGTDIR)\barren.exe &
	$(TGTDIR)\BARREN.DAT &
	$(TGTDIR)\BARREN.CAM

# Development tools
TOOLS : $(BINDIR)\playtest.exe

# Main Binary
$(TGTDIR)\barren.exe : &
	$(OBJDIR)\barren.obj &
	$(CGALIB)\cgalib.lib &
	$(CWGLIB)\cwg.lib &
	$(KEYLIB)\key-ml.lib &
	$(SPKLIB)\spk-ml.lib &
	$(OBJDIR)\fatal.obj &
	$(OBJDIR)\display.obj &
	$(OBJDIR)\controls.obj &
	$(OBJDIR)\config.obj &
	$(OBJDIR)\campaign.obj &
	$(OBJDIR)\scenario.obj &
	$(OBJDIR)\report.obj &
	$(OBJDIR)\game.obj &
	$(OBJDIR)\turn.obj &
	$(OBJDIR)\uiscreen.obj &
	$(OBJDIR)\uigame.obj &
	$(OBJDIR)\uibrief.obj &
	$(OBJDIR)\uireport.obj &
	$(OBJDIR)\uihuman.obj &
	$(OBJDIR)\uicomp.obj &
	$(OBJDIR)\uipbm.obj &
	$(OBJDIR)\uidbrief.obj &
	$(OBJDIR)\uiend.obj &
	$(OBJDIR)\timer.obj &
	$(OBJDIR)\ai.obj &
	$(OBJDIR)\beta.obj
	*$(LD) $(LDOPTS) -fe=$@ $<

# Asset Generation Binary
$(BINDIR)\mkassets.exe : &
	$(OBJDIR)\mkassets.obj &
	$(OBJDIR)\fatal.obj &
	$(CGALIB)\cgalib.lib &
	$(SPKLIB)\spk-ml.lib
	*$(LD) $(LDOPTS) -fe=$@ $<

# Campaign Generation Binary
$(CAMDIR)\mkcamp.exe : &
	$(OBJDIR)\mkcamp.obj &
	$(OBJDIR)\fatal.obj &
	$(OBJDIR)\campaign.obj &
	$(OBJDIR)\scenario.obj &
	$(OBJDIR)\debug.obj &
	$(CGALIB)\cgalib.lib &
	$(CWGLIB)\cwg.lib
	*$(LD) $(LDOPTS) -fe=$@ $<

# Playtest Binary
$(BINDIR)\playtest.exe : &
	$(OBJDIR)\playtest.obj &
	$(CWGLIB)\cwg.lib &
	$(CGALIB)\cgalib.lib &
	$(OBJDIR)\fatal.obj &
	$(OBJDIR)\campaign.obj &
	$(OBJDIR)\scenario.obj &
	$(OBJDIR)\report.obj &
	$(OBJDIR)\game.obj &
	$(OBJDIR)\config.obj &
	$(OBJDIR)\ai.obj
	*$(LD) $(LDOPTS) -fe=$@ $<

# Main Asset File
$(TGTDIR)\barren.dat : &
	$(ASSDIR)\barren0.pic &
	$(ASSDIR)\barren1.pic &
	$(ASSDIR)\music.tun &
	$(BINDIR)\mkassets.exe
	$(MKASSETS)

# Music
$(ASSDIR)\music.tun : &
	$(ASSDIR)\music.txt
	$(MKTUNE) $< $@

# Main Campaign File
$(TGTDIR)\barren.cam : $(CAMDIR)\barren.cmi &
	$(CAMDIR)\BARREN.PIC &
	$(CAMDIR)\mkcamp.exe
	$(MKCAMP)
	$(CP) $(CAMDIR)\BARREN.CAM $(TGTDIR)
	$(RM) $(CAMDIR)\BARREN.CAM

# Touch Utility
$(BINDIR)\touch.exe : &
	$(OBJDIR)\touch.obj
	*$(LD) $(LDOPTS) -fe=$@ $<

#
# Object Files
#

# Main barren module
$(OBJDIR)\barren.obj : &
	$(SRCDIR)\barren.c &
	$(INCDIR)\beta.h &
	$(INCDIR)\barren.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\config.h &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\game.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Beta Test Hander Module
$(OBJDIR)\beta.obj: &
	$(SRCDIR)\beta.c &
	$(INCDIR)\fatal.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Display module
$(OBJDIR)\display.obj : &
	$(SRCDIR)\display.c &
	$(CGAINC)\cgalib.h &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\display.h &
	$(SPKINC)\speaker.h &
	$(SPKINC)\effect.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\game.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Game controls handler module
$(OBJDIR)\controls.obj : &
	$(SRCDIR)\controls.c &
	$(KEYINC)\keylib.h &
	$(INCDIR)\controls.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Fatal error module
$(OBJDIR)\fatal.obj : &
	$(SRCDIR)\fatal.c &
	$(INCDIR)\fatal.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Campaign module
$(OBJDIR)\campaign.obj : &
	$(SRCDIR)\campaign.c &
	$(CGAINC)\cgalib.h &
	$(CWGINC)\cwg.h &
	$(CWGINC)\battle.h &
	$(CWGINC)\utype.h &
	$(CWGINC)\terrain.h &
	$(CWGINC)\map.h &
	$(CWGINC)\unit.h &
	$(INCDIR)\campaign.h &
	$(INCDIR)\scenario.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Scenario module
$(OBJDIR)\scenario.obj : &
	$(SRCDIR)\scenario.c &
	$(CWGINC)\cwg.h &
	$(CWGINC)\battle.h &
	$(INCDIR)\scenario.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Initialisation file module
$(OBJDIR)\config.obj : &
	$(SRCDIR)\config.c &
	$(CWGINC)\cwg.h &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\config.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Report module
$(OBJDIR)\report.obj : &
	$(SRCDIR)\report.c &
	$(INCDIR)\barren.h &
	$(INCDIR)\report.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Game module
$(OBJDIR)\game.obj : &
	$(SRCDIR)\game.c &
	$(CWGINC)\cwg.h &
	$(INCDIR)\game.h &
	$(INCDIR)\report.h &
	$(INCDIR)\config.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# PBM Turn module
$(OBJDIR)\turn.obj : &
	$(SRCDIR)\turn.c &
	$(CWGINC)\cwg.h &
	$(INCDIR)\game.h &
	$(INCDIR)\turn.h &
	$(INCDIR)\report.h &
	$(INCDIR)\config.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# User Interface Module
$(OBJDIR)\uiscreen.obj : &
	$(SRCDIR)\uiscreen.c &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\game.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# New Game Screen Module
$(OBJDIR)\uigame.obj : &
	$(SRCDIR)\uigame.c &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\config.h &
	$(INCDIR)\game.h &
	$(INCDIR)\campaign.h &
	$(INCDIR)\scenario.h &
	$(INCDIR)\timer.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Briefing Screen Module
$(OBJDIR)\uibrief.obj : &
	$(SRCDIR)\uibrief.c &
	$(CWGINC)\cwg.h &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\game.h &
	$(INCDIR)\campaign.h &
	$(INCDIR)\scenario.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Turn Report Screen Module
$(OBJDIR)\uireport.obj : &
	$(SRCDIR)\uireport.c &
	$(CWGINC)\cwg.h &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\config.h &
	$(INCDIR)\game.h &
	$(INCDIR)\timer.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Human Player Turn Screen Module
$(OBJDIR)\uihuman.obj : &
	$(SRCDIR)\uihuman.c &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\config.h &
	$(INCDIR)\game.h &
	$(INCDIR)\timer.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Computer Player Turn Screen Module
$(OBJDIR)\uicomp.obj : &
	$(SRCDIR)\uicomp.c &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\config.h &
	$(INCDIR)\game.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# PBM Player Turn Screen Module
$(OBJDIR)\uipbm.obj : &
	$(SRCDIR)\uipbm.c &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\config.h &
	$(INCDIR)\game.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Debriefing Screen Module
$(OBJDIR)\uidbrief.obj : &
	$(SRCDIR)\uidbrief.c &
	$(CWGINC)\cwg.h &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\game.h &
	$(INCDIR)\campaign.h &
	$(INCDIR)\scenario.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Debriefing Screen Module
$(OBJDIR)\uiend.obj : &
	$(SRCDIR)\uiend.c &
	$(CWGINC)\cwg.h &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\uiscreen.h &
	$(INCDIR)\display.h &
	$(INCDIR)\controls.h &
	$(INCDIR)\game.h &
	$(INCDIR)\campaign.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Timer Module
$(OBJDIR)\timer.obj : &
	$(SRCDIR)\timer.c &
	$(INCDIR)\timer.h &
	$(INCDIR)\fatal.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# AI Module
$(OBJDIR)\ai.obj : &
	$(SRCDIR)\ai.c &
	$(INCDIR)\ai.h &
	$(CWGINC)\cwg.h &
	$(INCDIR)\barren.h &
	$(INCDIR)\game.h &
	$(INCDIR)\campaign.h &
	$(INCDIR)\report.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Asset generator
$(OBJDIR)\mkassets.obj : &
	$(SRCDIR)\mkassets.c &
	$(CGAINC)\cgalib.h &
	$(INCDIR)\fatal.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Campaign generator
$(OBJDIR)\mkcamp.obj : &
	$(SRCDIR)\mkcamp.c &
	$(CGAINC)\cgalib.h &
	$(INCDIR)\fatal.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Main barren module
$(OBJDIR)\playtest.obj : &
	$(SRCDIR)\playtest.c &
	$(CWGINC)\cwg.h &
	$(INCDIR)\barren.h &
	$(INCDIR)\fatal.h &
	$(INCDIR)\campaign.h &
	$(INCDIR)\scenario.h &
	$(INCDIR)\report.h &
	$(INCDIR)\game.h &
	$(INCDIR)\ai.h
	*$(CC) $(CCOPTS) -fo=$@ $[@

# Debug handler module
$(OBJDIR)\debug.obj : &
	$(SRCDIR)\debug.c &
	$(INCDIR)\debug.h
	**$(CC) $(CCOPTS) -fo=$@ $[@

# Touch utility
$(OBJDIR)\touch.obj : &
	$(SRCDIR)\touch.c
	*$(CC) $(CCOPTS) -fo=$@ $[@

#
# Header Dependencies
#

# Barren Planet header
$(INCDIR)\barren.h : &
	$(TOUCH) &
	$(INCDIR)\controls.h &
	$(INCDIR)\display.h &
	$(INCDIR)\config.h
	$(TOUCH) $@

# Campaign header
$(INCDIR)\campaign.h : &
	$(TOUCH) &
	$(CWGINC)\cwg.h &
	$(CGAINC)\cgalib.h &
	$(INCDIR)\scenario.h
	$(TOUCH) $@

# Configuration header
$(INCDIR)\config.h : &
	$(TOUCH) &
	$(INCDIR)\uiscreen.h
	$(TOUCH) $@

# Display header
$(INCDIR)\display.h : &
	$(TOUCH) &
	$(CWGINC)\cwg.h &
	$(INCDIR)\game.h
	$(TOUCH) $@

# Fatal Error header
$(INCDIR)\fatal.h : &
	$(TOUCH) &
	$(CGAINC)\cgalib.h &
	$(INCDIR)\display.h
	$(TOUCH) $@

# Game header
$(INCDIR)\game.h : &
	$(TOUCH) &
	$(CWGINC)\cwg.h &
	$(INCDIR)\campaign.h &
	$(INCDIR)\report.h
	$(TOUCH) $@

# Scenario header
$(INCDIR)\scenario.h : &
	$(TOUCH) &
	$(CWGINC)\cwg.h
	$(TOUCH) $@

# User Interface Screen header
$(INCDIR)\uiscreen.h : &
	$(TOUCH) &
	$(INCDIR)\game.h
	$(TOUCH) $@

# PBM Turn header
$(INCDIR)\turn.h : &
	$(TOUCH) &
	$(CWGINC)\cwg.h &
	$(INCDIR)\campaign.h &
	$(INCDIR)\game.h &
	$(INCDIR)\report.h
	$(TOUCH) $@

# Controls Handler Header
$(INCDIR)\controls.h : &
	$(TOUCH) &
	$(KEYINC)\keylib.h
	$(TOUCH) $@
