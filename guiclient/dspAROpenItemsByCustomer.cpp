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

#include "dspAROpenItemsByCustomer.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include "arOpenItem.h"
#include "dspInvoiceInformation.h"
#include "invoice.h"
#include "incident.h"

/*
 *  Constructs a dspAROpenItemsByCustomer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspAROpenItemsByCustomer::dspAROpenItemsByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_cust, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _aropen->setRootIsDecorated(TRUE);
  _aropen->addColumn(tr("Doc. Type"),      _itemColumn,     Qt::AlignLeft );
  _aropen->addColumn(tr("Doc. #"),         _orderColumn,    Qt::AlignLeft  );
  _aropen->addColumn(tr("Order/Incdt."),   _itemColumn,     Qt::AlignLeft  );
  _aropen->addColumn(tr("Doc. Date"),      _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Due Date"),       _dateColumn,     Qt::AlignCenter );
  _aropen->addColumn(tr("Amount"),         _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Paid"),           _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Balance"),        _bigMoneyColumn, Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),       _currencyColumn, Qt::AlignLeft   );
  _aropen->addColumn(tr("Balance"),        _bigMoneyColumn, Qt::AlignRight  );

  if (omfgThis->singleCurrency())
  {
    _aropen->hideColumn(7);
    _aropen->hideColumn(8);
  }
  else
  {
    q.prepare("SELECT currConcat(baseCurrId()) AS currConcat;");
    q.exec();
    QString currConcat;
    if (q.first())
      currConcat = q.value("currConcat").toString();
    else
      currConcat = tr("?????");
    _aropen->headerItem()->setText(9, tr("Balance\n(in %1)").arg(currConcat));
  }

  _asOf->setDate(omfgThis->dbDate(), true);
  _cust->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspAROpenItemsByCustomer::~dspAROpenItemsByCustomer()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspAROpenItemsByCustomer::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspAROpenItemsByCustomer::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cust_id", &valid);
  if (valid)
    _cust->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());
    
  param = pParams.value("asofDate", &valid);
  if (valid)
    _asOf->setDate(param.toDate());
    _asOf->setEnabled(FALSE);

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}
 
void dspAROpenItemsByCustomer::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  int menuItem;

  if (((XTreeWidgetItem *)pItem)->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privileges->check("EditAROpenItem"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
    
    XTreeWidgetItem* item = (XTreeWidgetItem*)pItem;
    {
      if (item->text(0) == "Invoice")
      {
        pMenu->insertItem(tr("View Invoice..."), this, SLOT(sViewInvoice()), 0);
        pMenu->insertItem(tr("View Invoice Details..."), this, SLOT(sViewInvoiceDetails()), 0);
      }
    }

    menuItem = pMenu->insertItem(tr("Create Incident..."), this, SLOT(sIncident()), 0);
    if (!_privileges->check("AddIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
  else
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEditIncident()), 0);
    if (!_privileges->check("MaintainIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertItem(tr("View..."), this, SLOT(sViewIncident()), 0);
    if (!_privileges->check("ViewIncidents"))
      pMenu->setItemEnabled(menuItem, FALSE);
  }
}

void dspAROpenItemsByCustomer::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItemsByCustomer::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("aropen_id", _aropen->id());
  arOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItemsByCustomer::sViewInvoice()
{
  q.prepare("SELECT aropen_docnumber FROM aropen WHERE (aropen_id=:aropen_id);");
  q.bindValue(":aropen_id", _aropen->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("invoiceNumber", q.value("aropen_docnumber"));
    dspInvoiceInformation* newdlg = new dspInvoiceInformation();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAROpenItemsByCustomer::sViewInvoiceDetails()
{
  q.prepare("SELECT invchead_id FROM aropen, invchead WHERE ((aropen_id=:aropen_id) AND (invchead_invcnumber=aropen_docnumber));");
  q.bindValue(":aropen_id", _aropen->id());
  q.exec();
  if (q.first());
  {
    ParameterList params;
    params.append("invchead_id", q.value("invchead_id"));
    params.append("mode", "view");
    invoice* newdlg = new invoice();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspAROpenItemsByCustomer::sIncident()
{
  q.prepare("SELECT crmacct_id, crmacct_cntct_id_1 FROM crmacct WHERE (crmacct_cust_id=:cust_id);");
  q.bindValue(":cust_id", _cust->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("mode", "new");
    params.append("aropen_id", _aropen->id());
    params.append("crmacct_id", q.value("crmacct_id"));
    params.append("cntct_id", q.value("crmacct_cntct_id_1"));
    incident newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != XDialog::Rejected)
      sFillList();
  }
}

void dspAROpenItemsByCustomer::sEditIncident()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("incdt_id", _aropen->altId());
  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItemsByCustomer::sViewIncident()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("incdt_id", _aropen->altId());
  incident newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspAROpenItemsByCustomer::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("asofDate", _asOf->date());
  params.append("cust_id", _cust->id());

  orReport report("AROpenItemsByCustomer", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspAROpenItemsByCustomer::sFillList()
{
  _aropen->clear();
  
  XSqlQuery incident;
  incident.prepare( "SELECT incdt_id, incdt_number::TEXT AS incdtnumber "
                    "FROM incdt "
					"WHERE (incdt_aropen_id=:aropen_id);" );

  q.prepare( "SELECT aropen_id, aropen_docnumber, aropen_ordernumber,"
             "       CASE WHEN (aropen_doctype='C') THEN :creditMemo"
             "            WHEN (aropen_doctype='D') THEN :debitMemo"
             "            WHEN (aropen_doctype='I') THEN :invoice"
             "            WHEN (aropen_doctype='R') THEN :cashdeposit"
             "            ELSE :other"
             "       END AS f_doctype,"
             "       aropen_docdate, aropen_duedate, aropen_amount,"
             "       SUM(currtocurr(arapply_curr_id,aropen_curr_id,arapply_applied,aropen_docdate)) AS paid,"
             "       CASE WHEN (aropen_doctype IN ('C', 'R')) THEN ((aropen_amount - SUM(currtocurr(arapply_curr_id,aropen_curr_id,arapply_applied,aropen_docdate))) * -1)"
             "            ELSE (aropen_amount - SUM(currtocurr(arapply_curr_id,aropen_curr_id,arapply_applied,aropen_docdate)))"
             "       END AS balance,"
             "       currConcat(aropen_curr_id) AS currAbbr,"
             "       currToBase(aropen_curr_id,"
             "       CASE WHEN (aropen_doctype IN ('C', 'R')) THEN ((aropen_amount - SUM(currtocurr(arapply_curr_id,aropen_curr_id,arapply_applied,aropen_docdate))) * -1)"
             "            ELSE (aropen_amount - SUM(currtocurr(arapply_curr_id,aropen_curr_id,arapply_applied,aropen_docdate)))"
             "       END, aropen_docdate) AS base_balance "
             "  FROM aropen "
             "  LEFT OUTER JOIN arapply ON (((aropen_id=arapply_source_aropen_id) "
             "                             OR (aropen_id=arapply_target_aropen_id)) "
             "                             AND (arapply_distdate<=:asofdate)) "
             " WHERE ( (COALESCE(aropen_closedate,date :asofdate + integer '1')>:asofdate) "
             "   AND   (aropen_docdate<=:asofdate)"
             "   AND   (aropen_cust_id=:cust_id) "
             "   AND   (aropen_duedate BETWEEN :startDate AND :endDate)) "
             " GROUP BY aropen_id,aropen_docnumber,aropen_ordernumber,aropen_doctype,aropen_docdate,aropen_duedate,aropen_amount,aropen_curr_id,aropen_paid "
             " ORDER BY aropen_docdate;" );
  _dates->bindValue(q);
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":creditMemo", tr("Credit Memo"));
  q.bindValue(":debitMemo", tr("Debit Memo"));
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":cashdeposit", tr("Cash Deposit"));
  q.bindValue(":asofdate", _asOf->date());
  q.exec();
  if (q.first())
  {
    double total= 0.0;
    XTreeWidgetItem * last = 0;
    XTreeWidgetItem *document = 0;
    do
    {
      last = document = new XTreeWidgetItem( _aropen, last, q.value("aropen_id").toInt(), -1,
                                             q.value("f_doctype"), q.value("aropen_docnumber"),
                                             q.value("aropen_ordernumber"), formatDate(q.value("aropen_docdate").toDate()),
                                             formatDate(q.value("aropen_duedate").toDate()), formatMoney(q.value("aropen_amount").toDouble()),
                                             formatMoney(q.value("paid").toDouble()), formatMoney(q.value("balance").toDouble()),
                                             q.value("currAbbr"),
			                                       formatMoney(q.value("base_balance").toDouble()));
 
      total += q.value("base_balance").toDouble();

      incident.bindValue(":aropen_id", q.value("aropen_id").toInt());
	  incident.exec();
	  if (incident.first())
	  {
	    do
		{
          new XTreeWidgetItem( document, -1, incident.value("incdt_id").toInt(),
                               "", "",
                               incident.value("incdtnumber"), "",
                               "", "",
                               "", "",
		                       "",
			                   "");
        }
		while (incident.next());
      }
    }
    while (q.next());

    new XTreeWidgetItem( _aropen, last, -1,
                         QVariant(tr("Total")), "", "", "", "", "", "", "", "",
                         formatMoney(total) );
  }
}

