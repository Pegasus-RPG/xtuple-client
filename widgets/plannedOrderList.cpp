/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "plannedOrderList.h"

#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QVariant>

#include <parameter.h>
#include <xsqlquery.h>

#include "xtreewidget.h"
#include "warehouseCluster.h"

plannedOrderList::plannedOrderList(QWidget *parent, const char *name, bool modal, Qt::WFlags fl) :
  QDialog( parent, name, modal, fl )
{
  setWindowTitle(tr("Planned Orders"));

  _planordid = -1;

  if ( !name )
    setObjectName( "plannedOrderList" );

  QVBoxLayout *plannedOrderListLayout = new QVBoxLayout( this, 5, 5, "plannedOrderListLayout"); 
  QHBoxLayout *layout6387 = new QHBoxLayout( 0, 0, 7, "layout6387"); 
  QVBoxLayout *layout6385 = new QVBoxLayout( 0, 0, 0, "layout6385"); 
  QHBoxLayout *layout6384 = new QHBoxLayout( 0, 0, 5, "layout6384"); 
  QVBoxLayout *Layout183 = new QVBoxLayout( 0, 0, 5, "Layout183"); 
  QVBoxLayout *Layout185 = new QVBoxLayout( 0, 0, 0, "Layout185"); 
  QHBoxLayout *layout6386 = new QHBoxLayout( 0, 0, 0, "layout6386"); 

    _warehouseGroup = new QGroupBox( this );
    QVBoxLayout *_warehouseGroupLayout = new QVBoxLayout( _warehouseGroup );
    _warehouseGroupLayout->setSpacing( 0 );
    _warehouseGroupLayout->setMargin( 5 );
    _warehouseGroupLayout->setAlignment( Qt::AlignTop );

    _allWarehouses = new QRadioButton(tr("&All Sites"), _warehouseGroup, "_allWarehouses");
    _allWarehouses->setChecked( TRUE );
    _warehouseGroupLayout->addWidget( _allWarehouses );

    _selectedWarehouse = new QRadioButton(tr("Site:"), _warehouseGroup, "_selectedWarehouse");
    _selectedWarehouse->setChecked( FALSE );
    layout6384->addWidget( _selectedWarehouse );

    _warehouse = new WComboBox( _warehouseGroup, "_warehouse" );
    _warehouse->setEnabled( FALSE );
    layout6384->addWidget( _warehouse );
    _warehouseGroupLayout->addLayout( layout6384 );
    layout6385->addWidget( _warehouseGroup );
    _warehouseGroup->setLayout( _warehouseGroupLayout );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
    layout6385->addItem( spacer );
    layout6387->addLayout( layout6385 );

    QSpacerItem* spacer_2 = new QSpacerItem( 223, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout6386->addItem( spacer_2 );

    _close = new QPushButton(tr("&Cancel"), this, "_close");
    Layout183->addWidget( _close );

    _select = new QPushButton(tr("&Select"), this, "_select");
    _select->setAutoDefault( TRUE );
    _select->setDefault( TRUE );
    Layout183->addWidget( _select );
    layout6386->addLayout( Layout183 );
    layout6387->addLayout( layout6386 );
    plannedOrderListLayout->addLayout( layout6387 );

    QLabel *_plannedOrdersLit = new QLabel(tr("Planned Orders:"), this, "_plannedOrdersLit");
    Layout185->addWidget( _plannedOrdersLit );

    _planord = new XTreeWidget( this);
    _planord->setObjectName("_planord" );
    Layout185->addWidget( _planord );
    plannedOrderListLayout->addLayout( Layout185 );

    resize( QSize(496, 360).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    // signals and slots connections
    connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
    connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
    connect( _planord, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
    connect( _selectedWarehouse, SIGNAL( toggled(bool) ), this, SLOT( sFillList() ) );
    connect( _warehouse, SIGNAL( newID(int) ), this, SLOT( sFillList() ) );
    connect( _selectedWarehouse, SIGNAL( toggled(bool) ), _warehouse, SLOT( setEnabled(bool) ) );

  setTabOrder(_allWarehouses, _selectedWarehouse);
  setTabOrder(_selectedWarehouse, _warehouse);
  setTabOrder(_warehouse, _planord);
  setTabOrder(_planord, _select);
  setTabOrder(_select, _close);
  setTabOrder(_close, _allWarehouses);
  _planord->setFocus();

  _planord->addColumn(tr("PL/O #"),    _orderColumn, Qt::AlignLeft,  true, "number");
  _planord->addColumn(tr("Firmed"),              40, Qt::AlignCenter,true, "planord_firm");
  _planord->addColumn(tr("Whs."),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _planord->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft,  true, "item_number");
  _planord->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "descrip");

  sFillList();
}

void plannedOrderList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("planord_id", &valid);
  if (valid)
    _planordid = param.toInt();

  sFillList();
}

void plannedOrderList::sClose()
{
  done(_planordid);
}

void plannedOrderList::sSelect()
{
  done(_planord->id());
}

void plannedOrderList::sFillList()
{
  QString sql( "SELECT planord_id,"
               "       formatPloNumber(planord_id) AS number,"
               "       planord_firm,"
               "       warehous_code, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) AS descrip "
               "FROM planord, itemsite, warehous, item "
               "WHERE ((planord_itemsite_id=itemsite_id)"
               " AND (itemsite_warehous_id=warehous_id)"
               " AND (itemsite_item_id=item_id)" );

  if (_selectedWarehouse->isChecked())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY planord_number, planord_subnumber,"
         "         warehous_code, item_number;";

  XSqlQuery planord;
  planord.prepare(sql);
  planord.bindValue(":warehous_id", _warehouse->id());
  planord.exec();

  _planord->populate(planord, _planordid);
}
