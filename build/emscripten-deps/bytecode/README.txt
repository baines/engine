Put emscripten-compiled versions of libfreetype.a, libphysfs.a and libSDL2.a in here.

There is a bug when compiling libfreetype with emscripten due to function pointer
casting. If you get weird stack traces in _af_loader_load_glyph, then edit the 
freetype source as described here: 

https://groups.google.com/forum/?_escaped_fragment_=topic/emscripten-discuss/rLphBQHFs6o

