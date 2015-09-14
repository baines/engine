deps := ../../build/emscripten-deps
srcs := ../test.cpp ../test_state.h ../test_collision_state.h

includes = -I../../include -I../../include/renderer\
 -I$(deps)\
 -I$(deps)/boost/include\
 -I$(deps)/ft2/include\
 -I$(deps)/glm\
 -I$(deps)/SDL2/include

CXXFLAGS = -std=c++1y -Wall -pipe -O2 -g -ffast-math -fno-rtti

LDFLAGS = -s USE_SDL=0\
 $(deps)/bytecode/libfreetype.a\
 $(deps)/bytecode/libphysfs.a\
 $(deps)/bytecode/libSDL2.a\
 --preload-file ../data@/data --preload-file internal.zip\
 --pre-js args.js

all: test.html

test.html: $(srcs) ../../engine.bc args.js ../data internal.zip
	$(CXX) $(CXXFLAGS) $(includes) $< ../../engine.bc -o $@$(SUFFIX) $(LDFLAGS)
	
internal.zip: ../../build/internal.zip
	cp $< $@

clean:
	$(RM) test.html test.html.mem test.js test.data internal.zip
