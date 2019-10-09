.SUFFIXES:
.PHONY: run

WASM_CC_COMPILE=clang++.exe -c -DWASM=1 --target=wasm32 -fvisibility=hidden -std=c++14 -Wall -O2 -Wno-unused-variable -nostdlib -ffunction-sections -fdata-sections
WASM_CC_LINK=wasm-ld.exe --export-dynamic --no-entry --strip-all --gc-sections --allow-undefined --import-memory

run: gb-emu.exe
	find ../../data/cpu_instrs/individual/*.gb | while read f; do ./gb-emu.exe "$$f"; echo; done

main.wasm-o: main.o
	$(WASM_CC_COMPILE) -o main.wasm-o ../main.cc
../build/gb_emulator.wasm: main.wasm-o
	$(WASM_CC_LINK) -o $@ main.wasm-o

gb-emu.exe: main.o native_host.o deps.Makefile
	clang++.exe -o $@ main.o native_host.o

deps.Makefile: ../main.cc ../native_host.cc
	# clang -MM generates backslash-delimited paths, but Make wants forward-delimited paths
	# 
	clang++.exe -E -MM ../main.cc ../native_host.cc | sed 's/\\\([^\x0d]\)/\/\1/g' > deps.Makefile

include deps.Makefile

main.o: Makefile
	clang++.exe -D_CRT_SECURE_NO_WARNINGS -o $@ -c ../main.cc 

native_host.o: Makefile
	clang++.exe -D_CRT_SECURE_NO_WARNINGS -o $@ -c ../native_host.cc

