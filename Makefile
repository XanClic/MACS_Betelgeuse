CXX ?= g++
CXXFLAGS += -O3 -Wall -Wextra -Wshadow -std=gnu++0x -Iinclude
LINK ?= $(CXX)
LDFLAGS += -Llib -lmacs -lSDL -lGL

AR ?= ar
RM ?= rm
MAKE ?= make

LIBOBJS = $(patsubst %.cpp,%.o,$(wildcard lib/*.cpp))
TESTS = $(patsubst %/,tests/test_%,$(shell ls -p tests | grep /))

.PHONY: all lib tests doc doxygen pdf

all: lib tests

lib: lib/libmacs.a

lib/libmacs.a: $(LIBOBJS)
	$(AR) rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

tests: $(TESTS)

doc: doxygen-public doxygen-private
#pdf-public pdf-private

pdf-%: doxygen-%
	$(MAKE) -C doc/$*/latex

doxygen-%:
	doxygen doc/$*/Doxyfile

clean:
	$(RM) -f lib/libmacs.a lib/*.o tests/test_* tests/*/*.o doc/*/Doxyfile.bak
	$(RM) -rf doc/*/html doc/*/man doc/*/latex


# FIXME
PATSUBST_FROM = %.cpp
PATSUBST_TO   = %.o

.SECONDEXPANSION:
tests/test_%: $$(patsubst $$(PATSUBST_FROM),$$(PATSUBST_TO),$$(wildcard tests/%/*.cpp)) lib/libmacs.a
	$(LINK) $(subst lib/libmacs.a,,$^) -o $@ $(LDFLAGS)
