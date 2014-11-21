#
# This file is part of the xTuple ERP: PostBooks Edition, a free and
# open source Enterprise Resource Planning software suite,
# Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
# It is licensed to you under the Common Public Attribution License
# version 1.0, the full text of which (including xTuple-specific Exhibits)
# is available at www.xtuple.com/CPAL.  By using this software, you agree
# to be bound by its terms.
#
PROG=`basename $0`
VERFILE=csvimpcommon/csvimpdata.cpp
APP=csvimp.app
BUILD=false
DEMO=false
BASEDIR=`pwd`
if [ "$BASEDIR" = . ] ; then
  BASEDIR=`pwd`
fi

OS=`uname -s`
if [ "$OS" = Darwin ] ; then
  ARCH=`uname -p`
  OSVER=MAC${ARCH}
fi

usage() {
  echo "$PROG -h"
  echo "$PROG -b -d [ -x ]"
  echo
  fmt <<EOF
$PROG bundles the $APP application for release on the current platform
(currently only Mac OS X is supported).
$PROG reads $VERFILE to get release information used to
name the release package.
EOF
  echo
  echo "-h	get this usage message"
  echo "-b	force build (run qmake and make before bundling)"
  echo "	$PROG runs qmake and make if the required binaries are missing "
  echo "	from the bin directory even if -b is not specified."
  echo "-d <file>	build and bundle a demo release, including the help file named <file>"
  echo "-x	turn on shell debugging output"
}

bundle() {
  if [ "$#" -ne 2 ] ; then
    echo "Illegal call to bundle: bad arg count"
    echo "usage: bundle app dir_to_put_it_in"
    return 1
  fi

  STARTDIR=`pwd`

  APPNAME="$1"
  if expr "$APPNAME" : ".*\.app" ; then
    BINARY="$APPNAME"
    APPNAME=`basename "$APPNAME" .app`
  else
    BINARY="${APPNAME}.app"
  fi

  if $DEMO ; then
    BUNDLENAME="${APPNAME}-${VERSION}Demo-${OSVER}"
  else
    BUNDLENAME="${APPNAME}-${VERSION}-${OSVER}"
  fi
  BUNDLEDIR="${2}/${BUNDLENAME}"

  if [ -d "$BUNDLEDIR" ] ; then
    if ! rm -rf "$BUNDLEDIR" ; then
      return 3
    fi
  fi

  if ! mkdir "$BUNDLEDIR" ; then
    return 3
  fi

  if ! $DEMO ; then
    if ! cp -R "$BASEDIR/$BINARY" "$BUNDLEDIR" ; then
      return 4
    fi
  else
    if ! cp -R "$BASEDIR/$BINARY" "$BUNDLEDIR"/"${APPNAME}_Demo.app" ; then
      return 4
    fi
    BINARY="${APPNAME}_Demo.app"
    if [ -d "$HELPFILE" ] ; then
      if ! cp -R -L "$HELPFILE" \
	      "$BUNDLEDIR"/"${BINARY}"/Contents/Resources/helpOpenMFGGUIClient ; then
	return 5
      fi
    elif [ -f "$HELPFILE" ] ; then
      CURRDIR=`pwd`
      if ! cd "$BUNDLEDIR"/"${BINARY}"/Contents/Resources ; then
	return 5
      fi
      if ! jar xf "$HELPFILE" ; then
	return 5
      fi
      if [ ! -d helpOpenMFGGUIClient ] ; then
	echo "$PROG: help file $HELPFILE was not installed properly in demo client"
	return 5
      fi
      cd "$CURRDIR"
    fi
  fi

  cd "$BUNDLEDIR"/..
  if [ -f "$BUNDLENAME".dmg ] && ! rm "$BUNDLENAME".dmg ; then
    return 5
  fi
  if ! hdiutil create -fs HFS+ -volname "$BUNDLENAME" -srcfolder "$BUNDLENAME" "$BUNDLENAME".dmg ; then
    return 5
  fi

  cd $STARTDIR
}

ARGS=`getopt hbd:x $*`
if [ $? != 0 ] ; then
  usage
  exit 1
fi
set -- $ARGS

while [ "$1" != -- ] ; do
  case "$1" in
    -h)
	usage
	exit 0
	;;
    -b)
	BUILD=true
	;;
    -d)
	DEMO=true
	HELPFILE=$2
	if ! expr ${HELPFILE} : "\/" ; then
	  HELPFILE=`pwd`/"$HELPFILE"
	fi
	shift
	;;
    -x)
	set -x
	;;
    *)
	usage
	exit 1
	;;
  esac
  shift
done
shift # past the --

if [ "$#" -gt 0 ] ; then
  echo "$PROG: ignoring extra arguments $*"
fi

if [ `uname -s` != Darwin ] ; then
  echo "$PROG: only supports Macintosh OS X (Darwin) at this time"
  usage
  exit 2
fi

if [ -z "$QTDIR" ] ; then
  echo "$PROG: Cannot run properly without the QTDIR environment variable set"
  exit 2
fi
if [ -z "$PGDIR" ] ; then
  echo "$PROG: Cannot run properly without the PGDIR environment variable set"
  exit 2
fi

if [ ! -e "$BASEDIR"/${APP} ] ; then
  if ! $BUILD ; then
    echo "Building even though not explicitly told to do so"
  fi
  BUILD=true
fi

if [ ! -f "$VERFILE" ] ; then
  echo "Could not find $VERFILE"
  exit 2
fi
VERSION=`awk '/CSVImp::version/ { split($0,vers,"\""); print vers[2]}' $VERFILE |\
           tr -d [:blank:]`

if $BUILD ; then
  cd "$BASEDIR"
  qmake
  make
fi

./fixPackage -pD csvimp.app

if [ ! -e $APP ] ; then
  echo Cannot find the application!
  exit 3
fi

if ! bundle $APP "${BASEDIR}/.." ; then
  exit $?
fi

echo "DONE!"
