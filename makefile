#
# Master makefile for Miner source
#
# Makes all the executable by visiting subdirs and making there
#

# The only thing to make is the subdirs
SUBSYSTEMS = lib support includes mem misc fix cfile 2d bios iff div vecmat 3d texmap main

# What to make in the subdirs if nothing specified
SUBTARGETS = optimize no_mono no_debug linstall		#for release
##SUBTARGETS = linstall					#for debugging

# When making clean here, delete libs
CLEAN_TARGS = lib\*.lib

default: .SYMBOLIC
	cd bios
	wmake linstallh
	cd ..\\misc
	wmake linstallh
	cd ..\\ui
	wmake linstallh
	cd ..\\pslib
	wmake linstallh
	cd ..
	%make lsub
	%make bind

bind: .SYMBOLIC
	support\\gwbind original\\descentr.exe main\\descentr.exe

real_clean: .SYMBOLIC
	make clean
	del lib\*.h
	del lib\*.inc
	%if %exists(lib\win)
	  del lib\win\*.h
	  del lib\win\*.inc
	  del lib\win\*.lib
	%endif

!include wmake.def
