.EXTENSIONS:
.EXTENSIONS: .exe .obj .lib .def .c

LIBS = main.lib sosdigi.lib sosmidi.lib tactile.lib loadpats.lib commlib.lib

all: idpatch.exe binextr.exe gwbind.exe mklibs .SYMBOLIC

.c.obj:
	wcc386 /zq /i. $<
.obj.exe:
	wcl386 /zq $<

binextr.exe: binextr.obj loadle.obj loadmap.obj inslen.obj
	wcl386 /zq $<

clean: .SYMBOLIC
	del *.exe
	del *.obj
	del *.lib
	del *.asm

.def.lib:
	-mkdir tmp
	del /q tmp\*.*
	cd tmp
	..\binextr -e ..\..\original\descent2.exe -m ..\d2vertig.map ..\$<
	for %i in (*.asm) do ml /c %i
	%create tmp.lst
	for %i in (*.obj) do %append tmp.lst %i
	wlib ..\$@ @tmp.lst
	cd ..
	del /q tmp
	rmdir tmp
	copy $@ ..\source\ext
	
mklibs: $(LIBS) .SYMBOLIC
	wlib -x -d=..\source\ext main.lib
	