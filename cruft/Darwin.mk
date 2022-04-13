CFLAGS		+=	-D__unix__ -I/usr/include/malloc/ -arch i386 -arch ppc
CPPFLAGS	+=	-D__unix__ -I/usr/include/malloc/ -arch i386 -arch ppc
FLTKDIR	    ?=	/usr/local/bin

#post:
#	/usr/local/bin/fltk-config --post virtualt
