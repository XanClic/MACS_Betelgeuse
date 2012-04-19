CXX ?= g++
CXXFLAGS += -g -O3 -Wall -Wextra -Wshadow -Wno-switch -std=c++11 -Iinclude -D_POSIX_C_SOURCE=201204 -DGL_GLEXT_PROTOTYPES -UGL_GLEXT_LEGACY
LIBCXXFLAGS += -Iinclude/macs -Iinclude/betelgeuse
LINK ?= $(CXX)
LDFLAGS += -Llib -lbetelgeuse -lmacs -lSDL -lGL -lm

AR ?= ar
RM ?= rm
MAKE ?= make

MACSOBJS = $(patsubst %.cpp,%.o,$(wildcard lib/macs/*.cpp))
BETELOBJS = $(patsubst %.cpp,%.o,$(wildcard lib/betelgeuse/*.cpp))
TESTS = $(patsubst %/,tests/test_%,$(shell ls -p tests | grep /))

.PHONY: all libs tests doc doxygen pdf

all: libs tests

libs: lib/libmacs.a lib/libbetelgeuse.a

lib/libmacs.a: $(MACSOBJS)
	$(AR) rcs $@ $^

lib/libbetelgeuse.a: $(BETELOBJS)
	$(AR) rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(LIBCXXFLAGS) -c $< -o $@

tests: $(TESTS)

doc: doxygen-macs-public doxygen-macs-private doxygen-betelgeuse-public doxygen-betelgeuse-private

doxygen-%:
	doxygen doc/$(subst -,/,$*)/Doxyfile

clean:
	$(RM) -f lib/lib*.a lib/*/*.o tests/test_* tests/*/*.o doc/*/*/Doxyfile.bak
	$(RM) -rf doc/*/*/html doc/*/*/man doc/*/*/latex


# FIXME
PATSUBST_FROM = %.cpp
PATSUBST_TO   = %.o

.SECONDEXPANSION:
tests/test_%: $$(patsubst $$(PATSUBST_FROM),$$(PATSUBST_TO),$$(wildcard tests/%/*.cpp)) lib/libbetelgeuse.a lib/libmacs.a
	$(LINK) $(CXXFLAGS) $(subst lib/libbetelgeuse.a,,$(subst lib/libmacs.a,,$^)) -o $@ $(LDFLAGS)
