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

#include "applyARCreditMemo.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <metasql.h>

#include "arCreditMemoApplication.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

applyARCreditMemo::applyARCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_apply,         SIGNAL(clicked()),      this, SLOT(sApply()));
  connect(_applyToBalance,SIGNAL(clicked()),      this, SLOT(sApplyBalance()));
  connect(_available,     SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));
  connect(_clear,         SIGNAL(clicked()),      this, SLOT(sClear()));
  connect(_close,         SIGNAL(clicked()),      this, SLOT(sClose()));
  connect(_post,          SIGNAL(clicked()),      this, SLOT(sPost()));
  connect(_searchDocNum,  SIGNAL(textChanged(const QString&)), this, SLOT(sSearchDocNumChanged(const QString&)));

  _captive = FALSE;
  _overapplied = false;

  _aropen->addColumn(tr("Doc. Type"),   _docTypeColumn, Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. Number"), -1,             Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. Date"),   _dateColumn,    Qt::AlignCenter );
  _aropen->addColumn(tr("Due Date"),    _dateColumn,    Qt::AlignCenter );
  _aropen->addColumn(tr("Open"),        _moneyColumn,   Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),	_currencyColumn, Qt::AlignLeft, !omfgThis->singleCurrency());
  _aropen->addColumn(tr("Applied"),     _moneyColumn,    Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),	_currencyColumn, Qt::AlignLeft, !omfgThis->singleCurrency());
  _aropen->addColumn(tr("All Pending"), _moneyColumn,    Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),	_currencyColumn, Qt::AlignLeft, !omfgThis->singleCurrency());

  _cust->setReadOnly(TRUE);

  if(_metrics->boolean("HideApplyToBalance"))
    _applyToBalance->hide();

  sPriceGroup();
}

applyARCreditMemo::~applyARCreditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

void applyARCreditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse applyARCreditMemo::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("aropen_id", &valid);
  if (valid)
  {
    _captive = TRUE;
    _aropenid = param.toInt();
    populate();
  }

  return NoError;
}

