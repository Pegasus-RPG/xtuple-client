/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSourcesByVendor.h"

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
#include "dspPoItemsByVendor.h"
#include "guiclient.h"

dspItemSourcesByVendor::dspItemSourcesByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemsrc, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_vendor, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_vendor, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _itemsrc->addColumn(tr("Item Number"),        _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _itemsrc->addColumn(tr("Description"),        -1,          Qt::AlignLeft,   true,  "itemdescrip"   );
  _itemsrc->addColumn(tr("UOM"),                _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  _itemsrc->addColumn(tr("Vendor Item Number"), _itemColumn, Qt::AlignLeft,   true,  "itemsrc_vend_item_number"   );
  _itemsrc->addColumn(tr("Vendor UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "itemsrc_vend_uom"   );
  _itemsrc->addColumn(tr("UOM Ratio"),          _qtyColumn,  Qt::AlignRight,  true,  "itemsrc_invvendoruomratio"  );
}

dspItemSourcesByVendor::~dspItemSourcesByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemSourcesByVendor::languageChange()
{
  retranslateUi(this);
}

void dspItemSourcesByVendor::sPrint()
{
  if (!_vendor->isValid())
  {
    QMessageBox::warning( this, tr("Enter Vendor"),
                        tr("Please enter a valid Vendor.") );
    _vendor->setFocus();
    return;
  }

  ParameterList params;
  params.append("vend_id", _vendor->id());

  orReport report("ItemSourcesByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemSourcesByVendor::sPopulateMenu(QMenu *menuThis)
{
  menuThis->addAction("Edit...",           this, SLOT(sEdit()));
  menuThis->addAction("View Buy Card...",  this, SLOT(sBuyCard()));
}

void dspItemSourcesByVendor::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSourcesByVendor::sBuyCard()
{
  ParameterList params;
  params.append("itemsrc_id", _itemsrc->id());

  buyCard *newdlg = new buyCard();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSourcesByVendor::sFillList()
{
  if (_vendor->isValid())
  {
    ParameterList params;

    params.append("byVendor");
    params.append("vend_id", _vendor->id());

    MetaSQLQuery mql = mqlLoad("itemSources", "detail");
    q = mql.toQuery(params);
    _itemsrc->populate(q, true);  
  }
  else
    _itemsrc->clear();
}
