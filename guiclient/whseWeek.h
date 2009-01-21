/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef WHSEWEEK_H
#define WHSEWEEK_H

#include "guiclient.h"
#include "xdialog.h"

#include "ui_whseWeek.h"

class whseWeek : public XDialog, public Ui::whseWeek
{
    Q_OBJECT

  public:
    whseWeek(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~whseWeek();

  public slots:
    virtual void sPopulate();
    virtual void sSave();
    virtual void sClose();

  protected slots:
    virtual void languageChange();
    virtual void sChange();

  protected:
    bool save(int);
    bool checkAndSave();

  private:
    int _warehousid;
    bool _dirty;

};

#endif // WHSEWEEK_H
