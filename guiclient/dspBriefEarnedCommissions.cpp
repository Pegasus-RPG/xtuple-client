/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBriefEarnedCommissions.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspBriefEarnedCommissions as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBriefEarnedCommissions::dspBriefEarnedCommissions(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_selectedSalesrep, SIGNAL(toggled(bool)), _salesrep, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _salesrep->setType(XComboBox::SalesReps);

  _commission->addColumn(tr("#"),               _seqColumn,      Qt::AlignCenter, true,  "salesrep_number" );
  _commission->addColumn(tr("Sales Rep."),      _itemColumn,     Qt::AlignLeft,   true,  "salesrep_name"   );
  _commission->addColumn(tr("Cust. #"),         _orderColumn,    Qt::AlignLeft,   true,  "cust_number"   );
  _commission->addColumn(tr("Customer"),        -1,              Qt::AlignLeft,   true,  "cust_name"   );
  _commission->addColumn(tr("S/O #"),           _orderColumn,    Qt::AlignLeft,   true,  "cohist_ordernumber"   );
  _commission->addColumn(tr("Invoice #"),       _orderColumn,    Qt::AlignLeft,   true,  "cohist_invcnumber"   );
  _commission->addColumn(tr("Invc. Date"),      _dateColumn,     Qt::AlignCenter, true,  "cohist_invcdate" );
  _commission->addColumn(tr("Ext. Price"),      _moneyColumn,    Qt::AlignRight,  true,  "sumextprice"  );
  _commission->addColumn(tr("Commission"),      _moneyColumn,    Qt::AlignRight,  true,  "sumcommission"  );
  _commission->addColumn(tr("Currency"),        _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  _commission->addColumn(tr("Base Ext. Price"), _bigMoneyColumn, Qt::AlignRight,  true,  "sumbaseextprice"  );
  _commission->addColumn(tr("Base Commission"), _bigMoneyColumn, Qt::AlignRight,  true,  "sumbasecommission"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBriefEarnedCommissions::~dspBriefEarnedCommissions()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBriefEarnedCommissions::languageChange()
{
  retranslateUi(this);
}

void dspBriefEarnedCommissions::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Start and End Dates"),
                          tr("You must enter a valid Start and End Date for this report.") );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _dates->appendValue(params);

  if (_selectedSalesrep->isChecked())
    params.append("salesrep_id", _salesrep->id());

  orReport report("BriefEarnedCommissions", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBriefEarnedCommissions::sFillList()
{
  if (_dates->allValid())
  {
    QString sql( "SELECT cohist_salesrep_id, salesrep_number, salesrep_name, cust_number, cust_name,"
                 "       cohist_ordernumber, cohist_invcnumber, cohist_invcdate, currAbbr,"
                 "       SUM(extprice) AS sumextprice,"
                 "       SUM(cohist_commission) AS sumcommission,"
                 "       SUM(baseextprice) AS sumbaseextprice,"
                 "       SUM(basecommission) AS sumbasecommission,"
                 "       'curr' AS sumextprice_xtnumericrole,"
                 "       'curr' AS sumcommission_xtnumericrole,"
                 "       'curr' AS sumbaseextprice_xtnumericrole,"
                 "       'curr' AS sumbasecommission_xtnumericrole,"
                 "       0 AS sumbaseextprice_xttotalrole,"
                 "       0 AS sumbasecommission_xttotalrole "
                 "FROM saleshistory "
                 "WHERE ( (cohist_commission <> 0) "
                 "  AND   (cohist_invcdate BETWEEN :startDate AND :endDate)" );

    if (_selectedSalesrep->isChecked())
      sql += " AND (cohist_salesrep_id=:salesrep_id)";

    sql += ") "
           "GROUP BY cohist_salesrep_id, salesrep_number, salesrep_name, cust_number, cust_name,"
           "         cohist_ordernumber, cohist_invcnumber, cohist_invcdate, currAbbr "
           "ORDER BY salesrep_number, cust_number, cohist_invcdate";

    q.prepare(sql);
    _dates->bindValue(q);
    q.bindValue(":salesrep_id", _salesrep->id());
    q.exec();
    _commission->populate(q);
  }
}
