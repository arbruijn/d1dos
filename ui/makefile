#
# $Source: f:/miner/source/ui/rcs/makefile $
# $Revision: 1.8 $
# $Author: matt $
# $Date: 1994/11/27 14:53:50 $
#
# Makefile for user-interface functions
#

LIBDIR = ..\main
#

CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG


# What to install, and where to install it
INSTALL_TARGS = ui.lib ui.h func.h

# What to clean
CLEAN_TARGS = $(OBJDIR)\*.obj

# What libs to make
LIBS = ui

# What programs to make
TEST_PROGS = testg

# What object files for the lib
UI_OBJS = ui.obj mouse.obj window.obj uidraw.obj button.obj \
		  gadget.obj listbox.obj message.obj popup.obj scroll.obj \
		  radio.obj checkbox.obj inputbox.obj userbox.obj menu.obj \
		  harderr.obj file.obj keytrap.obj icon.obj keypress.obj \
		  menubar.obj keypad.obj 
#func.obj #Couldn't find func.c - where are the functions??

# What object files and libs for the test programs
TESTG_SRCS = testg.c
TESTG_LIBS = io.lib gr.lib ui.lib misc.lib mem.lib readfile.lib iff.lib func.lib cflib.lib cfile.lib
TESTG_LFLAGS = option stack=50k

!include ..\wmake.def
