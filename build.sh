#!/bin/bash

PKG_NAME=
TARGET=
OBJS=
LIBSYLPH=./lib/libsylph-0-1.a
LIBSYLPHEED=./lib/libsylpheed-plugin-0-1.a
#LIBS=" -lglib-2.0-0  -lintl"
LIBS=" `pkg-config --libs glib-2.0 gobject-2.0 gtk+-2.0`"
INC=" -I. -I./src -I./lib/sylplugin_factory/src -I../../ -I../../libsylph -I../../src  `pkg-config --cflags glib-2.0 cairo gdk-2.0 gtk+-2.0`"
DEF=" -DHAVE_CONFIG_H"

run() {
    "$@"
    if test $? -ne 0; then
        echo "Failed $@"
        exit 1
    fi
}

usage() {
    cat <<EOF 1>&2
Usage:
     $0 [-d|--debug]
        [-p|--po]
        [-m|--mo]
Mandatory args:
  -d,--debug enable debug build
  -p,--po    update po files
  -m,--mo    update mo files
Optional args:
  -h,--help  print this help
EOF
}

make_def() {
    for pkg in libsylph-0-1 libsylpheed-plugin-0-1; do
	(cd lib;pexports $pkg.dll > $pkg.dll.def)
	(cd lib;dlltool --dllname $pkg.dll --input-def $pkg.dll.def --output-lib $pkg.a)
    done
}

make_clean() {
    rm -f *.o *.lo *.la *.bak *~
}

make_distclean() {
    rm -f *.o *.lo *.la *.bak *.dll *.zip
}

make_mo() {
    run msgfmt po/ja.po -o po/$NAME.mo
    if [ -d "$SYLLOCALEDIR" ]; then
        run cp po/$NAME.mo $SYLLOCALEDIR/$NAME.mo
    fi
}

make_res() {
    run windres -i res/version.rc -o version.o
}


compile () {
    make_res

    for src in `find . -name '*.c'`; do
	src_base=${src%%.c}
	run gcc -Wall -c -o ${src_base}.o $DEF $INC ${src}
    done

    OBJS=`find . -name '*.o'`
    run gcc -shared -o $TARGET $OBJS -L./lib $LIBSYLPH $LIBSYLPHEED $LIBS -lssleay32 -leay32 -lws2_32 -liconv -lonig
    if [ -d "$SYLPLUGINDIR" ]; then
        com="cp $TARGET \"$SYLPLUGINDIR/$NAME.dll\""
        echo $com
        eval $com
    else
        :
    fi
}

make_release() {
    if [ -z "$1" ]; then
	return
    fi
    version=$1
    shift
    if [ -f src/$NAME.dll ]; then
	mv src/$NAME.dll .
    fi
    zip sylpheed-$NAME-${r}.zip $NAME.dll
    zip -r sylpheed-$NAME-$version.zip doc/README.ja.txt
    zip -r sylpheed-$NAME-$version.zip src/*.h
    zip -r sylpheed-$NAME-$version.zip src/auto*.c
    zip -r sylpheed-$NAME-$version.zip res/*.rc
    zip -r sylpheed-$NAME-$version.zip po/$NAME.mo
    zip -r sylpheed-$NAME-$version.zip res/*.xpm
    zip -r sylpheed-$NAME-$version.zip COPYING
    zip -r sylpheed-$NAME-$version.zip LICENSE
    zip -r sylpheed-$NAME-$version.zip README.md
    zip -r sylpheed-$NAME-$version.zip NEWS
    sha1sum sylpheed-$NAME-$version.zip > sylpheed-$NAME-$version.zip.sha1sum
}

mode=""
options=$(getopt -o -bcChdpmkte -l build,pkg:,debug,pot,po,mo,def,res -- "$@")

if [ $? -ne 0 ]; then
    usage
    exit 1
fi
eval set -- "${options}"

while true
do
    case "$1" in
	-h|--help)   usage && exit 0;;
        -d|--debug) mode=debug; shift;;
        -b|--build)
	    compile
	    ;;
	-p|--po)
            run msgmerge po/ja.po po/$NAME.pot -o po/ja.po
	    shift
	    ;;
	-k|--pkg)
	    shift
	    PKG_NAME=$1
	    TARGET=$1.dll
	    shift
	    ;;
	-t|--pot)
            mkdir -p po
	    run xgettext src/$NAME.c -k_ -kN_ -o po/$NAME.pot
	    shift
	    ;;
        -m|--mo) make_mo; shift;;
        -e|--def)
            make_def; shift;;
	-s|--res)
            make_res
	    shift
	    ;;
        -r|--release)
	    shift
	    if [ -z "$1" ]; then
		usage && exit 1
	    else
		make_release $1
		shift
	    fi
	    ;;
	-C|--dclean)
	    make_distclean; shift;;
	-c|--clean)
	    make_clean; shift;;
	*)
            break
	    ;;
    esac
done

