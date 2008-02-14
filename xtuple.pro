TEMPLATE = subdirs
SUBDIRS = common \
          widgets/dll.pro \
          widgets \
          guiclient
CONFIG += ordered

exists(../../../openrpt) {
    OPENRPT_DIR = ../../../openrpt
}
exists(../../openrpt) {
    OPENRPT_DIR = ../../openrpt
}
exists(../openrpt) {
    OPENRPT_DIR = ../openrpt
}

! exists($${OPENRPT_DIR}) {
    error("Could not set the OPENRPT_DIR qmake variable.")
}
