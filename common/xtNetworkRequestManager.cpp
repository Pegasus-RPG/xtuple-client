/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xtNetworkRequestManager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QDebug>
#include <QtWidgets>

#define DEBUG true

xtNetworkRequestManager::xtNetworkRequestManager(const QUrl & url, const QMutex &mutex) {
  nwam = new QNetworkAccessManager(this);
  _nwrep = 0;
  //toggleLock(mutex);
  if(DEBUG){
  qDebug() << "url= " << url.toEncoded();
  }
  QEventLoop loop;
  _nwrep = nwam->get(QNetworkRequest(url));
  connect(_nwrep, SIGNAL(finished()), SLOT(requestCompleted()));
  connect(_nwrep, SIGNAL(readyRead()), SLOT(readyRead()));
  //connect(_nwrep, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(replyError(QNetworkReply::NetworkError)));
  connect(nwam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
  connect(nwam, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit())); //loop through and quit on finished signal, is this correct?
  loop.exec();
}
void xtNetworkRequestManager::requestCompleted() {
  QByteArray response = _nwrep->readAll(); //we don't really care here but store it anyways
  _nwrep->close();
  QVariant possibleRedirect = _nwrep->attribute(QNetworkRequest::RedirectionTargetAttribute);
  if(DEBUG){
      qDebug() << "redirect=" << possibleRedirect;
      qDebug() << "replyError=" << _nwrep->errorString();
      qDebug() << "replyErrorCode=" << _nwrep->error();
  }
  if(_nwrep->error() != QNetworkReply::NoError){
      if(DEBUG){
      qDebug() << "error=" << _nwrep->errorString();
      }
  }
  else if(!possibleRedirect.isNull()){
      QUrl newUrl = possibleRedirect.toUrl();
      _nwrep->deleteLater();
      _nwrep = nwam->get(QNetworkRequest(newUrl));
      connect(nwam, SIGNAL(finished()), this, SLOT(requestCompleted()));
  }
  _nwrep->deleteLater(); //clean up
}
void xtNetworkRequestManager::toggleLock(QMutex & mutex) {
    if(mutex.tryLock()){
        mutex.unlock();
    }
    else {
        mutex.lock();
    }
    return;
}
void xtNetworkRequestManager::sslErrors(QNetworkReply*, const QList<QSslError> &errors) {
    QString errorString;
       foreach (const QSslError &error, errors) {
           if (!errorString.isEmpty())
               errorString += ", ";
           errorString += error.errorString();
       }

   qDebug() << "errorString= " << errorString;
}
void xtNetworkRequestManager::readyRead() {
    qDebug() << "readyReadCalled";
}
xtNetworkRequestManager::~xtNetworkRequestManager() {
 delete nwam;
    nwam = 0;
}

