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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "dspDetailedInventoryHistoryByLotSerial.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "adjustmentTrans.h"
#include "transferTrans.h"
#include "scrapTrans.h"
#include "expenseTrans.h"
#include "materialReceiptTrans.h"
#include "countTag.h"

/*
 *  Constructs a dspDetailedInventoryHistoryByLotSerial as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspDetailedInventoryHistoryByLotSerial::dspDetailedInventoryHistoryByLotSerial(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_invhist, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _transType->append(cTransAll,       tr("All Transactions")       );
  _transType->append(cTransReceipts,  tr("Receipts")               );
  _transType->append(cTransIssues,    tr("Issues")                 );
  _transType->append(cTransShipments, tr("Shipments")              );
  _transType->append(cTransAdjCounts, tr("Adjustments and Counts") );
  _transType->append(cTransTransfers, tr("Transfers")              );
  _transType->append(cTransScraps,    tr("Scraps")                 );
  _transType->setCurrentItem(0);

  _invhist->addColumn(tr("Date"),         (_dateColumn + 30),  Qt::AlignRight  );
  _invhist->addColumn(tr("Type"),         _transColumn,        Qt::AlignCenter );
  _invhist->addColumn(tr("Order #"),      _orderColumn,        Qt::AlignLeft   );
  _invhist->addColumn(tr("Item Number"),  _itemColumn,         Qt::AlignLeft   );
  _invhist->addColumn(tr("Location"),     _dateColumn,         Qt::AlignLeft   );
  _invhist->addColumn(tr("Lot/Serial #"), -1,                  Qt::AlignLeft   );
  _invhist->addColumn(tr("UOM"),          _uomColumn,          Qt::AlignCenter );
  _invhist->addColumn(tr("Trans-Qty"),    _qtyColumn,          Qt::AlignRight  );
  _invhist->addColumn(tr("Qty. Before"),  _qtyColumn,          Qt::AlignRight  );
  _invhist->addColumn(tr("Qty. After"),   _qtyColumn,          Qt::AlignRight  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspDetailedInventoryHistoryByLotSerial::~dspDetailedInventoryHistoryByLotSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspDetailedInventoryHistoryByLotSerial::languageChange()
{
  retranslateUi(this);
}

void dspDetailedInventoryHistoryByLotSerial::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Start Date and End Date"),
                          tr("You must enter a valid Start Date and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);
  params.append("lotSerial", _lotSerial->text());
  params.append("transType", _transType->id());

  orReport report("DetailedInventoryHistoryByLotSerial", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspDetailedInventoryHistoryByLotSerial::sViewTransInfo()
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(1));

  ParameterList params;
  params.append("mode", "view");
  params.append("invhist_id", _invhist->id());

  if (transType == "AD")
  {
    adjustmentTrans *newdlg = new adjustmentTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "TW")
  {
    transferTrans *newdlg = new transferTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "SI")
  {
    scrapTrans *newdlg = new scrapTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "EX")
  {
    expenseTrans *newdlg = new expenseTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "RX")
  {
    materialReceiptTrans *newdlg = new materialReceiptTrans();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if (transType == "CC")
  {
    countTag newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

void dspDetailedInventoryHistoryByLotSerial::sPopulateMenu(QMenu *menuThis)
{
  QString transType(((XTreeWidgetItem *)_invhist->currentItem())->text(1));

  if ( (transType == "AD") ||
       (transType == "TW") ||
       (transType == "SI") ||
       (transType == "EX") ||
       (transType == "RX") ||
       (transType == "CC") )
    menuThis->insertItem(tr("View Transaction Information..."), this, SLOT(sViewTransInfo()), 0);
}

void dspDetailedInventoryHistoryByLotSerial::sFillList()
{
  _invhist->clear();

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return;
  }

  if (_lotSerial->text().stripWhiteSpace().length() == 0)
  {
    QMessageBox::warning( this, tr("Enter Lot/Serial #"),
                          tr("<p>You must enter a Lot/Serial to view Inventory "
			     "Detail by Lot/Serial #.</p>") );
    _lotSerial->setFocus();
    return;
  }

  q.prepare( "SELECT invhist_id,"
             "       formatDateTime(invhist_transdate) AS transdate,"
             "       invhist_transtype, (invhist_ordtype || '-' || invhist_ordnumber) AS ordernumber,"
             "       invhist_invuom,"
             "       item_number, invdetail_lotserial,"
             "       CASE WHEN (invdetail_location_id=-1) THEN :undefined"
             "            ELSE formatLocationName(invdetail_location_id)"
             "       END AS locationname,"
             "       formatQty(invdetail_qty) AS transqty,"
             "       formatQty(invdetail_qty_before) AS qohbefore,"
             "       formatQty(invdetail_qty_after) AS qohafter,"
             "       invhist_posted "
             "FROM invdetail, invhist, itemsite, item "
             "WHERE ( (invdetail_invhist_id=invhist_id)"
             " AND (invhist_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (invdetail_lotserial ~ :lotserial)"
             " AND (DATE(invhist_transdate) BETWEEN :startDate AND :endDate)"
             " AND (transType(invhist_transtype, :transType)) ) "
             "ORDER BY invhist_transdate DESC, invhist_transtype;" );
  _dates->bindValue(q);
  q.bindValue(":undefined", tr("Undefined"));
  q.bindValue(":lotserial", _lotSerial->text());
  q.bindValue(":transType", _transType->id());
  q.exec();

  XTreeWidgetItem *last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _invhist, last, q.value("invhist_id").toInt(),
			     q.value("transdate"), q.value("invhist_transtype"),
			     q.value("ordernumber"), q.value("item_number"),
			     q.value("locationname"), q.value("invdetail_lotserial"),
			     q.value("invhist_invuom"), q.value("transqty") );

    if (q.value("invhist_posted").toBool())
    {
      last->setText(8, q.value("qohbefore").toString());
      last->setText(9, q.value("qohafter").toString());
    }
    else
      last->setTextColor("orange");
  }
}

