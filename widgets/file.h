/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef FILE_H
#define FILE_H

#include <parameter.h>
#include <xsqlquery.h>

#include "documents.h"
#include "widgets.h"
#include "ui_file.h"

class file : public QDialog, public Ui::file
{
    Q_OBJECT

public:
    file(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~file();

public slots:
    virtual void set( const ParameterList & pParams );
    virtual void sSave();
    virtual void sFileList();
    virtual void sHandleButtons();

protected slots:
    virtual void languageChange();

private:
    enum Documents::DocumentSources _source;
    int _mode;
    int _urlid;
    int _sourceid;

};

#endif // FILE_H
