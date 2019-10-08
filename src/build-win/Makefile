.SUFFIXES:
.PHONY: run


run: gb-emu.exe
	./gb-emu.exe ../../data/cpu_instrs/individual/01-special.gb

gb-emu.exe: main.o native_host.o deps.Makefile
	clang++.exe -o $@ main.o native_host.o

deps.Makefile: ../main.cc ../native_host.cc
	clang++.exe -E -MM ../main.cc ../native_host.cc | sed 's/\\\([^\x0d]\)/\/\1/g' > deps.Makefile

include deps.Makefile

main.o: Makefile
	clang++.exe -D_CRT_SECURE_NO_WARNINGS -o $@ -c ../main.cc 

native_host.o: Makefile
	clang++.exe -D_CRT_SECURE_NO_WARNINGS -o $@ -c ../native_host.cc

