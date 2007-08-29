include( ../global.pri )

TARGET      = xtuplewidgets

TEMPLATE    =  lib
CONFIG      += qt warn_on
#INCLUDEPATH += $$QT_SOURCE_TREE/tools/designer/interfaces ../common .
INCLUDEPATH += ../common .
DBFILE      =  widgets.db
LANGUAGE    =  C++

DEPENDPATH += ../common

dynamic {
  DESTDIR = $$[QT_INSTALL_PLUGINS]/designer
  CONFIG += designer plugin

  LIBS += -L../lib -L../$$OPENRPT_DIR/lib -lxtuplecommon -lcommon -lrenderer
  DEFINES += MAKEDLL

  MOC_DIR = tmp/dll
  OBJECTS_DIR = tmp/dll
  UI_DIR = tmp/dll

  HEADERS += \
             plugins/addressclusterplugin.h \
             plugins/calendarcomboboxplugin.h \
             plugins/clineeditplugin.h \
             plugins/commentsplugin.h \
             plugins/contactclusterplugin.h \
             plugins/crmacctclusterplugin.h \
             plugins/currclusterplugin.h \
             plugins/currdisplayplugin.h \
             plugins/custclusterplugin.h \
             plugins/custinfoplugin.h \
             plugins/dateclusterplugin.h \
             plugins/deptclusterplugin.h \
             plugins/dlineeditplugin.h \
             plugins/expenseclusterplugin.h \
             plugins/expenselineeditplugin.h \
             plugins/glclusterplugin.h \
             plugins/invoicelineeditplugin.h \
	     plugins/incidentclusterplugin.h \
             plugins/itemclusterplugin.h \
             plugins/itemlineeditplugin.h \
             plugins/lotserialclusterplugin.h \
	     plugins/opportunityclusterplugin.h \
             plugins/parametergroupplugin.h \
             plugins/periodslistviewplugin.h \
             plugins/planordclusterplugin.h \
             plugins/planordlineeditplugin.h \
             plugins/poclusterplugin.h \
             plugins/polineeditplugin.h \
             plugins/projectclusterplugin.h \
             plugins/projectlineeditplugin.h \
             plugins/shiftclusterplugin.h \
             plugins/shipmentclusterplugin.h \
             plugins/shiptoclusterplugin.h \
             plugins/shiptoeditplugin.h \
             plugins/soclusterplugin.h \
             plugins/solineeditplugin.h \
             plugins/toclusterplugin.h \
             plugins/usernameclusterplugin.h \
             plugins/usernamelineeditplugin.h \
             plugins/vendorclusterplugin.h \
             plugins/vendorinfoplugin.h \
             plugins/vendorlineeditplugin.h \
             plugins/warehousegroupplugin.h \
             plugins/wcomboboxplugin.h \
             plugins/woclusterplugin.h \
             plugins/wolineeditplugin.h \
             plugins/womatlclusterplugin.h \
             plugins/workcenterclusterplugin.h \
             plugins/workcenterlineeditplugin.h \
             plugins/xcomboboxplugin.h \
             plugins/xlineeditplugin.h \
             plugins/xtreewidgetplugin.h \
             plugins/xurllabelplugin.h
} else {
  DESTDIR = ../lib
  CONFIG += staticlib

  MOC_DIR = tmp/lib
  OBJECTS_DIR = tmp/lib
  UI_DIR = tmp/lib
}

SOURCES    += OpenMFGWidgets.cpp \
	      addressCluster.cpp contactCluster.cpp crmacctCluster.cpp \
              xlineedit.cpp xcombobox.cpp \
              xlistbox.cpp \
              custCluster.cpp \
              itemCluster.cpp itemList.cpp itemSearch.cpp itemAliasList.cpp \
              warehouseCluster.cpp warehousegroup.cpp \
              woCluster.cpp woList.cpp \
              glCluster.cpp accountList.cpp accountSearch.cpp \
              invoiceLineEdit.cpp incidentCluster.cpp \
              opportunitycluster.cpp \
              poCluster.cpp purchaseOrderList.cpp \
              plCluster.cpp plannedOrderList.cpp \
              vendorcluster.cpp \
              soCluster.cpp salesOrderList.cpp \
              shiptoCluster.cpp shipToList.cpp \
	      toCluster.cpp \
	      transferOrderList.cpp \
              calendarTools.cpp \
              parametergroup.cpp \
              comment.cpp comments.cpp \
              xurllabel.cpp \
	      currCluster.cpp usernameCluster.cpp usernameList.cpp \
              workcenterCluster.cpp \
              projectCluster.cpp projectList.cpp \
              expensecluster.cpp \
              datecluster.cpp \
	      virtualCluster.cpp deptCluster.cpp shiftCluster.cpp \
              xtreewidget.cpp \
	      lotserialCluster.cpp \
	      shipmentCluster.cpp

HEADERS    += OpenMFGWidgets.h \
	      addresscluster.h contactcluster.h crmacctcluster.h \
              xlineedit.h xcombobox.h \
              xlistbox.h \
              custcluster.h \
              itemcluster.h itemList.h itemSearch.h itemAliasList.h \
              warehouseCluster.h warehousegroup.h \
              woCluster.h woList.h \
              glcluster.h accountList.h accountSearch.h \
              invoicelineedit.h incidentcluster.h \
              opportunitycluster.h \
              pocluster.h purchaseOrderList.h \
              plCluster.h plannedOrderList.h \
              vendorcluster.h \
              socluster.h salesOrderList.h \
              shiptocluster.h shipToList.h \
	      tocluster.h \
	      transferOrderList.h \
              calendarTools.h \
              parametergroup.h \
              comment.h comments.h \
              xurllabel.h \
	      currcluster.h usernamecluster.h usernameList.h \
              workcentercluster.h \
              projectcluster.h projectList.h \
              expensecluster.h \
              datecluster.h \
	      virtualCluster.h deptcluster.h shiftcluster.h \
              xtreewidget.h \
	      lotserialCluster.h \
	      shipmentcluster.h

FORMS += accountSearch.ui

#The following line was inserted by qt3to4
QT +=  sql qt3support 
