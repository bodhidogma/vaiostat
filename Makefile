# Makefile for a basic kernel module

VER=1.0

CC=gcc
MODCFLAGS := -Wall -DMODULE -D__KERNEL__ -DLINUX -O2
KVER=$(shell uname -r)
INC=-I/lib/modules/$(KVER)/build/include

INSTDIR=/lib/modules/$(KVER)/misc

vaiostat.o: vaiostat.c
	$(CC) $(MODCFLAGS) $(INC) -c $^

realclean:
	-rm *.tar*

clean:
	-rm -f *.o *.tar*

install: vaiostat.o
	install -D -m644 $^ $(INSTDIR)/$^

dist: clean
	@-mkdir vaiostat-$(VER) > /dev/null 2>&1
	@-cp -f * vaiostat-$(VER) > /dev/null 2>&1
#	tar -cvf - vaiostat-$(VER)|bzip2 - >vaiostat-$(VER).tar.bz2
	tar -cvf - vaiostat-$(VER)|gzip -9 - >vaiostat-$(VER).tar.gz
	@rm -rf vaiostat-$(VER)

