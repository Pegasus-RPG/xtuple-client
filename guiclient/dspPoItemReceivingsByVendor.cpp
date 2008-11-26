/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspPoItemReceivingsByVendor.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <openreports.h>
#include <parameter.h>

#include "enterPoitemReceipt.h"
#include "guiclient.h"
#include "mqlutil.h"
#include "poLiabilityDistrib.h"
#include "voucher.h"

dspPoItemReceivingsByVendor::dspPoItemReceivingsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_showVariances, SIGNAL(toggled(bool)), this, SLOT(sHandleVariance(bool)));
  connect(_porecv, SIGNAL(populateMenu(QMenu*, QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  _porecv->addColumn(tr("P/O #"),      _orderColumn, Qt::AlignRight, true, "ponumber");
  _porecv->addColumn(tr("Sched. Date"), _dateColumn, Qt::AlignCenter,true, "duedate");
  _porecv->addColumn(tr("Recv. Date"),  _dateColumn, Qt::AlignCenter,true, "recvdate");
  _porecv->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "venditemnumber");
  _porecv->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, "venditemdescrip");
  _porecv->addColumn(tr("Rcvd/Rtnd"),   _qtyColumn,  Qt::AlignRight, true, "sense");
  _porecv->addColumn(tr("Qty."),        _qtyColumn,  Qt::AlignRight, true, "qty");
  _porecv->addColumn(tr("Invoiced"),     _ynColumn,  Qt::AlignRight, false,"invoiced");
  if (_privileges->check("ViewCosts"))
  {
    _porecv->addColumn(tr("Purch. Cost"),_priceColumn, Qt::AlignRight,true, "purchcost");
    _porecv->addColumn(tr("Recv. Cost"), _priceColumn, Qt::AlignRight,true, "recvcost");
    _porecv->addColumn(tr("Value"),      _priceColumn, Qt::AlignRight,true, "value");
  }

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);

  _showVariances->setEnabled(_privileges->check("ViewCosts"));
  sHandleVariance(_showVariances->isChecked());
}

dspPoItemReceivingsByVendor::~dspPoItemReceivingsByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPoItemReceivingsByVendor::languageChange()
{
  retranslateUi(this);
}

bool dspPoItemReceivingsByVendor::setParams(ParameterList &pParams)
{
  if (!_vendor->isValid() && _vendor->isVisible())
  {
    QMessageBox::warning( this, tr("Enter Vendor Number"),
                          tr( "Please enter a valid Vendor Number." ) );
    _vendor->setFocus();
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
  pParams.append("vend_id", _vendor->id());

  if (_selectedPurchasingAgent->isChecked())
    pParams.append("agentUsername", _agent->currentText());

  if (_showVariances->isChecked())
    pParams.append("showVariances");

  if (_showUnvouchered->isChecked())
    pParams.append("showUnvouchered");

  pParams.append("received", tr("Received"));
  pParams.append("returned", tr("Returned"));
  pParams.append("unvouchered", tr("Not Vouchered"));
  pParams.append("nonInv",   tr("NonInv - "));
  pParams.append("na",       tr("N/A"));

  return true;
}

void dspPoItemReceivingsByVendor::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("ReceiptsReturnsByVendor", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoItemReceivingsByVendor::sPopulateMenu(QMenu *menu, QTreeWidgetItem *)
{
  QAction *markInvoiced = 0;
  QAction *correctReceipt = 0;
  QAction *createVoucher = 0;
  switch (_porecv->altId())
  {
    case 1:     // id = recv_id
      markInvoiced   = new QAction(tr("Mark As Invoiced..."),  this);
      correctReceipt = new QAction(tr("Correct Receiving..."), this);

      markInvoiced->setEnabled(_privileges->check("MaintainUninvoicedReceipts")
                               && ! _porecv->currentItem()->rawValue("invoiced").isNull()
                               && ! _porecv->currentItem()->rawValue("invoiced").toBool());
      correctReceipt->setEnabled(_privileges->check("MaintainUninvoicedReceipts"));
      connect(markInvoiced, SIGNAL(triggered()), this, SLOT(sMarkAsInvoiced()));
      connect(correctReceipt, SIGNAL(triggered()), this, SLOT(sCorrectReceiving()));

      menu->addAction(markInvoiced);
      menu->addAction(correctReceipt);
      break;
    case 2:     // id = poreject_id
      break;
    case 3:     // id = unvouchered pohead_id
      createVoucher = new QAction(tr("Create Voucher..."),  this);
      createVoucher->setEnabled(_privileges->check("MaintainVouchers"));
      connect(createVoucher, SIGNAL(triggered()), this, SLOT(sCreateVoucher()));
      menu->addAction(createVoucher);
      break;
    default:
      break;
  }
}

void dspPoItemReceivingsByVendor::sHandleVariance(bool pShowVariances)
{
  if (pShowVariances)
  {
    _porecv->showColumn("purchcost");
    _porecv->showColumn("recvcost");
    _porecv->showColumn("value");
  }
  else
  {
    _porecv->hideColumn("purchcost");
    _porecv->hideColumn("recvcost");
    _porecv->hideColumn("value");
  }
}

void dspPoItemReceivingsByVendor::sFillList()
{
  ParameterList params;
  if (! setParams(params))
  {
    _porecv->clear();
    return;
  }
  MetaSQLQuery mql = mqlLoad("receivings", "detail");
  q = mql.toQuery(params);
  _porecv->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspPoItemReceivingsByVendor::sCorrectReceiving()
{
  if (enterPoitemReceipt::correctReceipt(_porecv->id(), this) != XDialog::Rejected)
    sFillList();
}

void dspPoItemReceivingsByVendor::sCreateVoucher()
{
  ParameterList params;
  params.append("mode",      "new");
  params.append("pohead_id", _porecv->id());

  voucher *newdlg = new voucher();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPoItemReceivingsByVendor::sMarkAsInvoiced()
{
  if (_porecv->currentItem()->rawValue("invoiced").toBool()
      || _porecv->currentItem()->rawValue("invoiced").isNull())
    return;

  bool update = true;
  
  if (recvHasValue())
  {
    ParameterList params;
    params.append("porecv_id", _porecv->id());  
    poLiabilityDistrib newdlg(this, "", TRUE);
    newdlg.set(params);
    if (newdlg.exec() == XDialog::Rejected)
      update = FALSE;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (update)
  {
    q.prepare("UPDATE recv "
	      "SET recv_invoiced=true "
	      "WHERE (recv_id=:porecv_id); ");
    q.bindValue(":porecv_id",_porecv->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}

bool dspPoItemReceivingsByVendor::recvHasValue()
{
  bool hasValue = false;

  if (_privileges->check("ViewCosts"))
    hasValue = _porecv->currentItem()->rawValue("value").toDouble() != 0;
  else
  {
    XSqlQuery valq;
    valq.prepare("SELECT * "
                 "FROM recv "
                 "WHERE ((recv_value <> 0)"
                 "   AND (recv_id=:recv_id));");
    valq.bindValue(":recv_id",_porecv->id());
    valq.exec();
    if (valq.first())
      hasValue = true;
    else if (valq.lastError().type() != QSqlError::None)
    {
      systemError(this, valq.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }

  return hasValue;
}
