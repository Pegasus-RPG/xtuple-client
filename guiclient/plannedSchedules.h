/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __PLANNEDSCHEDULES_H__
#define __PLANNEDSCHEDULES_H__

#include "xwidget.h"

#include "guiclient.h"
#include <parameter.h>

class QMenu;

#include "ui_plannedSchedules.h"

class plannedSchedules : public XWidget, public Ui::plannedSchedules
{
  Q_OBJECT

  public:
    plannedSchedules(QWidget * = 0, const char * = 0, Qt::WFlags = Qt::Window);
    ~plannedSchedules();

  public slots:
    virtual void sNew();
    virtual void sEdit();
    virtual void sCopy();
    virtual void sPrint();
    virtual void sFillList();
    virtual void sPopulateMenu(QMenu*);
    virtual void sRelease();

  protected slots:
    virtual void languageChange();
};

#endif // __PLANNEDSCHEDULES_H__
