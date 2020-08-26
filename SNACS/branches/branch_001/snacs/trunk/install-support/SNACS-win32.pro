# LINUX_DISTRIBUTION:
# other: self-compiled libs
# Debian
LINUX_DISTRIBUTION = other

# OPTIMIZING_LIB: ipp and/or (atlas or acml or mkl)
OPTIMIZING_LIB = acml #ipp

LIB_PATH = d:/libs/lib-binary
LIBSRC_PATH = /home/fschubert/libs2/lib-binary/include

IPP_INC_PATH = /home/fschubert/libs/intel/ipp/6.1.2.051/em64t/include
IPP_LIB_PATH = /home/fschubert/libs/intel/ipp/6.1.2.051/em64t/sharedlib

COMPILER_FLAGS = -march=opteron -mfpmath=sse -O3 -ffast-math
# for 32bit: -malign-double

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
    DEPENDPATH += $$LIB_PATH/lib
	INCLUDEPATH += $$LIB_PATH/include \
		$$LIB_PATH/../qwtplot3d-0.2.7/include \
		$$LIB_PATH/../hdf5-1.8.4-patch1_mingw/c++/src \
		$$LIB_PATH/../boost_1_44_0 
    LIBS += -mwindows \
        -mconsole \
		-L$$LIB_PATH/lib \
		-lblas \
		-llapack \
        -litpp \
        -lfftw3 \
		-lqwt5 \
        -lqwtplot3d \
        -lconfig++ \
        -lhdf5_cpp \
        -lhdf5 
    
    QMAKE_CXXFLAGS_RELEASE += -O9 -msse2
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
    LIBS += -Wl,-rpath,/usr/local/lib,-rpath,./,-rpath,$$LIB_PATH/lib

}

TEMPLATE = app

HEADERS += snWidget/snPlot.h \
    snWidget/snCurve.h \
    snSimulation/snWriteResultFile.h \
    snSimulation/snReadCIRFile.h \
    #snSourceBlocks/snSignalInputBinary.h \
    #snSinkBlocks/snSignalOutputBinary.h \
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
    snSimulation/ConcurrentBuffer3.h \
    snSimulation/snModule.h \
    snSimulation/snCreatingModule.h \
    snSimulation/snConsumingModule.h \
    snSimulation/snSerialModule.h \
    snSourceBlocks/snSignalGenerate.h \
    snSinkBlocks/snSignalOutputWidget.h \
    snProcessorBlocks/remez-algorithm/remez.h \ 
    snProcessorBlocks/snProcessorLPF.h \
    snProcessorBlocks/snProcessorADC.h \
    snSimulation/snIPPComplexFIRFilter.h \
    snProcessorBlocks/snProcessorNoise.h \
    snProcessorBlocks/snProcessorChannel.h \
    snSinkBlocks/snSDR.h \
    snSinkBlocks/snSDRStandard.h \
    snGNSS/snCN0_Estimators/snCN0_Estimator_Standard.h
SOURCES +=  snWidget/snPlot.cpp \
    snWidget/snCurve.cpp \
    snSimulation/snWriteResultFile.cpp \
    snSimulation/snReadCIRFile.cpp \
    #snSourceBlocks/snSignalInputBinary.cpp \
    #snSinkBlocks/snSignalOutputBinary.cpp \
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
    snSimulation/ConcurrentBuffer3.cpp \
    snSimulation/snModule.cpp \
    snSimulation/snCreatingModule.cpp \
    snSimulation/snConsumingModule.cpp \
    snSimulation/snSerialModule.cpp \
    snSourceBlocks/snSignalGenerate.cpp \
    snSinkBlocks/snSignalOutputWidget.cpp \
    snProcessorBlocks/remez-algorithm/remez.c \ 
    snProcessorBlocks/snProcessorLPF.cpp \
    snProcessorBlocks/snProcessorADC.cpp \
    snSimulation/snIPPComplexFIRFilter.cpp \
    snProcessorBlocks/snProcessorNoise.cpp \
    snProcessorBlocks/snProcessorChannel.cpp \
    snSinkBlocks/snSDR.cpp \
    snSinkBlocks/snSDRStandard.cpp \ 
    snGNSS/snCN0_Estimators/snCN0_Estimator_Standard.cpp
    