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

#include "dspGLTransactions.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include "voucher.h"
#include "invoice.h"
#include "purchaseOrder.h"
#include "glTransactionDetail.h"

/*
 *  Constructs a dspGLTransactions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspGLTransactions::dspGLTransactions(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_selectedSource, SIGNAL(toggled(bool)), _source, SLOT(setEnabled(bool)));
  connect(_selectedAccount, SIGNAL(toggled(bool)), _account, SLOT(setEnabled(bool)));
  connect(_showUsername, SIGNAL(toggled(bool)), this, SLOT(sShowUsername(bool)));

  _gltrans->addColumn(tr("Date"),      _dateColumn,    Qt::AlignCenter );
  _gltrans->addColumn(tr("Source"),    _orderColumn,   Qt::AlignCenter );
  _gltrans->addColumn(tr("Doc. Type"), _docTypeColumn, Qt::AlignCenter );
  _gltrans->addColumn(tr("Doc. #"),    _orderColumn,   Qt::AlignCenter );
  _gltrans->addColumn(tr("Reference"), -1,             Qt::AlignLeft   );
  _gltrans->addColumn(tr("Account"),   _itemColumn,    Qt::AlignLeft   );
  _gltrans->addColumn(tr("Debit"),     _moneyColumn,   Qt::AlignRight  );
  _gltrans->addColumn(tr("Credit"),    _moneyColumn,   Qt::AlignRight  );
  _gltrans->addColumn(tr("Posted"),    _ynColumn,      Qt::AlignCenter );
  _gltrans->addColumn(tr("Username"),  _userColumn,    Qt::AlignLeft );
  sShowUsername(_showUsername->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspGLTransactions::~dspGLTransactions()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspGLTransactions::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspGLTransactions::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
  {
    _selectedAccount->setChecked(TRUE);
    _account->setId(param.toInt());
  }

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("period_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT period_start, period_end "
               "FROM period "
               "WHERE (period_id=:period_id);" );
    q.bindValue(":period_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _dates->setStartDate(q.value("period_start").toDate());
      _dates->setEndDate(q.value("period_end").toDate());
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspGLTransactions::sPopulateMenu(QMenu * menuThis, QTreeWidgetItem* pItem)
{
  menuThis->insertItem(tr("View..."), this, SLOT(sViewTrans()), 0);

  XTreeWidgetItem * item = (XTreeWidgetItem*)pItem;
  if(0 == item)
    return;

  if(item->text(2) == "VO")
    menuThis->insertItem(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if(item->text(2) == "IN")
    menuThis->insertItem(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if(item->text(2) == "PO")
    menuThis->insertItem(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
}

void dspGLTransactions::sPrint()
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
      tr("You must specify a valid date range."));
    return;
  }

  ParameterList params;
  _dates->appendValue(params);

  if (_selectedAccount->isChecked())
    params.append("accnt_id", _account->id());

  if (_selectedSource->isChecked())
    params.append("source", _source->currentText());

  if (_showUsername->isChecked())
    params.append("showUsernames");

  orReport report("GLTransactions", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspGLTransactions::sFillList()
{
  _gltrans->clear();
  QString sql( "SELECT gltrans_id, formatDate(gltrans_date), gltrans_source,"
               "       gltrans_doctype,"
               "       CASE WHEN(gltrans_docnumber='Misc.' AND invhist_docnumber IS NOT NULL) THEN (gltrans_docnumber || ' - ' || invhist_docnumber)"
               "            ELSE gltrans_docnumber"
               "       END AS gltrans_docnumber,"
               "       firstLine(gltrans_notes),"
               "       (formatGLAccount(accnt_id) || ' - ' || accnt_descrip),"
               "       CASE WHEN (gltrans_amount < 0) THEN formatMoney(ABS(gltrans_amount))"
               "            ELSE ''"
               "       END,"
               "       CASE WHEN (gltrans_amount > 0) THEN formatMoney(gltrans_amount)"
               "            ELSE ''"
               "       END,"
               "       formatBoolYN(gltrans_posted),"
               "       gltrans_username "
               "FROM gltrans JOIN accnt ON (gltrans_accnt_id=accnt_id) "
               "     LEFT OUTER JOIN invhist ON (gltrans_misc_id=invhist_id AND gltrans_docnumber='Misc.') "
               "WHERE ((gltrans_date BETWEEN :startDate AND :endDate)" );

  if (_selectedAccount->isChecked())
    sql += " AND (gltrans_accnt_id=:accnt_id)";

  if (_selectedSource->isChecked())
    sql += " AND (gltrans_source=:source)";

  sql += ") "
         "ORDER BY gltrans_created DESC, gltrans_sequence, gltrans_amount;";

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":accnt_id", _account->id());
  q.bindValue(":source", _source->currentText());
  q.exec();
  _gltrans->populate(q);
}

void dspGLTransactions::sShowUsername( bool yes )
{
  _gltrans->setColumnHidden(9, !yes);
}

void dspGLTransactions::sViewTrans()
{
  ParameterList params;

  params.append("gltrans_id", _gltrans->id());

  glTransactionDetail newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspGLTransactions::sViewDocument()
{
  QTreeWidgetItem * item = (XTreeWidgetItem*)_gltrans->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  if(item->text(2) == "VO")
  {
    q.prepare("SELECT vohead_id"
              "  FROM vohead"
              " WHERE (vohead_number=:vohead_number)");
    q.bindValue(":vohead_number", item->text(3));
    q.exec();
    if(!q.first())
      return;

    params.append("vohead_id", q.value("vohead_id").toInt());
    params.append("mode", "view");

    voucher *newdlg = new voucher();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->text(2) == "IN")
  {
    q.prepare("SELECT invchead_id"
              "  FROM invchead"
              " WHERE (invchead_invcnumber=:invchead_invcnumber)");
    q.bindValue(":invchead_invcnumber", item->text(3));
    q.exec();
    if(!q.first())
      return;

    invoice::viewInvoice(q.value("invchead_id").toInt());
  }
  else if(item->text(2) == "PO")
  {
    q.prepare("SELECT pohead_id"
              "  FROM pohead"
              " WHERE (pohead_number=:pohead_number)");
    q.bindValue(":pohead_number", item->text(3));
    q.exec();
    if(!q.first())
      return;

    params.append("pohead_id", q.value("pohead_id").toInt());
    params.append("mode", "view");

    purchaseOrder *newdlg = new purchaseOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

