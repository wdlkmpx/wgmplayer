#!/bin/sh
#
# sh po/zupdatepo.sh
#
# sh po/zupdatepo.sh clean
#

if [ -f ar.po ] ; then
	echo "run:"
	echo "	sh po/zupdatepo.sh"
	exit 1
fi

#=============================

if [ "$1" = "clean" ] ; then
	cd po
	sed -i '/#~ /d' *.po
	echo "Done"
	exit
fi

#=============================

if ! [ -f configure ] ; then
	./autogen.sh
fi

if ! [ -f po/Makefile ] ; then
	./configure --prefix=/usr
fi

make -C po update-po
