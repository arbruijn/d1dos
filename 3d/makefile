#
# $Source: f:/miner/source/3d/rcs/makefile $
# $Revision: 1.9 $
# $Author: matt $
# $Date: 1994/03/15 21:22:47 $
#
# Makefile for 3d library
#

LIBDIR = ..\main
#

CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG


# What to install
INSTALL_TARGS = 3d.lib 3d.h 3d.inc

# What libs to build
LIBS = 3d

# What objs in the lib
3D_OBJS = globvars.obj setup.obj matrix.obj points.obj draw.obj clipper.obj &
	horizon.obj instance.obj rod.obj interp.obj

# What test programs to make
TEST_PROGS = 3dtest

# What object files and libs for the test programs
3DTEST_SRCS = 3dtest.c segment.c trakball.c
3DTEST_LIBS = 3d.lib vecmat.lib gr.lib io.lib fix.lib div0.lib texmap.lib misc.lib bm.lib

#rules and targets common to all Miner makefiles

!include ..\wmake.def
