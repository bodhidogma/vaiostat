# Makefile for a basic kernel module

CC=gcc
MODCFLAGS := -Wall -DMODULE -D__KERNEL__ -DLINUX
INC= -I/lib/modules/$(uname -r)/build/include

INSTDIR=/lib/modules/`uname -r`/misc

vaiostat.o: vaiostat.c
	$(CC) $(MODCFLAGS) $(INC) -c $^

clean:
	@-rm -f vaiostat.o

install: vaiostat.o
	@install -D -m644 $^ $(INSTDIR)/$^
