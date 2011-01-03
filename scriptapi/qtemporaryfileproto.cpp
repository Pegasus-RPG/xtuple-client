/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "qtemporaryfileproto.h"

#include <QTemporaryFile>

/** \ingroup scriptapi
    \class QTemporaryFile
    \brief This class exposes the QTemporaryFile class to Qt Scripting.

    The QTemporaryFile class exposes as much of the API to the
    QTemporaryFile class as possible.
*/

QScriptValue QTemporaryFiletoScriptValue(QScriptEngine *engine, QTemporaryFile* const &item)
{
  QScriptValue obj = engine->newQObject(item);

  return obj;
}

void QTemporaryFilefromScriptValue(const QScriptValue &obj, QTemporaryFile* &item)
{
  item = qobject_cast<QTemporaryFile*>(obj.toQObject());
}

Q_DECLARE_METATYPE(QFile*);
QScriptValue createLocalFile(QScriptContext *context,
                                   QScriptEngine  * engine)
{
  QTemporaryFile *obj = 0;

  if (context->argument(0).isString())
    obj = QTemporaryFile::createLocalFile(context->argument(0).toString());
  else if (qscriptvalue_cast<QFile*>(context->argument(0)))
    obj = QTemporaryFile::createLocalFile(*(qscriptvalue_cast<QFile*>(context->argument(0))));

  return engine->toScriptValue(obj);
}

void setupQTemporaryFileProto(QScriptEngine *engine)
{
 qScriptRegisterMetaType(engine, QTemporaryFiletoScriptValue, QTemporaryFilefromScriptValue);

  QScriptValue proto = engine->newQObject(new QTemporaryFileProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QTemporaryFile*>(), proto);

  QScriptValue constructor = engine->newFunction(constructQTemporaryFile, proto);
  engine->globalObject().setProperty("QTemporaryFile",  constructor);

  constructor.setProperty("createLocalFile", engine->newFunction(createLocalFile));
}

QScriptValue constructQTemporaryFile(QScriptContext *context, QScriptEngine  *engine)
{
  QTemporaryFile *obj = 0;

  if (context->argumentCount() == 0)
    obj = new QTemporaryFile();
  else if (context->argumentCount() == 1 &&
           context->argument(0).isString())
    obj = new QTemporaryFile(context->argument(0).toString());
  else if (context->argumentCount() == 1)
    obj = new QTemporaryFile(qscriptvalue_cast<QObject*>(context->argument(0)));
  else if (context->argumentCount() == 2)
    obj = new QTemporaryFile(context->argument(0).toString(),
                             qscriptvalue_cast<QObject*>(context->argument(1).toObject()));
  return engine->toScriptValue(obj);
}

QTemporaryFileProto::QTemporaryFileProto(QObject *parent)
  : QObject(parent)
{
}

bool QTemporaryFileProto::autoRemove() const
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->autoRemove();
  return false;
}

QString QTemporaryFileProto::fileName() const
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->fileName();
  return QString();
}

QString QTemporaryFileProto::fileTemplate() const
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->fileTemplate();
  return QString();
}

bool QTemporaryFileProto::open()
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->open();
  return false;
}

void QTemporaryFileProto::setAutoRemove( bool b)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    item->setAutoRemove(b);
}

void QTemporaryFileProto::setFileTemplate(const QString & name)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    item->setFileTemplate(name);
}

// QFile methods
bool QTemporaryFileProto::copy( const QString & newName)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->copy(newName);
  return false;
}

QFile::FileError QTemporaryFileProto::error() const
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->error();
  return QFile::NoError;
}

bool QTemporaryFileProto::exists() const
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->exists();
  return false;
}

bool QTemporaryFileProto::flush()
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->flush();
  return false;
}

int QTemporaryFileProto::handle() const
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->handle();
  return 0;
}

bool QTemporaryFileProto::link(const QString &linkName)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->link(linkName);
  return false;
}

/*
uchar* QTemporaryFileProto::map(qint64 offset, qint64 size, MemoryMapFlags flags)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->map(offset, size, flags);
  return 0;
}

bool QTemporaryFileProto::open(QIODevice::OpenMode mode)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->open(mode);
  return false;
}

bool QTemporaryFileProto::open(FILE *fh, QIODevice::OpenMode mode)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->open(fh, mode);
  return false;
}

bool QTemporaryFileProto::open(int fd, QIODevice::OpenMode mode)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->open(fd, mode);
  return false;
}
*/
QFile::Permissions QTemporaryFileProto::permissions() const
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->permissions();
  return 0;
}

bool QTemporaryFileProto::remove()
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->remove();
  return false;
}

bool QTemporaryFileProto::rename(const QString &newName)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->rename(newName);
  return false;
}

bool QTemporaryFileProto::resize(qint64 sz)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->resize(sz);
  return false;
}

void QTemporaryFileProto::setFileName(const QString &name)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    item->setFileName(name);
}

bool QTemporaryFileProto::setPermissions(QFile::Permissions permissions)
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->setPermissions(permissions);
  return false;
}

QString QTemporaryFileProto::symLinkTarget() const
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->symLinkTarget();
  return QString();
}

bool QTemporaryFileProto::unmap( uchar * address )
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    return item->unmap(address);
  return false;
}

void QTemporaryFileProto::unsetError()
{
  QTemporaryFile *item = qscriptvalue_cast<QTemporaryFile*>(thisObject());
  if (item)
    item->unsetError();
}
