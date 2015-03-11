all: 
	OS=linux $(MAKE) -C src
release:
	OS=linux RELEASE=1 $(MAKE) -C src
clean:
	@OS=linux $(MAKE) -C src clean
win: 
	@PREFIX=i686-w64-mingw32 OS=win32 $(MAKE) -C src
relwin:
	@PREFIX=i686-w64-mingw32 OS=win32 RELEASE=1 $(MAKE) -C src
cleanwin:
	@OS=win32 $(MAKE) -C src clean
	
test: all
	$(MAKE) -C test
	./test/test

.PHONY: all release clean win relwin cleanwin test

