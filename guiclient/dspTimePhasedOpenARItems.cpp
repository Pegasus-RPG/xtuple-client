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
#include "dspAROpenItemsByCustomer.h"
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

  _customerTypes->setType(XComboBox::CustomerTypes);
  
  _aropen->addColumn(tr("Cust. #"),  _orderColumn, Qt::AlignLeft, true, "araging_cust_number" );
  _aropen->addColumn(tr("Customer"), 180,          Qt::AlignLeft, true, "araging_cust_name" );

  _allCustomers->setFocus();
  
  _asOf->setDate(omfgThis->dbDate(), true);
  sToggleCustom();

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
}

dspTimePhasedOpenARItems::~dspTimePhasedOpenARItems()
{
  // no need to delete child widgets, Qt does it all for us
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

  if (_selectedCustomer->isChecked())
    params.append("cust_id", _cust->id());
  else if (_selectedCustomerType->isChecked())
    params.append("custtype_id", _customerTypes->id());
  else if (_customerTypePattern->isChecked())
    params.append("custtype_pattern", _customerType->text());

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
  params.append("run");
  params.append("asofDate", _asOf->date());

  dspAROpenItemsByCustomer *newdlg = new dspAROpenItemsByCustomer();
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
