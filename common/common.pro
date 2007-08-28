include( ../global.pri )

TARGET      = xtuplecommon
TEMPLATE    = lib
CONFIG      += qt warn_on staticlib
DEFINES     += MAKELIB

DESTDIR = ../lib
OBJECTS_DIR = tmp
MOC_DIR     = tmp
UI_DIR      = tmp

SOURCES = metrics.cpp \
          qbase64encode.cpp \
          qmd5.cpp \
          metricsenc.cpp \
          format.cpp \
          login2.cpp \
          login2Options.cpp
HEADERS = metrics.h \
          qbase64encode.h \
          qmd5.h \
          metricsenc.h \
          format.h \
          login2.h \
          login2Options.h
FORMS = login2.ui login2Options.ui


#The following line was inserted by qt3to4
QT +=  sql

RESOURCES += OpenMFGCommon.qrc
