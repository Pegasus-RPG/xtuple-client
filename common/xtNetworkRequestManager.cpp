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

#define DEBUG true

xtNetworkRequestManager::xtNetworkRequestManager(const QUrl & url, const QMutex &mutex) {
  //if(mutex){
  //toggleLock(mutex);
  //}
  nwam = new QNetworkAccessManager(this);
  _nwrep = 0;
  if(DEBUG){
  qDebug() << "url= " << url.toEncoded();
  }
  _nwrep = nwam->get(QNetworkRequest(url));
  connect(_nwrep, SIGNAL(finished()), this, SLOT(requestCompleted()));

}
void xtNetworkRequestManager::requestCompleted() {

  QVariant possibleRedirect = _nwrep->attribute(QNetworkRequest::RedirectionTargetAttribute);

  if(_nwrep->error()){
      if(DEBUG){
      qDebug() << "error=" << _nwrep->error();
      }
  }
  else if(!possibleRedirect.isNull()){
      QUrl newUrl = possibleRedirect.toUrl();
      _nwrep->deleteLater();
      _nwrep = nwam->get(QNetworkRequest(newUrl));
      connect(_nwrep, SIGNAL(finished()), this, SLOT(requestCompleted(QNetworkReply*)));
  }
}
void xtNetworkRequestManager::toggleLock(QMutex & mutex) {
    //QMutexLocker locker(mutex);
    //lock will be deleted on return
    return;
}
xtNetworkRequestManager::~xtNetworkRequestManager() {
 delete nwam;
    nwam = 0;
}

