includes = -I../include -I../include/renderer -I/usr/$(PREFIX)/include/freetype2

CXXFLAGS = -std=c++1y -Wall -Werror -pipe -O0 -g -ffast-math -fno-rtti -DDEBUG \
$(shell /usr/$(PREFIX)/bin/sdl2-config --cflags)

LDFLAGS = -lphysfs -lfreetype $(shell /usr/$(PREFIX)/bin/sdl2-config --libs)

CXX = $(PREFIX)-g++

all: test

win: test
win: PREFIX=i686-w64-mingw32
win: LDFLAGS := -lmingw32 -lSDL2main $(LDFLAGS)
win: SUFFIX=.exe

js: test.html

test.html:
	emmake $(MAKE) -C emscripten

test: test.cpp ../engine.a test_state.h test_collision_state.h
	$(CXX) $(CXXFLAGS) $(includes) $< ../engine.a -o $@$(SUFFIX) $(LDFLAGS)

clean:
	$(RM) test test.exe
	$(MAKE) -C emscripten clean

.PHONY: all win js

