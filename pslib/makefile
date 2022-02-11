#
# $Source: f:/miner/source/pslib/rcs/makefile $
# $Revision: 1.19 $
# $Author: john $
# $Date: 1994/08/05 17:25:56 $
#
# Makefile for compress library
#

LIBDIR = ..\main
#

CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG


# What to install, and where to install it
INSTALL_TARGS = cflib.lib cflib.h pslib.exe

# What to clean
CLEAN_TARGS = $(OBJDIR)\*.obj

# What libs to make
LIBS = cflib 

# What object files for the lib
CFLIB_OBJS = lzw.obj bitio.obj library.obj 
#cf.obj

# What test programs to make
PROGS = pslib
TEST_PROGS = cftest cftest2

# What object files and libs for the test programs
PSLIB_OBJS = pslib.obj
PSLIB_SRCS = pslib.c
PSLIB_LIBS = cflib.lib misc.lib mem.lib io.lib
CFTEST_OBJS = cftest.obj
CFTEST_SRCS = cftest.c
CFTEST_LIBS = cflib.lib misc.lib mem.lib io.lib
CFTEST2_OBJS = cftest2.obj
CFTEST2_SRCS = cftest2.c
CFTEST2_LIBS = cflib.lib misc.lib mem.lib io.lib

!include ..\wmake.def
