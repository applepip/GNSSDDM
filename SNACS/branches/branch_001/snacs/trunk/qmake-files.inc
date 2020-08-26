HEADERS += ../snWidget/snPlot.h \
    ../snWidget/snCurve.h \
    ../snSimulation/snWriteResultFile.h \
    ../snModules/snSinks/snParallelBuildCode.h \
    ../snSimulation/snCmplxFIRFilter.h \
    ../snSimulation/snFIRFilter.h \
    ../snSimulation/snParallelCorrelation.h \
    ../snSimulation/snSimulation.h \
    ../snWidget/snWidget.h \
	../snGNSS/snGNSSData.h \
	../snGNSS/snCodeGalileo.h \
	../snGNSS/snGNSSSignal.h \
	../snGNSS/snModulationAltBOC.h \
	../snGNSS/snModulationBOC.h \
	../snGNSS/snModulationCBOC.h \
	../snGNSS/snModulationBPSK.h \
	../snGNSS/snModulation.h \
    ../snGNSS/snGNSSCode.h \
    ../snGNSS/snGPS.h \
    ../snSimulation/ConcurrentBuffer3.h \
    ../snModules/snModule.h \
    ../snModules/snCreatingModule.h \
    ../snModules/snConsumingModule.h \
    ../snModules/snSerialModule.h \
    ../snModules/snSources/snSignalGenerate.h \
    ../snModules/snSinks/snSignalOutputWidget.h \
#   ../snModules/snSources/snSignalInputBinary.h \
#    ../snModules/snSinks/snSignalOutputBinary.h \
#    ../snModules/snProcessors/remez-algorithm/remez.h \ 
    ../snModules/snProcessors/snProcessorLPF.h \
    ../snModules/snProcessors/snProcessorADC.h \
#    ../snSimulation/snIPPComplexFIRFilter.h \
    ../snModules/snProcessors/snProcessorNoise.h \
    ../snModules/snProcessors/snProcessorChannel.h \
#    ../snModules/snProcessors/snMultiplicator.h \
    ../snModules/snSinks/snSDR.h \
    ../snModules/snSinks/snSDRStandard.h \
    ../snGNSS/snCN0_Estimators/snCN0_Estimator_Standard.h \
    ../snGNSS/snCN0_Estimators/snCN0_Estimator_Moment_Method.h \
    ../snGNSS/snCN0_Estimators/snCN0_Estimator_Beaulieus_Method.h \
    ../../../../../cdx_format/CDX_library_cpp/WriteCDXFile.h \
    ../../../../../cdx_format/CDX_library_cpp/WriteDiscreteDelayCDXFile.h \
    ../../../../../cdx_format/CDX_library_cpp/ReadCDXFile.h \
    ../../../../../cdx_format/CDX_library_cpp/ReadContinuousDelayCDXFile.h

SOURCES +=  ../snWidget/snPlot.cpp \
    ../snWidget/snCurve.cpp \
    ../snSimulation/snWriteResultFile.cpp \
    ../snModules/snSinks/snParallelBuildCode.cpp \
    ../snSimulation/snCmplxFIRFilter.cpp \
    ../snSimulation/snFIRFilter.cpp \
    ../snSimulation/snParallelCorrelation.cpp \
    ../snSimulation/snSimulation.cpp \
    ../snWidget/snWidget.cpp \
	../snGNSS/snGNSSData.cpp \
	../snGNSS/snCodeGalileo.cpp \
	../snGNSS/snGNSSSignal.cpp \
	../snGNSS/snModulationAltBOC.cpp \
	../snGNSS/snModulationBOC.cpp \
	../snGNSS/snModulationCBOC.cpp \
	../snGNSS/snModulationBPSK.cpp \
	../snGNSS/snModulation.cpp \
    ../snGNSS/snGNSSCode.cpp \
    ../snGNSS/snGPS.cpp \
    ../snSimulation/ConcurrentBuffer3.cpp \
    ../snModules/snModule.cpp \
    ../snModules/snCreatingModule.cpp \
    ../snModules/snConsumingModule.cpp \
    ../snModules/snSerialModule.cpp \
    ../snModules/snSources/snSignalGenerate.cpp \
    ../snModules/snSinks/snSignalOutputWidget.cpp \
#    ../snModules/snSources/snSignalInputBinary.cpp \
#    ../snModules/snSinks/snSignalOutputBinary.cpp \
#    ../snModules/snProcessors/remez-algorithm/remez.c \ 
    ../snModules/snProcessors/snProcessorLPF.cpp \
    ../snModules/snProcessors/snProcessorADC.cpp \
#    ../snSimulation/snIPPComplexFIRFilter.cpp \
    ../snModules/snProcessors/snProcessorNoise.cpp \
    ../snModules/snProcessors/snProcessorChannel.cpp \
#    ../snModules/snProcessors/snMultiplicator.cpp \
    ../snModules/snSinks/snSDR.cpp \
    ../snModules/snSinks/snSDRStandard.cpp \ 
    ../snGNSS/snCN0_Estimators/snCN0_Estimator_Standard.cpp \
    ../snGNSS/snCN0_Estimators/snCN0_Estimator_Moment_Method.cpp \
    ../snGNSS/snCN0_Estimators/snCN0_Estimator_Beaulieus_Method.cpp \
    ../../../../../cdx_format/CDX_library_cpp/WriteCDXFile.cpp \
    ../../../../../cdx_format/CDX_library_cpp/WriteDiscreteDelayCDXFile.cpp \
    ../../../../../cdx_format/CDX_library_cpp/ReadCDXFile.cpp \
    ../../../../../cdx_format/CDX_library_cpp/ReadContinuousDelayCDXFile.cpp
