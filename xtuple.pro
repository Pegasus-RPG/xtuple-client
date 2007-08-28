TEMPLATE = subdirs
SUBDIRS = common \
          widgets/dll.pro \
          widgets \
          guiclient

! exists(../openrpt) {
  error("The ../openrpt directory does not exist. Please get the sources for OpenRPT and build those before building OpenMFG.")
}
