# Makefile for a basic kernel module

CC=gcc
MODCFLAGS := -Wall -DMODULE -D__KERNEL__ -DLINUX
VER=$(shell uname -r)
INC=-I/lib/modules/$(VER)/build/include

INSTDIR=/lib/modules/$(VER)/misc

vaiostat.o: vaiostat.c
	$(CC) $(MODCFLAGS) $(INC) -c $^

clean:
	@-rm -f vaiostat.o

install: vaiostat.o
	install -D -m644 $^ $(INSTDIR)/$^
