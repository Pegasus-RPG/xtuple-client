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

#include "arCreditMemoApplication.h"
#include "storedProcErrorLookup.h"

applyARCreditMemo::applyARCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_aropen, SIGNAL(valid(bool)), _apply, SLOT(setEnabled(bool)));
  connect(_aropen, SIGNAL(valid(bool)), _clear, SLOT(setEnabled(bool)));
  connect(_applyToBalance, SIGNAL(clicked()), this, SLOT(sApplyBalance()));
  connect(_apply, SIGNAL(clicked()), this, SLOT(sApply()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));
  connect(_available, SIGNAL(idChanged(int)), this, SLOT(sPriceGroup()));
  connect(_available, SIGNAL(idChanged(int)), _applied, SLOT(setId(int)));
  connect(_available, SIGNAL(idChanged(int)), _balance, SLOT(setId(int)));
  connect(_available, SIGNAL(effectiveChanged(const QDate&)), _applied, SLOT(setEffective(const QDate&)));
  connect(_available, SIGNAL(effectiveChanged(const QDate&)), _balance, SLOT(setEffective(const QDate&)));
  connect(_docDate, SIGNAL(newDate(const QDate&)), _available, SLOT(setEffective(const QDate&)));
  connect(_searchDocNum, SIGNAL(textChanged(const QString&)), this, SLOT(sSearchDocNumChanged(const QString&)));

  _captive = FALSE;

  _aropen->addColumn(tr("Doc. Type"),   _docTypeColumn, Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. Number"), -1,             Qt::AlignCenter );
  _aropen->addColumn(tr("Doc. Date"),   _dateColumn,    Qt::AlignCenter );
  _aropen->addColumn(tr("Due Date"),    _dateColumn,    Qt::AlignCenter );
  _aropen->addColumn(tr("Open"),        _moneyColumn,   Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),	_currencyColumn, Qt::AlignLeft  );
  _aropen->addColumn(tr("Applied"),     _moneyColumn,   Qt::AlignRight  );
  _aropen->addColumn(tr("Currency"),	_currencyColumn, Qt::AlignLeft  );

  _cust->setReadOnly(TRUE);

  if(_metrics->boolean("HideApplyToBalance"))
    _applyToBalance->hide();

  if (omfgThis->singleCurrency())
  {
    _aropen->hideColumn(5);
    _aropen->hideColumn(7);
  }

  sPriceGroup();
}

/*
 *  Destroys the object and frees any allocated resources
 */
applyARCreditMemo::~applyARCreditMemo()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
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
  q.prepare("SELECT applyARCreditMemoToBalance(:aropen_id)");
  q.bindValue(":aropen_id", _aropenid);
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

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
             "       (aropen_amount - aropen_paid) AS available,"
             "       COALESCE(SUM(currToCurr(arcreditapply_curr_id, "
	     "				     aropen_curr_id, "
	     "				     arcreditapply_amount, "
	     "				     aropen_docdate)), 0) AS f_applied, "
	     "	     aropen_curr_id "
             "FROM aropen LEFT OUTER JOIN arcreditapply ON (arcreditapply_source_aropen_id=aropen_id) "
             "WHERE (aropen_id=:aropen_id) "
             "GROUP BY aropen_cust_id, aropen_docnumber, aropen_docdate,"
             "         aropen_amount, aropen_paid, aropen_curr_id;" );
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

  q.prepare( "SELECT aropen_id,"
             "       CASE WHEN (aropen_doctype='I') THEN :invoice"
             "            WHEN (aropen_doctype='D') THEN :debitMemo"
             "       END AS doctype,"
             "       aropen_docnumber,"
             "       formatDate(aropen_docdate),"
             "       formatDate(aropen_duedate),"
             "       formatMoney(aropen_amount - aropen_paid),"
	     "       currConcat(aropen_curr_id),"
             "       formatMoney(COALESCE(arcreditapply_amount, 0)),"
	     "       currConcat(arcreditapply_curr_id) "
             "FROM aropen LEFT OUTER JOIN arcreditapply "
             "             ON ( (arcreditapply_source_aropen_id=:parentAropenid) AND (arcreditapply_target_aropen_id=aropen_id) ) "
             "WHERE ( (aropen_doctype IN ('I', 'D'))"
             " AND (aropen_open)"
             " AND (aropen_cust_id=:cust_id) ) "
             "ORDER BY aropen_duedate, aropen_docnumber;" );
  q.bindValue(":parentAropenid", _aropenid);
  q.bindValue(":cust_id", _cust->id());
  q.bindValue(":invoice", tr("Invoice"));
  q.bindValue(":debitMemo", tr("D/M"));
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  _aropen->populate(q);
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
