/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedOpenARItems.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

#include <datecluster.h>

#include "printStatementByCustomer.h"
#include "dspAROpenItems.h"
#include "submitReport.h"

dspTimePhasedOpenARItems::dspTimePhasedOpenARItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_aropen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_custom, SIGNAL(toggled(bool)), this, SLOT(sToggleCustom()));
  connect(_print,  SIGNAL(clicked()),     this, SLOT(sPrint()));
  connect(_query,  SIGNAL(clicked()),     this, SLOT(sFillList()));
  connect(_submit, SIGNAL(clicked()),     this, SLOT(sSubmit()));

  
  _aropen->addColumn(tr("Cust. #"),  _orderColumn, Qt::AlignLeft, true, "araging_cust_number" );
  _aropen->addColumn(tr("Customer"), 180,          Qt::AlignLeft, true, "araging_cust_name" );
  
  _asOf->setDate(omfgThis->dbDate(), true);
  sToggleCustom();

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  if(_preferences->value("ARAgingDefaultDate") == "doc")
    _useDocDate->setChecked(true);
  else
    _useDistDate->setChecked(true);
}

dspTimePhasedOpenARItems::~dspTimePhasedOpenARItems()
{
  // no need to delete child widgets, Qt does it all for us
  QString str("dist");
  if(_useDocDate->isChecked())
    str = "doc";
  _preferences->set("ARAgingDefaultDate", str);
}

void dspTimePhasedOpenARItems::languageChange()
{
  retranslateUi(this);
}

bool dspTimePhasedOpenARItems::setParams(ParameterList &params)
{
  if ((_custom->isChecked() && ! _periods->isPeriodSelected()) ||
      (!_custom->isChecked() && !_asOf->isValid()))
  {
    QMessageBox::critical(this, tr("Incomplete criteria"),
                          tr("<p>The criteria you specified are not complete. "
                             "Please make sure all fields are correctly filled "
                             "out before running the report." ) );
    return false;
  }

  _customerSelector->appendValue(params);

  if(_custom->isChecked())
  {
    params.append("report_name", "TimePhasedOpenARItems");

    QList<QTreeWidgetItem*> selected = _periods->selectedItems();
    QList<QVariant> periodList;
    for (int i = 0; i < selected.size(); i++)
      periodList.append(((XTreeWidgetItem*)selected[i])->id());
    params.append("period_id_list", periodList);
  }
  else
  {
    params.append("report_name", "ARAging");
    params.append("relDate", _asOf->date());
  }

  // have both in case we add a third option
  params.append("useDocDate",  QVariant(_useDocDate->isChecked()));
  params.append("useDistDate", QVariant(_useDistDate->isChecked()));

  return true;
}

void dspTimePhasedOpenARItems::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  QString reportName;
  if(_custom->isChecked())
    reportName = "TimePhasedOpenARItems";
  else
    reportName = "ARAging";
  orReport report(reportName, params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    return;
  }
}

void dspTimePhasedOpenARItems::sSubmit()
{
  ParameterList params;
  if (! setParams(params))
    return;

  submitReport newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.check() == cNoReportDefinition)
    QMessageBox::critical( this, tr("Report Definition Not Found"),
                           tr("<p>The report defintions for this report, "
                              "\"TimePhasedOpenARItems\" cannot be found. "
                              "Please contact your Systems Administrator and "
                              "report this issue." ) );
  else
    newdlg.exec();
}

