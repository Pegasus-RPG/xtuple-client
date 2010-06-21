/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspGLSeries.h"
#include "glSeries.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>
#include "mqlutil.h"

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
  XWidget::set(pParams);
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

  bool editable = false;
  bool deletable = false;
  bool reversible = false;
  XTreeWidgetItem * item = (XTreeWidgetItem*)_gltrans->currentItem();
  if(0 != item)
  {
    if(item->altId() != -1)
      item = (XTreeWidgetItem*)item->QTreeWidgetItem::parent();
    if(0 != item)
    {
      if(item->rawValue("gltrans_doctype").toString() == "ST" ||
         item->rawValue("gltrans_doctype").toString() == "JE")
        reversible = _privileges->check("PostStandardJournals");

      // Make sure there is nothing to restricting edits
      ParameterList params;
      params.append("glSequence", _gltrans->id());
      MetaSQLQuery mql = mqlLoad("glseries", "checkeditable");
      XSqlQuery qry = mql.toQuery(params);
      if (!qry.first())
      {
        editable = _privileges->check("EditPostedJournals") &&
                   item->rawValue("gltrans_doctype").toString() == "JE";
        deletable = _privileges->check("DeletePostedJournals") &&
                    reversible;
      }
    }
  }

  menuItem = pMenu->insertItem(tr("Edit Journal..."), this, SLOT(sEdit()), 0);
  if (!editable)
    pMenu->setItemEnabled(menuItem, false);

  menuItem = pMenu->insertItem(tr("Delete Journal..."), this, SLOT(sDelete()), 0);
  if (!deletable)
    pMenu->setItemEnabled(menuItem, false);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Reverse Journal..."), this, SLOT(sReverse()), 0);
  if (!reversible)
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
                   "       gltrans_sequence, -1 AS gltrans_id, gltrans_date, "
                   "       gltrans_source, gltrans_journalnumber,"
                   "       gltrans_doctype, '' AS gltrans_docnumber,"
                   "       firstLine(gltrans_notes) AS account,"
                   "       0.0 AS gltrans_amount,"
                   "       CAST(NULL AS NUMERIC) AS debit,"
                   "       CAST(NULL AS NUMERIC) AS credit,"
                   "       gltrans_posted "
                   "FROM gltrans "
                   "WHERE ( (NOT gltrans_deleted) "
                   " AND (gltrans_date BETWEEN <? value(\"startDate\") ?>"
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
                   "SELECT gltrans_sequence, gltrans_id, gltrans_date, "
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
                   "WHERE ( (NOT gltrans_deleted ) "
                   "  AND (gltrans_date BETWEEN <? value(\"startDate\") ?>"
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
  _gltrans->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspGLSeries::sEdit()
{
  ParameterList params;
  ParameterList params2;
  params.append("glSequence", _gltrans->id());

  MetaSQLQuery mql("SELECT copyGlSeries(gltrans_sequence) AS sequence,"
                   "  gltrans_journalnumber "
                   "FROM gltrans "
                   "WHERE (gltrans_sequence=<? value(\"glSequence\") ?>) "
                   "LIMIT 1;");
  XSqlQuery qry;
  qry = mql.toQuery(params);
  if (qry.first())
  {
    params2.append("glSequence", qry.value("sequence"));
    params2.append("journalnumber", qry.value("gltrans_journalnumber"));
  }
  else if (qry.lastError().type() != QSqlError::NoError)
  {
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  else
    return;

  glSeries newdlg(this);
  if(newdlg.set(params2) != NoError)
    return;
  if(newdlg.exec() == XDialog::Accepted)
    sDelete(true);
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

void dspGLSeries::sDelete(bool edited)
{
  QString action = tr("deleted");
  if (edited)
    action = tr("edited");

  ParameterList params;
  params.append("glSequence", _gltrans->id());
  params.append("notes", tr("Journal %1 by %2 on %3")
                .arg(action)
                .arg(omfgThis->username())
                .arg(omfgThis->dbDate().toString()));

  MetaSQLQuery mql("SELECT deleteGlSeries(<? value(\"glSequence\") ?>, "
                   " <? value(\"notes\") ?>) AS result;");
  XSqlQuery del;
  del = mql.toQuery(params);
  if (del.lastError().type() != QSqlError::NoError)
  {
    systemError(this, del.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sGlSeriesUpdated();
  sFillList();
}
