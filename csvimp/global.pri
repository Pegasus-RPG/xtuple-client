#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

OPENRPT_HEADERS = $$(OPENRPT_HEADERS)
! isEmpty( OPENRPT_HEADERS ) {
  OPENRPT_DIR = $$OPENRPT_HEADERS
} else {
  exists(../../openrpt) { OPENRPT_DIR = ../../openrpt }
  exists(../openrpt)    { OPENRPT_DIR = ../openrpt    }
  OPENRPT_DIR_REL=true
}

OPENRPT_BLD = $${OPENRPT_DIR}
exists($${OPENRPT_DIR}-build-desktop) {
  OPENRPT_BLD = $${OPENRPT_DIR}-build-desktop
}

OPENRPT_LIBDIR = $$(OPENRPT_LIBDIR)
isEmpty( OPENRPT_LIBDIR ) {
  OPENRPT_LIBDIR = $${OPENRPT_BLD}/lib
  OPENRPT_LIBDIR_REL=true
}

! exists($${OPENRPT_DIR}) {
  error("Could not set the OPENRPT_DIR qmake variable.")
}

! exists($${OPENRPT_LIBDIR}) {
  error("Could not set the OPENRPT_LIBDIR qmake variable.")
}

! isEmpty( OPENRPT_DIR_REL    ) { OPENRPT_DIR    = ../$${OPENRPT_DIR}
                                  OPENRPT_BLD    = ../$${OPENRPT_BLD}    }
! isEmpty( OPENRPT_LIBDIR_REL ) { OPENRPT_LIBDIR = ../$${OPENRPT_LIBDIR} }

LIBEXT = $${QMAKE_EXTENSION_SHLIB}
win32-g++:LIBEXT = a
isEmpty( LIBEXT ) { LIBEXT = so }

macx:exists(macx.pri) {
  include(macx.pri)
}

win32:exists(win32.pri) {
  include(win32.pri)
}

unix:exists(unix.pri) {
  include(unix.pri)
}

