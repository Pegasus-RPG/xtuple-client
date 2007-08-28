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

#include "dspVendorAPHistory.h"

#include <QVariant>
#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QSqlError>
#include <Q3PopupMenu>
#include "apOpenItem.h"
#include "rptVendorAPHistory.h"

/*
 *  Constructs a dspVendorAPHistory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspVendorAPHistory::dspVendorAPHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_vendhist, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*,Q3ListViewItem*)));
  connect(_searchInvoiceNum, SIGNAL(textChanged(const QString&)), this, SLOT(sSearchInvoiceNum()));

  statusBar()->hide();

  _vendhist->setRootIsDecorated(TRUE);
  _vendhist->addColumn(tr("Open"),      _dateColumn,  Qt::AlignCenter );
  _vendhist->addColumn(tr("Doc. Type"), _itemColumn,  Qt::AlignCenter );
  _vendhist->addColumn(tr("Doc. #"),    _orderColumn, Qt::AlignRight  );
  _vendhist->addColumn(tr("Invoice #"),    _orderColumn, Qt::AlignRight  );
  _vendhist->addColumn(tr("Doc. Date"), _dateColumn,  Qt::AlignCenter );
  _vendhist->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignCenter );
  _vendhist->addColumn(tr("Amount"),    _moneyColumn, Qt::AlignRight  );
  _vendhist->addColumn(tr("Balance"),   _moneyColumn, Qt::AlignRight  );

  _vend->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspVendorAPHistory::~dspVendorAPHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspVendorAPHistory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspVendorAPHistory::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
    _vend->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspVendorAPHistory::sPopulateMenu(Q3PopupMenu *pMenu, Q3ListViewItem *pSelected)
{
  int menuItem;

  XListViewItem * item = (XListViewItem*)pSelected;
  if (item->id() != -1)
  {
    menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
    if (!_privleges->check("EditSalesHistory"))
      pMenu->setItemEnabled(menuItem, FALSE);

    pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

    if(item->altId() == -1 && item->text(1)==tr("Voucher") && item->text(6)==item->text(7))
    {
      menuItem = pMenu->insertItem(tr("Void"), this, SLOT(sVoidVoucher()), 0);
      if (!_privleges->check("MaintainAPMemos"))
        pMenu->setItemEnabled(menuItem, FALSE);
    } 
  }
}

void dspVendorAPHistory::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("apopen_id", _vendhist->id());

  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  sFillList();
}

void dspVendorAPHistory::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", _vendhist->id());

  apOpenItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspVendorAPHistory::sPrint()
{
  ParameterList params;
  _dates->appendValue(params);
  params.append("vend_id", _vend->id());
  params.append("print");

  rptVendorAPHistory newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspVendorAPHistory::sFillList()
{
  _vendhist->clear();

  if (!checkParameters())
    return;

  q.prepare( "SELECT 1 AS type, apopen_id, -1 AS applyid,"
             "       apopen_docdate AS sortdate, apopen_docnumber AS sortnumber,"
             "       apopen_docnumber AS docnumber,"
             "       formatBoolYN(apopen_open) AS f_open,"
             "       CASE WHEN (apopen_doctype='V') THEN :voucher"
             "            WHEN (apopen_doctype='C') THEN :creditMemo"
             "            WHEN (apopen_doctype='D') THEN :debitMemo"
             "            ELSE :other"
             "       END AS documenttype,"
             "       apopen_invcnumber AS invoicenumber,"
             "       formatDate(apopen_docdate) AS f_docdate,"
             "       formatDate(apopen_duedate) AS f_duedate,"
             "       formatMoney(apopen_amount) AS f_amount,"
             "       formatMoney((apopen_amount - apopen_paid)) AS f_balance "
             "FROM apopen "
             "WHERE ( (apopen_vend_id=:vend_id)" 
             " AND (apopen_docdate BETWEEN :startDate AND :endDate) ) "

             "UNION "
             "SELECT 2 AS type, apopen_id, apapply_source_apopen_id AS applyid,"
             "       apopen_docdate AS sortdate, apopen_docnumber AS sortnumber,"
             "       apapply_source_docnumber AS docnumber,"
             "       '' AS f_open,"
             "       CASE WHEN (apapply_source_doctype='C') THEN :creditMemo"
             "            WHEN (apapply_source_doctype='K') THEN :check"
             "            ELSE :other"
             "       END AS documenttype,"
             "       ' ' AS invoicenumber,"
             "       formatDate(apapply_postdate) AS f_docdate,"
             "       '' AS f_duedate,"
             "       formatMoney(apapply_amount) AS f_amount,"
             "       '' AS f_balance "
             "FROM apapply, apopen "
             "WHERE ( (apapply_target_apopen_id=apopen_id)"
             " AND (apapply_vend_id=:vend_id)"
             " AND (apopen_vend_id=:vend_id)"
             " AND (apopen_docdate BETWEEN :startDate AND :endDate) ) "

             "UNION "
             "SELECT 3 AS type, apopen_id, apapply_target_apopen_id AS applyid,"
             "       apopen_docdate AS sortdate, apopen_docnumber AS sortnumber,"
             "       apapply_target_docnumber AS docnumber,"
             "       '' AS f_open,"
             "       CASE WHEN (apapply_target_doctype='V') THEN :voucher"
             "            WHEN (apapply_target_doctype='D') THEN :debitMemo"
             "            ELSE :other"
             "       END AS documenttype,"
             "       apopen_invcnumber AS invoicenumber,"
             "       formatDate(apapply_postdate) AS f_docdate,"
             "       '' AS f_duedate,"
             "       formatMoney(apapply_amount) AS f_amount,"
             "       '' AS f_balance "
             "FROM apapply, apopen "
             "WHERE ( (apapply_source_doctype IN ('C', 'K'))"
             " AND (apapply_source_doctype=apopen_doctype)"
             " AND (apapply_source_docnumber=apopen_docnumber)"
             " AND (apapply_vend_id=:vend_id)"
             " AND (apopen_vend_id=:vend_id)"
             " AND (apopen_docdate BETWEEN :startDate AND :endDate) ) "

             "ORDER BY sortdate, apopen_id, type;" );

  _dates->bindValue(q);
  q.bindValue(":creditMemo", tr("C/M"));
  q.bindValue(":debitMemo", tr("D/M"));
  q.bindValue(":check", tr("Check"));
  q.bindValue(":voucher", tr("Voucher"));
  q.bindValue(":other", tr("Other"));
  q.bindValue(":vend_id", _vend->id());
  q.exec();
  if (q.first())
  {
    XListViewItem *document = NULL;
    do
    {
      if (q.value("type").toInt() == 1)
        document = new XListViewItem( _vendhist, _vendhist->lastItem(),
                                      q.value("apopen_id").toInt(), q.value("applyid").toInt(),
                                      q.value("f_open"), q.value("documenttype"),
                                      q.value("docnumber"), q.value("invoicenumber"),q.value("f_docdate"),
                                      q.value("f_duedate"), q.value("f_amount"),
                                      q.value("f_balance") );
      else if (document)
      {
        new XListViewItem( document,
                           -1, q.value("applyid").toInt(),
                           "", q.value("documenttype"),
                           q.value("docnumber"),q.value("invoicenumber"), q.value("f_docdate"),
                           "", q.value("f_amount") );
      }
    }
    while (q.next());
  }
  sSearchInvoiceNum();
}

bool dspVendorAPHistory::checkParameters()
{
  if (isVisible())
  {
    if (!_vend->isValid())
    {
      QMessageBox::warning( this, tr("Select Number"),
                            tr("Please select a valid Vendor.") );
      _vend->setFocus();
      return FALSE;
    }

    if (!_dates->startDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter Start Date"),
                            tr("Please enter a valid Start Date.") );
      _dates->setFocus();
      return FALSE;
    }

    if (!_dates->endDate().isValid())
    {
      QMessageBox::warning( this, tr("Enter End Date"),
                            tr("Please enter a valid End Date.") );
      _dates->setFocus();
      return FALSE;
    }
  }

  return TRUE;
}


void dspVendorAPHistory::sSearchInvoiceNum()
{
  QString sub = _searchInvoiceNum->text().stripWhiteSpace();
  if(sub.isEmpty())
    return;

  XListViewItem * item = 0;
  XListViewItem * foundSub = 0;
  for(item = _vendhist->firstChild(); item; item = item->nextSibling())
  {
    if(item->text(3) == sub)
    {
      _vendhist->setSelected(item, true);
      _vendhist->ensureItemVisible(item);
      return;
    }
    if(foundSub==0 && item->text(3).startsWith(sub))
      foundSub = item;
  }
  if(foundSub)
  {
    _vendhist->setSelected(foundSub, true);
    _vendhist->ensureItemVisible(foundSub);
  }
}

void dspVendorAPHistory::sVoidVoucher()
{
  q.prepare("SELECT voidApopenVoucher(:apopen_id) AS result;");
  q.bindValue(":apopen_id", _vendhist->id());
  q.exec();

  if(q.first())
  {
    if(q.value("result").toInt() < 0)
      systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );
    else
      sFillList();
  }
  else
    systemError( this, q.lastError().databaseText(), __FILE__, __LINE__);

}

