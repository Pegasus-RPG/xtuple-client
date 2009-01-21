/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef BOOITEMIMAGE_H
#define BOOITEMIMAGE_H

#include "guiclient.h"
#include "xdialog.h"
#include <parameter.h>

#include "ui_booitemImage.h"

class booitemImage : public XDialog, public Ui::booitemImage
{
    Q_OBJECT

public:
    booitemImage(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~booitemImage();

    virtual void populate();

public slots:
    virtual enum SetResponse set(const ParameterList & pParams );
    virtual void sSave();
    virtual void sNew();
    virtual void sFillList();

protected slots:
    virtual void languageChange();

private:
    int _mode;
    int _booitemid;
    int _booimageid;

};

#endif // BOOITEMIMAGE_H
