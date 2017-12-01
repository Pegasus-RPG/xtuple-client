/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSources.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include "itemSource.h"
#include "buyCard.h"
#include "dspPoItemsByVendor.h"
#include "dspPoItemReceivingsByItem.h"
#include "guiclient.h"
#include "parameterwidget.h"
#include "errorReporter.h"

dspItemSources::dspItemSources(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspItemSources", fl)
{
  setWindowTitle(tr("Item Sources"));
  setReportName("ItemSources");
  setMetaSQLOptions("itemSources", "detail");
  setParameterWidgetVisible(true);
  setNewVisible(true);
  setQueryOnStartEnabled(true);
  setSearchVisible(true);

  parameterWidget()->append(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->append(tr("Item Number Pattern"), "item_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item Description"), "item_descrip_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Vendor"), "vend_id", ParameterWidget::Vendor);
  parameterWidget()->append(tr("Vendor Name"), "vend_name_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Vendor Item Number Pattern"), "vend_item_number_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Contract"), "contrct_id", XComboBox::Contracts);
  parameterWidget()->append(tr("Contract Number Pattern"), "contract_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Manufacturer Pattern"), "manuf_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Manufacturer Item Number Pattern"), "manuf_item_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Show Expired"), "showExpired", ParameterWidget::Exists);
  parameterWidget()->append(tr("Show Future"), "showFuture", ParameterWidget::Exists);
  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);

  list()->addColumn(tr("Vendor #"),             -1,        Qt::AlignLeft,   true,  "vend_number"   );
  list()->addColumn(tr("Vendor"),             -1,          Qt::AlignLeft,   true,  "vend_name"   );
  list()->addColumn(tr("Active"),             -1,          Qt::AlignLeft,   true,  "itemsrc_active"   );
  list()->addColumn(tr("Contract #"),         _itemColumn, Qt::AlignLeft,   true,  "contrct_number"   );
  list()->addColumn(tr("Effective"),          _dateColumn, Qt::AlignCenter, true,  "itemsrc_effective"   );
  list()->addColumn(tr("Expires"),            _dateColumn, Qt::AlignCenter, true,  "itemsrc_expires"   );
  list()->addColumn(tr("Item Number"),        _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),        -1,          Qt::AlignLeft,   true,  "item_descrip"   );
  list()->addColumn(tr("UOM"),                _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  list()->addColumn(tr("Vendor Item Number"), _itemColumn, Qt::AlignLeft,   true,  "itemsrc_vend_item_number"   );
  list()->addColumn(tr("Vendor UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "itemsrc_vend_uom"   );
  list()->addColumn(tr("UOM Ratio"),          _qtyColumn,  Qt::AlignRight,  true,  "itemsrc_invvendoruomratio"  );
  list()->addColumn(tr("Manufacturer"),       _itemColumn, Qt::AlignLeft,   false, "itemsrc_manuf_name" );
  list()->addColumn(tr("Manuf. Item#"),       _itemColumn, Qt::AlignLeft,   false, "itemsrc_manuf_item_number" );
  list()->addColumn(tr("Default"),            -1,          Qt::AlignLeft,   true,  "itemsrc_default"   );
  list()->addColumn(tr("Vendor Currency"),    _itemColumn, Qt::AlignCenter, false, "vend_curr" );
  list()->addColumn(tr("Min. Order"),         _qtyColumn,  Qt::AlignRight,  false, "itemsrc_minordqty" );
  list()->addColumn(tr("Order Mult."),        _qtyColumn,  Qt::AlignRight,  false, "itemsrc_multordqty" );
  list()->addColumn(tr("Vendor Ranking"),     _qtyColumn,  Qt::AlignRight,  false, "itemsrc_ranking" );
  list()->addColumn(tr("Lead Time"),          _qtyColumn,  Qt::AlignRight,  false, "itemsrc_leadtime" );

  if (_privileges->check("MaintainItemSources"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

bool dspItemSources::setParams(ParameterList & params)
{
  if (!display::setParams(params))
    return false;
  params.append("always", tr("Always"));
  params.append("never", tr("Never"));

  return true;
}

void dspItemSources::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem* selected, int)
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)selected;
  QAction *menuItem;

  menuItem = menuThis->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = menuThis->addAction(tr("View..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources") || _privileges->check("ViewItemSource"));

  menuItem = menuThis->addAction(tr("Set as Default..."), this, SLOT(sDefault()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources") && item->rawValue("itemsrc_default") != "Yes");

  menuItem = menuThis->addAction(tr("Copy..."), this, SLOT(sCopy()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = menuThis->addAction(tr("Delete..."), this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuThis->addSeparator();

  menuThis->addAction("View Buy Card...",  this, SLOT(sBuyCard()));

  menuThis->addAction("View Receipts and Returns...",  this, SLOT(sReceipts()));
}

void dspItemSources::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemSource newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSources::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", list()->id());

  itemSource newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSources::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsrc_id", list()->id());

  itemSource newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSources::sDefault()
{
  XSqlQuery itemSave;
  itemSave.prepare("UPDATE itemsrc "
                      "SET itemsrc_default=true "
                    "WHERE itemsrc_id=:itemsrc_id;");

  itemSave.bindValue(":itemsrc_id", list()->id());
  itemSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Item Information"),
                           itemSave, __FILE__, __LINE__))
    return;

  sFillList();
}

void dspItemSources::sCopy()
{
  ParameterList params;
  params.append("mode", "copy");
  params.append("itemsrc_id", list()->id());

  itemSource newdlg(this, "", true);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSources::sDelete()
{
  XSqlQuery itemDelete;
  itemDelete.prepare("SELECT poitem_id, itemsrc_active "
            "FROM poitem, itemsrc "
            "WHERE ((poitem_itemsrc_id=:itemsrc_id) "
            "AND (itemsrc_id=:itemsrc_id)); ");
  itemDelete.bindValue(":itemsrc_id", list()->id());
  itemDelete.exec();
  if (itemDelete.first())
  {
    if (itemDelete.value("itemsrc_active").toBool())
    {
      if (QMessageBox::question(this, tr("Delete Item Source"),
                                tr("<p>This item source is used by existing "
                                   "purchase order records and may not be "
                                   "deleted. Would you like to deactivate it "
                                   "instead?"),
                                QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
      {
        itemDelete.prepare( "UPDATE itemsrc SET "
                   "  itemsrc_active=false "
                   "WHERE (itemsrc_id=:itemsrc_id);" );
        itemDelete.bindValue(":itemsrc_id", list()->id());
        itemDelete.exec();

        sFillList();
      }
    }
    else
      QMessageBox::critical(this, tr("Delete Item Source"),
                            tr("<p>This item source is used by existing "
                               "purchase order records and may not be deleted."));
    return;
  }

  itemDelete.prepare( "SELECT item_number "
             "FROM itemsrc, item "
             "WHERE ( (itemsrc_item_id=item_id)"
             " AND (itemsrc_id=:itemsrc_id) );" );
  itemDelete.bindValue(":itemsrc_id", list()->id());
  itemDelete.exec();
  if (itemDelete.first())
  {
    if (QMessageBox::question(this, tr("Delete Item Source"),
                              tr( "Are you sure that you want to delete the "
                                 "Item Source for %1?")
                                  .arg(itemDelete.value("item_number").toString()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) == QMessageBox::Yes)
    {
      // itemsrcp deleted on cascade
      itemDelete.prepare( "DELETE FROM itemsrc "
                          "WHERE (itemsrc_id=:itemsrc_id);" );
      itemDelete.bindValue(":itemsrc_id", list()->id());
      itemDelete.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Item Source Information"),
                                    itemDelete, __FILE__, __LINE__))
      {
        return;
      }

      sFillList();
    }
  }
}

void dspItemSources::sBuyCard()
{
  ParameterList params;
  params.append("itemsrc_id", list()->id());

  buyCard *newdlg = new buyCard();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSources::sReceipts()
{
  XSqlQuery item;
  int itemid = -1;
  item.prepare("SELECT item_id "
               "  FROM item "
               " WHERE item_number=:item_number;");
  item.bindValue(":item_number", list()->selectedItems()[0]->rawValue("item_number").toString());
  item.exec();
  if (item.first())
    itemid = item.value("item_id").toInt();
  else if(ErrorReporter::error(QtCriticalMsg, this, tr("Error fetching item id"),
                               item, __FILE__, __LINE__))
    return;

  dspPoItemReceivingsByItem *newdlg = new dspPoItemReceivingsByItem();
  newdlg->findChild<ItemCluster*>("_item")->setId(itemid);
  omfgThis->handleNewWindow(newdlg);
}
