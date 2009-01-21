/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __PLANNEDSCHEDULE_H__
#define __PLANNEDSCHEDULE_H__

#include "guiclient.h"
#include <parameter.h>

#include "xdialog.h"

#include "ui_plannedSchedule.h"

class plannedSchedule : public XDialog, public Ui::plannedSchedule
{
  Q_OBJECT

  public:
    plannedSchedule(QWidget * = 0, const char * = 0, bool = false, Qt::WFlags = 0);
    ~plannedSchedule();

  public slots:
    virtual enum SetResponse set(const ParameterList &);
    virtual void sSave();
    virtual void reject();
    virtual void sNew();
    virtual void sEdit();
    virtual void sCopy();
    virtual void sDelete();
    virtual void populate();
    virtual void sFillList();
    virtual void sNumberChanged();

  protected slots:
    virtual void languageChange();

  private:
    int _mode;
    int _pschheadid;
    bool _presaved;
};

#endif // __PLANNEDSCHEDULE_H__
