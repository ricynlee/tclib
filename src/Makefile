CC=clang++
CCFLAGS=--target=x86_64-pc-mingw32 -Wall -std=c++11 -pedantic
LD=clang++
LDFLAGS=--target=x86_64-pc-mingw32

tclib.exe:yamlite.o tclib.o main.o
	${LD} ${LDFLAGS} $^ -o $@

yamlite.o:yamlite.cpp yamlite.hpp
	${CC} ${CCFLAGS} $< -c -o $@

tclib.o:tclib.cpp yamlite.hpp tclib.hpp
	${CC} ${CCFLAGS} $< -c -o $@

main.o:main.cpp yamlite.hpp tclib.hpp
	${CC} ${CCFLAGS} $< -c -o $@

.PHONY:clean
clean:
	-del /F yamlite.o main.o tclib.o tclib.exe
	-del /F tclib.depend
	-del /F tclib.layout
	-rd /Q /S bin obj
