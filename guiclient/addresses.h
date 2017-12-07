/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef ADDRESSES_H
#define ADDRESSES_H

#include "display.h"
#include <parameter.h>

class addresses : public display
{
    Q_OBJECT

public:
    addresses(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::Window);

public slots:
    virtual void sPopulateMenu(QMenu *, QTreeWidgetItem* = NULL, int = 0);
    virtual void sNew();
    virtual void sEdit();
    virtual void sView();
    virtual void sDelete();
    virtual void sNewProspect();
    virtual void sEditProspect();
    virtual void sViewProspect();
    virtual void sOpenProspect(QString);

};

#endif // ADDRESSES_H
