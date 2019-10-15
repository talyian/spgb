.SUFFIXES:
.PHONY: run

WASM_CC_COMPILE=clang++.exe -c -DWASM=1 --target=wasm32 -fvisibility=hidden -std=c++14 -Wall -O2 -Wno-unused-variable -nostdlib -ffunction-sections -fdata-sections
WASM_CC_LINK=wasm-ld.exe --export-dynamic --no-entry --strip-all --gc-sections --allow-undefined --import-memory

run: gb-emu.exe
	./gb-emu.exe ../../data/cpu_instrs/cpu_instrs.gb

run_blargg: gb-emu.exe
	find ../../data/cpu_instrs/individual/*.gb | while read f; do ./gb-emu.exe "$$f"; echo; done

main.wasm-o: main.o
	$(WASM_CC_COMPILE) -o main.wasm-o ../main.cc

platform.wasm-o: platform_wasm.o
	$(WASM_CC_COMPILE) -o platform.wasm-o ../platform_wasm.cc

../build/gb_emulator.wasm: main.wasm-o platform.wasm-o
	$(WASM_CC_LINK) -o $@ main.wasm-o platform.wasm-o

gb-emu.exe: deps.Makefile
gb-emu.exe: main.o platform_windows.o
	clang++.exe -o $@ -fsanitize=undefined -flto -O2 main.o platform_windows.o \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/User32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/OpenGL32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/Gdi32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/Comdlg32.Lib \


# clang -MM generates backslash-delimited paths, but Make wants forward-delimited paths
deps.Makefile: ../main.cc ../platform_windows.cc ../platform_wasm.cc ../platform_linux.cc
	clang++.exe -E -MM ../main.cc ../platform_windows.cc ../platform_linux.cc ../platform_wasm.cc | sed 's/\\\([^\x0d]\)/\/\1/g' > deps.Makefile

include deps.Makefile

platform_wasm.o:
	# ok

%.o: ../%.cc Makefile
	clang++.exe -D_CRT_SECURE_NO_WARNINGS -fsanitize=undefined -O2 -o $@ -c $<
