.EXTENSIONS:
.EXTENSIONS: .exe .obj .lib .def .c

SUBSYSTEMS = binextr

LIBS = main.lib sosdigi.lib sosmidi.lib loadpats.lib commlib.lib

all: binextr\binextr.exe gwbind.exe mklibs .SYMBOLIC

.c.obj:
	wcc386 /zq /i. /ibinextr $<
.obj.exe:
	wcl386 /zq $<

binextr\binextr.exe:
	cd binextr
	wmake
	cd ..

clean: .SYMBOLIC
	del *.exe
	del *.obj
	del *.lib
	del *.asm
	cd binextr
	wmake clean
	cd ..

.def.lib:
	-mkdir tmp
	del /q tmp\*.*
	cd tmp
	..\binextr\binextr -e ..\..\original\descentr.exe -m ..\descentr.map ..\$<
	for %i in (*.asm) do ml /c %i
	%create tmp.lst
	for %i in (*.obj) do %append tmp.lst %i
	wlib ..\$@ @tmp.lst
	cd ..
	del /q tmp
	rmdir tmp
	#copy $@ ..\external
	
mklibs: $(LIBS) .SYMBOLIC
	wlib -x -d=..\lib main.lib
