#!/bin/bash

INSTALL_TO=$(pwd)/lib-binary

HAVE_ATLAS=0
HAVE_ACML=1

mkdir -p $INSTALL_TO

#FILES[1]="qt-everywhere-opensource-src-4.6.3.tar.gz"
#DWNLD[1]="wget http://get.qt.nokia.com/qt/source/qt-everywhere-opensource-src-4.6.3.tar.gz"
#LDIRS[1]="qt-everywhere-opensource-src-4.6.3"

FILES[1]="qt-everywhere-opensource-src-4.7.0.tar.gz"
DWNLD[1]="wget http://get.qt.nokia.com/qt/source/qt-everywhere-opensource-src-4.7.0.tar.gz"
LDIRS[1]="qt-everywhere-opensource-src-4.7.0"
LCMDS[1]="tar xfz ${FILES[1]}"

FILES[2]="qwt-5.2"
DWNLD[2]="svn co https://qwt.svn.sourceforge.net/svnroot/qwt/branches/qwt-5.2"
LDIRS[2]="qwt-5.2"
LCMDS[2]=""

FILES[3]="qwtplot3d-0.2.7.tgz"
LDIRS[3]="qwtplot3d"
DWNLD[3]="wget http://downloads.sourceforge.net/project/qwtplot3d/qwtplot3d/0.2.7/qwtplot3d-0.2.7.tgz?r=http%3A%2F%2Fqwtplot3d.sourceforge.net%2Fweb%2Fnavigation%2Fdownload.html&ts=1281441248&mirror=mesh"
LCMDS[3]="tar xzf ${FILES[3]}"

FILES[4]="fftw-3.2.2.tar.gz"
LDIRS[4]="fftw-3.2.2"
DWNLD[4]="wget http://www.fftw.org/fftw-3.2.2.tar.gz"
LCMDS[4]="tar xzf ${FILES[4]}"

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

# hdf5 source: http://www.hdfgroup.org/ftp/HDF5/current/src/hdf5-1.8.5.tar.bz2
FILES[8]="hdf5-1.8.5-linux-x86_64-shared.tar.gz"
LDIRS[8]="hdf5-1.8.5-linux-x86_64-shared"
DWNLD[8]="wget http://www.hdfgroup.org/ftp/HDF5/current/bin/linux-x86_64/hdf5-1.8.5-linux-x86_64-shared.tar.gz"
LCMDS[8]="tar xfz ${FILES[8]}"

FILES[9]="boost_1_43_0.tar.bz2"
LDIRS[9]="boost_1_43_0"
DWNLD[9]="wget http://downloads.sourceforge.net/project/boost/boost/1.43.0/boost_1_43_0.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fboost%2Ffiles%2Fboost%2F1.43.0%2F&ts=1281442617&mirror=sunet"
LCMDS[9]="tar xjf ${FILES[9]}"

FILES[10]="atlas3.9.25.tar.bz2"
LDIRS[10]="atlas3.9.25"
DWNLD[10]="wget http://downloads.sourceforge.net/project/math-atlas/Developer%20%28unstable%29/3.9.25/atlas3.9.25.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fmath-atlas%2F&ts=1281446649&mirror=freefr"
LCMDS[10]="tar xjf ${FILES[10]}"

FILES[11]="itpp-external-3.2.0.tar.bz2"
LDIRS[11]="itpp-external-3.2.0"
DWNLD[11]="wget http://downloads.sourceforge.net/project/itpp/itpp-external/3.2.0/itpp-external-3.2.0.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fitpp%2Ffiles%2F&ts=1281448321&mirror=sunet"
LCMDS[11]="tar xjf ${FILES[11]}"

FILES[12]="itpp-4.0.7.tar.bz2"
LDIRS[12]="itpp-4.0.7"
DWNLD[12]="wget http://downloads.sourceforge.net/project/itpp/itpp/4.0.7/itpp-4.0.7.tar.bz2?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fitpp%2Ffiles%2F&ts=1281447702&mirror=sunet"
LCMDS[12]="tar xjf ${FILES[12]}"

