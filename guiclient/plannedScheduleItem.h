/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __PLANNEDSCHEDULEITEM_H__
#define __PLANNEDSCHEDULEITEM_H__

#include "guiclient.h"
#include <parameter.h>

#include "xdialog.h"

#include "ui_plannedScheduleItem.h"

class plannedScheduleItem : public XDialog, public Ui::plannedScheduleItem
{
  Q_OBJECT

  public:
    plannedScheduleItem(QWidget * = 0, const char * = 0, bool = false, Qt::WFlags = 0);
    ~plannedScheduleItem();

  public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual void sSave();
    virtual void populate();

  protected slots:
    virtual void languageChange();

  private:
    int _mode;
    int _pschheadid;
    int _pschitemid;
    int _warehousid;
};

#endif // __PLANNEDSCHEDULEITEM_H__
