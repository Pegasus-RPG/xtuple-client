/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "itemList.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVariant>

#include <parameter.h>

#include "itemcluster.h"
#include "xtreewidget.h"

QString buildItemLineEditQuery(const QString, const QStringList, const QString, const unsigned int);
QString buildItemLineEditTitle(const unsigned int, const QString);

itemList::itemList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog( parent, name, modal, fl )
{
  if ( !name )
    setObjectName( "itemList" );

  _itemid = -1;
  _itemType = ItemLineEdit::cUndefined;
  _useQuery = FALSE;

  setWindowTitle(tr("Item List"));

  QVBoxLayout *itemListLayout = new QVBoxLayout( this, 5, 5, "itemListLayout"); 
  QHBoxLayout *Layout69 = new QHBoxLayout( 0, 0, 0, "Layout69"); 
  QHBoxLayout *Layout72 = new QHBoxLayout( 0, 0, 7, "Layout72"); 
  QVBoxLayout *Layout71 = new QVBoxLayout( 0, 0, 0, "Layout71"); 
  QVBoxLayout *Layout70 = new QVBoxLayout( 0, 0, 5, "Layout70"); 
  QHBoxLayout *Layout5 = new QHBoxLayout( 0, 0, 5, "Layout5"); 
  QVBoxLayout *Layout18 = new QVBoxLayout( 0, 0, 5, "Layout18"); 
  QVBoxLayout *Layout20 = new QVBoxLayout( 0, 0, 0, "Layout20"); 

    QLabel *_searchForLit = new QLabel(tr("S&earch for:"), this, "_searchForLit");
    _searchForLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
    Layout5->addWidget( _searchForLit );

    _searchFor = new QLineEdit( this, "_searchFor" );
    _searchForLit->setBuddy(_searchFor);
    Layout5->addWidget( _searchFor );
    Layout70->addLayout( Layout5 );

    _showInactive = new QCheckBox(tr("Show &Inactive Items"), this, "_showInactive");
    Layout69->addWidget( _showInactive );

    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout69->addItem( spacer );
    _showMake = new QCheckBox(tr("&Make Items Only"), this, "_showMake");
    _showBuy = new QCheckBox(tr("&Buy Items Only"), this, "_showBuy");
    Layout70->addWidget( _showMake );
    Layout70->addWidget( _showBuy );
    Layout70->addLayout( Layout69 );
    Layout71->addLayout( Layout70 );

    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
    Layout71->addItem( spacer_2 );
    Layout72->addLayout( Layout71 );

    _close = new QPushButton(tr("&Cancel"), this, "_close");
    Layout18->addWidget( _close );

    _select = new QPushButton(tr("&Select"), this, "_select");
    _select->setEnabled( FALSE );
    _select->setAutoDefault( TRUE );
    _select->setDefault( TRUE );
    Layout18->addWidget( _select );
    Layout72->addLayout( Layout18 );
    itemListLayout->addLayout( Layout72 );

    QLabel *_itemsLit = new QLabel(tr("&Items:"), this, "_itemsLit");
    Layout20->addWidget( _itemsLit );

    _item = new XTreeWidget(this);
    _item->setObjectName("_item");
    _itemsLit->setBuddy(_item );
    Layout20->addWidget( _item );
    itemListLayout->addLayout( Layout20 );

    resize( QSize(467, 393).expandedTo(minimumSizeHint()) );
    //clearWState( WState_Polished );

    connect( _item, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
    connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
    connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
    connect( _searchFor, SIGNAL( textChanged(const QString&) ), this, SLOT( sSearch(const QString&) ) );
    connect( _showInactive, SIGNAL( clicked() ), this, SLOT( sFillList() ) );
    connect( _showMake, SIGNAL( clicked() ), this, SLOT( sFillList() ) );
    connect( _showBuy, SIGNAL( clicked() ), this, SLOT( sFillList() ) );
    connect( _item, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );


  _item->addColumn(tr("Item Number"), 100, Qt::AlignLeft, true, "item_number");
  _item->addColumn(tr("Description"),  -1, Qt::AlignLeft, true, "itemdescrip");
  _item->addColumn(tr("UPC Code"),    100, Qt::AlignLeft, true, "item_upccode");
}

void itemList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();
  else
    _itemid = -1;

  param = pParams.value("sql", &valid);
  if (valid)
  {
    _sql = param.toString();
    _useQuery = TRUE;
  }
  else
    _useQuery = FALSE;

  param = pParams.value("itemType", &valid);
  if (valid)
  {
    _itemType = param.toUInt();
    setWindowTitle(buildItemLineEditTitle(_itemType, tr("Items")));
    _showMake->setChecked(_itemType & ItemLineEdit::cGeneralManufactured);
    _showBuy->setChecked(_itemType & ItemLineEdit::cGeneralPurchased);
  }
  else
  {
    _itemType = ItemLineEdit::cUndefined;
    _showMake->hide();
    _showBuy->hide();
  }

  param = pParams.value("extraClauses", &valid);
  if (valid)
    _extraClauses = param.toStringList();

  _showInactive->setChecked(FALSE);
  _showInactive->setEnabled(!(_itemType & ItemLineEdit::cActive));
  if(!_showInactive->isEnabled())
    _showInactive->hide();

  param = pParams.value("caption", &valid);
  if (valid)
    setWindowTitle(param.toString());

  sFillList();
}

