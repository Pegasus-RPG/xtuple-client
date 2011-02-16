/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QWidget>
#include <QVariant>

#include <QDebug>

#include "helpViewBrowser.h"
#include "xtHelp.h"

#define DEBUG TRUE

helpViewBrowser::helpViewBrowser(QWidget *parent)
  : QTextBrowser(parent)
{
  setSource(QUrl("http://www.xtuple.org/sites/default/files/refguide/RefGuide-3.6/index.html"));
}

QVariant helpViewBrowser::loadResource(int type, const QUrl &name)
{
  if (DEBUG) qDebug() << "loadResource: type [" << type << "] name [" << name.toString() << "]";
  QByteArray data;
  if(type < 4)
  {
    QUrl url(name);
    if(name.isRelative())
    {
      url = source().resolved(url);
      if (DEBUG) qDebug() << "url was relative [" << url.toString() << "]";
    }
    if(xtHelp::getInstance()->isOnline())
    {
      if (DEBUG) qDebug() << "url for online request [" << url.toString() << "]";
      data = xtHelp::getInstance()->urlData(url);
      if (DEBUG) qDebug() << "data returned from online source with size [" << data.size() << "] for url [" << url.toString() << "]";
    }
    else
    {
      data = xtHelp::getInstance()->fileData(url);
      if (DEBUG) qDebug() << "data returned from local source with size [" << data.size() << "]";
    }
  }
  return data;
}

helpViewBrowser::~helpViewBrowser()
{
}
