#!/bin/bash

INSTALL_TO=/home/fschubert/libs2/lib-binary
IPP_LIB_PATH=/home/fschubert/libs/intel/ipp/6.1.2.051/em64t/sharedlib

PATH=$INSTALL_TO/gcc/bin:$INSTALL_TO/bin:$PATH

LD_LIBRARY_PATH=$INSTALL_TO/lib:$INSTALL_TO/gcc/lib64:$INSTALL_TO/gfortran64/lib:$IPP_LIB_PATH
LIBRARY_PATH=$INSTALL_TO/lib:$INSTALL_TO/gcc/lib64:$INSTALL_TO/gfortran64/lib:$IPP_LIB_PATH
LD_RUN_PATH=$INSTALL_TO/lib:$INSTALL_TO/gcc/lib64:$INSTALL_TO/gfortran64/lib:$IPP_LIB_PATH


