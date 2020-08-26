#!/bin/bash

INSTALL_TO=d:/libs/lib-binary
ACML_PATH=d:/libs/acml4.4.0/gfortran32

FILES[1]="fftw-3.2.2.tar.gz"
LDIRS[1]="fftw-3.2.2"
DWNLD[1]="wget http://www.fftw.org/fftw-3.2.2.tar.gz"
LCMDS[1]="tar xzf ${FILES[1]}"

FILES[2]="itpp-external-3.2.0.tar.bz2"
LDIRS[2]="itpp-external-3.2.0"
DWNLD[2]="wget http://downloads.sourceforge.net/project/itpp/itpp-external/3.2.0/itpp-external-3.2.0.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fitpp%2Ffiles%2F&ts=1281448321&mirror=sunet"
LCMDS[2]="tar xjf ${FILES[2]}"

FILES[3]="itpp-4.2.tar.bz2"
LDIRS[3]="itpp-4.2"
DWNLD[3]="wget "
LCMDS[3]="tar xjf ${FILES[3]}"

FILES[4]="qwtplot3d-0.2.7.tgz"
LDIRS[4]="qwtplot3d-0.2.7"
DWNLD[4]="wget http://downloads.sourceforge.net/project/qwtplot3d/qwtplot3d/0.2.7/qwtplot3d-0.2.7.tgz?r=http%3A%2F%2Fqwtplot3d.sourceforge.net%2Fweb%2Fnavigation%2Fdownload.html&ts=1281441248&mirror=mesh"
LCMDS[4]="tar xzf ${FILES[3]}"

FILES[5]="libconfig-1.4.5.tar.gz"
LDIRS[5]="libconfig-1.4.5"
DWNLD[5]="wget http://www.hyperrealm.com/libconfig/libconfig-1.4.5.tar.gz"
LCMDS[5]="tar xzf ${FILES[5]}"

FILES[6]="zlib-1.2.5.tar.gz"
LDIRS[6]="zlib-1.2.5"
DWNLD[6]="wget http://zlib.net/zlib-1.2.5.tar.gz"
LCMDS[6]="tar xfz ${FILES[6]}"

FILES[7]="tclap-1.2.0.tar.gz"
LDIRS[7]="tclap-1.2.0"
DWNLD[7]="wget http://downloads.sourceforge.net/project/tclap/tclap-1.2.0.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Ftclap%2Ffiles%2F&ts=1281442557&mirror=sunet"
LCMDS[7]="tar xfz ${FILES[7]}"

FILES[8]="qwt-5.2"
DWNLD[8]="svn co https://qwt.svn.sourceforge.net/svnroot/qwt/branches/qwt-5.2"
LDIRS[8]="qwt-5.2"
LCMDS[8]=""

LTOT="1 2 3 4 5 6 7 8"

if [ "$1" == "clean" ]; then
	for i in $LTOT; do
		echo -n "check for ${LDIRS[i]}... "
		if [ ! -e ${LDIRS[i]} ]; then
			echo file/directory not found. 
		else
			echo cleaning...
			cd ${LDIRS[i]}
			make clean distclean mocclean
			echo OK
			cd ..
		fi
	done
fi

# compile Qt 4.7.0

	cd qt-everywhere-opensource-src-4.7.0
	g++ -v
	pwd
#	./configure -platform win32-g++ -opensource --prefix=$INSTALL_TO -fast -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-openssl -silent -no-nis -no-cups -no-dbus -no-nas-sound -no-qt3support || exit
	./configure -platform win32-g++ -opensource --prefix=$INSTALL_TO -fast -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-openssl -no-nis -no-cups -no-dbus -no-nas-sound -no-qt3support || exit
	gmake || exit
exit	
	gmake install || exit


#export QMAKE=c:/Qt/4.7.0/bin/qmake



# COMPILE Qwt
echo -n check if we need to compile Qwt... 
if [ ! -e $INSTALL_TO/lib/qwt5.dll ]; then
	echo yes
	cd ${LDIRS[8]}
   # todo change line 52 to "release"
	newstring=NR==15{"$"0='"'INSTALLBASE=$INSTALL_TO'"'}1
	echo using awk to substitute 15th line: awk $newstring ...
	awk $newstring qwtconfig.pri > qwtconfig_modified.pri
	rm qwtconfig.pri 
	mv qwtconfig_modified.pri qwtconfig.pri
	$QMAKE || exit
	make $MAKEOPTS || exit
	make install || exit
	cd ..
