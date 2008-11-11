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

#include "dspInventoryHistoryByOrderNumber.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "adjustmentTrans.h"
#include "countTag.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "mqlutil.h"
#include "scrapTrans.h"
#include "transactionInformation.h"
#include "transferTrans.h"

dspInventoryHistoryByOrderNumber::dspInventoryHistoryByOrderNumber(QWidget* parent, const char* name, Qt::WFlags fl)
  : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _invhist->addColumn(tr("Transaction Time"),_timeDateColumn, Qt::AlignLeft,  true, "invhist_transdate");
  _invhist->addColumn(tr("Created Time"),    _timeDateColumn, Qt::AlignLeft,  false, "invhist_created");
  _invhist->addColumn(tr("Type"),               _transColumn, Qt::AlignCenter,true, "invhist_transtype");
  _invhist->addColumn(tr("Site."),                _whsColumn, Qt::AlignLeft,  true, "warehous_code");
  _invhist->addColumn(tr("Item Number"),                  -1, Qt::AlignLeft,  true, "item_number");
  _invhist->addColumn(tr("UOM"),                  _uomColumn, Qt::AlignCenter,true, "invhist_invuom");
  _invhist->addColumn(tr("Trans-Qty"),            _qtyColumn, Qt::AlignRight, true, "invhist_invqty" );
  _invhist->addColumn(tr("Order #"),             _itemColumn, Qt::AlignCenter,true, "ordernumber");
  _invhist->addColumn(tr("QOH Before"),           _qtyColumn, Qt::AlignRight, false, "invhist_qoh_before");
  _invhist->addColumn(tr("QOH After"),            _qtyColumn, Qt::AlignRight, false, "invhist_qoh_after");
  _invhist->addColumn(tr("Cost Method"),          _qtyColumn, Qt::AlignLeft,  false, "invhist_costmethod");
  _invhist->addColumn(tr("Value Before"),         _qtyColumn, Qt::AlignRight,false, "invhist_value_before");
  _invhist->addColumn(tr("Value After"),          _qtyColumn, Qt::AlignRight,false, "invhist_value_after");
  _invhist->addColumn(tr("User"),               _orderColumn, Qt::AlignCenter,true, "invhist_user");

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentIndex(0);
}

dspInventoryHistoryByOrderNumber::~dspInventoryHistoryByOrderNumber()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspInventoryHistoryByOrderNumber::languageChange()
{
  retranslateUi(this);
}

void dspInventoryHistoryByOrderNumber::setParams(ParameterList & params)
{
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("orderNumber", _orderNumber->text());
  params.append("transType", _transType->id());
}

void dspInventoryHistoryByOrderNumber::sPrint()
{
  if ( (!_dates->allValid()) || (_orderNumber->text().trimmed().length() == 0) )
  {
    QMessageBox::warning( this, tr("Invalid Data"),
                          tr("You must enter an Order Number along with a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  params.append("orderNumber", _orderNumber->text().trimmed());
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("transType", _transType->id());

  orReport report("InventoryHistoryByOrderNumber", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspInventoryHistoryByOrderNumber::sViewTransInfo()
{
  QString transtype(((XTreeWidgetItem *)_invhist->currentItem())->text(_invhist->column("invhist_transtype")));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", _invhist->id());

  if (transtype == "AD")
  {
    adjustmentTrans *newdlg = new adjustmentTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "TW")
  {
    transferTrans *newdlg = new transferTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "SI")
  {
    scrapTrans *newdlg = new scrapTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "EX")
  {
    expenseTrans *newdlg = new expenseTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "RX")
  {
    materialReceiptTrans *newdlg = new materialReceiptTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transtype == "CC")
  {
    countTag newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
  else
  {
    transactionInformation newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspInventoryHistoryByOrderNumber::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *)
{
  menuThis->insertItem(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()), 0);
}

void dspInventoryHistoryByOrderNumber::sFillList()
{
  _invhist->clear();

  if (_orderNumber->text().trimmed().length() == 0)
  {
    QMessageBox::critical( this, tr("Enter Order Search Pattern"),
                           tr("You must enter a Order # pattern to search for." ) );
    _orderNumber->setFocus();
    return;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter an End Date.") );
    _dates->setFocus();
    return;
  }

  if (_orderNumber->text().trimmed().length())
  {
    ParameterList params;
    setParams(params);
    MetaSQLQuery mql = mqlLoad("inventoryHistoryByOrderNumber", "detail");

    q = mql.toQuery(params);
    _invhist->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
