
VER = 1.0

CC	= gcc
INCLUDE	= -I./ -I/usr/local/include
OPTS	= -Wall -shared `gtk-config --cflags` `glib-config --cflags`
DEFINES = 
LIBS	= -lpthread `gtk-config --cflags` `glib-config --cflags`

INSTDIR = /usr/local/lib/gkrellm/plugins

.SUFFIXES: .c .so

.c.so:	$(DEPEND)
	$(CC) $(OPTS) $(INCLUDE) $(DEFINES) -fPIC -c $*.c -o $@ 

GKV	 = vaiobat.so 
SETB = vaiolcd.so

all:	$(GKV) $(SETB)

install: $(SETB) $(GKV)
	install -s -m644 $^ $(INSTDIR)

realclean:
	-rm *.tar*

clean:
	-rm *.o *.so

dist: clean
	@-mkdir vaio_krellm-$(VER) > /dev/null 2>&1
	@-cp -f * vaio_krellm-$(VER) > /dev/null 2>&1
	tar -cvf - vaio_krellm-$(VER)|gzip -9 - >vaio_krellm-$(VER).tar.gz
	@rm -rf vaio_krellm-$(VER)

	
