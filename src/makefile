dirs := . renderer
output := ../engine.a
builddir := ../build/$(OS)
headers := $(foreach dir,$(dirs),$(wildcard ../include/$(dir)/*.h))
sources := $(foreach dir,$(dirs),$(wildcard $(dir)/*.cpp))
objects := $(patsubst %.cpp,$(builddir)/%.o,$(sources))
includes := $(foreach dir,$(dirs),-I../include/$(dir))
CXXFLAGS := $(includes) -std=c++11 -ffast-math -fno-rtti -fno-exceptions

ifdef RELEASE
    CXXFLAGS += -Os -fno-stack-protector -U_FORTIFY_SOURCE
else
    CXXFLAGS += -Og -g
endif

all: $(output)

$(output): $(objects)
	$(AR) rcs $@ $^

$(builddir)/%.o: %.cpp $(headers)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(objects) $(output) *~

.PHONY: all clean

