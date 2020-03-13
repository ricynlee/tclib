CC=g++
CCFLAGS=-Wall -std=c++11 -pedantic
LD=g++
LDFLAGS=

tclib.exe:tclib.o main.o tclib-engine.o
	${LD} ${LDFLAGS} $^ -o $@

tclib.o:tclib.cpp tclib.hpp
	${CC} ${CCFLAGS} $< -c -o $@

tclib-engine.o:tclib-engine.cpp tclib-engine.hpp tclib.hpp
	${CC} ${CCFLAGS} $< -c -o $@

main.o:main.cpp tclib.hpp
	${CC} ${CCFLAGS} $< -c -o $@

.PHONY:clean
clean:
	-rm main.o tclib.o tclib-engine.o tclib.exe
