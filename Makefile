# Makefile for a basic kernel module

CC=gcc
MODCFLAGS := -Wall -DMODULE -D__KERNEL__ -DLINUX
INC= -I/lib/modules/$(uname -r)/build/include

#hello.o:	hello.c /usr/include/linux/version.h

vaiostat.o: vaiostat.c
	$(CC) $(MODCFLAGS) $(INC) -c $^

hello.o:	hello.c
		$(CC) $(MODCFLAGS) -c hello.c
		@echo insmod hello.o to turn it on
		@echo rmmod hello to turn if off

