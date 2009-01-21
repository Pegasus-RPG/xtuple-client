/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __CMCLUSTER_H__
#define __CMCLUSTER_H__

#include "widgets.h"
#include "xlineedit.h"

#include <QLabel>

#include <xsqlquery.h>

#include "virtualCluster.h"

class XTUPLEWIDGETS_EXPORT CmLineEdit : public VirtualClusterLineEdit
{
  Q_OBJECT

  public slots:
    void setCustId(int pItem);

  public:
    CmLineEdit(QWidget *, const char * = 0);

  private:
    int _custId;

};

class XTUPLEWIDGETS_EXPORT CmCluster : public VirtualCluster
{
  Q_OBJECT

  public slots:
    virtual void setCustId(int pCust);

  public:
    CmCluster(QWidget *, const char * = 0);
};

#endif
