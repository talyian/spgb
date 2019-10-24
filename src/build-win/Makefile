.SUFFIXES:
.PHONY: run clean run_blargg

WASM_CC_COMPILE=clang++.exe -c -DWASM=1 --target=wasm32 -fvisibility=hidden -std=c++14 -Wall -O2 -Wno-unused-variable -nostdlib -ffunction-sections -fdata-sections -I .. -I ../utils
WASM_CC_LINK=wasm-ld.exe --export-dynamic --no-entry --strip-all --gc-sections --allow-undefined --import-memory

run: gb-emu.exe
	./gb-emu.exe ../../data/cpu_instrs/cpu_instrs.gb

run_blargg: gb-emu.exe
	find ../../data/cpu_instrs/individual/*.gb | while read f; do ./gb-emu.exe "$$f"; echo; done

lib_gb.wasm-o: lib_gb.o
	$(WASM_CC_COMPILE) -o $@ ../lib_gb.cpp

platform_wasm.o:
	@true

platform.wasm-o: platform_wasm.o
	$(WASM_CC_COMPILE) -o $@ ../platform_wasm.cpp

../../wasmgui/build/gb_emulator.wasm: lib_gb.wasm-o platform.wasm-o
	mkdir -p ../../wasmgui/build
	$(WASM_CC_LINK) -o $@ lib_gb.wasm-o platform.wasm-o

CLANG_OPTS:=-D_CRT_SECURE_NO_WARNINGS -Wall -pedantic -O2 -Wno-language-extension-token
CLANG_LINK_OPTS:=-flto -O2

./gb-emu.exe: deps.Makefile
./gb-emu.exe: gb\lib_gb.o main.o 
	clang++.exe ${CLANG_LINK_OPTS} -o $@ gb\lib_gb.o main.o \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/User32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/OpenGL32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/Gdi32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/Comdlg32.Lib \
C:/Program\ Files\ \(x86\)/Windows\ Kits/10/Lib/10.0.17134.0/um/x64/OLE32.Lib \

%.o: ../%.cpp Makefile
	clang++.exe ${CLANG_OPTS} -o $@ -c $< -I .. -I ../utils
%%/%.o: ../%%/%.cpp Makefile
	clang++.exe ${CLANG_OPTS} -o $@ -c $< -I .. -I ../utils

clean:
	git clean -xf .

# clang -MM generates backslash-delimited paths, but Make wants forward-delipu.mited paths
deps.Makefile: ../gb/lib_gb.cpp ../win32/main.cpp ../host_wasm.cpp ../linux/main.cpp
	clang++.exe -E -MM $^ | sed 's/\\\([^\x0d]\)/\/\1/g' > deps.Makefile

include deps.Makefile