else
	echo no.
fi

# COMPILE qwtplot3d
echo -n check if we need to compile qwtplot3d... 
if [ ! -e $INSTALL_TO/lib/qwtplot3d.dll ]; then
	echo yes
	cd ${LDIRS[4]}
	# change in qwtplot3d.pro: DESTDIR = ...
	newstring=NR==11{"$"0='"'DESTDIR=$INSTALL_TO/lib'"'}1
	echo using awk to substitute 11th line: awk $newstring ...
	awk $newstring qwtplot3d.pro > qwtplot3d_modified.pro
	rm qwtplot3d.pro
	mv qwtplot3d_modified.pro qwtplot3d.pro
	newstring=NR==6{"$"0='"'CONFIG           += qt warn_on opengl thread zlib release'"'}1
	awk $newstring qwtplot3d.pro > qwtplot3d_modified.pro
	rm qwtplot3d.pro
	mv qwtplot3d_modified.pro qwtplot3d.pro
#include <stdio.h>
	echo "#include <stdio.h>" > src/qwt3d_function.cpp_new
	cat src/qwt3d_function.cpp >> src/qwt3d_function.cpp_new
	rm src/qwt3d_function.cpp
	mv src/qwt3d_function.cpp_new src/qwt3d_function.cpp
	$QMAKE || exit
	make $MAKEOPTS || exit
	make install || exit
	cd ..
else
	echo no.
fi

echo -n check if we need to compile fftw...
if [ ! -e $INSTALL_TO/lib/libfftw3.a ]; then
	echo yes.
	cd ${LDIRS[1]} 
	./configure --prefix=$INSTALL_TO || exit
	make || exit
	make install || exit
	cd ..
else
	echo no.
fi

# itpp external
echo -n check if we need to compile itpp-external...
if [ ! -e $INSTALL_TO/lib/libblas.la ]; then
	echo yes.
	cd ${LDIRS[2]} 
	./configure --prefix=$INSTALL_TO --disable-fftw --enable-shared || exit
	make || exit
	make install || exit
else
	echo no.
fi

# itpp
export LDFLAGS="-L$INSTALL_TO/lib"
export CPPFLAGS="-I$INSTALL_TO/include"

echo -n check if we need to compile itpp...
if [ ! -e $INSTALL_TO/lib/libitpp.la ]; then
	echo yes.
	cd ${LDIRS[3]}
	./configure --prefix=$INSTALL_TO --enable-static --with-fft="-lfftw3" --without-blas --without-lapack --disable-html-doc --disable-comm  --disable-fixed --disable-optim --disable-protocol --disable-srccode    
 	echo check if everything is alright. press enter then.
	read
	make || exit 1
	make check || exit 1
	make install || exit 1
	cd ..
else
	echo no.
fi

# COMPILE libconfig
echo -n check if we need to compile libconfig... 
if [ ! -e $INSTALL_TO/lib/libconfig++.la ]; then
	echo yes. Compiling libconfig...
	cd ${LDIRS[5]}
	./configure --prefix=$INSTALL_TO || exit
	make $MAKEOPTS || exit
	make install || exit
	cd ..
else
	echo no.
fi

# COMPILE tclap
echo -n check if we need to compile tclap... 
if [ ! -e $INSTALL_TO/include/tclap/Arg.h ]; then
	echo yes.
	cd ${LDIRS[7]}
	./configure --prefix=$INSTALL_TO || exit
	make || exit
	make install || exit
	cd ..
else
	echo no.
fi

# COMPILE zlib
echo -n check if we need to compile zlib... 
if [ ! -e $INSTALL_TO/lib/libz.a ]; then
	echo yes.
	cd ${LDIRS[6]}
	make -fwin32/Makefile.gcc || exit
	export BINARY_PATH=$INSTALL_TO/bin
	export INCLUDE_PATH=$INSTALL_TO/include
	export LIBRARY_PATH=$INSTALL_TO/lib
	
	make install -fwin32/Makefile.gcc || exit
	cd ..
