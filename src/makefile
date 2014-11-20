dirs := . renderer
output := ../engine.a
builddir := ../build/$(OS)
headers := $(foreach dir,$(dirs),$(wildcard ../include/$(dir)/*.h))
sources := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))
objects := $(patsubst %.cpp,$(builddir)/%.o,$(sources))
includes := $(foreach dir,$(dirs),-I../include/$(dir))
includes += -I../build/
embeds := $(wildcard ../build/embed_data/*)
CXXFLAGS := $(includes) -std=c++11 -ffast-math -fno-rtti -fno-exceptions

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

../build/internal.zip: $(embeds)
	zip -j $@ $^

clean:
	rm -f ../build/internal.zip $(objects) $(output) *~

.PHONY: all clean

