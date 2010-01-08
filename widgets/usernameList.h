/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef __USERNAMELIST_H__
#define __USERNAMELIST_H__

#include <qdialog.h>

#include "widgets.h"

#include "qstringlist.h"

class QLineEdit;
class QPushButton;
class XTreeWidget;
class ParameterList;

class XTUPLEWIDGETS_EXPORT usernameList : public QDialog
{
    Q_OBJECT

  public:
    usernameList( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );

    QLineEdit* _searchFor;
    QPushButton* _close;
    QPushButton* _select;
    XTreeWidget* _user;

  public slots:
    virtual void set( ParameterList & pParams );
    virtual void reject();
    virtual void sClose();
    virtual void sSelect();
    virtual void sSearch( const QString & pTarget );
    virtual void sFillList();

  private:
    int _id;
    unsigned int _type;
};

#endif