else
	echo no.
fi

# COMPILE hdf5 
echo -n check if we need to compile HDF5 ... 
if [ ! -e $INSTALL_TO/lib/libhdf5_cpp.a ]; then
	echo yes.
	# see http://www.hdfgroup.org/ftp/HDF5/current/src/unpacked/release_docs/INSTALL_MinGW.txt
#	cd hdf5-1.8.4-patch1_mingw
	cd hdf5-1.8.5-patch1_mingw
	export LIBS=-lws2_32
##	./configure --prefix=$INSTALL_TO  --enable-cxx --with-zlib=$INSTALL_TO/include,$INSTALL_TO/lib --libdir=/c/mingw/lib --includedir=/c/mingw/include
	#./configure --prefix=$INSTALL_TO  --enable-cxx --enable-production --with-zlib=$INSTALL_TO/include,$INSTALL_TO/lib || exit 1
#	make || exit 1
make check
#	make install || exit 1
else
	echo no.
fi


exit
exit

# itpp
# COMPILE itpp
echo -n check if we need to compile itpp... 
if [ ! -e $INSTALL_TO/lib/libitpp.la ]; then
	echo yes.
	cd ${LDIRS[12]}
	export LDFLAGS="-L$INSTALL_TO/lib -L$INSTALL_TO/gcc/lib64"
	export CFLAGS=""	
	export CXXFLAGS="-DNDEBUG -O3"
	export CPPFLAGS="-DNDEBUG -O3 -I$INSTALL_TO/include -I$INSTALL_TO/gcc/include/c++/$GCCVER -I$INSTALL_TO/gcc/lib/gcc/$GCCARCH/$GCCVER/include"
	export F77=gfortran

#	WITH_BLAS_STR=""
#	WITH_LAPACK_STR=""
# TODO use only one of ATLAS, ACML, MKL
	if [ $HAVE_ATLAS -eq 1 ]; then
		WITH_BLAS_STR="-latlas -lblas"	
	fi

	echo CPPFLAGS: $CPPFLAGS
	echo LDFLAGS: $LDFLAGS
	echo WITH_BLAS_STR: $WITH_BLAS_STR
	echo WITH_LAPACK_STR: $WITH_LAPACK_STR
	gfortran -v
	gcc -v
	g++ -v
	echo press return to compile itpp
	read

	if [ $HAVE_ACML -eq 1 ]; then
		export LDFLAGS="$LDFLAGS -L$INSTALL_TO/gfortran64/lib"
		export CPPFLAGS="$CPPFLAGS -I$INSTALL_TO/gfortran64/include"
		WITH_BLAS_STR="'-lacml -lacml_mv -lgfortran $WITH_BLAS_STR'"
		WITH_LAPACK_STR="$WITH_LAPACK_STR"
		./configure --prefix=$INSTALL_TO --disable-shared --enable-static --with-blas="-lacml -lacml_mv -lgfortran" --with-fft="-lacml"
		echo check if everything is alright. press enter then.
		read
	fi
#	 ./configure --prefix=$INSTALL_TO --disable-shared --enable-static --with-fft-include=$INSTALL_TO/include --with-blas="$WITH_BLAS_STR" --with-fft=-lfftw3
 #--with-lapack='"'$WITH_LAPACK_STR'"'
#g++ -v
#	exit
	make $MAKEOPTS || exit 1
	make check $MAKEOPTS || exit 1
	make install || exit 1
	cd ..
else
	echo no.
fi

exit

echo INSTALL_TO: $INSTALL_TO
g++ -v
echo press return to continue or press ctrl-c to abort.
read
./configure -platform win32-g++ -opensource --prefix=$INSTALL_TO -fast -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-openssl -silent -no-nis -no-cups -no-dbus -no-nas-sound -no-qt3support || exit
#./configure -platform win32-g++ -opensource --prefix=$INSTALL_TO -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-openssl -silent -no-nis -no-cups -no-dbus -no-nas-sound -no-qt3support

exit

# ACML
# get from 
# extract
# run install-acml-4-4-0-gfortran-32bit.sh inside msys.bat

