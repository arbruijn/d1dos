#
# Makefile for DESCENT execuable
#

DESCENT_LIBS =	..\\lib\\3d.lib ..\\lib\\gr.lib ..\\lib\\fix.lib &
				..\\lib\\io.lib ..\\lib\\iff.lib ..\\lib\\vecmat.lib &
				..\\lib\\cfile.lib &
				..\\lib\\mem.lib &
				..\\lib\\div0.lib ..\\lib\\misc.lib ..\\lib\\texmap.lib &
				..\\support\\sosdigi.lib &
				..\\support\\sosmidi.lib &
				..\\support\\loadpats.lib &
				..\\support\\commlib.lib

#		sosdigi.lib #sosmidi.lib &
		#iglasses.lib gclfr32.lib loadpats.lib
		#lzw.lib sosdwxcr.lib sosmwxcr.lib 

DESCENT_OBJS = inferno.obj game.obj gauges.obj mglobal.obj &
 	         scores.obj object.obj laser.obj physics.obj bm.obj &
             menu.obj gamesave.obj switch.obj effects.obj texmerge.obj polyobj.obj &
             gamemine.obj fireball.obj ai.obj aipath.obj hostage.obj &
             powerup.obj fuelcen.obj digi.obj render.obj hash.obj piggy.obj args.obj &
             gameseg.obj wall.obj collide.obj lighting.obj &
             robot.obj morph.obj vclip.obj weapon.obj fvi.obj newdemo.obj titles.obj &
             vfx.obj &
             ..\lib\vfxread.obj &
             gameseq.obj controls.obj automap.obj text.obj &
             network.obj victor.obj newmenu.obj netmisc.obj &
             gamefont.obj joydefs.obj hud.obj playsave.obj &
             endlevel.obj terrain.obj kconfig.obj modem.obj  &
             multi.obj cntrlcen.obj credits.obj config.obj soscodec.obj kmatrix.obj &
             paging.obj mission.obj iglasses.obj songs.obj bmread.obj multibot.obj &
             state.obj
             #nocomlib.obj slew.obj dumpmine.obj 
             #demo.obj garage.obj radar.obj coindev.obj serial.obj vfxread.obj victor.obj


GLOBAL_INCFILES = settings.h
GLOBAL_CFLAGS = /fi=$(GLOBAL_INCFILES)

default: descentr.exe .SYMBOLIC

descent.exe: $(DESCENT_OBJS) $(DESCENT_LIBS)
	*wlink option map,stack=50k,stub=$(%watcom)\binb\wstub.exe name $@ sys dos4g file { $(DESCENT_OBJS) } library { $(DESCENT_LIBS) }

descentr.exe: descent.exe
	copy descent.exe $@

LCLEAN = 1
lclean: .SYMBOLIC
	del /q descent.exe
	del /q descentr.exe

!include ..\wmake.def
