/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __CSVADDMAPINPUTDIALOG_H__
#define __CSVADDMAPINPUTDIALOG_H__

#include <QString>

#include "ui_csvaddmapinputdialog.h"

class CSVAddMapInputDialog : public QDialog, Ui::CSVAddMapInputDialog
{
  Q_OBJECT

  public:
    CSVAddMapInputDialog(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~CSVAddMapInputDialog();

    virtual QString mapname()          const;
    virtual QString qualifiedTable()   const;
    virtual QString schema()           const;
    virtual QString table()            const;
    virtual QString unqualifiedTable() const;

  public slots:
    virtual void setMapname(const QString mapname);
    virtual void setSchema(const  QString schema);
    virtual void setTable(const   QString table);

  protected slots:
    void languageChange();
    void populateSchema();
    void populateTable();

  private:
};

#endif

