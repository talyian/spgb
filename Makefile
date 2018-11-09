SHELL:=/bin/bash
ALL_SRC=$(wildcard core/*.cc core/*.hh)
CC_OPTS=-Wall -pedantic -fsanitize=address,undefined -g -std=c++14
CC_OPTS=-Wall -pedantic -O3 -std=c++14 -s

run: bin/main.exe
	$<

core/opcodes.cc: core/table_opcodes.csv core/make_opcodes.py
	pushd core; python3 make_opcodes.py < table_opcodes.csv > opcodes.cc; popd

bin/main.exe: core/main.cc core/opcodes.cc $(ALL_SRC) Makefile
	mkdir -p bin
	clang++-6.0 ${CC_OPTS} -o $@ $<
