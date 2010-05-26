/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspGLTransactions.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
#include "glTransactionDetail.h"
#include "dspGLSeries.h"
#include "invoice.h"
#include "purchaseOrder.h"
#include "voucher.h"
#include "miscVoucher.h"
#include "dspShipmentsByShipment.h"
#include "apOpenItem.h"
#include "arOpenItem.h"
#include "salesOrder.h"
#include "dspWoHistoryByNumber.h"
#include "transactionInformation.h"
#include "storedProcErrorLookup.h"

dspGLTransactions::dspGLTransactions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QString qryType = QString( "SELECT  1, '%1' UNION "
                             "SELECT  2, '%2' UNION "
                             "SELECT  3, '%3' UNION "
                             "SELECT  4, '%4' UNION "
                             "SELECT  5, '%5'")
      .arg(tr("Asset"))
      .arg(tr("Expense"))
      .arg(tr("Liability"))
      .arg(tr("Equity"))
      .arg(tr("Revenue"));

  QString qrySubType = QString("SELECT subaccnttype_id, "
                               "       (subaccnttype_code || '-' || subaccnttype_descrip) "
                               "FROM subaccnttype "
                               "ORDER BY subaccnttype_code;");

  QString qrySource = QString("SELECT 1 AS id, 'A/P' AS source  UNION "
                              "SELECT 2 AS id, 'A/R' AS source  UNION "
                              "SELECT 3 AS id, 'G/L' AS source  UNION "
                              "SELECT 4 AS id, 'I/M' AS source  UNION "
                              "SELECT 5 AS id, 'P/D' AS source  UNION "
                              "SELECT 6 AS id, 'P/O' AS source  UNION "
                              "SELECT 7 AS id, 'S/O' AS source  UNION "
                              "SELECT 8 AS id, 'S/R' AS source  UNION "
                              "SELECT 9 AS id, 'W/O' AS source;");

  QString qryAccNum = QString("SELECT min(accnt_id) as num_id,"
                              "       accnt_number AS num_number "
                              "FROM accnt "
                              "GROUP BY accnt_number "
                              "ORDER BY accnt_number;");
 
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_gltrans, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_parameterWidget, SIGNAL(filterChanged()), this, SLOT(handleTotalCheckbox()));
  connect(_showRunningTotal, SIGNAL(toggled(bool)), this, SLOT(handleTotalCheckbox()));

  _gltrans->addColumn(tr("Date"),      _dateColumn,    Qt::AlignCenter, true, "gltrans_date");
  _gltrans->addColumn(tr("Source"),    _orderColumn,   Qt::AlignCenter, true, "gltrans_source");
  _gltrans->addColumn(tr("Doc. Type"), _docTypeColumn, Qt::AlignCenter, true, "gltrans_doctype");
  _gltrans->addColumn(tr("Doc. #"),    _orderColumn,   Qt::AlignCenter, true, "docnumber");
  _gltrans->addColumn(tr("Reference"), -1,             Qt::AlignLeft,   true, "notes");
  _gltrans->addColumn(tr("Account"),   _itemColumn,    Qt::AlignLeft,   true, "account");
  _gltrans->addColumn(tr("Debit"),     _moneyColumn,   Qt::AlignRight,  true, "debit");
  _gltrans->addColumn(tr("Credit"),    _moneyColumn,   Qt::AlignRight,  true, "credit");
  _gltrans->addColumn(tr("Posted"),    _ynColumn,      Qt::AlignCenter, true, "gltrans_posted");
  _gltrans->addColumn(tr("Username"),  _userColumn,    Qt::AlignLeft,   true, "gltrans_username");
  _gltrans->addColumn(tr("Running Total"), _moneyColumn, Qt::AlignRight,false,"running");

  _beginningBalance->setPrecision(omfgThis->moneyVal());

  _beginningBalanceLit->setVisible(_showRunningTotal->isChecked());
  _beginningBalance->setVisible(_showRunningTotal->isChecked());
  if (_showRunningTotal->isChecked())
    _gltrans->showColumn("running");
  else
    _gltrans->hideColumn("running");

  _parameterWidget->append(tr("Start Date"), "startDate", ParameterWidget::Date, QDate::currentDate(), true);
  _parameterWidget->append(tr("End Date"),   "endDate",   ParameterWidget::Date, QDate::currentDate(), true);
  _parameterWidget->append(tr("GL Account"), "accnt_id",  ParameterWidget::GLAccount);
  _parameterWidget->append(tr("Document #"), "docnum",    ParameterWidget::Text);
  _parameterWidget->appendComboBox(tr("Source"), "source_id",    qrySource);
  if (_metrics->value("GLCompanySize").toInt() > 0)
  _parameterWidget->appendComboBox(tr("Company"), "company_id", XComboBox::Companies);
  if (_metrics->value("GLProfitSize").toInt() >  0)
    _parameterWidget->appendComboBox(tr("Profit Center"), "prfcntr_id", XComboBox::ProfitCenters);
  _parameterWidget->appendComboBox(tr("Main Segment"), "num_id", qryAccNum);
  if (_metrics->value("GLSubaccountSize").toInt() > 0)
    _parameterWidget->appendComboBox(tr("Sub Account"), "subaccnt_id", XComboBox::Subaccounts);
  _parameterWidget->appendComboBox(tr("Account Type"), "accnttype_id", qryType);
  _parameterWidget->appendComboBox(tr("Sub Type"), "subType",   qrySubType);

  _parameterWidget->applyDefaultFilterSet();
}

