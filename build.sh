gcc -shared -o autoforward.dll ./.libs/autoforward.o  ../../libsylph/.libs/libsylph-0.a ../../src/.libs/libsylpheed-plugin-0.a `pkg-config --libs gtk+-2.0` `pkg-config --libs glib-2.0` -L/mingw/local/lib -lssleay32 -leay32 -lws2_32 -liconv -lonig   /mingw/lib/libiconv.a 