#FILES[13]="gcc-4.4.4.tar.bz2"
#LDIRS[13]="gcc-4.4.4"
#DWNLD[13]="wget http://gcc.cybermirror.org/releases/gcc-4.4.4/gcc-4.4.4.tar.bz2"
#LCMDS[13]="tar xjf ${FILES[13]}"
#GCCVER=4.4.4
#GCCARCH=x86_64-unknown-linux-gnu

FILES[13]="gcc-4.5.1.tar.bz2"
LDIRS[13]="gcc-4.5.1"
DWNLD[13]="wget http://gcc.cybermirror.org/releases/gcc-4.5.1/gcc-4.5.1.tar.bz2"
#or http://gcc-uk.internet.bs/releases/gcc-4.5.1/gcc-4.5.1.tar.bz2
LCMDS[13]="tar xjf ${FILES[13]}"
GCCVER=4.5.1
GCCARCH=x86_64-unknown-linux-gnu

FILES[14]="mpfr-3.0.0.tar.bz2"
LDIRS[14]="mpfr-3.0.0"
DWNLD[14]="wget http://www.mpfr.org/mpfr-current/mpfr-3.0.0.tar.bz2"
LCMDS[14]="tar xjf ${FILES[14]}"

# agree to EULA and download ACML from http://developer.amd.com/Downloads/acml-4-4-0-gfortran-64bit.tgz
FILES[15]="acml-4-4-0-gfortran-64bit.tgz"
LDIRS[15]="install-acml-4-4-0-gfortran-64bit.sh"
DWNLD[15]="echo agree to EULA and download ACML from http://developer.amd.com/Downloads/acml-4-4-0-gfortran-64bit.tgz"
LCMDS[15]="tar xzf ${FILES[15]}"

FILES[16]="mpc-0.8.2.tar.gz"
LDIRS[16]="mpc-0.8.2"
DWNLD[16]="wget http://www.multiprecision.org/mpc/download/mpc-0.8.2.tar.gz"
LCMDS[16]="tar xzf ${FILES[16]}"

FILES[17]="gmp-5.0.1.tar.bz2"
LDIRS[17]="gmp-5.0.1"
DWNLD[17]="wget http://ftp.gnu.org/gnu/gmp/gmp-5.0.1.tar.bz2"
LCMDS[17]="tar xjf ${FILES[17]}"

LTOT=17;

echo This script installs all necessary libraries for SNACS from their sources.

MAKEOPTS="-j16"

echo checking if all libraries are untared
for i in $(seq 1 $LTOT); do
	echo -n "check for ${LDIRS[i]}... "
	if [ ! -e ${LDIRS[i]} ]; then
		echo file/directory not found. 
		echo -n "check for ${FILES[i]}... "
		if [ ! -e ${FILES[i]} ]; then
			echo file not found. Downloading ${FILES[i]}...
			${DWNLD[i]}
		else
			echo OK
		fi
		echo Extracting ${FILES[i]}...
		echo command: 	${LCMDS[i]}
		${LCMDS[i]}
		if [ $i -eq 10 ]; then
			mv ATLAS ${LDIRS[10]}
		fi
	else
		echo OK
	fi
done

#exit

# COMPILE gmp, gcc needs this
echo -n check if we need to compile gmp... 
if [ ! -e $INSTALL_TO/lib/libgmp.so ]; then
	echo yes
	cd ${LDIRS[17]}
	./configure --prefix=$INSTALL_TO
	make $MAKEOPTS || exit
	make check || exit
	make install || exit
	cd ..
else
	echo no.
fi

# COMPILE mpfr, gcc needs this
echo -n check if we need to compile mpfr... 
if [ ! -e $INSTALL_TO/lib/libmpfr.so ]; then
	echo yes
	cd ${LDIRS[14]}
	./configure --prefix=$INSTALL_TO --with-gmp=$INSTALL_TO
	make $MAKEOPTS || exit
	make install || exit
	cd ..
else
	echo no.
fi

# COMPILE mpc, gcc needs this
echo -n check if we need to compile mpc... 
if [ ! -e $INSTALL_TO/lib/libmpc.so ]; then
	echo yes
	cd ${LDIRS[16]}
	./configure --prefix=$INSTALL_TO --with-mpfr=$INSTALL_TO --with-gmp=$INSTALL_TO
	make $MAKEOPTS || exit
	#make check || exit
	make install || exit
	cd ..
