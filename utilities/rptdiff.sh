PROG=`basename $0`
OLDDIR=
NEWDIR=
TMPDIR="${TMPDIR:-/tmp}/${PROG}_`date +%Y_%m_%d`"
RUNEXTRACT=true	#undocumented feature

usage () {
  echo "$PROG -h"
  echo "$PROG [ -o oldversiondir -n newversiondir ] [ -t temporarydir ]"
  echo
  echo "-h	print this usage message"
  echo "-o	directory to search for the old versions of the reports"
  echo "-n	directory to search for the new versions of the reports"
  echo "-t	directory in which to store extracted queries [$TMPDIR]"
}

# x is an undocumented option
set -- `getopt ho:n:t:x $*`

while [ $1 != -- ] ; do
  case "$1" in
    -h)
      usage
      exit 0
      ;;
    -o)
      OLDDIR=$2
      shift
      ;;
    -n)
      NEWDIR=$2
      shift
      ;;
    -t)
      TMPDIR=$2
      shift
      ;;
    -x)
      RUNEXTRACT=false	#undocumented feature
      ;;
  esac
  shift
done
shift #past the --

if [ -z "$OLDDIR" ] ; then
  echo "$PROG: no oldversiondir given"
  usage
  exit 1
elif [ ! -d "$OLDDIR" ] ; then
  echo "$PROG: oldversiondir is not a directory"
  usage
  exit 1
elif [ `ls $OLDDIR/*.xml | wc -l` -le 0 ] ; then
  echo "$PROG: $OLDDIR doesn't seem to have any xml files in it"
  exit 1
fi

if [ -z "$NEWDIR" ] ; then
  echo "$PROG: no newversiondir given"
  usage
  exit 1
elif [ ! -d "$NEWDIR" ] ; then
  echo "$PROG: newversiondir is not a directory"
  usage
  exit 1
elif [ `ls $NEWDIR/*.xml | wc -l` -le 0 ] ; then
  echo "$PROG: $NEWDIR doesn't seem to have any xml files in it"
  exit 1
fi

if [ -z "$TMPDIR" ] ; then
  echo "$PROG: no temporarydir given"
  usage
  exit 1
elif [ -e "$TMPDIR" -a ! -d "$TMPDIR" ] ; then
  echo "$PROG: temporarydir is not a directory"
  usage
  exit 1
elif ! mkdir -p $TMPDIR/old || ! mkdir -p $TMPDIR/new ; then
  exit 2
fi

if $RUNEXTRACT ; then
for DIR in $OLDDIR $NEWDIR ; do
  if [ "$DIR" = "$OLDDIR" ] ; then
    DESTDIR="$TMPDIR/old"
  elif [ "$DIR" = "$NEWDIR" ] ; then
    DESTDIR="$TMPDIR/new"
  else
    echo "$PROG: I'm confused and don't know where to write my output"
    exit 3
  fi
  if ! rm -f "$DESTDIR/*" ; then
    exit 4
  fi

  FILELIST=`ls $DIR/*.xml`
  if [ -z "$FILELIST" ] ; then
    echo no .xml files in $DIR
    exit 2
  fi

  for FILE in $FILELIST ; do
    INPUTFILE=$FILE
    OUTPUTFILE=$DESTDIR/`basename $INPUTFILE`
    echo processing $INPUTFILE > /dev/stderr

    while ! Xalan "${INPUTFILE}" - > "${OUTPUTFILE}" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" >
  <xsl:output indent="yes" method="text" />

  <xsl:template match="text()"/>

  <xsl:template match="report/name">
====================================================================
REPORT: <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="report/querysource/name">
QUERY: <xsl:value-of select="."/><xsl:text>
</xsl:text>
  </xsl:template>

  <xsl:template match="report/querysource/sql">
<xsl:value-of select="."/>
--------------------------------------------------------------------
  </xsl:template>

</xsl:stylesheet>
EOF
    do
      sleep 5
      echo retrying $INPUTFILE
    done

  done
done
fi	# if $RUNEXTRACT

RMLIST=
NEWLIST=
CHANGEDLIST=

if [ -s $TMPDIR/diffs ] && ! rm $TMPDIR/diffs ; then
  exit 3
fi

for FILE in `ls $TMPDIR/old/* $TMPDIR/new/* | xargs -J X -n 1 basename X | sort -u` ; do
  if  [ -f $TMPDIR/old/$FILE -a ! -f $TMPDIR/new/$FILE ] ; then
    REPORTNAME="`head -3 $TMPDIR/old/$FILE | tail -1 | cut -f2 -d:`"
    RMLIST="$RMLIST $REPORTNAME"
  elif [ ! -f $TMPDIR/old/$FILE -a -f $TMPDIR/new/$FILE ] ; then
    REPORTNAME="`head -3 $TMPDIR/new/$FILE | tail -1 | cut -f2 -d:`"
    NEWLIST="$NEWLIST $REPORTNAME"
  else
    cmp -s $TMPDIR/old/$FILE $TMPDIR/new/$FILE
    EXITCODE=$?
    if [ $EXITCODE -eq 1 ]; then
      REPORTNAME="`head -3 $TMPDIR/new/$FILE | tail -1 | cut -f2 -d:`"
      CHANGEDLIST="$CHANGEDLIST $REPORTNAME"
      diff -E -b -i -U `wc -l $TMPDIR/new/$FILE | 
                        awk '{print $1}'` $TMPDIR/old/$FILE $TMPDIR/new/$FILE | tail +4 >> $TMPDIR/diffs
    fi
  fi
done

if [ -n "$RMLIST" ] ; then
  echo ====================================================================
  echo REMOVED REPORTS:
  echo $RMLIST | tr -s [:blank:] "\n"
fi

if [ -n "$NEWLIST" ] ; then
  echo ====================================================================
  echo NEW REPORTS:
  echo $NEWLIST | tr -s [:blank:] "\n"
fi

if [ -n "$CHANGEDLIST" ] ; then
  echo ====================================================================
  echo CHANGED REPORTS:
  echo $CHANGEDLIST | tr -s [:blank:] "\n"
  echo
  cat $TMPDIR/diffs
fi
