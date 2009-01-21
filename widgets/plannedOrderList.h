/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef plannedorderlist_h
#define plannedorderlist_h

#include <qdialog.h>

#include "widgets.h"

class WComboBox;
class QGroupBox;
class QRadioButton;
class QPushButton;
class XTreeWidget;
class ParameterList;

class XTUPLEWIDGETS_EXPORT plannedOrderList : public QDialog
{
    Q_OBJECT

  public:
    plannedOrderList(QWidget * = 0, const char * = 0, bool = FALSE, Qt::WFlags = 0);

    QGroupBox* _warehouseGroup;
    QRadioButton* _allWarehouses;
    QRadioButton* _selectedWarehouse;
    WComboBox* _warehouse;
    QPushButton* _close;
    QPushButton* _select;
    XTreeWidget* _planord;

  public slots:
    virtual void set( ParameterList & pParams );
    virtual void sClose();
    virtual void sSelect();
    virtual void sFillList();


  private:
    int _planordid;

};

#endif
