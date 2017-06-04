CPP_FILES := $(wildcard src/*.cpp)
OBJ_FILES := $(patsubst src/%.cpp,bin/%.o,$(CPP_FILES))
LD_FLAGS := -lnetcdf_c++4 -lnetcdf
CC_FLAGS := -I include -I lib/pugixml

.PHONY: all fast debug clean

all: fast

fast: CXX = g++
fast: LD_FLAGS += -fopenmp -flto
fast: CC_FLAGS += -fopenmp -pedantic -O3 -flto
fast: acclimate

debug: CXX = g++
debug: LD_FLAGS +=
debug: CC_FLAGS += -pedantic -g
debug: acclimate

clean:
	@rm -f $(OBJ_FILES) acclimate

cleanall:
	@rm -rf bin acclimate

acclimate: bin/pugixml.o $(OBJ_FILES)
	@echo Linking to $@
	@$(CXX) -o $@ $^ $(LD_FLAGS)

bin/%.o: src/%.cpp include/%.h
	@echo Compiling $<
	@mkdir -p bin
	@$(CXX) $(CC_FLAGS) -c -o $@ $<

bin/acclimate.o: src/acclimate.cpp
	@echo Compiling $<
	@mkdir -p bin
	@$(CXX) $(CC_FLAGS) -c -o $@ $<

bin/pugixml.o: lib/pugixml/pugixml.cpp lib/pugixml/pugixml.hpp lib/pugixml/pugiconfig.hpp
	@echo Compiling $<
	@mkdir -p bin
	@$(CXX) $(CC_FLAGS) -c -o $@ $<
