/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSourcesByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

#include <metasql.h>
#include "mqlutil.h"

#include "itemSource.h"
#include "buyCard.h"
#include "dspPoItemsByItem.h"
#include "guiclient.h"

dspItemSourcesByItem::dspItemSourcesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemsrc, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _item->setType(ItemLineEdit::cGeneralPurchased);

  _itemsrc->addColumn(tr("Vendor"),      -1,          Qt::AlignLeft,   true,  "vend_name"   );
  _itemsrc->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "itemsrc_vend_item_number"   );
  _itemsrc->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignCenter, true,  "itemsrc_vend_uom" );
  _itemsrc->addColumn(tr("UOM Ratio"),   _qtyColumn,  Qt::AlignRight,  true,  "itemsrc_invvendoruomratio"  );

  _item->setFocus();
}

dspItemSourcesByItem::~dspItemSourcesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemSourcesByItem::languageChange()
{
  retranslateUi(this);
}

void dspItemSourcesByItem::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  orReport report("ItemSourcesByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemSourcesByItem::sPopulateMenu(QMenu *menuThis)
{
  menuThis->addAction("Edit...",           this, SLOT(sEdit()));
  menuThis->addAction("View Buy Card...",  this, SLOT(sBuyCard()));
  menuThis->addAction("View P/Os...",      this, SLOT(sViewPOs()));
}

void dspItemSourcesByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSourcesByItem::sBuyCard()
{
  ParameterList params;
  params.append("itemsrc_id", _itemsrc->id());

  buyCard *newdlg = new buyCard();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSourcesByItem::sViewPOs()
{
  ParameterList params;
  params.append("itemsrc_id", _itemsrc->id());
  params.append("run");
  
  dspPoItemsByItem *newdlg = new dspPoItemsByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSourcesByItem::sFillList()
{
  if (_item->isValid())
  {
    ParameterList params;
    
    params.append("byItem");
    params.append("item_id", _item->id());

    MetaSQLQuery mql = mqlLoad("itemSources", "detail");
    q = mql.toQuery(params);
    _itemsrc->populate(q, true);  
  }
  else
    _itemsrc->clear();
}
