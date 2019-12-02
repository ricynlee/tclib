CC=g++
CCFLAGS=-Wall -std=c++11 -pedantic
LD=g++
LDFLAGS=

tclib.exe:yamlite.o tclib.o main.o tclib-engine.o
	${LD} ${LDFLAGS} $^ -o $@

yamlite.o:yamlite.cpp yamlite.hpp
	${CC} ${CCFLAGS} $< -c -o $@

tclib.o:tclib.cpp yamlite.hpp tclib.hpp
	${CC} ${CCFLAGS} $< -c -o $@

tclib-engine.o:tclib-engine.cpp tclib-engine.hpp tclib.hpp
	${CC} ${CCFLAGS} $< -c -o $@

main.o:main.cpp tclib.hpp
	${CC} ${CCFLAGS} $< -c -o $@

.PHONY:clean
clean:
	-rm yamlite.o main.o tclib.o tclib-engine.o tclib.exe
