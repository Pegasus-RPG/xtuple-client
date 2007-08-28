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

#include "dspPOsByVendor.h"

#include <QVariant>
#include <QWorkspace>
#include <QMessageBox>
#include <QSqlError>
#include <openreports.h>
#include "purchaseOrder.h"
#include "mqlutil.h"

/*
 *  Constructs a dspPOsByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPOsByVendor::dspPOsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_poitem, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*)));

  _poitem->addColumn(tr("P/O #"),       _orderColumn, Qt::AlignRight  );
  _poitem->addColumn(tr("Whs."),        _whsColumn,   Qt::AlignCenter );
  _poitem->addColumn(tr("Status"),      _dateColumn,  Qt::AlignCenter );
  _poitem->addColumn(tr("Vendor"),      -1,           Qt::AlignLeft   );
  _poitem->addColumn(tr("Date"),        _dateColumn,  Qt::AlignCenter );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPOsByVendor::~dspPOsByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPOsByVendor::languageChange()
{
  retranslateUi(this);
}

void dspPOsByVendor::sPrint()
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

  if(_selectedVendor->isChecked())
    params.append("vend_id", _vend->id());

  if(_showClosed->isChecked())
    params.append("showClosed");

  if(_byReceiptDate->isChecked())
    params.append("byReceiptDate");
  else if(_byDueDate->isChecked())
    params.append("byDueDate");
  else //if(_byOrderDate->isChecked())
    params.append("byOrderDate");

  if(_searchDescrip->isChecked())
    params.append("descrip_pattern", _descrip->text());

  orReport report("POsByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPOsByVendor::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *pSelected)
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

void dspPOsByVendor::sEditOrder()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByVendor::sViewOrder()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pohead_id", _poitem->id());

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspPOsByVendor::sFillList()
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

  MetaSQLQuery mql = mqlLoad(":/po/displays/POsByVendor/FillListDetail.mql");

  ParameterList params;
  _dates->appendValue(params);
  _warehouse->appendValue(params);

  if(_selectedVendor->isChecked())
    params.append("vend_id", _vend->id());

  if(_showClosed->isChecked())
    params.append("showClosed");

  if(_byReceiptDate->isChecked())
    params.append("byReceiptDate");
  else if(_byDueDate->isChecked())
    params.append("byDueDate");
  else //if(_byOrderDate->isChecked())
    params.append("byOrderDate");

  if(_searchDescrip->isChecked())
    params.append("descrip_pattern", _descrip->text());

  params.append("closed", tr("Closed"));
  params.append("unposted", tr("Unposted"));
  params.append("partial", tr("Partial"));
  params.append("received", tr("Received"));
  params.append("open", tr("Open"));

  q = mql.toQuery(params);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  _poitem->clear();
  if (q.first())
  {
    do
    {
      XListViewItem *last = new XListViewItem( _poitem, _poitem->lastItem(),
                                               q.value("pohead_id").toInt(), -1,
                                               q.value("pohead_number"), q.value("warehousecode"),
                                               q.value("poitemstatus"), q.value("vend_number"),
                                               q.value("f_date"));
      if (q.value("late").toBool())
        last->setColor(4, "red");
    }
    while (q.next());
  }
}
