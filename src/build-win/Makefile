.SUFFIXES:
.PHONY: run

WASM_CC_COMPILE=clang++.exe -c -DWASM=1 --target=wasm32 -fvisibility=hidden -std=c++14 -Wall -O2 -Wno-unused-variable -nostdlib -ffunction-sections -fdata-sections -I .. -I ../utils
WASM_CC_LINK=wasm-ld.exe --export-dynamic --no-entry --strip-all --gc-sections --allow-undefined --import-memory

run: gb-emu.exe
	./gb-emu.exe ../../data/cpu_instrs/cpu_instrs.gb

run_blargg: gb-emu.exe
	find ../../data/cpu_instrs/individual/*.gb | while read f; do ./gb-emu.exe "$$f"; echo; done

main.wasm-o: main.o
	$(WASM_CC_COMPILE) -o main.wasm-o ../main.cpp

platform.wasm-o: platform_wasm.o
	$(WASM_CC_COMPILE) -o platform.wasm-o ../platform_wasm.cpp

../../wasmgui/build/gb_emulator.wasm: main.wasm-o platform.wasm-o
	mkdir -p ../../wasmgui/build
	$(WASM_CC_LINK) -o $@ main.wasm-o platform.wasm-o

gb-emu.exe: deps.Makefile
gb-emu.exe: lib_gb.o main.o
	clang++.exe -o $@ -flto -O2 lib_gb.o main.o \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/User32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/OpenGL32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/Gdi32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/Comdlg32.Lib \

# clang -MM generates backslash-delimited paths, but Make wants forward-delimited paths
deps.Makefile: ../lib_gb.cpp ../platform_windows/main.cpp ../platform_wasm.cpp ../platform_linux/main.cpp
	clang++.exe -E -MM $^ | sed 's/\\\([^\x0d]\)/\/\1/g' > deps.Makefile

include deps.Makefile

platform_wasm.o:
	@true

%.o: ../%.cpp Makefile
	clang++.exe -D_CRT_SECURE_NO_WARNINGS -Wall -pedantic -O2 -o $@ -c $< -I .. -I ../utils
%%/%.o: ../%%/%.cpp Makefile
	clang++.exe -D_CRT_SECURE_NO_WARNINGS -Wall -pedantic -O2 -o $@ -c $< -I .. -I ../utils
