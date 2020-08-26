CDX_CONVERT_BINARY = ../../../../../cdx_format/CDX_library_functions/support_programs/convert-continuous-to-discrete-cdx/convert-continuous-to-discrete-cdx
PYTHON = python

MOVING_LOS_CONT_CDX = snacs_cir_moving_los_cont.h5
MOVING_LOS_DISCR_CDX = snacs_cir_moving_los_discr.h5

all: $(MOVING_LOS_DISCR_CDX)

$(MOVING_LOS_CONT_CDX): 
	$(PYTHON) snacs_generate_moving_los.py

030_convert: $(MOVING_LOS_DISCR_CDX)

$(MOVING_LOS_DISCR_CDX): $(MOVING_LOS_CONT_CDX)
	$(CDX_CONVERT_BINARY) -b 1e-6 -a 1e-6 -s 20e6 -i $(MOVING_LOS_CONT_CDX) -o $(MOVING_LOS_DISCR_CDX)

clean:
	rm -f *.h5
