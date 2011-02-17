/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QCoreApplication>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QDebug>

#include "guiclient.h"
#include "xtHelp.h"

#ifdef Q_OS_MAC
#define QHC_PATH "/../Resources/XTupleGUIClient.qhc"
#else
#define QHC_PATH "/XTupleGUIClient.qhc"
#endif

#define DEBUG FALSE

static xtHelp *xtHelpSingleton = 0;

xtHelp* xtHelp::getInstance(QWidget *parent)
{
  if(!xtHelpSingleton)
    xtHelpSingleton = new xtHelp(parent);
  return xtHelpSingleton;
}

xtHelp::xtHelp(QWidget *parent)
  : QHelpEngine(QCoreApplication::instance()->applicationDirPath() + QString(QHC_PATH), parent),
  //: QHelpEngine(loc, parent),
  _online(false),
  _nam(new QNetworkAccessManager),
  _rep(0)
{
  //connect the QNetworkAccessManager to the error slot
  connect(_nam,         SIGNAL(finished(QNetworkReply *)),              this,           SLOT(sError(QNetworkReply *)));

  //QHelpEngine will create the file if it is not there
  //so we test to see if the contents is valid
  if(!setupData())
    warning("Error setting up the help data");
  _online = fileData(QUrl("qthelp://xtuple.org/postbooks/index.html")) == QByteArray("");
}

xtHelp::~xtHelp()
{
} 

bool xtHelp::isOnline()
{
  return _online;
}

QByteArray xtHelp::urlData(const QUrl &url)
{
  if (DEBUG) qDebug() << "urlData: request url [" << url.toString() << "]";
  _req.setUrl(url);
  _rep = _nam->get(_req);
  while(!_rep->isFinished())
    QCoreApplication::instance()->processEvents();
  return _rep->readAll();
}

void xtHelp::sError(QNetworkReply *rep)
{
  QVariant statCode = rep->attribute(QNetworkRequest::HttpStatusCodeAttribute);
  QVariant url = rep->url();
  if (DEBUG) qDebug() << "xtHelp Network Response Information-----------------------------";
  if (DEBUG) qDebug() << "xtHelp: http request url [" << url.toString() << "]";
  if (DEBUG) qDebug() << "xtHelp: status code received [" << statCode.toString() << "]";
  if(rep->error() == QNetworkReply::NoError)
    if (DEBUG) qDebug() << "xtHelp: no error";
  else
  {
    if (DEBUG) qDebug() << "xtHelp: error received";
    if (DEBUG) qDebug() << "xtHelp: " << rep->errorString();
  }
  if (DEBUG) qDebug() << "----------------------------------------------------------------";
}
