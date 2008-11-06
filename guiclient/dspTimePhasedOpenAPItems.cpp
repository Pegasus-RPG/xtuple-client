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

#include "dspTimePhasedOpenAPItems.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include <datecluster.h>

#include "dspAPOpenItemsByVendor.h"
#include "guiclient.h"
#include "submitReport.h"

dspTimePhasedOpenAPItems::dspTimePhasedOpenAPItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_apopen, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*,int)));
  connect(_custom, SIGNAL(toggled(bool)), this, SLOT(sToggleCustom()));
  connect(_print,  SIGNAL(clicked()),     this, SLOT(sPrint()));
  connect(_query,  SIGNAL(clicked()),     this, SLOT(sFillList()));
  connect(_submit, SIGNAL(clicked()),     this, SLOT(sSubmit()));

  _vendorTypes->setType(XComboBox::VendorTypes);

  _asOf->setDate(omfgThis->dbDate(), true);
  sToggleCustom();

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();

  if(_preferences->value("APAgingDefaultDate") == "doc")
    _useDocDate->setChecked(true);
  else
    _useDistDate->setChecked(true);
}

dspTimePhasedOpenAPItems::~dspTimePhasedOpenAPItems()
{
  // no need to delete child widgets, Qt does it all for us
  QString str("dist");
  if(_useDocDate->isChecked())
    str = "doc";
  _preferences->set("APAgingDefaultDate", str);
}

void dspTimePhasedOpenAPItems::languageChange()
{
  retranslateUi(this);
}

bool dspTimePhasedOpenAPItems::setParams(ParameterList &params)
{
  if ((_custom->isChecked() && ! _periods->isPeriodSelected()) ||
      (!_custom->isChecked() && ! _asOf->isValid()))
  {
    QMessageBox::critical(this, tr("Incomplete criteria"),
                          tr("<p>The criteria you specified are not complete. "
                             "Please make sure all fields are correctly filled "
                             "out before running the report." ) );
    return false;
  }

  if (_selectedVendor->isChecked())
    params.append("vend_id", _vend->id());
  else if (_selectedVendorType->isChecked())
    params.append("vendtype_id", _vendorTypes->id());
  else if (_vendorTypePattern->isChecked())
    params.append("vendtype_pattern", _vendorType->text());

  if(_custom->isChecked())
  {
    QList<QTreeWidgetItem*> selected = _periods->selectedItems();
    QList<QVariant> periodList;
    for (int i = 0; i < selected.size(); i++)
      periodList.append(((XTreeWidgetItem*)selected[i])->id());
    params.append("period_id_list", periodList);

    params.append("report_name", "TimePhasedOpenAPItems");
  }
  else
  {
    params.append("relDate", _asOf->date());
    params.append("report_name", "APAging");
  }

  // have both in case we add a third option
  params.append("useDocDate",  QVariant(_useDocDate->isChecked()));
  params.append("useDistDate", QVariant(_useDistDate->isChecked()));

  return true;
}

void dspTimePhasedOpenAPItems::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  QString reportName;
  if(_custom->isChecked())
    reportName = "TimePhasedOpenAPItems";
  else
    reportName = "APAging";
  
  orReport report(reportName, params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    return;
  }
}

void dspTimePhasedOpenAPItems::sSubmit()
{
  ParameterList params;
  if (! setParams(params))
    return;

  submitReport newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.check() == cNoReportDefinition)
    QMessageBox::critical(this, tr("Report Definition Not Found"),
                          tr("<p>The report defintions for this report, "
                             "\"TimePhasedOpenAPItems\" cannot be found. "
                             "Please contact your Systems Administrator and "
                             "report this issue." ) );
  else
    newdlg.exec();
}

