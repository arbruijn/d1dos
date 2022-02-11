#
# $Source: f:/miner/source/texmap/rcs/makefile $
# $Revision: 1.23 $
# $Author: john $
# $Date: 1995/02/20 18:23:00 $
#
# Makefile for hack3df testbed
#

LIBDIR = ..\main
#

CCFLAGS = /dNETWORK /dRELEASE /dNDEBUG


#Uncomment the following 3 lines if you want to build with only C code.
#NASM = 1
#GLOBAL_CFLAGS+=/dNASM
#GLOBAL_AFLAGS+=/DNASM

# What to install
INSTALL_TARGS = texmap.lib texmap.h

# What libs to build
LIBS = texmap

# What objs in the TEXMAP lib
#TEXMAP_OBJS = ntmap.obj tmapsky.obj tmapv.obj tmapflat.obj tmap_per.obj tmap_p2.obj tmap_lin.obj tmap_ll.obj tmap_kl.obj\
#		tmap_ld.obj tmap_flt.obj tmap_sky.obj tmap_skv.obj tmap_fl1.obj tmapz.obj tmap_z.obj tmapfade.obj tmap_mul.obj

TEXMAP_OBJS = 	ntmap.obj tmapflat.obj  

#%if !%defined(NASM)
TEXMAP_OBJS +=	tmap_per.obj tmap_lin.obj tmap_ll.obj tmap_flt.obj tmapfade.obj tmap_fl1.obj
#%else
TEXMAP_OBJS += scanline.obj
#%endif

# What to clean
CLEAN_TARGS = $(OBJDIR)\*.obj

# What test programs to make
TEST_PROGS = textest

# What object files and libs for the test programs
TEXTEST_SRCS = TEXTEST.obj
TEXTEST_LIBS = TEXMAP.lib

!include ..\wmake.def
