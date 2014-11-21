#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#

TEMPLATE = app
CONFIG += qt warn_on release

HARDCODE_APPLICATION_DIR = $$(HARDCODE_APPLICATION_DIR)
! isEmpty( HARDCODE_APPLICATION_DIR ) {
  # Note the way we wrap the variable with '"  "' to ensure it is
  # properly quoted for C/C++ syntax
  QMAKE_CXXFLAGS += -DHARDCODE_APPLICATION_DIR=\'\"$(HARDCODE_APPLICATION_DIR)\"\'
}

include(../global.pri)

TARGET = ../csvimp

OBJECTS_DIR = tmp
MOC_DIR     = tmp
UI_DIR      = tmp

INCLUDEPATH += ../csvimpcommon ../csvimpcommon/images ../plugin \
               $${OPENRPT_DIR}/common $${OPENRPT_DIR}/MetaSQL \
               $${OPENRPT_BLD}/common $${OPENRPT_BLD}/MetaSQL
INCLUDEPATH = $$unique(INCLUDEPATH)

win32:INCLUDEPATH += .
DEPENDPATH  += $${INCLUDEPATH}

QMAKE_LIBDIR = $${OPENRPT_LIBDIR} $$QMAKE_LIBDIR
LIBS += -lopenrptcommon -lMetaSQL

win32-msvc* {
  PRE_TARGETDEPS += $${OPENRPT_LIBDIR}/openrptcommon.$${LIBEXT} \
                    $${OPENRPT_LIBDIR}/MetaSQL.$${LIBEXT}
} else {
  PRE_TARGETDEPS += $${OPENRPT_LIBDIR}/libopenrptcommon.$${LIBEXT} \
                    $${OPENRPT_LIBDIR}/libMetaSQL.$${LIBEXT}
}

win32:RC_FILE = application.rc

macx {
  RC_FILE = ../csvimpcommon/images/icons.icns
  QMAKE_INFO_PLIST = Info.plist
}

# Input
FORMS   = 
HEADERS = ../csvimpcommon/csvimpdata.h  \
          ../csvimpcommon/csvimpplugininterface.h

SOURCES = main.cpp      \
          ../csvimpcommon/csvimpdata.cpp

QT += sql widgets core
RESOURCES += ../csvimpcommon/csvimp.qrc
