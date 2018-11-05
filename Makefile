ALL_SRC=$(wildcard core/*.cc core/*.hh)
CC_OPTS=-Wall -pedantic -g -std=c++17

run: bin/main.exe
	$<

bin/main.exe: core/main.cc $(ALL_SRC) Makefile
	mkdir -p bin
	clang++-6.0 ${CC_OPTS} -o $@ $<