else
	echo no.
fi

# we need to set this, otherwise gcc won't compile:
export LD_LIBRARY_PATH=$INSTALL_TO/lib
export LIBRARY_PATH=$INSTALL_TO/lib
export LD_RUN_PATH=$INSTALL_TO/lib

	if [ $HAVE_ACML -eq 1 ]; then
		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$INSTALL_TO/gfortran64/lib
		export LIBRARY_PATH=$LIBRARY_PATH:$INSTALL_TO/gfortran64/lib
		export LD_RUN_PATH=$LD_RUN_PATH:$INSTALL_TO/gfortran64/lib
	fi

# ACML:
# call
# ./install-acml-4-4-0-gfortran-64bit.sh
# install to: lib-binary
if [ $HAVE_ACML -eq 1 ]; then
	if [ ! -e $INSTALL_TO/gfortran64/lib/libacml.so ]; then
		echo enter $INSTALL_TO as installation path. Press enter to start ACML installation program.
		read
		./install-acml-4-4-0-gfortran-64bit.sh
	fi
fi
 
# COMPILE gcc
# from http://developer.amd.com/documentation/articles/pages/ACMLwithDifferentGCCGFORTRAN.aspx
echo -n check if we need to compile gcc... 
if [ ! -e $INSTALL_TO/gcc/bin/g++ ]; then
	echo yes
	mkdir -p gcc-objdir
	cd gcc-objdir
	../${LDIRS[13]}/configure --prefix=$INSTALL_TO/gcc --with-mpfr=$INSTALL_TO --with-mpc=$INSTALL_TO --with-gmp=$INSTALL_TO --enable-shared --enable-languages=c,c++,fortran || exit
	make $MAKEOPTS || exit
	make install || exit
	cd ..
else
	echo no.
fi

export PATH=$INSTALL_TO/gcc/bin:$PATH
export CPATH=$INSTALL_TO/include:$INSTALL_TO/gcc/lib/gcc/$GCCARCH/$GCCVER/include
export CPLUS_INCLUDE_PATH=$INSTALL_TO/gcc/include/c++/$GCCVER:$INSTALL_TO/gcc/lib/gcc/$GCCARCH/$GCCVER/include
export LD_LIBRARY_PATH=$INSTALL_TO/lib:$INSTALL_TO/gcc/lib64:$INSTALL_TO/gfortran64/lib
export LIBRARY_PATH=$INSTALL_TO/lib:$INSTALL_TO/gcc/lib64:$INSTALL_TO/gfortran64/lib
export LD_RUN_PATH=$INSTALL_TO/lib:$INSTALL_TO/gcc/lib64:$INSTALL_TO/gfortran64/lib

g++ -v

# COMPILE Qt
echo -n check if we need to compile Qt... 
if [ ! -e $INSTALL_TO/lib/libQtGui.so.4.7.0 ]; then
	echo yes
	cd ${LDIRS[1]}
	echo this is g++:
	g++ -v
	echo press return to continue or press ctrl-c to abort.
	read
	./configure -opensource --prefix=$INSTALL_TO -fast -no-xmlpatterns -no-multimedia -no-audio-backend -no-phonon -no-phonon-backend -no-svg -no-webkit -no-javascript-jit -no-script -no-scripttools -no-declarative -no-openssl -silent -no-nis -no-cups -no-dbus -no-nas-sound -no-qt3support || exit
	gmake $MAKEOPTS || exit
	gmake install || exit
	cd ..
else
	echo no.
fi

export PATH=$INSTALL_TO/bin:$PATH

QMAKE=$INSTALL_TO/bin/qmake

echo -n "check for qmake... "
if [ ! -e $QMAKE ]; then
	echo qmake file not found. exiting.
	exit 1;
else
	echo OK
fi

# COMPILE Qwt
echo -n check if we need to compile Qwt... 
if [ ! -e $INSTALL_TO/lib/libqwt.so.5.2.1 ]; then
	echo yes
	cd ${LDIRS[2]}
   
	newstring=NR==11{"$"0='"'INSTALLBASE=$INSTALL_TO'"'}1
	echo using awk to substitute 11th line: awk $newstring ...
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
if [ ! -e $INSTALL_TO/lib/libqwtplot3d.so.0.2.6 ]; then
	echo yes
	cd ${LDIRS[3]}
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

