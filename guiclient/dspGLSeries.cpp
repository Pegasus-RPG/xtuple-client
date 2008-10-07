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

#include "dspGLSeries.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "reverseGLSeries.h"

dspGLSeries::dspGLSeries(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _gltrans->addColumn(tr("Date"),      _dateColumn, Qt::AlignCenter,true, "transdate");
  _gltrans->addColumn(tr("Journal #"),_orderColumn, Qt::AlignRight, true, "gltrans_journalnumber");
  _gltrans->addColumn(tr("Source"),   _orderColumn, Qt::AlignCenter,true, "gltrans_source");
  _gltrans->addColumn(tr("Doc. Type"), _itemColumn, Qt::AlignCenter,true, "gltrans_doctype");
  _gltrans->addColumn(tr("Doc. Num."),_orderColumn, Qt::AlignCenter,true, "gltrans_docnumber");
  _gltrans->addColumn(tr("Notes/Account"),      -1, Qt::AlignLeft,  true, "account");
  _gltrans->addColumn(tr("Debit"), _bigMoneyColumn, Qt::AlignRight, true, "debit");
  _gltrans->addColumn(tr("Credit"),_bigMoneyColumn, Qt::AlignRight, true, "credit");
  _gltrans->addColumn(tr("Posted"),      _ynColumn, Qt::AlignCenter,true, "gltrans_posted");
}

dspGLSeries::~dspGLSeries()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspGLSeries::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspGLSeries::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("startDate", &valid);
  if(valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if(valid)
    _dates->setEndDate(param.toDate());

  param = pParams.value("journalnumber", &valid);
  if(valid)
  {
    _jrnlGroup->setChecked(true);
    _startJrnlnum->setText(param.toString());
    _endJrnlnum->setText(param.toString());
  }

  sFillList();

  return NoError;
}

void dspGLSeries::sPopulateMenu(QMenu * pMenu)
{
  int menuItem;

  bool reversible = false;
  XTreeWidgetItem * item = (XTreeWidgetItem*)_gltrans->currentItem();
  if(0 != item)
  {
    if(item->altId() != -1)
      item = (XTreeWidgetItem*)item->parent();
    if(0 != item)
    {
      if(item->text(3) == "ST" || item->text(3) == "JE")
        reversible = true;
    }
  }

  menuItem = pMenu->insertItem(tr("Reverse Journal..."), this, SLOT(sReverse()), 0);
  if (!reversible || !_privileges->check("PostStandardJournals"))
    pMenu->setItemEnabled(menuItem, false);
}

bool dspGLSeries::setParams(ParameterList &params)
{
  if(!_dates->allValid())
  {
    QMessageBox::warning(this, tr("Invalid Date Range"),
                         tr("You must first specify a valid date range.") );
    _dates->setFocus();
    return false;
  }

  _dates->appendValue(params);

  if(_selectedSource->isChecked())
    params.append("source", _source->currentText());

  if(_jrnlGroup->isChecked())
  {
    params.append("startJrnlnum", _startJrnlnum->text().toInt());
    params.append("endJrnlnum", _endJrnlnum->text().toInt());
  }

  return true;
}

void dspGLSeries::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("GLSeries", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspGLSeries::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql("SELECT *, "
                   "       CASE WHEN gltrans_id = -1 THEN 0"
                   "       ELSE 1 END AS xtindentrole,"
                   "       CASE WHEN gltrans_id = -1 THEN gltrans_date"
                   "       END AS transdate,"    // qtdisplayrole isn't working?
                   "       'curr' AS debit_xtnumericrole,"
                   "       'curr' AS credit_xtnumericrole "
                   "FROM (SELECT DISTINCT "
                   "       -1 AS gltrans_id, gltrans_sequence, gltrans_date, "
                   "       gltrans_source, gltrans_journalnumber,"
                   "       gltrans_doctype, '' AS gltrans_docnumber,"
                   "       firstLine(gltrans_notes) AS account,"
                   "       0.0 AS gltrans_amount,"
                   "       CAST(NULL AS NUMERIC) AS debit,"
                   "       CAST(NULL AS NUMERIC) AS credit,"
                   "       gltrans_posted "
                   "FROM gltrans "
                   "WHERE ((gltrans_date BETWEEN <? value(\"startDate\") ?>"
                   "                         AND <? value(\"endDate\") ?>)"
                   "<? if exists(\"source\") ?>"
                   "   AND (gltrans_source=<? value(\"source\") ?>)"
                   "<? endif ?>"
                   "<? if exists(\"startJrnlnum\") ?>"
                   "   AND (gltrans_journalnumber BETWEEN <? value(\"startJrnlnum\") ?>"
                   "                                  AND <? value(\"endJrnlnum\") ?>)"
                   "<? endif ?>"
                   ")"
                   "UNION "
                   "SELECT gltrans_id, gltrans_sequence, gltrans_date, "
                   "       NULL, NULL,"
                   "       NULL, gltrans_docnumber,"
                   "       (formatGLAccount(accnt_id) || ' - ' || accnt_descrip) AS account,"
                   "       gltrans_amount,"
                   "       CASE WHEN (gltrans_amount < 0) THEN (gltrans_amount * -1)"
                   "       END AS debit,"
                   "       CASE WHEN (gltrans_amount > 0) THEN gltrans_amount"
                   "       END AS credit,"
                   "       NULL AS gltrans_posted "
                   "FROM gltrans JOIN accnt ON (gltrans_accnt_id=accnt_id)"
                   "WHERE ((gltrans_date BETWEEN <? value(\"startDate\") ?>"
                   "                         AND <? value(\"endDate\") ?>)"
                   "<? if exists(\"source\") ?>"
                   "   AND (gltrans_source=<? value(\"source\") ?>)"
                   "<? endif ?>"
                   "<? if exists(\"startJrnlnum\") ?>"
                   "   AND (gltrans_journalnumber BETWEEN <? value(\"startJrnlnum\") ?>"
                   "                                  AND <? value(\"endJrnlnum\") ?>)"
                   "<? endif ?>"
                   " ) "
                   ") AS dummy "
                   "ORDER BY gltrans_date, gltrans_sequence,"
                   "         xtindentrole, gltrans_amount;");
  q = mql.toQuery(params);
  _gltrans->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspGLSeries::sReverse()
{
  ParameterList params;
  params.append("glseries", _gltrans->id());

  reverseGLSeries newdlg(this);
  if(newdlg.set(params) != NoError)
    return;
  if(newdlg.exec() == XDialog::Accepted)
    sFillList();
}
