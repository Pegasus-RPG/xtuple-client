/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef itemsearch_h
#define itemsearch_H

#include <qdialog.h>

#include "widgets.h"
#include "itemcluster.h"
#include "xcheckbox.h"

class QLineEdit;
class QPushButton;
class XTreeWidget;
class ParameterList;

class XTUPLEWIDGETS_EXPORT itemSearch : public QDialog
{
    Q_OBJECT

public:
    itemSearch( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, Qt::WFlags fl = 0 );

    QLineEdit* _search;
    XCheckBox* _searchNumber;
    XCheckBox* _searchDescrip1;
    XCheckBox* _searchDescrip2;
    XCheckBox* _searchUpc;
    XCheckBox* _showInactive;
    QPushButton* _close;
    QPushButton* _select;
    XTreeWidget* _item;

public slots:
    virtual void set( ParameterList & pParams );
    virtual void sClose();
    virtual void sSelect();
    virtual void sFillList();


private:
    int _itemid;
    unsigned int _itemType;
    bool _useQuery;
    QString _sql;
    QStringList _extraClauses;
};

#endif
