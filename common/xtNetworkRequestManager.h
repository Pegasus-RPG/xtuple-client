/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2015 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XTNETWORKREQUESTMANAGER_H__
#define __XTNETWORKREQUESTMANAGER_H__

#include <QObject>
#include <QString>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

class xtNetworkRequestManager : public QObject
{
    Q_OBJECT

public:
    xtNetworkRequestManager(const QUrl &, const QMutex &);
    virtual ~xtNetworkRequestManager();


protected slots:
    virtual void requestCompleted();
    virtual void toggleLock(QMutex & mutex);

private:
    QNetworkAccessManager * nwam;
    QNetworkReply * _nwrep;
};

#endif
