dirs := . renderer
output := ../engine.a
builddir := ../build/$(OS)
sources := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))
depends := $(foreach dir,$(dirs),$(wildcard $(builddir)/$(dir)/*.d))
objects := $(patsubst %.cpp,$(builddir)/%.o,$(sources)) $(builddir)/zip.o
includes := $(foreach dir,$(dirs),-I../include/$(dir)) -I/usr/$(PREFIX)/include/freetype2
CXXFLAGS := -std=c++1y -Wall -pipe -ffast-math -fno-rtti -fno-exceptions\
 $(shell /usr/$(PREFIX)/bin/sdl2-config --cflags) -Wno-missing-braces

#CXX := clang++-3.5
#CXX := include-what-you-use

ifdef PREFIX
	CXX := $(PREFIX)-g++
	AR  := $(PREFIX)-ar
	LD  := $(PREFIX)-ld
endif

ifdef RELEASE
    CXXFLAGS += -Os -fno-stack-protector -U_FORTIFY_SOURCE -fomit-frame-pointer
else
    CXXFLAGS += -O0 -DDEBUG -g
endif

all: $(output) 

include $(depends)

$(output): $(objects)
	@echo [AR] $@
	@$(AR) rcs $@ $^

../build/internal.zip: ../build/embed_data
	@echo [ZIP] $<
	@zip -j -r $@ $<

$(builddir)/zip.o: ../build/internal.zip
	@(cd $(<D) && $(LD) -r -b binary -o $@ $(<F))

$(builddir)/%.o: %.cpp
	@echo [CXX] $<
	@$(CXX) -MMD $(includes) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f ../build/internal.zip $(objects) $(output) $(depends) *~

.PHONY: all clean FORCE
