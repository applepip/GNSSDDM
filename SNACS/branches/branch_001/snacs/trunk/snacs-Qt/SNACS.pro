# LINUX_DISTRIBUTION:
# other: self-compiled libs
# Debian
LINUX_DISTRIBUTION = Debian

# OPTIMIZING_LIB: ipp and/or (atlas or acml or mkl)
OPTIMIZING_LIB = ipp

LIB_PATH = /home/fschubert/libs2/lib-binary
LIBSRC_PATH = /home/fschubert/libs2/lib-binary/include

IPP_INC_PATH = /home/fschubert/libs/intel/ipp/6.1.2.051/em64t/include
IPP_LIB_PATH = /home/fschubert/libs/intel/ipp/6.1.2.051/em64t/sharedlib

TEMPLATE = app

CONFIG = qt console warn_on
#CONFIG += debug
CONFIG += release

QMAKE_CXXFLAGS += -O9 -msse2 -pg
#-mfpmath=sse -O3 -ffast-math -march=core2 -msse2 -march=opteron 
# for 32bit: -malign-double
QMAKE_LFLAGS += -pg

RC_FILE = snacs.rc

debug { 
    message(building in debug mode)
    DESTDIR = debug
    TARGET = SNACS-debug
	OBJECTS_DIR = debug/
}

release { 
    message(building in release mode)
    DESTDIR = release
    TARGET = SNACS-release
	OBJECTS_DIR = release/
}

win32 { 
    DEPENDPATH += C:\libs\qwt-5.2\src \
        C:\libs\qwtplot3d\include \
        C:\libs\qwtplot3d\src 
	INCLUDEPATH += C:\libs\itpp-4.2 \
		C:\libs\fftw-3.2.2 \
        C:\libs\qwt-5.2\src \
        C:\libs\qwtplot3d\include \
        C:\libs\qwtplot3d\src \
        C:\libs\libconfig-1.4.3\lib \
        c:\libs\tclap-1.2.0\include \
		C:\libs\boost_1_42_0 \
        c:\libs\hdf5\include
    LIBS += -mwindows \
        -mconsole \
        -L"c:\libs\itpp-4.2\itpp\.libs" \
        -litpp \
        -L"c:\libs\fftw-3.2.2" \
        -lfftw3 \
        -L"c:\libs\qwt-5.2\lib" \
        -L"c:\libs\qwtplot3d\lib" \
        -lqwtplot3d \
        #-L"c:\libs\libconfig-1.4.3\lib\.libs" \
        -lconfig++ \
        -L"c:\libs\hdf5\lib" \
        -lhdf5_cpp \
        -lhdf5

    release:LIBS += -lqwt5 -L/mingw/lib -litpp -lfftw3 -llapack -lblas -lgfortran -lmingw32 -lmoldname -lmingwex -lmsvcrt -lm -ladvapi32 -lshell32 -luser32 -lkernel32
    
    # debug:LIBS += -lqwtd
    #QMAKE_CXXFLAGS_RELEASE += -O9 -msse2 -DUSE_SSE
}

unix { 
    QMAKE_CXXFLAGS_RELEASE += $$COMPILER_FLAGS
    LIBS += -lfftw3 -lhdf5 -lhdf5_cpp 

contains(LINUX_DISTRIBUTION, other) {
    # for self compiled libs:
    message(building with self compiled libs)
    INCLUDEPATH += $$LIB_PATH/include $$LIB_PATH/../qwtplot3d/include

    LIBS += -L$$LIB_PATH/lib \
    -lconfig++ \
    -lqwt \
    -lqwtplot3d \
    -litpp \
    -lsz 

	contains(OPTIMIZING_LIB, acml) {
		QMAKE_CXXFLAGS += -DHAVE_ACML
		INCLUDEPATH += $$LIB_PATH/gfortran64/include
		LIBS += -L$$LIB_PATH/gfortran64/lib -lacml -lacml_mv -lgfortran
		LIBS += -Wl,-rpath,$$LIB_PATH/gfortran64/lib -Wl,-rpath,$$LIB_PATH/gcc/lib64
	}

	contains(OPTIMIZING_LIB, atlas) {
    		LIBS += -latlas \
    		-lblas \
    		-llapack
	}

	contains(OPTIMIZING_LIB, ipp) {
		QMAKE_CXXFLAGS += -DHAVE_IPP
		INCLUDEPATH += $$IPP_INC_PATH
		LIBS += -L$$IPP_LIB_PATH
		LIBS += -lippiem64t -lippsem64t -lippcoreem64t -liomp5 -lpthread -lm
	}
}


contains(LINUX_DISTRIBUTION, Debian) {
	message(building with Debian libs)
    # for debian libs:
    INCLUDEPATH = /usr/include/qwt-qt4 \
        /usr/include/qwtplot3d-qt4 
    LIBS += -lqwt-qt4 -lqwtplot3d-qt4 -lGLU -lconfig++ -litpp
}    

    # search for libraries also in /usr/local/lib and ./ :
    #LIBS += -Wl,-rpath,/usr/local/lib,-rpath,./,-rpath,$$LIB_PATH/lib
}

include(../qmake-files.inc)
SOURCES += snMainWindow.cpp main.cpp snWidgetQwt.cpp snPlotQwt.cpp
HEADERS += snMainWindow.h snWidgetQwt.h snPlotQwt.h 
