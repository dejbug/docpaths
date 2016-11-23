
SHELL := cmd.exe
CXX := @g++
CXXFLAGS := -std=c++11 -O2 -Wall -pedantic
CXXFLAGS += -I.
LIBS := 
LIBS_STATIC := 

.PHONY: all clean run

all: docpaths.exe

docpaths.exe: main.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS_STATIC) $(LIBS)

clean:
	@del *.exe 2>NUL
	@del *.o 2>NUL

run: docpaths.exe
	@docpaths.exe
