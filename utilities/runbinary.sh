#!/bin/sh

# Get the path to this program. All the libraries and
# executables we need should be in the same location.
OPENMFG_DIR=`dirname $0`
OPENMFG_DIR=`(cd $OPENMFG_DIR; /bin/pwd)`

# Get the name of the executable we are supposed
# to be running.
OPENMFG_EXE=`basename $0 .sh`

# Check to see if LD_LIBRARY_PATH is already set with
# a value. If so then we will prepend our values.
if [ -z $LD_LIBRARY_PATH ]; then
  LD_LIBRARY_PATH="$OPENMFG_DIR"
else
  LD_LIBRARY_PATH="$OPENMFG_DIR":$LD_LIBRARY_PATH
fi
export LD_LIBRARY_PATH

# Execute the real binary we want now that our
# environment has been setup correctly.
"$OPENMFG_DIR/$OPENMFG_EXE.bin" $@
