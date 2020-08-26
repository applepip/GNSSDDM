# LINUX_DISTRIBUTION:
# other: self-compiled libs
# Debian
LINUX_DISTRIBUTION = Debian

TEMPLATE = app
QT -= gui # Only the core module is used, no GUI
CONFIG = qt console warn_on
#CONFIG += debug
CONFIG += release

debug { 
    message(building in debug mode)
    DESTDIR = debug
    TARGET = snacs-debug
	OBJECTS_DIR = debug/
}

release { 
    message(building in release mode)
    DESTDIR = release
    TARGET = snacs-release
	OBJECTS_DIR = release/
}

win32 { 
	INCLUDEPATH += C:\libs\itpp-4.2 \
		C:\libs\fftw-3.2.2 \
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
        #-L"c:\libs\libconfig-1.4.3\lib\.libs" \
        -lconfig++ \
        -L"c:\libs\hdf5\lib" \
        -lhdf5_cpp \
        -lhdf5

    release:LIBS += -lqwt5 -L/mingw/lib -litpp -lfftw3 -llapack -lblas -lgfortran -lmingw32 -lmoldname -lmingwex -lmsvcrt -lm -ladvapi32 -lshell32 -luser32 -lkernel32
}

unix { 
    QMAKE_CXXFLAGS_RELEASE += $$COMPILER_FLAGS
    LIBS += -lfftw3 -lhdf5 -lhdf5_cpp 

	contains(LINUX_DISTRIBUTION, other) {
	    # for self compiled libs:
	    message(building with self compiled libs)
	    INCLUDEPATH += $$LIB_PATH/include 
	
	    LIBS += -L$$LIB_PATH/lib \
	    -lconfig++ \
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
}


contains(LINUX_DISTRIBUTION, Debian) {
	message(building with Debian libs)
    # for debian libs:
    LIBS += -lconfig++ -litpp

    # search for libraries also in /usr/local/lib and ./ :
    #LIBS += -Wl,-rpath,/usr/local/lib,-rpath,./,-rpath,$$LIB_PATH/lib
}

include(../qmake-files.inc)
SOURCES += main.cpp snConsole.cpp snWidgetConsole.cpp snPlotConsole.cpp
HEADERS += snConsole.h snWidgetConsole.h snPlotConsole.h 
 