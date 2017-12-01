/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSourcePrices.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>
#include <QSqlError>

#include "itemSource.h"
#include "dspPoItemsByVendor.h"
#include "guiclient.h"
#include "parameterwidget.h"
#include "errorReporter.h"

dspItemSourcePrices::dspItemSourcePrices(QWidget* parent, const char*, Qt::WindowFlags fl)
  : display(parent, "dspItemSourcePrices", fl)
{
  setWindowTitle(tr("Item Source Prices"));
  setReportName("ItemSourcePrices");
  setMetaSQLOptions("itemSources", "prices");
  setUseAltId(true);
  setParameterWidgetVisible(true);
  setQueryOnStartEnabled(true);
  setSearchVisible(true);
  setExpandVisible(true);
  setCollapseVisible(true);

  QString qrySite = QString("SELECT 0, '%1' AS name, '' AS code "
                    "UNION "
                    "SELECT warehous_id, warehous_code, warehous_code "
                    "FROM site() "
                    "WHERE warehous_active "
                    "ORDER BY code;")
                    .arg(tr("[ Not Site Restricted ]"));

  if (_metrics->boolean("MultiWhs"))
  {
    parameterWidget()->appendComboBox(tr("Site"), "warehous_id", qrySite);
    parameterWidget()->append(tr("Drop Ship Only"), "showDropShip", ParameterWidget::Exists);
  }
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
  parameterWidget()->append(tr("Effective Start"), "effectiveStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Effective End"), "effectiveEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Expires Start"), "expireStartDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Expires End"), "expireEndDate", ParameterWidget::Date);
  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);

  list()->addColumn(tr("Item Number"),        _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  list()->addColumn(tr("Description"),        -1,          Qt::AlignLeft,   false, "item_descrip"   );
  list()->addColumn(tr("UOM"),                _uomColumn,  Qt::AlignCenter, false, "uom_name" );
  list()->addColumn(tr("Vendor #"),           _itemColumn, Qt::AlignLeft,   true,  "vend_number"   );
  list()->addColumn(tr("Vendor Name"),        -1,          Qt::AlignLeft,   true,  "vend_name"   );
  list()->addColumn(tr("Contract #"),         _itemColumn, Qt::AlignLeft,   true,  "contrct_number"   );
  list()->addColumn(tr("Effective"),          _dateColumn, Qt::AlignCenter, false, "itemsrc_effective"   );
  list()->addColumn(tr("Expires"),            _dateColumn, Qt::AlignCenter, false, "itemsrc_expires"   );
  list()->addColumn(tr("Vendor Currency"),    _itemColumn, Qt::AlignCenter, false, "vend_curr"   );
  list()->addColumn(tr("Vendor Item Number"), _itemColumn, Qt::AlignLeft,   true,  "itemsrc_vend_item_number"   );
  list()->addColumn(tr("Manufacturer"),       _itemColumn, Qt::AlignLeft,   false, "itemsrc_manuf_name" );
  list()->addColumn(tr("Manuf. Item#"),       _itemColumn, Qt::AlignLeft,   false, "itemsrc_manuf_item_number" );
  list()->addColumn(tr("Default"),            _uomColumn,  Qt::AlignCenter, true,  "itemsrc_default" );
  list()->addColumn(tr("Vendor UOM"),         _uomColumn,  Qt::AlignCenter, false, "itemsrc_vend_uom"   );
  list()->addColumn(tr("UOM Ratio"),          _qtyColumn,  Qt::AlignRight,  false, "itemsrc_invvendoruomratio"  );
  list()->addColumn(tr("Active"),             -1,          Qt::AlignLeft,   true,  "itemsrc_active"   );
  list()->addColumn(tr("Default"),            -1,          Qt::AlignLeft,   true,  "itemsrc_default"   );
  list()->addColumn(tr("Min. Order"),         _qtyColumn,  Qt::AlignRight,  false, "itemsrc_minordqty" );
  list()->addColumn(tr("Order Mult."),        _qtyColumn,  Qt::AlignRight,  false, "itemsrc_multordqty" );
  list()->addColumn(tr("Vendor Ranking"),     _qtyColumn,  Qt::AlignRight,  false, "itemsrc_ranking" );
  list()->addColumn(tr("Lead Time"),          _qtyColumn,  Qt::AlignRight,  false, "itemsrc_leadtime" );
  list()->addColumn(tr("Qty. Break"),         _qtyColumn,  Qt::AlignRight,  true,  "itemsrcp_qtybreak" );
  list()->addColumn(tr("Base Unit Price"),    _moneyColumn,Qt::AlignRight,  true,  "price_base" );
  list()->addColumn(tr("Item Currency"),      _itemColumn, Qt::AlignRight,  false, "item_curr" );
  list()->addColumn(tr("Unit Price"),         _moneyColumn,Qt::AlignRight,  false, "price_local" );
  if (_metrics->boolean("MultiWhs"))
  {
    list()->addColumn(tr("Site"),              _qtyColumn, Qt::AlignCenter, true, "warehous_code");
    list()->addColumn(tr("Order Type"),                -1, Qt::AlignCenter, true, "itemsrcp_dropship");
  }

  if (_privileges->check("MaintainItemSources"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(false);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }
}

void dspItemSourcePrices::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem*, int)
{
  QAction *menuItem;

  menuItem = menuThis->addAction(tr("Edit Item Source..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources"));

  menuItem = menuThis->addAction(tr("View Item Source..."), this, SLOT(sView()));
  menuItem->setEnabled(_privileges->check("MaintainItemSources") || _privileges->check("ViewItemSource"));
}

void dspItemSourcePrices::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", list()->altId());

  itemSource newdlg(this, "", true);
  newdlg.set(params);
  newdlg.findChild<QTabWidget*>("_tab")->setCurrentIndex(1);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspItemSourcePrices::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsrc_id", list()->altId());

  itemSource newdlg(this, "", true);
  newdlg.set(params);
  newdlg.findChild<QTabWidget*>("_tab")->setCurrentIndex(1);
  newdlg.exec();
}

bool dspItemSourcePrices::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  params.append("indented");
  params.append("always", tr("'Always'"));
  params.append("never", tr("'Never'"));
  params.append("all", tr("All"));
  params.append("stock", tr("Into Stock"));
  params.append("dropship", tr("Drop Ship"));

  return true;
}
