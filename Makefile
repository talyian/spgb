ALL_SRC=$(wildcard *.cc *.h)
CC_OPTS=-Wall -Werror -fsanitize=address -O0 -g -std=c++17

run: bin/main.exe
	$<

bin/main.exe: main.cc $(ALL_SRC)
	mkdir -p bin
	clang++-6.0 ${CC_OPTS} -o $@ main.cc
