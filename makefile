all: 
	@CXX=g++-4.8 OS=linux $(MAKE) -C src
release:
	@CXX=g++-4.8 OS=linux RELEASE=1 $(MAKE) -C src
clean:
	@OS=linux $(MAKE) -C src clean
win: 
	@CXX=i686-w64-mingw32-g++ OS=win32 $(MAKE) -C src
relwin:
	@CXX=i686-w64-mingw32-g++ OS=win32 RELEASE=1 $(MAKE) -C src
cleanwin:
	@OS=win32 $(MAKE) -C src clean

.PHONY: all release clean win relwin cleanwin

