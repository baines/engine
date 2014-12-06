includes := -I../include -I../include/renderer -I/usr/include/freetype2
CXXFLAGS := -std=c++1y -pipe -Og -g -ffast-math -fno-rtti -DDEBUG

test: test.cpp ../engine.a test_state.h
	g++ $(CXXFLAGS) $(includes) $^ -o $@ -lSDL2 -lphysfs -lfreetype
