include( ../../global.pri )

SOURCES     = main.cpp
FORMS       = sandboxwindow.ui
TEMPLATE    = app
CONFIG      += qt warn_on
INCLUDEPATH += ..

unix:LIBS += -L$(QTDIR)/plugins/designer -lOpenMFGWidgets
win32:LIBS += $(QTDIR)/plugins/designer/OpenMFGWidgets.lib

MOC_DIR = tmp
UI_DIR = tmp
OBJECTS_DIR = tmp

