/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef shiptolist_h
#define shiptolist_h

#include <qvariant.h>
#include <qdialog.h>
//Added by qt3to4:
#include <QLabel>

#include "widgets.h"

class QLabel;
class QLineEdit;
class QPushButton;
class XTreeWidget;
class ParameterList;

class XTUPLEWIDGETS_EXPORT shipToList : public QDialog
{
  Q_OBJECT

  public:
    shipToList(QWidget * = 0, const char * = 0, bool = FALSE, Qt::WFlags = 0);

    QLabel* _custNumber;
    QLabel* _custName;
    QLabel* _custAddress;
    QLineEdit* _searchFor;
    QPushButton* _close;
    QPushButton* _select;
    QLabel* _shipTosList;
    XTreeWidget* _shipto;

  public slots:
    virtual void set( ParameterList & pParams );
    virtual void sClose();
    virtual void sSelect();
    virtual void sSearch( const QString & pTarget );

  private:
    int _shiptoid;
};

#endif
