all: 
	$(MAKE) -C src
release:
	RELEASE=1 $(MAKE) -C src
win: 
	WINDOWS=1 $(MAKE) -C src
relwin:
	RELEASE=1 WINDOWS=1 $(MAKE) -C src
cleanwin:
	WINDOWS=1 $(MAKE) -C src clean
clean:
	$(MAKE) -C src clean

.PHONY: all win clean cleanwin

