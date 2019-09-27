.SUFFIXES:
.PHONY: run

run: sdl-gb-emu.exe
	$<

sdl-gb-emu.exe: sdlgui.cpp
	clang++ ..\src\main.cc sdlgui.cpp -I ..\libs\SDL2-2.0.9\include -Wno-pragma-pack ..\src\platform_utils.cc ..\libs\SDL2-2.0.9\lib\x64\SDL2.lib -o $@

