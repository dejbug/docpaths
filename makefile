
SHELL := cmd.exe
CXX := @g++
CXXFLAGS := -std=c++11 -O2 -Wall -pedantic
CXXFLAGS += -I.
LIBS := -lntdll
LIBS_STATIC := 
LIB_HANDLES := sys.o text.o path.o handles.o
LIB_HANDLES := $(addprefix handles/,$(LIB_HANDLES))

.PHONY: all clean run

all: docpaths.exe

docpaths.exe: main.o $(LIB_HANDLES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS_STATIC) $(LIBS)

handles/%.o: handles/%.cpp handles/%.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	@del *.exe 2>NUL
	@del *.o 2>NUL
	@del handles\*.o 2>NUL

run: docpaths.exe
	@docpaths.exe
