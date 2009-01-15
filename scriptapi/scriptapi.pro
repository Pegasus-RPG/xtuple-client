include( ../global.pri )

TARGET      = xtuplescriptapi

TEMPLATE    =  lib
CONFIG      += qt warn_on staticlib
DBFILE      =  scriptapi.db
LANGUAGE    =  C++

DEPENDPATH  += . ../common ../widgets
INCLUDEPATH += . ../common ../widgets .

DESTDIR = ../lib

MOC_DIR     = tmp
OBJECTS_DIR = tmp
UI_DIR      = tmp

HEADERS +=  setupscriptapi.h                    \
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
            qnetworkreplyproto.h                \
            qnetworkrequestproto.h              \
            qsqldatabaseproto.h                 \
            qurlproto.h                         \
            xdatawidgetmapperproto.h            \
            xnetworkaccessmanager.h             \
            xsqltablemodelproto.h               \
            xtreewidgetitemproto.h              \

SOURCES +=  setupscriptapi.cpp                  \
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
            qnetworkreplyproto.cpp              \
            qnetworkrequestproto.cpp            \
            qsqldatabaseproto.cpp               \
            qurlproto.cpp                       \
            xdatawidgetmapperproto.cpp          \
            xnetworkaccessmanager.cpp           \
            xsqltablemodelproto.cpp             \
            xtreewidgetitemproto.cpp            \

QT +=  sql xml script network
