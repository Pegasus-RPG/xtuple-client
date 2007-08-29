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

#include "dspPOsByDate.h"

#include <QVariant>
#include <QWorkspace>
#include <QMessageBox>
#include <openreports.h>
#include "purchaseOrder.h"

/*
 *  Constructs a dspPOsByDate as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPOsByDate::dspPOsByDate(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_poitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Due Date:"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Due Date:"));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());

  _poitem->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight  );
  _poitem->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
  _poitem->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter );
  _poitem->addColumn(tr("Vendor"),      _itemColumn,  Qt::AlignLeft   );
  _poitem->addColumn(tr("Due Date"),    _dateColumn,  Qt::AlignCenter );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPOsByDate::~dspPOsByDate()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPOsByDate::languageChange()
{
  retranslateUi(this);
}

void dspPOsByDate::sPrint()
{
  if (!_dates->startDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr( "Please enter a valid Start Date." ) );
    _dates->setFocus();
    return;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr( "Please eneter a valid End Date." ) );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);
  _warehouse->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  if (_showClosed->isChecked())
    params.append("showClosed");

  orReport report("POsByDate", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPOsByDate::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  int menuItem;

  if (pSelected->text(2) == "U")
  {
    menuItem = pMenu->insertItem(tr("Edit Order..."), this, SLOT(sEditOrder()), 0);
    if (!_privleges->check("MaintainPurchaseOrders"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }

  menuItem = pMenu->insertItem(tr("View Order..."), this, SLOT(sViewOrder()), 0);
  if ((!_privleges->check("MaintainPurchaseOrders")) && (!_privleges->check("ViewPurchaseOrders")))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspPOsByDate::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByDate::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByDate::sFillList()
{
  _poitem->clear();

  QString sql( "SELECT pohead_id, pohead_number,"
               "       warehous_code AS warehousecode,"
               "       CASE WHEN(poitem_status='C') THEN :closed"
               "            WHEN(poitem_status='U') THEN :unposted"
               "            WHEN(poitem_status='O' AND (SUM(poitem_qty_received-poitem_qty_returned) > 0) AND (SUM(poitem_qty_ordered)>SUM(poitem_qty_received-poitem_qty_returned))) THEN :partial"
               "            WHEN(poitem_status='O' AND (SUM(poitem_qty_received-poitem_qty_returned) > 0) AND (SUM(poitem_qty_ordered)=SUM(poitem_qty_received-poitem_qty_returned))) THEN :received"
               "            WHEN(poitem_status='O') THEN :open"
               "            ELSE poitem_status"
               "       END AS poitemstatus,"
               "       vend_name,"
               "       formatDate(MIN(poitem_duedate)) AS f_duedate,"
               "       MIN(poitem_duedate) AS minDueDate,"
               "       (MIN(poitem_duedate) < CURRENT_DATE) AS late "
               "  FROM vend, poitem, pohead "
               "       LEFT OUTER JOIN warehous ON (pohead_warehous_id=warehous_id)"
               " WHERE ((poitem_pohead_id=pohead_id)"
               "   AND  (pohead_vend_id=vend_id)"
               "   AND  (poitem_duedate BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (pohead_warehous_id=:warehous_id) ";

  if (!_showClosed->isChecked())
    sql += " AND (poitem_status!='C')";

  if (_selectedPurchasingAgent->isChecked())
    sql += " AND (pohead_agent_username=:username)";

  sql += ") "
         "GROUP BY pohead_id, pohead_number, warehous_code,"
         "         poitem_status, vend_name "
         "ORDER BY minDueDate ";

  q.prepare(sql);
  _dates->bindValue(q);
  _warehouse->bindValue(q);
  q.bindValue(":username", _agent->currentText());
  q.bindValue(":closed", tr("Closed"));
  q.bindValue(":unposted", tr("Unposted"));
  q.bindValue(":partial", tr("Partial"));
  q.bindValue(":received", tr("Received"));
  q.bindValue(":open", tr("Open"));
  q.exec();
  XTreeWidgetItem *last = 0;
  while (q.next())
  {
    last = new XTreeWidgetItem( _poitem, last,
			       q.value("pohead_id").toInt(), -1,
			       q.value("pohead_number"),
			       q.value("warehousecode"),
			       q.value("poitemstatus"),
			       q.value("vend_name"),
			       q.value("f_duedate"));
    if (q.value("late").toBool())
      last->setTextColor(4, "red");
  }
}
