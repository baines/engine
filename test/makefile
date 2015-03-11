includes := -I../include -I../include/renderer -I/usr/include/freetype2
CXXFLAGS := -std=c++1y -Wall -Werror -pipe -Og -g -ffast-math -fno-rtti -DDEBUG

test: test.cpp ../engine.a test_state.h test_collision_state.h
	$(CXX) $(CXXFLAGS) $(includes) $< ../engine.a -o $@ -lSDL2 -lphysfs -lfreetype
