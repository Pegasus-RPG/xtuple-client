/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspInventoryLocator.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>
#include <metasql.h>
#include "mqlutil.h"

#include "relocateInventory.h"
#include "reassignLotSerial.h"

dspInventoryLocator::dspInventoryLocator(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_itemloc, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _itemloc->addColumn(tr("Site"),       _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _itemloc->addColumn(tr("Location"),          200, Qt::AlignLeft,  true, "locationname");
  _itemloc->addColumn(tr("Netable"),  _orderColumn, Qt::AlignCenter,true, "netable");
  _itemloc->addColumn(tr("Lot/Serial #"),       -1, Qt::AlignLeft,  true, "lotserial");
  _itemloc->addColumn(tr("Expiration"),_dateColumn, Qt::AlignCenter,true, "expiration");
  _itemloc->addColumn(tr("Warranty"),  _dateColumn, Qt::AlignCenter,true, "warranty");
  _itemloc->addColumn(tr("Qty."),       _qtyColumn, Qt::AlignRight, true, "qoh");

  _item->setFocus();
}

dspInventoryLocator::~dspInventoryLocator()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryLocator::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspInventoryLocator::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

  return NoError;
}

bool dspInventoryLocator::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Item Number"),
                      tr("You must enter a valid Item Number for this report.") );
    return false;
  }

  _warehouse->appendValue(params);

  params.append("item_id", _item->id());
  params.append("yes",     tr("Yes"));
  params.append("no",      tr("No"));
  params.append("na",      tr("N/A"));

  //if (_showZeroLevel->isChecked())
  //  params.append("showZeroLevel");

  return true;
}

void dspInventoryLocator::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("LocationLotSerialNumberDetail", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryLocator::sRelocateInventory()
{
  ParameterList params;
  params.append("itemloc_id", _itemloc->id());

  relocateInventory newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec())
    sFillList();
}

void dspInventoryLocator::sReassignLotSerial()
{
  ParameterList params;
  params.append("itemloc_id", _itemloc->id());

  reassignLotSerial newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspInventoryLocator::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (((XTreeWidgetItem *)pSelected)->altId() == -1)
  {
    menuItem = pMenu->insertItem(tr("Relocate..."), this, SLOT(sRelocateInventory()), 0);
    if (!_privileges->check("RelocateInventory"))
      pMenu->setItemEnabled(menuItem, FALSE);

    menuItem = pMenu->insertItem(tr("Reassign Lot/Serial #..."), this, SLOT(sReassignLotSerial()), 0);
    if (!_privileges->check("ReassignLotSerial"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspInventoryLocator::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("inventoryLocator", "detail");
  q = mql.toQuery(params);
  _itemloc->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