void itemList::sClose()
{
  done(_itemid);
}

void itemList::sSelect()
{
  done(_item->id());
}

void itemList::sSearch(const QString &pTarget)
{
  if (_item->currentItem())
    _item->setCurrentItem(0);

  _item->clearSelection();

  int i;
  for (i = 0; i < _item->topLevelItemCount(); i++)
    if (_item->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive) ||
        _item->topLevelItem(i)->text(1).startsWith(pTarget, Qt::CaseInsensitive) ||
        _item->topLevelItem(i)->text(2).startsWith(pTarget, Qt::CaseInsensitive))
      break;

  if (i < _item->topLevelItemCount())
  {
    _item->setCurrentItem(_item->topLevelItem(i));
    _item->scrollToItem(_item->topLevelItem(i));
  }
}

void itemList::sFillList()
{
  if (_useQuery)
  {
    _item->populate(_sql, _itemid);
  }
  else
  { 
      QString pre;
      QString post;
      if(_x_preferences && _x_preferences->boolean("ListNumericItemNumbersFirst"))
      {
        pre =  "SELECT DISTINCT ON (toNumeric(item_number, 999999999999999), item_number) item_id, item_number,"
               "(item_descrip1 || ' ' || item_descrip2) AS itemdescrip, item_upccode ";
        post = "ORDER BY toNumeric(item_number, 999999999999999), item_number, item_upccode ";
      }
      else
      {
        pre =  "SELECT DISTINCT item_id, item_number,"
               "(item_descrip1 || ' ' || item_descrip2) AS itemdescrip, item_upccode ";
        post = "ORDER BY item_number";
      }

      QStringList clauses;
      clauses = _extraClauses;
      if(!(_itemType & ItemLineEdit::cActive) && !_showInactive->isChecked())
        clauses << "(item_active)";

      if (_showMake->isChecked())
	    _itemType = (_itemType | ItemLineEdit::cGeneralManufactured);
      else if (_itemType & ItemLineEdit::cGeneralManufactured)
	    _itemType = (_itemType ^ ItemLineEdit::cGeneralManufactured);
	  
      if (_showBuy->isChecked())
	    _itemType = (_itemType | ItemLineEdit::cGeneralPurchased);
      else if (_itemType & ItemLineEdit::cGeneralPurchased)
	    _itemType = (_itemType ^ ItemLineEdit::cGeneralPurchased);

      setWindowTitle(buildItemLineEditTitle(_itemType, tr("Items")));

      _item->populate(buildItemLineEditQuery(pre, clauses, post, _itemType), _itemid);
  }
}

void itemList::reject()
{
  done(_itemid);
}
