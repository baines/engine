dirs := . renderer
output := ../engine.a
builddir := ../build/$(OS)
headers := $(foreach dir,$(dirs),$(wildcard ../include/$(dir)/*.h))
sources := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))
objects := $(patsubst %.cpp,$(builddir)/%.o,$(sources)) $(builddir)/zip.o
includes := $(foreach dir,$(dirs),-I../include/$(dir)) -I../build/ -I/usr/include/freetype2
CXXFLAGS := $(includes) -std=c++1y -Wall -Werror -pipe -ffast-math -fno-rtti -fno-exceptions

ifdef RELEASE
    CXXFLAGS += -Os -fno-stack-protector -U_FORTIFY_SOURCE -fomit-frame-pointer
else
    CXXFLAGS += -Og -g -DDEBUG
endif

all: $(output) 

$(output): $(objects)
	$(AR) rcs $@ $^

../build/internal.zip: ../build/embed_data
	zip -j -r $@ $<

$(builddir)/zip.o: ../build/internal.zip
	(cd $(<D) && $(LD) -r -b binary -o $@ $(<F))

$(builddir)/%.o: %.cpp $(headers)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f ../build/internal.zip $(objects) $(output) *~

.PHONY: all clean