void applyARCreditMemo::sPost()
{
  populate(); // one more time to double-check _overapplied
  if (_overapplied &&
      QMessageBox::question(this, tr("Overapplied?"),
                            tr("This Credit Memo appears to apply too much to "
                               "at least one of the Open Items. Do you want "
                               "to post the current applications anyway?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT postARCreditMemoApplication(:aropen_id) AS result;");
  q.bindValue(":aropen_id", _aropenid);
  q.exec();
  if (q.first())
  {
    QString msg;
    int result = q.value("result").toInt();
    if (result < 0)
    {
      rollback.exec();
      systemError(this, storedProcErrorLookup("postARCreditMemoApplication", result),
		  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    rollback.exec();
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
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

void applyARCreditMemo::sApplyBalance()
{
  q.prepare("SELECT applyARCreditMemoToBalance(:aropen_id) AS result;");
  q.bindValue(":aropen_id", _aropenid);
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("applyARCreditMemoToBalance", result));
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  populate();
}

void applyARCreditMemo::sApply()
{
  ParameterList params;
  params.append("sourceAropenid", _aropenid);
  params.append("targetAropenid", _aropen->id());

  arCreditMemoApplication newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    populate();
}

void applyARCreditMemo::sClear()
{
  q.prepare( "DELETE FROM arcreditapply "
             "WHERE ( (arcreditapply_source_aropen_id=:sourceAropenid) "
             " AND (arcreditapply_target_aropen_id=:targetAropenid) );" );
  q.bindValue(":sourceAropenid", _aropenid);
  q.bindValue(":targetAropenid", _aropen->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  populate();
}

void applyARCreditMemo::sClose()
{
  q.prepare( "DELETE FROM arcreditapply "
             "WHERE (arcreditapply_source_aropen_id=:sourceAropenid);" );
  q.bindValue(":sourceAropenid", _aropenid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  reject();
}

void applyARCreditMemo::populate()
{
  q.prepare( "SELECT aropen_cust_id, aropen_docnumber, aropen_docdate,"
             "       (aropen_amount - aropen_paid - COALESCE(prepared,0.0)) AS available,"
             "       COALESCE(SUM(currToCurr(arcreditapply_curr_id, "
	         "				     aropen_curr_id, "
	         "				     arcreditapply_amount, "
	         "				     aropen_docdate)), 0) AS f_applied, "
	         "	     aropen_curr_id "
             "FROM aropen LEFT OUTER JOIN arcreditapply ON (arcreditapply_source_aropen_id=aropen_id) "
 	         "       LEFT OUTER JOIN (SELECT aropen_id AS prepared_aropen_id,"
             "                               SUM(currToCurr(checkitem_curr_id, aropen_curr_id, checkitem_amount + checkitem_discount, checkitem_docdate)) AS prepared"
             "                          FROM checkhead JOIN checkitem ON (checkitem_checkhead_id=checkhead_id)"
             "                                     JOIN aropen ON (checkitem_aropen_id=aropen_id)"
             "                         WHERE ((NOT checkhead_posted)"
             "                           AND  (NOT checkhead_void))"
             "                         GROUP BY aropen_id) AS sub1"
             "         ON (prepared_aropen_id=aropen_id)"
             "WHERE (aropen_id=:aropen_id) "
             "GROUP BY aropen_cust_id, aropen_docnumber, aropen_docdate,"
             "         aropen_amount, aropen_paid, aropen_curr_id, prepared;" );
  q.bindValue(":aropen_id", _aropenid);
  q.exec();
  if (q.first())
  {
    _available->set(q.value("available").toDouble(),
		    q.value("aropen_curr_id").toInt(),
		    q.value("aropen_docdate").toDate(), false);
    _cust->setId(q.value("aropen_cust_id").toInt());
    _applied->setLocalValue(q.value("f_applied").toDouble());
    _balance->setLocalValue(_available->localValue() - _applied->localValue());
    _docNumber->setText(q.value("aropen_docnumber").toString());
    _docDate->setDate(q.value("aropen_docdate").toDate(), true);
  
    _cachedAmount = q.value("available").toDouble();
  }
  else
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _aropen->clear();
  MetaSQLQuery mql = mqlLoad(":/ar/aropenApplications.mql");
  ParameterList params;
  params.append("cust_id",          _cust->id());
  params.append("debitMemo",        tr("D/M"));
  params.append("invoice",          tr("Invoice"));
  params.append("source_aropen_id", _aropenid);
  q = mql.toQuery(params);
  XTreeWidgetItem *last = 0;
  _overapplied = false;
  while (q.next())
  {
    last = new XTreeWidgetItem(_aropen, last,
                               q.value("aropen_id").toInt(),
                               q.value("doctype"),
                               q.value("aropen_docnumber"),
                               q.value("f_docdate"),
                               q.value("f_duedate"),
                               q.value("f_balance"),
                               q.value("balance_curr"),
                               q.value("f_applied"),
                               q.value("applied_curr"),
                               q.value("f_pending"),
                               q.value("balance_curr"));
    if (q.value("pending").toDouble() > q.value("balance").toDouble())
    {
      last->setTextColor("red");
      _overapplied = true;
    }
  }
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

void applyARCreditMemo::sPriceGroup()
{
  if (! omfgThis->singleCurrency())
    _priceGroup->setTitle(tr("In %1:").arg(_available->currAbbr()));
}


void applyARCreditMemo::sSearchDocNumChanged(const QString &pTarget)
{
  _aropen->clearSelection();

  if (pTarget.isEmpty())
    return;

  int i;
  for (i = 0; i < _aropen->topLevelItemCount(); i++)
  {
    if (_aropen->topLevelItem(i)->text(1).startsWith(pTarget))
      break;
  }

  if (i < _aropen->topLevelItemCount())
  {
    _aropen->setCurrentItem(_aropen->topLevelItem(i));
    _aropen->scrollToItem(_aropen->topLevelItem(i));
  }
}
