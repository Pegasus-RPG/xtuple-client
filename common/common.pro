include( ../global.pri )

TARGET      = xtuplecommon
TEMPLATE    = lib
CONFIG      += qt warn_on dll
# TEMPORARY HACK
win32 {
  CONFIG -= dll
  CONFIG += staticlib
}
macx {
  CONFIG -= dll
  CONFIG += staticlib
}

DEFINES     += MAKELIB

INCLUDEPATH += $(QTDIR)/src/3rdparty/zlib
INCLUDEPATH += $(QTSRC)/src/3rdparty/zlib
INCLUDEPATH += $(CSVIMP_HEADERS)/csvimpcommon $(CSVIMP_HEADERS)/plugin

DESTDIR = ../lib
OBJECTS_DIR = tmp
MOC_DIR     = tmp
UI_DIR      = tmp

QMAKE_LIBDIR += $${OPENRPT_LIBDIR}
LIBS += -lopenrptcommon -lMetaSQL $${LIBDMTX} -lz

SOURCES = applock.cpp              \
          calendarcontrol.cpp      \
          calendargraphicsitem.cpp \
          checkForUpdates.cpp      \
          cmdlinemessagehandler.cpp \
          errorReporter.cpp        \
          exporthelper.cpp \
          guimessagehandler.cpp \
          importhelper.cpp \
          format.cpp \
          graphicstextbuttonitem.cpp \
          gunzip.cpp \
          login2.cpp \
          metrics.cpp \
          metricsenc.cpp \
          mqlhash.cpp                   \
          qbase64encode.cpp \
          qmd5.cpp \
          shortcuts.cpp \
          statusbarmessagehandler.cpp \
          storedProcErrorLookup.cpp \
          tarfile.cpp \
          xabstractmessagehandler.cpp \
          xbase32.cpp \
          xcachedhash.cpp               \
          xtupleproductkey.cpp \
          xtNetworkRequestManager.cpp \
          xtsettings.cpp

HEADERS = applock.h              \
          calendarcontrol.h      \
          calendargraphicsitem.h \
          cmdlinemessagehandler.h \
          checkForUpdates.h      \
          errorReporter.h        \
          exporthelper.h \
          importhelper.h \
          format.h \
          graphicstextbuttonitem.h \
          guimessagehandler.h \
          gunzip.h \
          login2.h \
          metrics.h \
          metricsenc.h \
          mqlhash.h                     \
          qbase64encode.h \
          qmd5.h \
          shortcuts.h \
          statusbarmessagehandler.h \
          storedProcErrorLookup.h \
          tarfile.h \
          xabstractmessagehandler.h \
          xbase32.h \
          xcachedhash.h                 \
          xtupleproductkey.h \
          xtNetworkRequestManager.h \
          xtsettings.h

FORMS = login2.ui checkForUpdates.ui

QT +=  script sql xml xmlpatterns network widgets

RESOURCES += xTupleCommon.qrc
