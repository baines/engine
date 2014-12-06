dirs := . renderer
output := ../engine.a
builddir := ../build/$(OS)
headers := $(foreach dir,$(dirs),$(wildcard ../include/$(dir)/*.h))
sources := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))
objects := $(patsubst %.cpp,$(builddir)/%.o,$(sources))
includes := $(foreach dir,$(dirs),-I../include/$(dir)) -I../build/ -I/usr/include/freetype2
CXXFLAGS := $(includes) -std=c++1y -pipe -ffast-math -fno-rtti -fno-exceptions

ifdef RELEASE
    CXXFLAGS += -Os -fno-stack-protector -U_FORTIFY_SOURCE -fomit-frame-pointer
else
    CXXFLAGS += -Og -g -DDEBUG
endif

all: $(output) 

$(output): $(objects)
	$(AR) rcs $@ $^

$(builddir)/%.o: %.cpp $(headers) ../build/internal.zip
	$(CXX) $(CXXFLAGS) -c $< -o $@

../build/internal.zip: ../build/embed_data
	zip -j -r $@ $<

clean:
	rm -f ../build/internal.zip $(objects) $(output) *~

.PHONY: all clean