void dspTimePhasedOpenAPItems::sViewOpenItems()
{
  ParameterList params;
  params.append("vend_id", _apopen->id());
  if (_custom->isChecked())
  {
    params.append("startDate", _columnDates[_column - 2].startDate);
    params.append("endDate", _columnDates[_column - 2].endDate);
  }
  else
  {
    QDate asOfDate;
    asOfDate = _asOf->date();
    if (_column == 3)
    {
      params.append("startDate", asOfDate );
    }
    else if (_column == 4)
    {
      params.append("startDate", asOfDate.addDays(-30) );
      params.append("endDate", asOfDate);
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
  params.append("asofDate",_asOf->date());

  dspAPOpenItemsByVendor *newdlg = new dspAPOpenItemsByVendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspTimePhasedOpenAPItems::sPopulateMenu(QMenu *menuThis, QTreeWidgetItem *, int pColumn)
{
  int intMenuItem;
  _column = pColumn;

  if ((_column > 1) && (_apopen->id() > 0))
    intMenuItem = menuThis->insertItem(tr("View Open Items..."), this, SLOT(sViewOpenItems()), 0);
}

void dspTimePhasedOpenAPItems::sFillList()
{
  if (_custom->isChecked())
    sFillCustom();
  else
    sFillStd();
}

void dspTimePhasedOpenAPItems::sFillCustom()
{
  if (!_periods->isPeriodSelected())
  {
    if (isVisible())
      QMessageBox::warning( this, tr("Select Calendar Periods"),
                            tr("Please select one or more Calendar Periods") );
    return;
  }

  _columnDates.clear();
  _apopen->setColumnCount(2);

  QString sql("SELECT vend_id, vend_number, vend_name");
  QStringList linetotal;

  int columns = 1;
  QList<QTreeWidgetItem*> selected = _periods->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    PeriodListViewItem *cursor = (PeriodListViewItem*)selected[i];
    QString bucketname = QString("bucket%1").arg(columns++);
    sql += QString(", openAPItemsValue(vend_id, %2) AS %1,"
                   " 'curr' AS %3_xtnumericrole, 0 AS %4_xttotalrole")
          .arg(bucketname)
          .arg(cursor->id())
          .arg(bucketname)
          .arg(bucketname);

    _apopen->addColumn(formatDate(cursor->startDate()), _bigMoneyColumn, Qt::AlignRight, true, bucketname);
    _columnDates.append(DatePair(cursor->startDate(), cursor->endDate()));
    linetotal << QString("openAPItemsValue(vend_id, %1)").arg(cursor->id());
  }

  _apopen->addColumn(tr("Total"), _bigMoneyColumn, Qt::AlignRight, true, "linetotal");
  sql += ", " + linetotal.join("+") + " AS linetotal,"
         " 'curr' AS linetotal_xtnumericrole,"
         " 0 AS linetotal_xttotalrole,"
         " (" + linetotal.join("+") + ") = 0.0 AS xthiddenrole "
         "FROM vend "
         "<? if exists(\"vend_id\") ?>"
         "WHERE (vend_id=<? value (\"vend_id\") ?>)"
         "<? elseif exists(\"vendtype_id\") ?>"
         "WHERE (vend_vendtype_id=<? value (\"vendtype_id\") ?>"
         "<? elseif exists(\"vendtype_code\") ?>"
         "WHERE (vend_vendtype_id IN (SELECT vendtype_id FROM vendtype WHERE (vendtype_code ~ <? value (\"vendtype_pattern\") ?>)) "
         "<? endif ?>"
         "ORDER BY vend_number;";

  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;

  q = mql.toQuery(params);
  _apopen->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspTimePhasedOpenAPItems::sFillStd()
{
  MetaSQLQuery mql = mqlLoad("apAging", "detail");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _apopen->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspTimePhasedOpenAPItems::sToggleCustom()
{
  if (_custom->isChecked())
  {
    _calendarLit->setHidden(FALSE);
    _calendar->setHidden(FALSE);
    _periods->setHidden(FALSE);
    _asOf->setDate(omfgThis->dbDate(), true);
    _asOf->setEnabled(FALSE);
    _useGroup->setHidden(TRUE);

    _apopen->setColumnCount(0);
    _apopen->addColumn(tr("Vend. #"), _orderColumn, Qt::AlignLeft, true, "vend_number");
    _apopen->addColumn(tr("Vendor"),  180,          Qt::AlignLeft, true, "vend_name");
  }
  else
  {
    _calendarLit->setHidden(TRUE);
    _calendar->setHidden(TRUE);
    _periods->setHidden(TRUE);
    _asOf->setEnabled(TRUE);
    _useGroup->setHidden(FALSE);

    _apopen->addColumn(tr("Total Open"), _bigMoneyColumn, Qt::AlignRight, true, "total_val");
    _apopen->addColumn(tr("0+ Days"),    _bigMoneyColumn, Qt::AlignRight, true, "cur_val");
    _apopen->addColumn(tr("0-30 Days"),  _bigMoneyColumn, Qt::AlignRight, true, "thirty_val");
    _apopen->addColumn(tr("31-60 Days"), _bigMoneyColumn, Qt::AlignRight, true, "sixty_val");
    _apopen->addColumn(tr("61-90 Days"), _bigMoneyColumn, Qt::AlignRight, true, "ninety_val");
    _apopen->addColumn(tr("90+ Days"),   _bigMoneyColumn, Qt::AlignRight, true, "plus_val");
    _apopen->setColumnCount(0);
    _apopen->addColumn(tr("Vend. #"), _orderColumn, Qt::AlignLeft, true, "apaging_vend_number");
    _apopen->addColumn(tr("Vendor"),  180,          Qt::AlignLeft, true, "apaging_vend_name");
    _apopen->addColumn(tr("Total Open"), _bigMoneyColumn, Qt::AlignRight, true, "apaging_total_val_sum");
    _apopen->addColumn(tr("0+ Days"),    _bigMoneyColumn, Qt::AlignRight, true, "apaging_cur_val_sum");
    _apopen->addColumn(tr("0-30 Days"),  _bigMoneyColumn, Qt::AlignRight, true, "apaging_thirty_val_sum");
    _apopen->addColumn(tr("31-60 Days"), _bigMoneyColumn, Qt::AlignRight, true, "apaging_sixty_val_sum");
    _apopen->addColumn(tr("61-90 Days"), _bigMoneyColumn, Qt::AlignRight, true, "apaging_ninety_val_sum");
    _apopen->addColumn(tr("90+ Days"),   _bigMoneyColumn, Qt::AlignRight, true, "apaging_plus_val_sum");
  }
}
