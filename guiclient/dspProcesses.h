/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DSPPROCESSES_H
#define DSPPROCESSES_H

#include "display.h"

class dspProcesses : public display
{
  Q_OBJECT

  public:
    dspProcesses(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

  public slots:
    virtual bool setParams(ParameterList&);
    virtual void sPopulateMenu(QMenu*, QTreeWidgetItem*, int);
    void sKill();
    void sRelease();

  private:
    bool _mobilized;
};

#endif
