#!/bin/bash

rm -rf /tmp/snacs-only-view
#../../trunk/snacs-Qt/release/SNACS-release -f only-view-signal.cfg $@
../../trunk/snacs-console/release/snacs-release -f only-view-signal.cfg $@
