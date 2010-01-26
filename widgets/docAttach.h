/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef DOCATTACH_H
#define DOCATTACH_H

#include <parameter.h>
#include <xsqlquery.h>

#include "documents.h"
#include "widgets.h"
#include "ui_docAttach.h"

class docAttach : public QDialog, public Ui::docAttach
{
    Q_OBJECT

public:
    docAttach(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~docAttach();

    //virtual void populate();

public slots:
    virtual void set( const ParameterList & pParams );
    virtual void sSave();

protected slots:
    virtual void languageChange();

private:
    int _sourceid;
    int _source;
    QString _targettype;
    int _targetid;
    QString _purpose;

};

#endif // docAttach_H
