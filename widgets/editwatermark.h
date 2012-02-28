/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef EDITWATERMARK_H
#define EDITWATERMARK_H

#include <QDialog>

#include <parameter.h>

#include "ui_editwatermark.h"

class EditWatermark : public QDialog, public Ui::EditWatermark
{
    Q_OBJECT

  public:
    EditWatermark(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~EditWatermark();

    virtual bool    showPrices();
    virtual QString watermark();

  public slots:
    virtual bool set(const ParameterList &pParams);

  protected slots:
    virtual void languageChange();

};

#endif // EDITWATERMARK_H