dspGLTransactions::~dspGLTransactions()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspGLTransactions::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspGLTransactions::set(const ParameterList &pParams)
{
  XWidget::set(pParams);

  _parameterWidget->setSavedFilters();

  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
    _parameterWidget->setDefault("GL Account", param.toInt(), true);

  param = pParams.value("startDate", &valid);
  if (valid)
    _parameterWidget->setDefault("Start Date", param.toDate(), true);

  param = pParams.value("endDate", &valid);
  if (valid)
    _parameterWidget->setDefault("End Date", param.toDate(), true);

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
      _parameterWidget->setDefault("Start Date", q.value("period_start").toDate(), true);
      _parameterWidget->setDefault("End Date", q.value("period_end").toDate(), true);
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
  menuThis->insertItem(tr("View GL Series..."), this, SLOT(sViewSeries()), 0);

  XTreeWidgetItem * item = (XTreeWidgetItem*)pItem;
  if(0 == item)
    return;

  if(item->rawValue("gltrans_doctype").toString() == "VO")
    menuThis->insertItem(tr("View Voucher..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "IN")
    menuThis->insertItem(tr("View Invoice..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "PO")
    menuThis->insertItem(tr("View Purchase Order..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "SH")
    menuThis->insertItem(tr("View Shipment..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "CM")
    menuThis->insertItem(tr("View Credit Memo..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "DM")
    menuThis->insertItem(tr("View Debit Memo..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "SO")
    menuThis->insertItem(tr("View Sales Order..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_doctype").toString() == "WO")
    menuThis->insertItem(tr("View WO History..."), this, SLOT(sViewDocument()));
  else if(item->rawValue("gltrans_source").toString() == "I/M")
    menuThis->insertItem(tr("View Inventory History..."), this, SLOT(sViewDocument()));
}

bool dspGLTransactions::setParams(ParameterList &params)
{
  _parameterWidget->appendValue(params);
  bool valid;
  QVariant param;

  param = params.value("accnttype_id", &valid);
  if (valid)
  {
    int typid = param.toInt();
    QString type;

    if (typid == 1)
      type = "A";
    else if (typid ==2)
      type = "E";
    else if (typid ==3)
      type = "L";
    else if (typid ==4)
      type = "Q";
    else if (typid ==5)
      type = "R";

    params.append("accntType", type);
  }

  param = params.value("source_id", &valid);
  if (valid)
  {
    int srcid = param.toInt();
    QString src;

    if (srcid == 1)
      src = "A/P";
    else if (srcid ==2)
      src = "A/R";
    else if (srcid ==3)
      src = "G/L";
    else if (srcid ==4)
      src = "I/M";
    else if (srcid ==5)
      src = "P/D";
    else if (srcid ==6)
      src = "P/O";
    else if (srcid ==7)
      src = "S/O";
    else if (srcid ==8)
      src = "S/R";
    else if (srcid ==9)
      src = "W/O";

    params.append("source", src);
  }

  param = params.value("num_id", &valid);
  if (valid)
  {
    XSqlQuery num;
    num.prepare("SELECT accnt_number "
                "FROM accnt "
                "WHERE (accnt_id=:accnt_id);");
    num.bindValue(":accnt_id", params.value("num_id").toInt());
    num.exec();
    if (num.first())
      params.append("accnt_number", num.value("accnt_number").toString());
  }

  param = params.value("accnt_id", &valid);
  if (valid)
  {
    if (_showRunningTotal->isChecked() &&
        _showRunningTotal->isEnabled())
    {
      double beginning = 0;
      QDate  periodStart = params.value("startDate").toDate();
      XSqlQuery begq;
      begq.prepare("SELECT "
                   "  CASE WHEN accnt_type IN ('A','E') THEN "
                   "    trialbal_beginning * -1 "
                   "  ELSE trialbal_beginning END AS trialbal_beginning,"
                   "  period_start "
                   "FROM trialbal "
                   "  JOIN accnt ON (trialbal_accnt_id=accnt_id), "
                   "  period "
                   "WHERE ((trialbal_period_id=period_id)"
                   "  AND  (trialbal_accnt_id=:accnt_id)"
                   "  AND  (:start BETWEEN period_start AND period_end));");
      begq.bindValue(":accnt_id", params.value("accnt_id").toInt());
      begq.bindValue(":start", params.value("startDate").toDate());
      begq.exec();
      if (begq.first())
      {
        beginning   = begq.value("trialbal_beginning").toDouble();
        periodStart = begq.value("period_start").toDate();
      }
      else if (begq.lastError().type() != QSqlError::NoError)
      {
	systemError(this, begq.lastError().databaseText(), __FILE__, __LINE__);
	return false;
      }
      XSqlQuery glq;
      glq.prepare("SELECT CASE WHEN accnt_type IN ('A','E') THEN "
                  "         COALESCE(SUM(gltrans_amount),0) * -1"
                  "       ELSE COALESCE(SUM(gltrans_amount),0) END AS glamount "
                  "FROM gltrans "
                  "  JOIN accnt ON (gltrans_accnt_id=accnt_id) "
                  "WHERE ((gltrans_date BETWEEN :periodstart AND date :querystart - interval '1 day')"
                  "  AND  (gltrans_accnt_id=:accnt_id)) "
                  "GROUP BY accnt_type;");
      glq.bindValue(":periodstart", periodStart);
      glq.bindValue(":querystart",  params.value("startDate").toDate());
      glq.bindValue(":accnt_id",    params.value("accnt_id").toInt());
      glq.exec();
      if (glq.first())
        beginning   += glq.value("glamount").toDouble();
      else if (glq.lastError().type() != QSqlError::NoError)
      {
	systemError(this, glq.lastError().databaseText(), __FILE__, __LINE__);
	return false;
      }

      params.append("beginningBalance", beginning);
    }
  }

  return true;
}

void dspGLTransactions::sPrint()
{
  if (!_metrics->boolean("ManualForwardUpdate") && 
       _showRunningTotal->isChecked())
  {
    if (!forwardUpdate())
      return;
  }
  
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("GLTransactions", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspGLTransactions::sFillList()
{
  if (!_metrics->boolean("ManualForwardUpdate") && 
      _showRunningTotal->isChecked() &&
      _showRunningTotal->isEnabled())
  {
    if (!forwardUpdate())
      return;
  }

  MetaSQLQuery mql = mqlLoad("gltransactions", "detail");
  ParameterList params;
  if (! setParams(params))
    return;

  if (_showRunningTotal->isChecked() &&
      _showRunningTotal->isEnabled())
  {
    _gltrans->showColumn("running");
    qDebug("begbal %f", params.value("beginningBalance").toDouble());
    _beginningBalance->setDouble(params.value("beginningBalance").toDouble());
  }
  else
    _gltrans->hideColumn("running");

  XSqlQuery r = mql.toQuery(params);
  _gltrans->populate(r);
  if (r.lastError().type() != QSqlError::NoError)
  {
    systemError(this, r.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspGLTransactions::sViewTrans()
{
  ParameterList params;

  params.append("gltrans_id", _gltrans->id());

  glTransactionDetail newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspGLTransactions::sViewSeries()
{
  q.prepare("SELECT gltrans_date, gltrans_journalnumber"
            "  FROM gltrans"
            " WHERE (gltrans_id=:gltrans_id)");
  q.bindValue(":gltrans_id", _gltrans->id());
  q.exec();
  if(!q.first())
    return;

  ParameterList params;

  params.append("startDate", q.value("gltrans_date").toDate());
  params.append("endDate", q.value("gltrans_date").toDate());
  params.append("journalnumber", q.value("gltrans_journalnumber").toString());

  dspGLSeries *newdlg = new dspGLSeries();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspGLTransactions::sViewDocument()
{
  XTreeWidgetItem * item = (XTreeWidgetItem*)_gltrans->currentItem();
  if(0 == item)
    return;

  ParameterList params;
  if(item->rawValue("gltrans_doctype").toString() == "VO")
  {
    q.prepare("SELECT vohead_id, vohead_misc"
              "  FROM vohead"
              " WHERE (vohead_number=:vohead_number)");
    q.bindValue(":vohead_number", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    params.append("vohead_id", q.value("vohead_id").toInt());
    params.append("mode", "view");
    
    if(q.value("vohead_misc").toBool())
    {
      miscVoucher *newdlg = new miscVoucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }
    else
    {
      voucher *newdlg = new voucher();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
    }

  }
  else if(item->rawValue("gltrans_doctype").toString() == "IN")
  {
    q.prepare("SELECT invchead_id"
              "  FROM invchead"
              " WHERE (invchead_invcnumber=:invchead_invcnumber)");
    q.bindValue(":invchead_invcnumber", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    invoice::viewInvoice(q.value("invchead_id").toInt());
  }
  else if(item->rawValue("gltrans_doctype").toString() == "PO")
  {
    QStringList docnumber = item->rawValue("docnumber").toString().split("-");
    q.prepare("SELECT pohead_id"
              "  FROM pohead"
              " WHERE (pohead_number=:docnumber)");
    q.bindValue(":docnumber", docnumber[0]);
    q.exec();
    if(!q.first())
      return;

    params.append("pohead_id", q.value("pohead_id").toInt());
    params.append("mode", "view");

    purchaseOrder *newdlg = new purchaseOrder();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->rawValue("gltrans_doctype").toString() == "SH")
  {
    q.prepare("SELECT shiphead_id"
              "  FROM shiphead"
              " WHERE (shiphead_number=:shiphead_number)");
    q.bindValue(":shiphead_number", item->rawValue("docnumber").toString());
    q.exec();
    if(!q.first())
      return;

    params.append("shiphead_id", q.value("shiphead_id").toInt());

    dspShipmentsByShipment *newdlg = new dspShipmentsByShipment();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if( (item->rawValue("gltrans_doctype").toString() == "CM") || (item->rawValue("gltrans_doctype").toString() == "DM") )
  {
    if(item->rawValue("gltrans_source").toString() == "A/P")
    {
      q.prepare("SELECT apopen_id"
                "  FROM apopen"
                " WHERE ((apopen_docnumber=:docnumber) "
                "  AND (apopen_doctype='C'));");
      q.bindValue(":docnumber", item->rawValue("docnumber").toString());
      q.exec();
      if(!q.first())
        return;

      params.append("mode", "view");
      params.append("apopen_id", q.value("apopen_id").toInt());
      apOpenItem newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
    }
    else if(item->rawValue("gltrans_source").toString() == "A/R")
    {
      q.prepare("SELECT aropen_id"
                "  FROM aropen"
                " WHERE ((aropen_docnumber=:docnumber) "
                "  AND (aropen_doctype='C'));");
      q.bindValue(":docnumber", item->rawValue("docnumber").toString());
      q.exec();
      if(!q.first())
        return;

      params.append("mode", "view");
      params.append("aropen_id", q.value("aropen_id").toInt());
      arOpenItem newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
    }
  }
  else if(item->rawValue("gltrans_doctype").toString() == "SO")
  {
    QStringList docnumber = item->rawValue("docnumber").toString().split("-");
    q.prepare("SELECT cohead_id"
              "  FROM cohead"
              " WHERE (cohead_number=:docnumber)");
    q.bindValue(":docnumber", docnumber[0]);
    q.exec();
    if(q.first())
      salesOrder::viewSalesOrder(q.value("cohead_id").toInt());
  }
  else if(item->rawValue("gltrans_doctype").toString() == "WO")
  {
    QStringList docnumber = item->rawValue("docnumber").toString().split("-");
    params.append("wo_number", docnumber[0]);

    dspWoHistoryByNumber *newdlg = new dspWoHistoryByNumber();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
  else if(item->rawValue("gltrans_source").toString() == "I/M")
  {
    q.prepare("SELECT gltrans_misc_id"
              "  FROM gltrans"
              " WHERE (gltrans_id=:gltrans_id)");
    q.bindValue(":gltrans_id", item->id());
    q.exec();
    if(!q.first())
      return;

    params.append("mode", "view");
    params.append("invhist_id", q.value("gltrans_misc_id").toInt());

    transactionInformation newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
  }
}

bool dspGLTransactions::forwardUpdate()
{
  QString sql( "SELECT MIN(forwardUpdateAccount(accnt_id)) AS result "
               "FROM accnt "
               "<? if exists(\"accnt_id\") ?>"
               " WHERE (accnt_id=<? value(\"accnt_id\") ?>)"
               "<? endif ?>"
               ";" );

  ParameterList params;
  setParams(params);
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("forwardUpdateTrialBalance", result), __FILE__, __LINE__);
      return false;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  return true;
}

void dspGLTransactions::handleTotalCheckbox()
{
  QVariant param;
  ParameterList params;
  _parameterWidget->appendValue(params);

  _showRunningTotal->setEnabled(params.inList("accnt_id") &&
                                !params.inList("source_id") &&
                                !params.inList("docnum"));

  _beginningBalanceLit->setVisible(_showRunningTotal->isChecked() &&
                                   _showRunningTotal->isEnabled());
  _beginningBalance->setVisible(_showRunningTotal->isChecked() &&
                                _showRunningTotal->isEnabled());
}

