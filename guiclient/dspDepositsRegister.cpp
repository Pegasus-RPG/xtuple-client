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

#include "dspDepositsRegister.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMenu>
#include <QMessageBox>
#include <openreports.h>

/*
 *  Constructs a dspDepositsRegister as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspDepositsRegister::dspDepositsRegister(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _gltrans->addColumn(tr("Date"),             _dateColumn,    Qt::AlignCenter, true,  "gltrans_date" );
  _gltrans->addColumn(tr("Source"),           _orderColumn,   Qt::AlignCenter, true,  "gltrans_source" );
  _gltrans->addColumn(tr("Doc Type"),         _orderColumn,   Qt::AlignLeft,   true,  "doctype"   );
  _gltrans->addColumn(tr("Doc. #"),           _orderColumn,   Qt::AlignCenter, true,  "gltrans_docnumber" );
  _gltrans->addColumn(tr("Reference"),        -1,             Qt::AlignLeft,   true,  "f_notes"   );
  _gltrans->addColumn(tr("Account"),          _itemColumn,    Qt::AlignLeft,   true,  "f_accnt"   );
  _gltrans->addColumn(tr("Amount Rcv'd"),     _moneyColumn,   Qt::AlignRight,  true,  "debit"  );
  _gltrans->addColumn(tr("Credit to A/R"),    _moneyColumn,   Qt::AlignRight,  true,  "credit"  );
  _gltrans->addColumn(tr("Balance"),          _moneyColumn,   Qt::AlignRight,  true,  "balance"  );
  _gltrans->addColumn(tr("Currency"),      _currencyColumn,    Qt::AlignCenter, true,  "currAbbr"  );
  _gltrans->addColumn(tr("Base Balance"),  _bigMoneyColumn,   Qt::AlignRight,  false, "base_balance"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspDepositsRegister::~dspDepositsRegister()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspDepositsRegister::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspDepositsRegister::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

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

void dspDepositsRegister::sPopulateMenu(QMenu *)
{
}

void dspDepositsRegister::sPrint()
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date(s)"),
      tr("You must specify a valid date range.") );
    return;
  }

  ParameterList params;
  _dates->appendValue(params);

  orReport report("DepositsRegister", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspDepositsRegister::sFillList()
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date(s)"),
      tr("You must specify a valid date range.") );
    return;
  }

  _gltrans->clear();

  QString sql( "SELECT gltrans_id, gltrans_date, gltrans_source,"
               "       CASE WHEN(gltrans_doctype='IN') THEN :invoice"
               "            WHEN(gltrans_doctype='CM') THEN :creditmemo"
               "            ELSE gltrans_doctype"
               "       END AS doctype,"
               "       gltrans_docnumber, firstLine(gltrans_notes) AS f_notes,"
               "       (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS f_accnt,"
               "       CASE WHEN (gltrans_amount < 0) THEN formatMoney(ABS(gltrans_amount))"
               "            ELSE ''"
               "       END AS f_debit,"
               "       CASE WHEN (gltrans_amount < 0) THEN ABS(gltrans_amount)"
               "            ELSE 0"
               "       END AS debit,"
               "       CASE WHEN (gltrans_amount > 0) THEN gltrans_amount"
               "            ELSE 0"
               "       END AS credit,"
               "       (aropen_amount - aropen_paid) AS balance,"
               "       currtobase(aropen_curr_id,(aropen_amount - aropen_paid),aropen_docdate) AS base_balance,"
               "       currconcat(aropen_curr_id) AS currAbbr,"
               "       'curr' AS debit_xtnumericrole,"
               "       'curr' AS credit_xtnumericrole,"
               "       'curr' AS balance_xtnumericrole,"
               "       'curr' AS base_balance_xtnumericrole,"
               "       0 AS debit_xttotalrole,"
               "       0 AS credit_xttotalrole,"
               "       0 AS base_balance_xttotalrole "
               "FROM gltrans LEFT OUTER JOIN aropen ON ((text(gltrans_docnumber) = 'I-' || text(aropen_docnumber)) "
               "                                    AND (aropen_doctype='I')), "
               "     accnt "
               "WHERE ((gltrans_accnt_id=accnt_id)"
               " AND (gltrans_doctype = 'CR')"
               " AND (gltrans_date BETWEEN :startDate AND :endDate) ) "
               "ORDER BY gltrans_created DESC, gltrans_sequence, gltrans_amount;");

  q.prepare(sql);
  _dates->bindValue(q);
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":creditmemo", tr("Credit Memo"));
  q.exec();
  _gltrans->populate(q);
}

