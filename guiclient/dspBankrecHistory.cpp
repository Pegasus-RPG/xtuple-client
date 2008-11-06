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

#include "dspBankrecHistory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>
#include "guiclient.h"

dspBankrecHistory::dspBankrecHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  _bankaccnt->populate("SELECT bankaccnt_id,"
                       "       (bankaccnt_name || '-' || bankaccnt_descrip) "
                       "FROM bankaccnt "
                       "ORDER BY bankaccnt_name;");

  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sBankaccntChanged()));
  connect(_bankrec,   SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_print,      SIGNAL(clicked()), this, SLOT(sPrint()));

  _details->addColumn(tr("Date"),       _dateColumn, Qt::AlignCenter,true, "gltrans_date");
  _details->addColumn(tr("Doc Number/Notes"),    -1, Qt::AlignLeft,  true, "gltrans_docnumber");
  _details->addColumn(tr("Amount"), _bigMoneyColumn, Qt::AlignRight, true, "amount");

  sBankaccntChanged();
}

dspBankrecHistory::~dspBankrecHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBankrecHistory::languageChange()
{
  retranslateUi(this);
}

void dspBankrecHistory::sPrint()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id());
  params.append("bankrec_id", _bankrec->id());

  orReport report("BankrecHistory", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBankrecHistory::sBankaccntChanged()
{
  XSqlQuery bq;
  bq.prepare( "SELECT bankrec_id, (formatDate(bankrec_opendate) || '-' || formatDate(bankrec_enddate)) "
             "FROM bankrec "
             "WHERE (bankrec_bankaccnt_id=:bankaccnt_id) "
             "ORDER BY bankrec_opendate, bankrec_enddate; ");
  bq.bindValue(":bankaccnt_id", _bankaccnt->id());
  bq.exec();
  _bankrec->populate(bq);

  sFillList();
}

void dspBankrecHistory::sFillList()
{
  q.prepare( "SELECT bankrec_username, bankrec_created,"
             "       bankrec_opendate,"
             "       bankrec_enddate,"
             "       bankrec_openbal,"
             "       bankrec_endbal "
             "FROM bankrec "
             "WHERE (bankrec_id=:bankrecid);" );
  q.bindValue(":bankrecid", _bankrec->id());
  q.exec();
  if(q.first())
  {
    _poster->setText(q.value("bankrec_username").toString());
    _postdate->setDate(q.value("bankrec_created").toDate());

    XSqlQuery brq;
    brq.prepare("SELECT -1 AS gltrans_id, 0 AS seq,"
                "       CAST(:opendate AS DATE) AS gltrans_date,"
                "       :opening AS gltrans_docnumber, :openbal AS amount,"
                "       'curr' AS amount_xtnumericrole "
                "UNION "
                "SELECT -1 AS gltrans_id, 2 AS seq,"
                "       CAST(:enddate AS DATE) AS gltrans_date,"
                "       :ending AS gltrans_docnumber, :endbal AS amount,"
                "       'curr' AS amount_xtnumericrole "
                "UNION "
                "SELECT gltrans_id, 1 AS seq, gltrans_date,"
                "       gltrans_docnumber,"
                "       (gltrans_amount * -1) AS amount,"
                "       'curr' AS amount_xtnumericrole "
                "FROM gltrans, bankrecitem "
                "WHERE ((bankrecitem_bankrec_id=:bankrecid)"
                "  AND (bankrecitem_source='GL')"
                "  AND (bankrecitem_source_id=gltrans_id) ) "
                "ORDER BY seq, gltrans_date, gltrans_id;" );
    brq.bindValue(":bankrecid", _bankrec->id());
    brq.bindValue(":opendate", q.value("bankrec_opendate").toDate());
    brq.bindValue(":openbal",  q.value("bankrec_openbal").toDouble());
    brq.bindValue(":opening",  tr("Opening Balance"));
    brq.bindValue(":enddate",  q.value("bankrec_enddate").toDate());
    brq.bindValue(":endbal",   q.value("bankrec_endbal").toDouble());
    brq.bindValue(":ending",   tr("Ending Balance"));
    brq.exec();
    _details->populate(brq);
    if (brq.lastError().type() != QSqlError::NoError)
    {
      systemError(this, brq.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
