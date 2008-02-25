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

#include "applyAPCreditMemo.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include "apCreditMemoApplication.h"

/*
 *  Constructs a applyAPCreditMemo as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
applyAPCreditMemo::applyAPCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_apopen, SIGNAL(valid(bool)), _apply, SLOT(setEnabled(bool)));
  connect(_apopen, SIGNAL(valid(bool)), _clear, SLOT(setEnabled(bool)));
  connect(_applyToBalance, SIGNAL(clicked()), this, SLOT(sApplyBalance()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_docDate, SIGNAL(newDate(const QDate&)), _available, SLOT(setEffective(const QDate&)));
  connect(_available, SIGNAL(idChanged(int)), _applied, SLOT(setId(int)));
  connect(_available, SIGNAL(idChanged(int)), _balance, SLOT(setId(int)));
  connect(_available, SIGNAL(effectiveChanged(const QDate&)), _applied, SLOT(setEffective(const QDate&)));
  connect(_available, SIGNAL(effectiveChanged(const QDate&)), _balance, SLOT(setEffective(const QDate&)));
  connect(_available, SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));

  _captive = FALSE;

  _apopen->addColumn(tr("Doc. Type"),   _docTypeColumn,  Qt::AlignCenter );
  _apopen->addColumn(tr("Doc. Number"), -1,              Qt::AlignCenter );
  _apopen->addColumn(tr("Doc. Date"),   _dateColumn,     Qt::AlignCenter );
  _apopen->addColumn(tr("Due Date"),    _dateColumn,     Qt::AlignCenter );
  _apopen->addColumn(tr("Open"),        _moneyColumn,    Qt::AlignRight  );
  _apopen->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft   );
  _apopen->addColumn(tr("Applied"),     _moneyColumn,    Qt::AlignRight  );
  _apopen->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft   );
  if (omfgThis->singleCurrency())
  {
    _apopen->hideColumn(5);
    _apopen->hideColumn(7);
  }

  _vend->setReadOnly(TRUE);
  sPriceGroup();
}

/*
 *  Destroys the object and frees any allocated resources
 */
applyAPCreditMemo::~applyAPCreditMemo()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void applyAPCreditMemo::languageChange()
{
    retranslateUi(this);
}

enum SetResponse applyAPCreditMemo::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("apopen_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _apopenid = param.toInt();
    populate();
  }

  return NoError;
}

