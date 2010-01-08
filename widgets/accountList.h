/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef accountlist_h
#define accountlist_h

#include <QDialog>

#include "widgets.h"

#include "xtreewidget.h"

class QPushButton;
class ParameterList;

class XTUPLEWIDGETS_EXPORT accountList : public QDialog
{
    Q_OBJECT

  public:
    accountList(QWidget * = 0, const char * = 0, bool = FALSE, Qt::WFlags = 0);

    XTreeWidget* _accnt;
    QPushButton* _close;
    QPushButton* _select;
    QPushButton* _clear;

  public slots:
    virtual void set( const ParameterList & pParams );
    virtual void sClear();
    virtual void sFillList();
    virtual void sClose();
    virtual void sSelect();

  private:
    int _accntid;
    bool         _showExternal;
    unsigned int _type;

};

#endif
