
VER = 1.0

CC	= gcc
INCLUDE	= -I./ -I/usr/local/include
OPTS	= -Wall -shared `gtk-config --cflags` `glib-config --cflags`
DEFINES = 
LIBS	= -lpthread `gtk-config --cflags` `glib-config --cflags`

INSTDIR = /usr/local/lib/gkrellm/plugins

.SUFFIXES: .c .so .o

.c.so:	$(DEPEND)
	$(CC) $(OPTS) $(INCLUDE) $(DEFINES) -c $*.c -o $*.o 
	$(CC) $(OPTS) -o $@ $*.o $(LIBS)

.c.o:	$(DEPEND)
	$(CC) $(OPTS) $(INCLUDE) $(DEFINES) -c $*.c -o $@ 

VBATc	= vaiobat.c 
VBATso	= vaiobat.so 

VLCDc	= vaiolcd.c
VLCDso	= vaiolcd.so

all: $(VBATso) $(VLCDso)

$(VBATso): $(VBATc)

$(VLCDso): $(VLCDc)

install: $(VLCDso) $(VBATso)
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