void applyAPCreditMemo::sPost()
{
  q.exec("BEGIN;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT postAPCreditMemoApplication(:apopen_id) AS result;");
  q.bindValue(":apopen_id", _apopenid);
  q.exec();
  if (q.first())
  {
    QString msg;
    switch (q.value("result").toInt())
    {
      case -1:
      case -2:
        msg = tr("There are no A/P Credit Memo applications to post.");
	break;

      case -3:
        msg = tr( "The total value of the applications that are you attempting to post is greater\n"
		   "than the value of the A/P Credit Memo itself." );
        break;

      case -4:
	msg = tr("At least one A/P Credit Memo application cannot be posted\n"
		 "because there is no current exchange rate for its currency.");
	break;

      case -5:
	msg = tr("The A/P Credit Memo to apply was not found.");
	break;

      case -6:
	msg = tr("The amount to apply for this A/P Credit Memo is NULL.");
	break;
    }
    if (!msg.isEmpty())
    {
      q.exec("ROLLBACK;");
      QMessageBox::critical(this, tr("Cannot Post A/P Credit Memo Applications"),
			     msg);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    QString msg = q.lastError().databaseText();
    q.exec("ROLLBACK;");
    systemError(this, msg, __FILE__, __LINE__);
    return;
  }

  q.exec("COMMIT;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

void applyAPCreditMemo::sApplyBalance()
{
  q.prepare("SELECT applyAPCreditMemoToBalance(:apopen_id)");
  q.bindValue(":apopen_id", _apopenid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  populate();
}

void applyAPCreditMemo::sApply()
{
  ParameterList params;
  params.append("sourceApopenid", _apopenid);
  params.append("targetApopenid", _apopen->id());

  apCreditMemoApplication newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    populate();
}

void applyAPCreditMemo::sClear()
{
  q.prepare( "DELETE FROM apcreditapply "
             "WHERE ( (apcreditapply_source_apopen_id=:sourceApopenid) "
             " AND (apcreditapply_target_apopen_id=:targetApopenid) );" );
  q.bindValue(":sourceApopenid", _apopenid);
  q.bindValue(":targetApopenid", _apopen->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  populate();
}

void applyAPCreditMemo::sClose()
{
  q.prepare( "DELETE FROM apcreditapply "
             "WHERE (apcreditapply_source_apopen_id=:sourceApopenid);" );
  q.bindValue(":sourceApopenid", _apopenid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
  }

  reject();
}

void applyAPCreditMemo::populate()
{
  q.prepare( "SELECT apopen_vend_id, apopen_docnumber, apopen_docdate,"
             "       (apopen_amount - apopen_paid) AS available, apopen_curr_id, "
             "       COALESCE(SUM(currToCurr(apcreditapply_curr_id,"
	     "				apopen_curr_id, apcreditapply_amount, "
	     "				CURRENT_DATE)), 0) AS f_applied "
             "FROM apopen LEFT OUTER JOIN apcreditapply ON (apcreditapply_source_apopen_id=apopen_id) "
             "WHERE (apopen_id=:apopen_id) "
             "GROUP BY apopen_vend_id, apopen_docnumber, apopen_docdate,"
             "         apopen_curr_id, apopen_amount, apopen_paid;" );
  q.bindValue(":apopen_id", _apopenid);
  q.exec();
  if (q.first())
  {
    _vend->setId(q.value("apopen_vend_id").toInt());
    _docDate->setDate(q.value("apopen_docdate").toDate(), true);
    _available->setId(q.value("apopen_curr_id").toInt());
    _available->setLocalValue(q.value("available").toDouble());
    _applied->setLocalValue(q.value("f_applied").toDouble());
    _balance->setLocalValue(_available->localValue() - _applied->localValue());
    _docNumber->setText(q.value("apopen_docnumber").toString());
  
    _cachedAmount = q.value("available").toDouble();
  }
  else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  q.prepare( "SELECT apopen_id,"
             "       CASE WHEN (apopen_doctype='V') THEN :voucher"
             "            WHEN (apopen_doctype='D') THEN :debitMemo"
             "       END AS doctype,"
             "       apopen_docnumber,"
             "       formatDate(apopen_docdate),"
             "       formatDate(apopen_duedate),"
             "       formatMoney(apopen_amount - apopen_paid - COALESCE(selected,0.0) - COALESCE(prepared,0.0)),"
	     "       currConcat(apopen_curr_id), "
             "       formatMoney(apcreditapply_amount), "
	     "       currConcat(apcreditapply_curr_id) "
             "  FROM apopen LEFT OUTER JOIN apcreditapply "
             "         ON ( (apcreditapply_source_apopen_id=:parentApopenid) AND (apcreditapply_target_apopen_id=apopen_id) ) "
             "       LEFT OUTER JOIN (SELECT apopen_id AS selected_apopen_id,"
             "                             SUM(currToCurr(apselect_curr_id, apopen_curr_id, apselect_amount + apselect_discount, apselect_date)) AS selected"
             "                        FROM apselect JOIN apopen ON (apselect_apopen_id=apopen_id)"
             "                       GROUP BY apopen_id) AS sub1"
             "         ON (apopen_id=selected_apopen_id)"
             "       LEFT OUTER JOIN (SELECT apopen_id AS prepared_apopen_id,"
             "                               SUM(currToCurr(checkitem_curr_id, apopen_curr_id, checkitem_amount + checkitem_discount, checkitem_docdate)) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN apopen ON (checkitem_apopen_id=apopen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY apopen_id) AS sub2"
             "         ON (prepared_apopen_id=apopen_id)"
             " WHERE ( (apopen_doctype IN ('V', 'D'))"
             "   AND   (apopen_open)"
             "   AND   ((apopen_amount - apopen_paid - COALESCE(selected,0.0) - COALESCE(prepared,0.0)) > 0.0)"
             "   AND   (apopen_vend_id=:vend_id) ) "
             " ORDER BY apopen_duedate, apopen_docnumber;" );
  q.bindValue(":parentApopenid", _apopenid);
  q.bindValue(":vend_id", _vend->id());
  q.bindValue(":voucher", tr("Voucher"));
  q.bindValue(":debitMemo", tr("D/M"));
  q.exec();
  if (q.first())
      _apopen->populate(q);
  else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

}

void applyAPCreditMemo::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_available->currAbbr()));
}

