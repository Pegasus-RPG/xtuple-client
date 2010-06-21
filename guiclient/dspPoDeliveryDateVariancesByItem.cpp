/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoDeliveryDateVariancesByItem.h"

#include <QMessageBox>

#include <openreports.h>
#include <metasql.h>
#include <parameter.h>

#include "guiclient.h"
#include "mqlutil.h"

dspPoDeliveryDateVariancesByItem::dspPoDeliveryDateVariancesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);
  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   TRUE);
  
  _recv->addColumn(tr("P/O #"),      _orderColumn, Qt::AlignRight, true, "recv_order_number");
  _recv->addColumn(tr("Vendor"),     _orderColumn, Qt::AlignLeft,  true, "vend_name");
  _recv->addColumn(tr("Vend. Item #"),_itemColumn, Qt::AlignLeft,  true, "itemnumber");
  _recv->addColumn(tr("Vendor Description"),   -1, Qt::AlignLeft,  true, "itemdescrip");
  _recv->addColumn(tr("Qty."),         _qtyColumn, Qt::AlignRight, true, "recv_qty");
  _recv->addColumn(tr("Due Date"),    _dateColumn, Qt::AlignCenter,true, "recv_duedate");
  _recv->addColumn(tr("Recv. Date"),  _dateColumn, Qt::AlignCenter,true, "recv_date");
}

dspPoDeliveryDateVariancesByItem::~dspPoDeliveryDateVariancesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoDeliveryDateVariancesByItem::languageChange()
{
  retranslateUi(this);
}

bool dspPoDeliveryDateVariancesByItem::setParams(ParameterList &pParams)
{
  pParams.append("byItem");
  pParams.append("item_id", _item->id());

  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  return true;
}

void dspPoDeliveryDateVariancesByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("DeliveryDateVariancesByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoDeliveryDateVariancesByItem::sFillList()
{
  ParameterList params;

  MetaSQLQuery mql = mqlLoad("poDeliveryDateVariances", "detail");
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _recv->populate(q);
}
