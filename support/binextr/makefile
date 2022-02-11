binextr.exe: binextr.obj loadle.obj loadmap.obj inslen.obj
	wcl386 /zq $<
.c.obj:
	wcc386 /zq /i. $<

clean: .SYMBOLIC
	del /q *.obj
	del /q *.exe
	