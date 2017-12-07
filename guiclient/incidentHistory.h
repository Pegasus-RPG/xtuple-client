/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef INCIDENTHISTORY_H
#define INCIDENTHISTORY_H

#include "xdialog.h"
#include "ui_incidentHistory.h"

class incidentHistory : public XDialog, public Ui::incidentHistory
{
  Q_OBJECT

  public:
    incidentHistory(QWidget* = 0, const char* = 0, bool = false, Qt::WindowFlags = 0);

  public slots:
    virtual enum SetResponse set(const ParameterList&);
};

#endif 
