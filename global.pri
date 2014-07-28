#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

#
# This file is included by all the other project files
# and is where options or configurations that affect all
# of the projects can be place.
# Note: because this is included by .pro files within /subdirs/ of CWD, paths
# based on PWD must compensate with an extra '..'.

OPENRPT_DIR = $$(OPENRPT_DIR)
isEmpty(OPENRPT_DIR) {
  exists(openrpt)    { OPENRPT_DIR = $$(PWD)/../openrpt    }
  exists(../openrpt) { OPENRPT_DIR = $$(PWD)/../../openrpt } # .. takes precedence
}
! exists( $$OPENRPT_DIR ) {
  error("Could not set the OPENRPT_DIR qmake variable.")
}

OPENRPT_BLD = $$(OPENRPT_BLD)
isEmpty(OPENRPT_BLD) {
  OPENRPT_BLD = $$OPENRPT_DIR
  exists($${OPENRPT_DIR}-build-desktop) { OPENRPT_BLD = $${OPENRPT_DIR}-build-desktop }
}

CSVIMP_DIR = $$(CSVIMP_DIR)
isEmpty(CSVIMP_DIR) {
  exists(csvimp)    { CSVIMP_DIR = $$(PWD)/../csvimp    }
  exists(../csvimp) { CSVIMP_DIR = $$(PWD)/../../csvimp }  # .. takes precedence
}
! exists( $$CSVIMP_DIR ) {
    error("Could not set the CSVIMP_DIR qmake variable.")
}

CSVIMP_BLD = $$(CSVIMP_BLD)
isEmpty(CSVIMP_BLD) {
  CSVIMP_BLD = $$CSVIMP_DIR
  exists($${CSVIMP_DIR}-build-desktop) { CSVIMP_BLD = $${CSVIMP_DIR}-build-desktop }
}

INCLUDEPATH += $${OPENRPT_DIR}/common           $${OPENRPT_BLD}/common \
	       $${OPENRPT_DIR}/OpenRPT/renderer $${OPENRPT_BLD}/OpenRPT/renderer \
	       $${OPENRPT_DIR}/OpenRPT/wrtembed $${OPENRPT_BLD}/OpenRPT/wrtembed \
	       $${OPENRPT_DIR}/MetaSQL          $${OPENRPT_BLD}/MetaSQL \
	       $${OPENRPT_DIR}/MetaSQL/tmp      $${OPENRPT_BLD}/MetaSQL/tmp \
	       $${CSVIMP_DIR}/csvimpcommon      $${CSVIMP_BLD}/csvimpcommon
INCLUDEPATH =  $$unique(INCLUDEPATH)

XTUPLE_DIR = ../
XTUPLE_BLD = $${XTUPLE_DIR}
exists(../xtuple-build-desktop) {
  XTUPLE_BLD = ../../xtuple-build-desktop
}

DEPENDPATH  += $${INCLUDEPATH}

CONFIG += release thread
#CONFIG += debug

macx:exists(macx.pri) {
  include(macx.pri)
}

win32:exists(win32.pri) {
  include(win32.pri)
}

unix:exists(unix.pri) {
  include(unix.pri)
}

