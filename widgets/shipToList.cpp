/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "shipToList.h"

#include <QVariant>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>

#include <parameter.h>
#include <xsqlquery.h>

#include "xtreewidget.h"

shipToList::shipToList(QWidget * parent, const char *name, bool modal, Qt::WFlags fl) :
  QDialog( parent, name, modal, fl )
{
  setWindowTitle(tr("Ship To's"));

  _shiptoid = -1;

  if (!name)
    setObjectName("shipToList");

  QVBoxLayout *shipToListLayout = new QVBoxLayout( this, 5, 5, "shipToListLayout"); 
  QHBoxLayout *Layout54 = new QHBoxLayout( 0, 0, 7, "Layout54"); 
  QHBoxLayout *Layout53 = new QHBoxLayout( 0, 0, 5, "Layout53"); 
  QVBoxLayout *Layout51 = new QVBoxLayout( 0, 0, 5, "Layout51"); 
  QVBoxLayout *Layout49 = new QVBoxLayout( 0, 0, 0, "Layout49"); 
  QVBoxLayout *Layout52 = new QVBoxLayout( 0, 0, 5, "Layout52"); 
  QVBoxLayout *Layout50 = new QVBoxLayout( 0, 0, 0, "Layout50"); 
  QHBoxLayout *Layout6 = new QHBoxLayout( 0, 0, 0, "Layout6"); 
  QVBoxLayout *Layout2 = new QVBoxLayout( 0, 0, 5, "Layout2"); 
  QVBoxLayout *Layout55 = new QVBoxLayout( 0, 0, 0, "Layout55"); 

    QLabel *_custNumberLit = new QLabel(tr("Customer #:"), this, "_custNumberLit");
    _custNumberLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout49->addWidget( _custNumberLit );

    QLabel *_custNameLit = new QLabel(tr("Cust Name:"), this, "_custNameLit");
    _custNameLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout49->addWidget( _custNameLit );

    QLabel *_custAddressLit = new QLabel(tr("Cust Address:"), this, "_custAddressLit");
    _custAddressLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout49->addWidget( _custAddressLit );
    Layout51->addLayout( Layout49 );

    QLabel *_searchForLit = new QLabel(tr("Search For:"), this, "_searchForLit");
    _searchForLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout51->addWidget( _searchForLit );
    Layout53->addLayout( Layout51 );

    _custNumber = new QLabel( this, "_custNumber" );
    Layout6->addWidget( _custNumber );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout6->addItem( spacer );
    Layout50->addLayout( Layout6 );

    _custName = new QLabel( this, "_custName" );
    Layout50->addWidget( _custName );

    _custAddress = new QLabel( this, "_custAddress" );
    Layout50->addWidget( _custAddress );
    Layout52->addLayout( Layout50 );

    _searchFor = new QLineEdit( this, "_searchFor" );
    Layout52->addWidget( _searchFor );
    Layout53->addLayout( Layout52 );
    Layout54->addLayout( Layout53 );

    _close = new QPushButton(tr("&Cancel"), this, "_close");
    Layout2->addWidget( _close );

    _select = new QPushButton(tr("&Select"), this, "_select");
    _select->setDefault( TRUE );
    Layout2->addWidget( _select );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
    Layout2->addItem( spacer_2 );
    Layout54->addLayout( Layout2 );
    shipToListLayout->addLayout( Layout54 );

    _shipTosList = new QLabel(tr("Ship To's:"), this, "_shipTosList");
    Layout55->addWidget( _shipTosList );

    _shipto = new XTreeWidget(this);
    _shipto->setObjectName("_shipto");
    Layout55->addWidget( _shipto );
    shipToListLayout->addLayout( Layout55 );

    resize( QSize(580, 365).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    // signals and slots connections
    connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
    connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
    connect( _searchFor, SIGNAL( textChanged(const QString&) ), this, SLOT( sSearch(const QString&) ) );
    connect( _shipto, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );

  _shipto->addColumn(tr("Number"),           70, Qt::AlignLeft, true, "shipto_num");
  _shipto->addColumn(tr("Name"),            150, Qt::AlignLeft, true, "shipto_name");
  _shipto->addColumn(tr("Address"),         150, Qt::AlignLeft, true, "shipto_address1");
  _shipto->addColumn(tr("City, State, Zip"), -1, Qt::AlignLeft, true, "csz");
  _shipto->sortByColumn(1, Qt::AscendingOrder);
}

void shipToList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("shipto_id", &valid);
  if (valid)
    _shiptoid = param.toInt();
  else
    _shiptoid = -1;

  param = pParams.value("cust_id", &valid);
  if (valid)
  {
    XSqlQuery cust;
    cust.prepare( "SELECT cust_number, cust_name, cust_address1 "
                  "FROM cust "
                  "WHERE (cust_id=:cust_id);" );
    cust.bindValue(":cust_id", param.toInt());
    cust.exec();
    if (cust.first())
    {
      _custNumber->setText(cust.value("cust_number").toString());
      _custName->setText(cust.value("cust_name").toString());
      _custAddress->setText(cust.value("cust_address1").toString());
    }

    XSqlQuery shipto;
    shipto.prepare( "SELECT shipto_id, shipto_num, shipto_name, shipto_address1,"
                    " (shipto_city || ', ' || shipto_state || '  ' || shipto_zipcode)  AS csz "
                    "FROM shipto "
                    "WHERE (shipto_cust_id=:cust_id) "
                    "ORDER BY shipto_num;" );
    shipto.bindValue(":cust_id", param.toInt());
    shipto.exec();
    _shipto->populate(shipto, _shiptoid );
  }
}

void shipToList::sClose()
{
  done(_shiptoid);
}

void shipToList::sSelect()
{
  done(_shipto->id());
}

void shipToList::sSearch(const QString &pTarget)
{
  _shipto->clearSelection();
  int i;
  for (i = 0; i < _shipto->topLevelItemCount(); i++)
  {
    if (_shipto->topLevelItem(i)->text(1).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < _shipto->topLevelItemCount())
  {
    _shipto->setCurrentItem(_shipto->topLevelItem(i));
    _shipto->scrollToItem(_shipto->topLevelItem(i));
  }
}
