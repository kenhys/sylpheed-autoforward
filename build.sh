#gcc -shared -o autoforward.dll ./.libs/autoforward.o  ../../libsylph/.libs/libsylph-0.a ../../src/.libs/libsylpheed-plugin-0.a `pkg-config --libs gtk+-2.0` `pkg-config --libs glib-2.0` -L/mingw/local/lib -lssleay32 -leay32 -lws2_32 -liconv -lonig   /mingw/lib/libiconv.a 

TARGET=autoforward.dll
OBJS=autoforward.o
LIBSYLPH=./lib/libsylph-0-1.a
LIBSYLPHEED=./lib/libsylpheed-plugin-0-1.a
#LIBS=" -lglib-2.0-0  -lintl"
LIBS=" `pkg-config --libs glib-2.0` `pkg-config --libs gobject-2.0`"
INC=" -I. -I../../ -I../../libsylph -I../../src `pkg-config --cflags glib-2.0` `pkg-config --cflags cairo` `pkg-config --cflags gdk-2.0`"
DEF=" -DHAVE_CONFIG_H"
com="gcc -c $DEF $INC autoforward.c"
eval $com
gcc -shared -o $TARGET $OBJS $LIBSYLPH $LIBSYLPHEED -L./lib $LIBS -lssleay32 -leay32 -lws2_32 -liconv -lonig
