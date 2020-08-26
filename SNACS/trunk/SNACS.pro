#LINUX_DISTRIBUTION = other
LINUX_DISTRIBUTION = Debian

#LIB_PATH = /SNACS/64bit/lib
#LIBSRC_PATH = /SNACS/64bit/libsrc

LIB_PATH = c:\libs
LIBSRC_PATH = c:\libs

CONFIG += console warn_on
CONFIG -= debug
CONFIG += release

RC_FILE = snacs.rc

debug { 
    message(building in debug mode)
    DESTDIR = debug
    TARGET = SNACS-debug
}

release { 
    message(building in release mode)
    DESTDIR = release
    TARGET = SNACS-release
}

win32 { 
    DEPENDPATH += C:\libs\qwt-5.2\src \
        C:\libs\qwtplot3d-0.2.7\include \
        C:\libs\qwtplot3d-0.2.7\src \
        C:\libs\libconfig-1.3.2\libs
	INCLUDEPATH += C:\libs\fftw-3.2.2 \
        C:\libs\qwt-5.2\src \
        C:\libs\qwtplot3d-0.2.7\include \
        C:\libs\qwtplot3d-0.2.7\src \
        C:\libs\libconfig-1.4.3\lib \
        c:\libs\tclap-1.2.0\include \
		C:\libs\boost_1_42_0 \
        c:\libs\hdf5\include
    LIBS += -mwindows \
        -mconsole \
        -L"c:\libs\fftw-3.2.2" \
        -lfftw3-3 \
        -L"c:\libs\qwt-5.2\lib" \
        -L"c:\libs\qwtplot3d-0.2.7\lib" \
        -lqwtplot3d \
        -L"c:\libs\libconfig-1.4.3\lib\.libs" \
        -lconfig++-8 \
        -L"c:\libs\hdf5\lib" \
        -lhdf5_cpp \
        -lhdf5

    release:LIBS += -lqwt5
    
    # debug:LIBS += -lqwtd
    QMAKE_CXXFLAGS_RELEASE += -O9 -msse2 -DUSE_SSE
}

unix { 
    QMAKE_CXXFLAGS_RELEASE += -msse3 \
        -mfpmath=sse \
        -mtune=nocona \
        -march=nocona \
        -O9 \
        -DUSE_SSE \
	-pg
    LIBS += -lfftw3 -lhdf5 -lhdf5_cpp 

!contains(LINUX_DISTRIBUTION, Debian) {
    # for self compiled libs:
    message(building with self compiled libs)
    LIBS += -L$$LIB_PATH/libconfig/lib \
    -lconfig++ \
    -L$$LIB_PATH/qwt/lib \
    -lqwt \
    -L$$LIB_PATH/hdf5/lib \
    -L$$LIB_PATH/fftw/lib \
    -L$$LIB_PATH/qwtplot3d \
    -lqwtplot3d 
    # -L$$LIB_PATH/itpp/lib \
    INCLUDEPATH += $$LIB_PATH/libconfig/include \
        $$LIB_PATH/qwt/include \
        $$LIBSRC_PATH/qwtplot3d/include \
        $$LIB_PATH/fftw/include \
        $$LIB_PATH/tclap/include \
        $$LIB_PATH/hdf5/include
        # $$LIB_PATH/itpp/include
}


contains(LINUX_DISTRIBUTION, Debian) {
    # for debian libs:
    message(building with Debian libs)
    INCLUDEPATH = /usr/include/qwt-qt4 \
        /usr/include/qwtplot3d-qt4 
    #DEPENDPATH = /usr/include/qwt-qt4
    LIBS += -lqwt-qt4 \
     -lqwtplot3d-qt4 \
     -lconfig++
}    

    # search for libraries also in /usr/local/lib and ./ :
    LIBS += -Wl,-rpath,/usr/local/lib,-rpath,./

}

TEMPLATE = app

HEADERS += snWidget/snPlot.h \
    snWidget/snCurve.h \
    snSimulation/snWriteResultFile.h \
    snSimulation/snReadCIRFile.h \
    snSourceBlocks/snSignalInputBinary.h \
    snSinkBlocks/snSignalOutputBinary.h \
    snSinkBlocks/snParallelBuildCode.h \
    snSimulation/snCmplxFIRFilter.h \
    snSimulation/snFIRFilter.h \
    snSimulation/snParallelCorrelation.h \
    snSimulation/snMainWindow.h \
    snSimulation/snSimulation.h \
    snWidget/snWidget.h \
snGNSS/snGNSSData.h \
snGNSS/snCodeGalileo.h \
snGNSS/snGNSSSignal.h \
snGNSS/snModulationAltBOC.h \
snGNSS/snModulationBOC.h \
snGNSS/snModulationCBOC.h \
snGNSS/snModulationBPSK.h \
snGNSS/snModulation.h \
    snGNSS/snGNSSCode.h \
    snGNSS/snGPS.h \
    snSimulation/ConcurrentBuffer.h \
    snSimulation/snBlock.h \
    snSimulation/snBlockSerial.h \
    snSourceBlocks/snSignalGenerate.h \
    snSinkBlocks/snSignalOutputWidget.h \
    snProcessorBlocks/remez-algorithm/remez.h \
    snProcessorBlocks/snProcessorLPF.h \
    snProcessorBlocks/snProcessorADC.h \
    snProcessorBlocks/snProcessorNoise.h \
    snProcessorBlocks/snProcessorChannel.h \
    snSinkBlocks/snSDR.h \
    snSinkBlocks/snSDRStandard.h
SOURCES +=  snWidget/snPlot.cpp \
    snWidget/snCurve.cpp \
    snSimulation/snWriteResultFile.cpp \
    snSimulation/snReadCIRFile.cpp \
    snSourceBlocks/snSignalInputBinary.cpp \
    snSinkBlocks/snSignalOutputBinary.cpp \
    snSinkBlocks/snParallelBuildCode.cpp \
    snSimulation/snCmplxFIRFilter.cpp \
    snSimulation/snFIRFilter.cpp \
    snSimulation/snParallelCorrelation.cpp \
    snSimulation/snMainWindow.cpp \
    snSimulation/snSimulation.cpp \
    main.cpp \
    snWidget/snWidget.cpp \
snGNSS/snGNSSData.cpp \
snGNSS/snCodeGalileo.cpp \
snGNSS/snGNSSSignal.cpp \
snGNSS/snModulationAltBOC.cpp \
snGNSS/snModulationBOC.cpp \
snGNSS/snModulationCBOC.cpp \
snGNSS/snModulationBPSK.cpp \
snGNSS/snModulation.cpp \
    snGNSS/snGNSSCode.cpp \
    snGNSS/snGPS.cpp \
    snSimulation/ConcurrentBuffer.cpp \
    snSimulation/snBlock.cpp \
    snSimulation/snBlockSerial.cpp \
    snSourceBlocks/snSignalGenerate.cpp \
    snSinkBlocks/snSignalOutputWidget.cpp \
    snProcessorBlocks/remez-algorithm/remez.c \
    snProcessorBlocks/snProcessorLPF.cpp \
    snProcessorBlocks/snProcessorADC.cpp \
    snProcessorBlocks/snProcessorNoise.cpp \
    snProcessorBlocks/snProcessorChannel.cpp \
    snSinkBlocks/snSDR.cpp \
    snSinkBlocks/snSDRStandard.cpp
