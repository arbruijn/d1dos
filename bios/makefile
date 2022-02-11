#
# $Source: f:/miner/source/bios/rcs/makefile $
# $Revision: 1.18 $
# $Author: john $
# $Date: 1995/03/23 18:27:04 $
#
# Makefile for bios functions
#

LIBDIR = ..\main
#

CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG

# What to install, and where to install it
INSTALL_TARGS = io.lib joy.h key.h mono.h mouse.h timer.h swift.h dpmi.h ipx.h

# What to clean
CLEAN_TARGS = $(OBJDIR)\*.obj

# What libs to make
LIBS = io

# What test programs to make
TEST_PROGS = demo
# testj testk testm testms

# What object files for the lib
IO_OBJS = key.obj joy.obj mouse.obj timer.obj mono.obj joyc.obj dpmi.obj ipx.obj

# What object files and libs for the test programs
DEMO_SRCS = demo.c
DEMO_LIBS = io.lib 

!include ..\wmake.def
