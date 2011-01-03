/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __QTEMPORARYFILE_H__
#define __QTEMPORARYFILE_H__

#include <QtScript>
#include <QFile>
#include <QIODevice>
#include <QTemporaryFile>

Q_DECLARE_METATYPE(QTemporaryFile*)

void setupQTemporaryFileProto(QScriptEngine *engine);
QScriptValue constructQTemporaryFile(QScriptContext *context, QScriptEngine *engine);

class QTemporaryFileProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QTemporaryFileProto(QObject * parent = 0);

    // these are handled as properties of the global object
    //  static QTemporaryFile *	createLocalFile ( QFile & file );
    //  static QTemporaryFile *	createLocalFile ( const QString & fileName  );

    Q_INVOKABLE bool  autoRemove() const;
    Q_INVOKABLE QString fileName() const;
    Q_INVOKABLE QString fileTemplate() const;
    Q_INVOKABLE bool open();
    Q_INVOKABLE void setAutoRemove( bool b );
    Q_INVOKABLE void setFileTemplate( const QString & name);

    // QFile Inherited
    Q_INVOKABLE bool    copy ( const QString & newName );
    Q_INVOKABLE QFile::FileError error () const;
    Q_INVOKABLE bool	exists () const;
    Q_INVOKABLE bool	flush ();
    Q_INVOKABLE int	handle () const;
    Q_INVOKABLE bool	link ( const QString & linkName );
    //Q_INVOKABLE uchar *	map ( qint64 offset, qint64 size, MemoryMapFlags flags = NoOptions );
    //Q_INVOKABLE bool open(QIODevice::OpenMode mode);
    //Q_INVOKABLE bool	open ( FILE * fh, QIODevice::OpenMode mode );
    //Q_INVOKABLE bool	open ( int fd, QIODevice::OpenMode mode );
    Q_INVOKABLE QFile::Permissions permissions () const;
    Q_INVOKABLE bool	remove ();
    Q_INVOKABLE bool	rename ( const QString & newName );
    Q_INVOKABLE bool	resize ( qint64 sz );
    Q_INVOKABLE void	setFileName ( const QString & name );
    Q_INVOKABLE bool	setPermissions ( QFile::Permissions permissions );
    Q_INVOKABLE QString	symLinkTarget () const;
    Q_INVOKABLE bool	unmap ( uchar * address );
    Q_INVOKABLE void	unsetError ();
};

#endif
