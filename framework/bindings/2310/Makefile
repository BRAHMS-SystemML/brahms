
MKOCTFILE=mkoctfile

LD=$(MKOCTFILE)
LDFLAGS=-v --link-stand-alone
CXX=$(MKOCTFILE)
CXX_FLAGS=-v
RM=rm -f

all: runoct

runoct: test.o embed.o
	$(LD) $(LDFLAGS) test.o embed.o -o runoct

test.o: test.cpp embed.h
	$(CXX) $(CXX_FLAGS) -c test.cpp

embed.o: embed.cpp embed.h
	$(CXX) $(CXX_FLAGS) -c embed.cpp

clean:
	$(RM) embed.o test.o runoct *~ core octave-core

