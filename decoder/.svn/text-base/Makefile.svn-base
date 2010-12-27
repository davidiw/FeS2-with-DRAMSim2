
SOURCES=$(wildcard *.cpp)
OBJECTS=$(patsubst %.cpp,%.o,$(SOURCES))

FLAGS=-march=pentium4 -O3 -g
INCLUDES=-I.

.PHONY: all

all: $(OBJECTS)

%.o: %.cpp
	g++ $(FLAGS) $(INCLUDES) -c $< -o ../obj/$@

test-decoder: test/driver.cpp
	g++ $(FLAGS) $(INCLUDES) $< ../obj/*.o -o $@

%.cpp: %.spasm
	$(FES2_HOME)/tools/bin/spasm.py $< > $@