void dspTimePhasedOpenARItems::sViewOpenItems()
{
  ParameterList params;
  params.append("cust_id", _aropen->id());
  if (_custom->isChecked())
  {
    params.append("startDate", _periods->getSelected(_column - 1)->startDate());
    params.append("endDate", _periods->getSelected(_column - 1)->endDate());
  }
  else
  {
    QDate asOfDate;
    asOfDate = _asOf->date();
    if (_column == 3)
      params.append("startDate", asOfDate );
    else if (_column == 4)
    {
      params.append("startDate", asOfDate.addDays(-30) );
      params.append("endDate", asOfDate.addDays(-1));
    }
    else if (_column == 5)
    {
      params.append("startDate",asOfDate.addDays(-60) );
      params.append("endDate", asOfDate.addDays(-31));
    }
    else if (_column == 6)
    {
      params.append("startDate",asOfDate.addDays(-90) );
      params.append("endDate", asOfDate.addDays(-61));
    }
    else if (_column == 7)
      params.append("endDate",asOfDate.addDays(-91) );
  }
  params.append("byDueDate");
  params.append("run");
  params.append("asofDate", _asOf->date());

  dspAROpenItems *newdlg = new dspAROpenItems();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedOpenARItems::sPrintStatement()
{
  ParameterList params;
  params.append("cust_id", _aropen->id());
  params.append("print");

  printStatementByCustomer newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspTimePhasedOpenARItems::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *, int pColumn)
{
  int menuItem;
  _column = pColumn;
  
  if ((_column > 1) && (_aropen->id() > 0))
  {
    menuItem = pMenu->insertItem(tr("View Open Items..."), this, SLOT(sViewOpenItems()), 0);

    pMenu->insertSeparator();

    menuItem = pMenu->insertItem(tr("Print Statement..."), this, SLOT(sPrintStatement()), 0);
  }
}

void dspTimePhasedOpenARItems::sFillList()
{
  if (_custom->isChecked())
    sFillCustom();
  else
    sFillStd();
}

void dspTimePhasedOpenARItems::sFillCustom()
{
  if (!_periods->isPeriodSelected())
  {
    if (isVisible())
      QMessageBox::warning( this, tr("Select Calendar Periods"),
                            tr("Please select one or more Calendar Periods") );
    return;
  }

  _columnDates.clear();
  _aropen->setColumnCount(2);

  QString sql("SELECT cust_id, cust_number, cust_name");
  QStringList linetotal;

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);
    sql += QString(", openARItemsValue(cust_id, %2) AS %1,"
                   " 'curr' AS %3_xtnumericrole, 0 AS %4_xttotalrole")
	   .arg(bucketname)
	   .arg(cursor->id())
           .arg(bucketname)
           .arg(bucketname);

    _aropen->addColumn(formatDate(cursor->startDate()), _bigMoneyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
    linetotal << QString("openARItemsValue(cust_id, %2)").arg(cursor->id());
  }

  _aropen->addColumn(tr("Total"), _bigMoneyColumn, Qt::AlignRight, true, "linetotal");

  sql += ", " + linetotal.join("+") + " AS linetotal,"
         " 'curr' AS linetotal_xtnumericrole,"
         " 0 AS linetotal_xttotalrole,"
         " (" + linetotal.join("+") + ") = 0.0 AS xthiddenrole "
         "FROM cust "
         "<? if exists(\"cust_id\") ?>"
         "WHERE (cust_id=<? value(\"cust_id\") ?>)"
         "<? elseif exists(\"custtype_id\") ?>"
         "WHERE (cust_custtype_id=<? value(\"custtype_id\") ?>)"
         "<? elseif exists(\"custtype_pattern\") ?>"
         "WHERE (cust_custtype_id IN (SELECT custtype_id FROM custtype WHERE (custtype_code ~ <? value(\"custtype_pattern\") ?>))) "
         "<? endif ?>"
         "ORDER BY cust_number;";

  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _aropen->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspTimePhasedOpenARItems::sFillStd()
{
  MetaSQLQuery mql = mqlLoad("arAging", "detail");
  ParameterList params;
  if (! setParams(params))
    return;

  q = mql.toQuery(params);
  _aropen->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspTimePhasedOpenARItems::sToggleCustom()
{
  if (_custom->isChecked())
  {
    _calendarLit->setHidden(FALSE);
    _calendar->setHidden(FALSE);
    _periods->setHidden(FALSE);
    _asOf->setDate(omfgThis->dbDate(), true);
    _asOf->setEnabled(FALSE);
    _useGroup->setHidden(TRUE);

    _aropen->setColumnCount(0);
    _aropen->addColumn(tr("Cust. #"), _orderColumn, Qt::AlignLeft, true, "cust_number");
    _aropen->addColumn(tr("Customer"),         180, Qt::AlignLeft, true, "cust_name");
  }
  else
  {
    _calendarLit->setHidden(TRUE);
    _calendar->setHidden(TRUE);
    _periods->setHidden(TRUE);
    _asOf->setEnabled(TRUE);
    _useGroup->setHidden(FALSE);

    _aropen->setColumnCount(0);
    _aropen->addColumn(tr("Cust. #"),       _orderColumn, Qt::AlignLeft,  true, "araging_cust_number");
    _aropen->addColumn(tr("Customer"),               180, Qt::AlignLeft,  true, "araging_cust_name");
    _aropen->addColumn(tr("Total Open"), _bigMoneyColumn, Qt::AlignRight, true, "araging_total_val_sum");
    _aropen->addColumn(tr("0+ Days"),    _bigMoneyColumn, Qt::AlignRight, true, "araging_cur_val_sum");
    _aropen->addColumn(tr("0-30 Days"),  _bigMoneyColumn, Qt::AlignRight, true, "araging_thirty_val_sum");
    _aropen->addColumn(tr("31-60 Days"), _bigMoneyColumn, Qt::AlignRight, true, "araging_sixty_val_sum");
    _aropen->addColumn(tr("61-90 Days"), _bigMoneyColumn, Qt::AlignRight, true, "araging_ninety_val_sum");
    _aropen->addColumn(tr("90+ Days"),   _bigMoneyColumn, Qt::AlignRight, true, "araging_plus_val_sum");
  }
}
