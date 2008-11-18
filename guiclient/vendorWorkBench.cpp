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

#include "vendorWorkBench.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "crmaccount.h"
#include "dspAPApplications.h"
#include "dspPOsByVendor.h"
#include "dspPoItemReceivingsByVendor.h"
#include "selectPayments.h"
#include "unappliedAPCreditMemos.h"
#include "vendor.h"

vendorWorkBench::vendorWorkBench(QWidget* parent, const char *name, Qt::WFlags fl)
    : XWidget (parent, name, fl)
{
  setupUi(this);

  QWidget *hideme = 0;

  _po = new dspPOsByVendor(this, "dspPOsByVendor", Qt::Widget);
  _poTab->layout()->addWidget(_po);
  hideme = _po->findChild<QWidget*>("_close");
  hideme->hide();
  hideme = _po->findChild<QWidget*>("_vendGroup");
  hideme->hide();
  QRadioButton *radiobutton = _po->findChild<QRadioButton*>("_selectedVendor");
  radiobutton->setChecked(true);
  VendorInfo *povend = _po->findChild<VendorInfo*>("_vend");
  _po->show();

  _receipts = new dspPoItemReceivingsByVendor(this, "dspPoItemReceivingsByVendor", Qt::Widget);
  _receiptsTab->layout()->addWidget(_receipts);
  hideme = _receipts->findChild<QWidget*>("_close");
  hideme->hide();
  VendorCluster *rcptvend = _receipts->findChild<VendorCluster*>("_vendor");
  rcptvend->hide();

  _payables = new selectPayments(this, "selectPayments", Qt::Widget);
  _payablesTab->layout()->addWidget(_payables);
  hideme = _payables->findChild<QWidget*>("_close");
  hideme->hide();
  VendorGroup *payvend = _payables->findChild<VendorGroup*>("_vendorgroup");
  payvend->setState(VendorGroup::Selected);
  payvend->hide();

  _credits = new unappliedAPCreditMemos(this, "unappliedAPCreditMemos", Qt::Widget);
  _cmTab->layout()->addWidget(_credits);
  hideme = _credits->findChild<QWidget*>("_close");
  hideme->hide();
  VendorGroup *cmvend = _credits->findChild<VendorGroup*>("_vendorgroup");
  cmvend->setState(VendorGroup::Selected);
  cmvend->hide();

  _applications = new dspAPApplications(this, "dspAPApplications", Qt::Widget);
  _applicationsTab->layout()->addWidget(_applications);
  hideme = _applications->findChild<QWidget*>("_close");
  hideme->hide();
  if (! _applications->findChild<QCheckBox*>("_showChecks")->isChecked() &&
      ! _applications->findChild<QCheckBox*>("_showChecks")->isChecked())
  {
    _applications->findChild<QCheckBox*>("_showChecks")->setChecked(true);
    _applications->findChild<QCheckBox*>("_showChecks")->setChecked(true);
  }
  VendorGroup *apvend = _applications->findChild<VendorGroup*>("_vendorgroup");
  apvend->setState(VendorGroup::Selected);
  apvend->hide();

  connect(_crmacct,     SIGNAL(clicked()), this,          SLOT(sCRMAccount()));
  connect(_edit,        SIGNAL(clicked()), this,          SLOT(sVendor()));
  connect(_print,       SIGNAL(clicked()), this,          SLOT(sPrint()));
  connect(_vend,       SIGNAL(newId(int)), apvend,        SLOT(setVendId(int)));
  connect(_vend,       SIGNAL(newId(int)), cmvend,        SLOT(setVendId(int)));
  connect(_vend,       SIGNAL(newId(int)), payvend,       SLOT(setVendId(int)));
  connect(_vend,       SIGNAL(newId(int)), povend,        SLOT(setId(int)));
  connect(_vend,       SIGNAL(newId(int)), rcptvend,      SLOT(setId(int)));
  connect(_vend,       SIGNAL(newId(int)), this,          SLOT(sPopulate()));
  connect(apvend,  SIGNAL(newVendId(int)), _applications, SLOT(sFillList()));
  connect(cmvend,  SIGNAL(newVendId(int)), _credits,      SLOT(sFillList()));
  connect(payvend, SIGNAL(newVendId(int)), _payables,     SLOT(sFillList()));
  connect(povend,      SIGNAL(newId(int)), _po,           SLOT(sFillList()));
  connect(rcptvend,    SIGNAL(newId(int)), _receipts,     SLOT(sFillList()));

  _edit->setText(_privileges->check("MaintainVendors") ? tr("Edit") : tr("View"));

  _backlog->setPrecision(omfgThis->moneyVal());
  _lastYearsPurchases->setPrecision(omfgThis->moneyVal());
  _ytdPurchases->setPrecision(omfgThis->moneyVal());
  _openBalance->setPrecision(omfgThis->moneyVal());

  clear();
}

vendorWorkBench::~vendorWorkBench()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendorWorkBench::languageChange()
{
  retranslateUi(this);
}

enum SetResponse vendorWorkBench::set(const ParameterList & pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _vend->setId(param.toInt());
    _vend->setReadOnly(TRUE);
  }

  return NoError;
}

