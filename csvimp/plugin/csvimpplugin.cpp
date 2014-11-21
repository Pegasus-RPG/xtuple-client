/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QtWidgets>
#include "csvimpplugin.h"

#include <QMainWindow>

#include "batchmessagehandler.h"
#include "csvatlaswindow.h"
#include "csvtoolwindow.h"
#include "interactivemessagehandler.h"

#define DEBUG false

CSVImpPlugin::CSVImpPlugin(QObject *parent)
  : QObject(parent)
{
  _atlasdir      = QString::null;
  _atlaswindow   = 0;
  _csvdir        = QString::null;
  _csvtoolwindow = 0;
  _msghandler    = 0;
  
  Q_INTERFACES(CSVImpPluginInterface);

}

QMainWindow *CSVImpPlugin::getCSVAtlasWindow(QWidget *parent, Qt::WindowFlags flags)
{
  if (! _atlaswindow)
  {
    CSVToolWindow *csvtool = qobject_cast<CSVToolWindow*>(getCSVToolWindow(parent, flags));
    if (csvtool)
    {
      _atlaswindow = csvtool->atlasWindow();
      if (_msghandler)
        _atlaswindow->setMessageHandler(_msghandler);
      connect(_atlaswindow, SIGNAL(destroyed(QObject*)), this, SLOT(cleanupDestroyedObject(QObject*)));
      if (_atlasdir.isEmpty())
      _atlaswindow->setDir(_csvdir);
    else
      _atlaswindow->setDir(_atlasdir);
    }
  }

  if (DEBUG)
    qDebug("CSVImpPlugin::getAtlasToolWindow() returning %p, Atlas dir [%s]",
           _atlaswindow, qPrintable(_atlasdir));
  return _atlaswindow;
}

QMainWindow *CSVImpPlugin::getCSVToolWindow(QWidget *parent, Qt::WindowFlags flags)
{
  if (! _csvtoolwindow)
  {
    _csvtoolwindow = new CSVToolWindow(parent, flags);
    connect(_csvtoolwindow, SIGNAL(destroyed(QObject*)), this, SLOT(cleanupDestroyedObject(QObject*)));

    _csvtoolwindow->sFirstRowHeader(_firstLineIsHeader);
    _csvtoolwindow->setDir(_csvdir);
    if (_atlasdir.isEmpty())
      _csvtoolwindow->atlasWindow()->setDir(_csvdir);
    else
      _csvtoolwindow->atlasWindow()->setDir(_atlasdir);

    if (_msghandler)
      _csvtoolwindow->setMessageHandler(_msghandler);
  }

  if (DEBUG)
    qDebug("CSVImpPlugin::getCSVToolWindow() returning %p "
           "with CSV dir %s, Atlas dir %s",
           _csvtoolwindow, qPrintable(_csvdir), qPrintable(_atlasdir));
  return _csvtoolwindow;
}

bool CSVImpPlugin::importCSV()
{
  _csvtoolwindow->clearImportLog();
  return _csvtoolwindow->importStart();
}

bool CSVImpPlugin::isInteractive()
{
  if (_msghandler && qobject_cast<BatchMessageHandler*>(_msghandler))
    return false;

  return true;  // assume true since that's the tool window's default
}

QString CSVImpPlugin::lastError()
{
  QString msg = QString::null;

  if (_msghandler)
  {
    QtMsgType type = QtCriticalMsg;
    QStringList msgs = _msghandler->unhandledMessages(&type);
    if (! msgs.isEmpty())
      msg = msgs.last();
  }

  return msg;
}

bool CSVImpPlugin::openAtlas(QString filename)
{
  if (DEBUG) qDebug("CSVImpPlugin::openAtlas(%s)", qPrintable(filename));

  CSVAtlasWindow *atlaswind = qobject_cast<CSVAtlasWindow*>(getCSVAtlasWindow(qobject_cast<QWidget*>(parent())));
  if (atlaswind)
  {
    atlaswind->fileOpen(filename);
    if (DEBUG)
      qDebug("CSVImpPlugin::openAtlas() opened [%s]", qPrintable(filename));
    return true;
  }

  return false;
}

bool CSVImpPlugin::openCSV(QString filename)
{
  CSVToolWindow *csvtool = qobject_cast<CSVToolWindow*>(getCSVToolWindow(qobject_cast<QWidget*>(parent())));
  if (csvtool)
  {
    csvtool->fileOpen(filename);
    return true;
  }

  return false;
}

void CSVImpPlugin::setAtlasDir(QString dirname)
{
  if (DEBUG) qDebug("CSVImpPlugin::setAltasDir(%s)", qPrintable(dirname));

  _atlasdir = dirname;
  if (_csvtoolwindow)
    _csvtoolwindow->atlasWindow()->setDir(dirname);
}

bool CSVImpPlugin::setAtlasMap(const QString mapname)
{
  if (_csvtoolwindow && _csvtoolwindow->atlasWindow())
    return _csvtoolwindow->atlasWindow()->setMap(mapname);

  return false;
}

void CSVImpPlugin::setCSVDir(QString dirname)
{
  if (DEBUG) qDebug("CSVImpPlugin::setCSVDir(%s)", qPrintable(dirname));
  _csvdir = dirname;
  if (_csvtoolwindow)
    _csvtoolwindow->setDir(dirname);
}

bool CSVImpPlugin::setFirstLineHeader(bool isheader)
{
  if (DEBUG) qDebug("CSVImpPlugin::setFirstLineHeader(%d)", isheader);
  _firstLineIsHeader = isheader;
  if (_csvtoolwindow)
    _csvtoolwindow->sFirstRowHeader(isheader);

  return true;
}

void CSVImpPlugin::setInteractive(bool interactive)
{
  if (isInteractive() != interactive)
  {
    if (_msghandler)
      delete _msghandler;

    if (interactive)
      _msghandler = new InteractiveMessageHandler(parent());
    else
      _msghandler = new BatchMessageHandler(parent());
  }

  if (_msghandler)
  {
    if (_csvtoolwindow)
      _csvtoolwindow->setMessageHandler(_msghandler);
    if (_atlaswindow)
      _atlaswindow->setMessageHandler(_msghandler);
  }
}

void CSVImpPlugin::cleanupDestroyedObject(QObject *object)
{
  if (DEBUG)
    qDebug("CSVImpPlugin::cleanupDestroyedObject(%s %p)",
           object ? object->metaObject()->className() : "[unknown]", object);
  if (object == _csvtoolwindow)
  {
    _csvtoolwindow = 0;
    _firstLineIsHeader = false;
  }
  else if (object == _atlaswindow)
    _atlaswindow = 0;
  else if (object == _msghandler)
    _msghandler = 0;
}

Q_PLUGIN_METADATA(IID "org.xtuple.Plugin.CSVImpPluginInterface/0.4");
