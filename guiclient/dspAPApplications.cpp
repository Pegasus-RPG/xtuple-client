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

#include "dspAPApplications.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>
#include <parameter.h>
#include <openreports.h>

#include "apOpenItem.h"
#include "mqlutil.h"
#include "voucher.h"

dspAPApplications::dspAPApplications(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_apapply,	SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)),
                  this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print,	SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query,	SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), true);
  _dates->setEndNull(tr("Latest"),     omfgThis->endOfTime(),   true);
    
  _apapply->addColumn(tr("Vend. #"),    _orderColumn, Qt::AlignLeft,  true, "vend_number");
  _apapply->addColumn(tr("Vendor"),               -1, Qt::AlignLeft,  true, "vend_name");
  _apapply->addColumn(tr("Post Date"),   _dateColumn, Qt::AlignCenter,true, "apapply_postdate");
  _apapply->addColumn(tr("Source"),      _itemColumn, Qt::AlignCenter,true, "apapply_source_doctype");
  _apapply->addColumn(tr("Doc #"),      _orderColumn, Qt::AlignRight, true, "apapply_source_docnumber");
  _apapply->addColumn(tr("Apply-To"),    _itemColumn, Qt::AlignCenter,true, "apapply_target_doctype");
  _apapply->addColumn(tr("Doc #"),      _orderColumn, Qt::AlignRight, true, "apapply_target_docnumber");
  _apapply->addColumn(tr("Amount"),     _moneyColumn, Qt::AlignRight, true, "apapply_amount");
  _apapply->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "currAbbr");
  _apapply->addColumn(tr("Amount (in %1)").arg(CurrDisplay::baseCurrAbbr()),_moneyColumn, Qt::AlignRight, true, "base_applied");

  _vendorgroup->setFocus();
}

dspAPApplications::~dspAPApplications()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspAPApplications::languageChange()
{
  retranslateUi(this);
}

void dspAPApplications::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("APApplications", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspAPApplications::sViewCreditMemo()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", _apapply->id("apapply_source_docnumber"));
  params.append("docType",   "creditMemo");
  apOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspAPApplications::sViewDebitMemo()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("apopen_id", _apapply->id("apapply_target_docnumber"));
  params.append("docType", "debitMemo");
  apOpenItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void dspAPApplications::sViewVoucher()
{
  ParameterList params;
  params.append("mode",      "view");
  params.append("vohead_id", _apapply->id("apapply_target_docnumber"));
  voucher *newdlg = new voucher(this, "voucher");
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspAPApplications::sPopulateMenu(QMenu* pMenu)
{
  int menuItem;

  if (_apapply->currentItem()->rawValue("apapply_source_doctype") == "C")
  {
    menuItem = pMenu->insertItem(tr("View Source Credit Memo..."), this, SLOT(sViewCreditMemo()), 0);
    pMenu->setItemEnabled(menuItem,
                          _privileges->check("MaintainAPMemos") ||
                          _privileges->check("ViewAPMemos"));
  }

  if (_apapply->currentItem()->rawValue("apapply_target_doctype") == "D")
  {
    menuItem = pMenu->insertItem(tr("View Apply-To Debit Memo..."), this, SLOT(sViewDebitMemo()), 0);
    pMenu->setItemEnabled(menuItem,
                          _privileges->check("MaintainAPMemos") ||
                          _privileges->check("ViewAPMemos"));
  }
  else if (_apapply->currentItem()->rawValue("apapply_target_doctype") == "V")
  {
    menuItem = pMenu->insertItem(tr("View Apply-To Voucher..."), this, SLOT(sViewVoucher()), 0);
    pMenu->setItemEnabled(menuItem,
                          _privileges->check("MaintainVouchers") ||
                          _privileges->check("ViewVouchers"));
  }
}

void dspAPApplications::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("apApplications", "detail");
  q = mql.toQuery(params);
  if (q.first())
    _apapply->populate(q);
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool dspAPApplications::setParams(ParameterList & params)
{
  if (! _vendorgroup->isValid())
  {
    QMessageBox::warning( this, tr("Select Vendor"),
                          tr("You must select the Vendor(s) whose A/R Applications you wish to view.") );
    _vendorgroup->setFocus();
    return false;
  }

  if (!_dates->startDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter Start Date"),
                           tr("You must enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid())
  {
    QMessageBox::critical( this, tr("Enter End Date"),
                           tr("You must enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  if ( !_showChecks->isChecked() && !_showCreditMemos->isChecked())
  {
    if (windowFlags() & (Qt::Window | Qt::Dialog))
      QMessageBox::critical( this, tr("Select Document Type"),
                             tr("You must indicate which Document Type(s) you wish to view.") );
    _showChecks->setFocus();
    return false;
  }
  
  if (_showChecks->isChecked() && _showCreditMemos->isChecked())
    params.append("doctypeList", "'C', 'K'");
  else if (_showChecks->isChecked())
    params.append("doctypeList", "'K'");
  else if (_showCreditMemos->isChecked())
    params.append("doctypeList", "'C'");

  _dates->appendValue(params);
  params.append("creditMemo", tr("C/M"));
  params.append("debitMemo",  tr("D/M"));
  params.append("check",      tr("Check"));
  params.append("voucher",    tr("Voucher"));

  _vendorgroup->appendValue(params);

  return true;
}