bool vendorWorkBench::setParams(ParameterList &params)
{
  if(!_vend->isValid())
  {
    QMessageBox::warning(this, tr("No Vendor Selected"),
      tr("You must select a valid Vendor.") );
    _vend->setFocus();
    return false;
  }

  params.append("vend_id", _vend->id());

  return true;
}

void vendorWorkBench::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("VendorInformation", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void vendorWorkBench::clear()
{
  _name->setText("");
  _vendType->setId(-1);
  _terms->setId(-1);
  _shipvia->setId(-1);
  _active->setChecked(true);
  _primaryContact->setId(-1);
  _secondaryContact->setId(-1);
  _crmacctId = -1;
  _backlog->clear();
  _lastYearsPurchases->clear();
  _ytdPurchases->clear();
  _backlog->clear();
}

void vendorWorkBench::sCRMAccount()
{
  ParameterList params;
  if (!_privileges->check("MaintainCRMAccounts"))
    params.append("mode", "view");
  else
    params.append("mode", "edit");

  params.append("crmacct_id", _crmacctId);

  crmaccount *newdlg = new crmaccount(this, "crmaccount");
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendorWorkBench::sPopulate()
{
  ParameterList params;
  if (! setParams(params))
  {
    clear();
    return;
  }

  MetaSQLQuery mql("SELECT vend_name,      vend_vendtype_id, vend_terms_id,"
                   "       vend_shipvia,   vend_active,      vend_cntct1_id,"
                   "       vend_cntct2_id, crmacct_id,"
                   "       MIN(pohead_orderdate) AS minpodate, "
                   "       MAX(pohead_orderdate) AS maxpodate, "
                   "       SUM(currToBase(pohead_curr_id,"
                   "           (poitem_qty_ordered - poitem_qty_received) * poitem_unitprice,"
                   "           CURRENT_DATE)) AS backlog "
                   "FROM vendinfo JOIN crmacct ON (crmacct_vend_id=vend_id)"
                   "     LEFT OUTER JOIN pohead ON (pohead_vend_id=vend_id)"
                   "     LEFT OUTER JOIN poitem ON (poitem_pohead_id=pohead_id"
                   "                            AND poitem_status='O')"
                   "WHERE (vend_id=<? value(\"vend_id\") ?>) "
                   "GROUP BY vend_name,      vend_vendtype_id, vend_terms_id,"
                   "         vend_shipvia,   vend_active,      vend_cntct1_id,"
                   "         vend_cntct2_id, crmacct_id;");

  q = mql.toQuery(params);
  if (q.first())
  {
    _name->setText(q.value("vend_name").toString());
    _vendType->setId(q.value("vend_vendtype_id").toInt());
    _terms->setId(q.value("vend_terms_id").toInt());
    _shipvia->setText(q.value("vend_shipvia").toString());
    _active->setChecked(q.value("vend_active").toBool());
    _primaryContact->setId(q.value("vend_cntct1_id").toInt());
    _secondaryContact->setId(q.value("vend_cntct2_id").toInt());
    _crmacctId = q.value("crmacct_id").toInt();
    _firstPurchase->setDate(q.value("minpodate").toDate());
    _lastPurchase->setDate(q.value("maxpodate").toDate());
    _backlog->setDouble(q.value("backlog").toDouble());
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  MetaSQLQuery purchbydate("SELECT SUM(currToBase(vohead_curr_id,"
                           "             vohead_amount,"
                           "             vohead_gldistdate)) AS purchases "
                           "FROM vohead "
                           "WHERE (vohead_posted"
                           "  AND (vohead_gldistdate "
                           "       BETWEEN (<? literal(\"older\") ?>)"
                           "           AND (<? literal(\"younger\") ?>))"
                           "  AND (vohead_vend_id=<? value(\"vend_id\") ?>));");
  params.append("older",   "DATE_TRUNC('year', CURRENT_DATE) - INTERVAL '1 year'");
  params.append("younger", "DATE_TRUNC('year', CURRENT_DATE) - INTERVAL '1 day'");
  q = purchbydate.toQuery(params);
  if (q.first())
    _lastYearsPurchases->setDouble(q.value("purchases").toDouble());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  ParameterList ytdparams;
  ytdparams.append("vend_id", _vend->id());
  ytdparams.append("older",   "DATE_TRUNC('year', CURRENT_DATE)");
  ytdparams.append("younger", "CURRENT_DATE - INTERVAL '1 day'");
  q = purchbydate.toQuery(ytdparams);
  if (q.first())
    _ytdPurchases->setDouble(q.value("purchases").toDouble());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  MetaSQLQuery balm("SELECT SUM(vohead_amount) AS balance "
                    "FROM vohead "
                    "WHERE (NOT vohead_posted"
                    "   AND (vohead_vend_id=<? value(\"vend_id\") ?>));");
  q = balm.toQuery(params);
  if (q.first())
    _openBalance->setDouble(q.value("balance").toDouble());
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void vendorWorkBench::sVendor()
{
  ParameterList params;
  if (! setParams(params))
    return;

  params.append("mode", _privileges->check("MaintainVendors") ?  "edit" : "view");

  vendor *newdlg = new vendor(this, "vendor");
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}
