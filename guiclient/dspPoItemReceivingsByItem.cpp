/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoItemReceivingsByItem.h"

#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>
#include <parameter.h>

#include "guiclient.h"
#include "mqlutil.h"

dspPoItemReceivingsByItem::dspPoItemReceivingsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showVariances, SIGNAL(toggled(bool)), this, SLOT(sHandleVariance(bool)));

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  
  _showVariances->setEnabled(_privileges->check("ViewCosts"));

  _porecv->addColumn(tr("P/O #"),        _orderColumn, Qt::AlignRight, true, "ponumber");
  _porecv->addColumn(tr("Vendor"),       120,          Qt::AlignLeft,  true, "vend_name");
  _porecv->addColumn(tr("Due Date"),     _dateColumn,  Qt::AlignCenter,true, "duedate");
  _porecv->addColumn(tr("Recv. Date"),   _dateColumn,  Qt::AlignCenter,true, "recvdate");
  _porecv->addColumn(tr("Vend. Item #"), _itemColumn,  Qt::AlignLeft,  true, "venditemnumber");
  _porecv->addColumn(tr("Description"),  -1,           Qt::AlignLeft,  true, "venditemdescrip");
  _porecv->addColumn(tr("Rcvd/Rtnd"),    _qtyColumn,   Qt::AlignRight, true, "sense");
  _porecv->addColumn(tr("Qty."),         _qtyColumn,   Qt::AlignRight, true, "qty");
  if (_privileges->check("ViewCosts"))
  {
    _porecv->addColumn(tr("Purch. Cost"), _priceColumn,  Qt::AlignRight,true,"purchcost");
    _porecv->addColumn(tr("Recv. Cost"),  _priceColumn,  Qt::AlignRight,true,"recvcost");
  }

  sHandleVariance(_showVariances->isChecked());
}

dspPoItemReceivingsByItem::~dspPoItemReceivingsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoItemReceivingsByItem::languageChange()
{
  retranslateUi(this);
}

bool dspPoItemReceivingsByItem::setParams(ParameterList &pParams)
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter Item Number"),
                          tr( "Please enter a valid Item Number." ) );
    _item->setFocus();
    return false;
  }

  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return false;
  }

  _warehouse->appendValue(pParams);
  _dates->appendValue(pParams);

  pParams.append("item_id",  _item->id() );
  pParams.append("received", tr("Received"));
  pParams.append("returned", tr("Returned"));
  pParams.append("nonInv",   tr("NonInv - "));
  pParams.append("na",       tr("N/A"));

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_showVariances->isChecked())
    pParams.append("showVariances");

  return true;
}

void dspPoItemReceivingsByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("ReceiptsReturnsByItem", params);
  if (report.isValid())
      report.print();
  else
    report.reportError(this);
}

void dspPoItemReceivingsByItem::sHandleVariance(bool pShowVariances)
{
  if (pShowVariances)
  {
    _porecv->showColumn(8);
    _porecv->showColumn(9);
  }
  else
  {
    _porecv->hideColumn(8);
    _porecv->hideColumn(9);
  }
}

void dspPoItemReceivingsByItem::sFillList()
{
  ParameterList params;
  if (! setParams(params))
  {
    _porecv->clear();
    return;
  }
  MetaSQLQuery mql = mqlLoad("receivings", "detail");
  q = mql.toQuery(params);

  _porecv->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
