include( ../global.pri )

TARGET      = xtuplescriptapi

TEMPLATE    =  lib
CONFIG      += qt warn_on staticlib
DBFILE      =  scriptapi.db
LANGUAGE    =  C++

DEPENDPATH  += . ../common ../widgets ../widgets/tmp/lib
INCLUDEPATH += . ../common ../widgets ../widgets/tmp/lib .

DESTDIR = ../lib

MOC_DIR     = tmp
OBJECTS_DIR = tmp
UI_DIR      = tmp

HEADERS +=  setupscriptapi.h                    \
            qactionproto.h                      \
            qdomattrproto.h                     \
            qdomcdatasectionproto.h             \
            qdomcharacterdataproto.h            \
            qdomcommentproto.h                  \
            qdomdocumentfragmentproto.h         \
            qdomdocumentproto.h                 \
            qdomdocumenttypeproto.h             \
            qdomelementproto.h                  \
            qdomentityproto.h                   \
            qdomentityreferenceproto.h          \
            qdomimplementationproto.h           \
            qdomnamednodemapproto.h             \
            qdomnodelistproto.h                 \
            qdomnodeproto.h                     \
            qdomnotationproto.h                 \
            qdomprocessinginstructionproto.h    \
            qdomtextproto.h                     \
            qmenuproto.h                        \
            qmessageboxsetup.h                  \
            qnetworkreplyproto.h                \
            qnetworkrequestproto.h              \
            qsqldatabaseproto.h                 \
            qsqlrecordproto.h                   \
            qstackedwidgetproto.h               \
            qtabwidgetproto.h                   \
            qurlproto.h                         \
            xdatawidgetmapperproto.h            \
            xnetworkaccessmanager.h             \
            xsqltablemodelproto.h               \
            xtreewidgetitemproto.h              \
            addressclustersetup.h		\
            alarmssetup.h			\
            clineeditsetup.h			\
            commentssetup.h			\
            contactclustersetup.h		\
            crmacctlineeditsetup.h		\
            currdisplaysetup.h			\
            documentssetup.h			\
            glclustersetup.h			\
            itemlineeditsetup.h			\
            orderlineeditsetup.h		\
            parametergroupsetup.h		\
            polineeditsetup.h			\
            projectlineeditsetup.h		\
            ralineeditsetup.h			\
            revisionlineeditsetup.h		\
            screensetup.h			\
            shipmentclusterlineeditsetup.h	\
            solineeditsetup.h			\
            tolineeditsetup.h			\
            usernamelineeditsetup.h		\
            vendorgroupsetup.h			\
            wcomboboxsetup.h			\
            wolineeditsetup.h			\
            womatlclustersetup.h		\
            xcomboboxsetup.h			\
            xdateeditsetup.h			\

SOURCES +=  setupscriptapi.cpp                  \
            qactionproto.cpp                    \
            qdomattrproto.cpp                   \
            qdomcdatasectionproto.cpp           \
            qdomcharacterdataproto.cpp          \
            qdomcommentproto.cpp                \
            qdomdocumentfragmentproto.cpp       \
            qdomdocumentproto.cpp               \
            qdomdocumenttypeproto.cpp           \
            qdomelementproto.cpp                \
            qdomentityproto.cpp                 \
            qdomentityreferenceproto.cpp        \
            qdomimplementationproto.cpp         \
            qdomnamednodemapproto.cpp           \
            qdomnodelistproto.cpp               \
            qdomnodeproto.cpp                   \
            qdomnotationproto.cpp               \
            qdomprocessinginstructionproto.cpp  \
            qdomtextproto.cpp                   \
            qmenuproto.cpp                      \
            qmessageboxsetup.cpp                \
            qnetworkreplyproto.cpp              \
            qnetworkrequestproto.cpp            \
            qsqldatabaseproto.cpp               \
            qsqlrecordproto.cpp                 \
            qstackedwidgetproto.cpp             \
            qtabwidgetproto.cpp                 \
            qurlproto.cpp                       \
            xdatawidgetmapperproto.cpp          \
            xnetworkaccessmanager.cpp           \
            xsqltablemodelproto.cpp             \
            xtreewidgetitemproto.cpp            \
            addressclustersetup.cpp		\
            alarmssetup.cpp			\
            clineeditsetup.cpp			\
            commentssetup.cpp			\
            contactclustersetup.cpp		\
            crmacctlineeditsetup.cpp		\
            currdisplaysetup.cpp		\
            documentssetup.cpp			\
            glclustersetup.cpp			\
            itemlineeditsetup.cpp		\
            orderlineeditsetup.cpp		\
            parametergroupsetup.cpp		\
            polineeditsetup.cpp			\
            projectlineeditsetup.cpp		\
            ralineeditsetup.cpp			\
            revisionlineeditsetup.cpp		\
            screensetup.cpp			\
            shipmentclusterlineeditsetup.cpp	\
            solineeditsetup.cpp			\
            tolineeditsetup.cpp			\
            usernamelineeditsetup.cpp		\
            vendorgroupsetup.cpp		\
            wcomboboxsetup.cpp			\
            wolineeditsetup.cpp			\
            womatlclustersetup.cpp		\
            xcomboboxsetup.cpp			\
            xdateeditsetup.cpp			\

QT +=  sql xml script network