# COMPILE hdf5 
echo -n check if HDF5 shared lib is present... 
if [ ! -e $INSTALL_TO/lib/libhdf5_cpp.so.6.0.4 ]; then
	echo no. Copying precompiled hdf5 libraries to $INSTALL_TO
	cp -r ${LDIRS[8]}/bin/* $INSTALL_TO/bin/
	cp -r ${LDIRS[8]}/lib/* $INSTALL_TO/lib/
	cp -r ${LDIRS[8]}/share/* $INSTALL_TO/share/
	cp -r ${LDIRS[8]}/include/* $INSTALL_TO/include/
else
	echo yes.
fi

# COMPILE fftw
echo -n check if we need to compile fftw... 
if [ ! -e $INSTALL_TO/lib/libfftw3.a ]; then
	echo yes.
	cd ${LDIRS[4]}
	./configure --prefix=$INSTALL_TO || exit
	make $MAKEOPTS || exit
	make install || exit
	cd ..
else
	echo no.
fi

# COMPILE tclap
echo -n check if we need to compile tclap... 
if [ ! -e $INSTALL_TO/include/tclap ]; then
	echo yes.
	cd ${LDIRS[7]}
	./configure --prefix=$INSTALL_TO || exit
	make $MAKEOPTS || exit
	make install || exit
	cd ..
else
	echo no.
fi

if [ $HAVE_ATLAS -eq 1 ]; then
# COMPILE atlas	
echo -n check if we need to compile atlas... 
if [ ! -e $INSTALL_TO/lib/libatlas.a ]; then
	cd ${LDIRS[10]}
	srcdir=$(pwd)
	mkdir -p this-arch
	cd this-arch
	../configure --prefix=$INSTALL_TO || exit
	make build || exit                                    # tune & build lib
	make check || exit                                    # sanity check correct answer
	make time || exit                                     # check if lib is fast
        make install || exit                                  # copy libs to install dir
        cd ..; cd ..
else
	echo no.
fi
else
	echo HAVE_ATLAS: $HAVE_ATLAS := do not compile ATLAS
fi

# COMPILE itpp-external
# we only need that if we don't have ATLAS or ACML or MKL:
if [ $HAVE_ATLAS -eq 0 ] && [ $HAVE_ACML -eq 0 ]; then
	echo -n check if we need to compile itpp-external... 
	if [ ! -e $INSTALL_TO/lib/liblapack.so.3 ]; then
		echo yes.
		cd ${LDIRS[11]}
		# we have fftw already:
		./configure --prefix=$INSTALL_TO --disable-fftw || exit
		make $MAKEOPTS || exit
		make install || exit
		cd ..
	else
		echo no.
	fi
else
	echo HAVE_ATLAS: $HAVE_ATLAS, HAVE_ACML: $HAVE_ACML := do not compile itpp-external
fi

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

echo exiting.
exit

#mkdir -p release/
#export LIB_PATH=/data/propag/Roberto_Data/SNACS/64bit/lib
#cp $LIB_PATH/libconfig/lib/libconfig++.so.8 release
#cp $LIB_PATH/qwt/lib/libqwt.so.5 release
#cp $LIB_PATH/qwtplot3d/libqwtplot3d.so.0 release
exit

# Compile SNACS:
cd to SNACS/src
~/libs/qt/bin/qmake

Check what libraries the binary in release/ need:
ldd SNSignal-release

then do in release/:
ln -s ~/libs/qwt/lib/libqwt.so.5
ln -s ~/libs/qwtplot3d/libqwtplot3d.so.0
ln -s ~/libs/libconfig/lib/libconfig++.so.8

and start from release/ 
or with 
$ LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/users/fschubert/SNSignal-v0.1/src/release; release/SNSignal-0.1-release 



    INSTALLBASE    = /usr/local/qwt-$$VERSION-svn
    INSTALLBASE    = C:/Qwt-$$VERSION-svn
target.path    = $$INSTALLBASE/lib
headers.path   = $$INSTALLBASE/include
doc.path       = $$INSTALLBASE/doc
