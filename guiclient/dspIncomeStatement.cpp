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

#include "dspIncomeStatement.h"

#include <qvariant.h>
#include "rptIncomeStatement.h"

/*
 *  Constructs a dspIncomeStatement as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspIncomeStatement::dspIncomeStatement(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspIncomeStatement::~dspIncomeStatement()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspIncomeStatement::languageChange()
{
    retranslateUi(this);
}


void dspIncomeStatement::init()
{
  _period->setType(XComboBox::AccountingPeriods);

  _list->addColumn(tr("Account"),                     -1, Qt::AlignLeft  );
  _list->addColumn(tr("Current Period"), _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("Prior Period"),   _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("Difference"),     _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("% Diff"),        _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("Year to Date"),   _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("Prior YTD"),      _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("Difference"),     _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("% Diff"),        _bigMoneyColumn, Qt::AlignRight );
}

void dspIncomeStatement::sPrint()
{
  ParameterList params;
  params.append("period_id", _period->id());
  params.append("print");
  
  rptIncomeStatement newdlg( this, "", TRUE);
  newdlg.set(params);
}

void dspIncomeStatement::sFillList()
{
  _list->clear();

  q.prepare( "SELECT createGLReport(:periodid) AS result;");
  q.bindValue(":periodid", _period->id());
  q.exec();

  XListViewItem * root = new XListViewItem(_list, 0, -1, QVariant(tr("Revenue")));
  XListViewItem * child = new XListViewItem(root, 0, -1, QVariant(tr("Sales")));
  XListViewItem * last = 0;

  q.prepare( "SELECT (workglitem_account||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cyp_balance) AS CYP,"
             "       formatMoney(workglitem_pyp_balance) AS PYP,"
             "       formatMoney(workglitem_cyp_balance - workglitem_pyp_balance) AS period_diff,"
             "       CASE WHEN workglitem_cyp_balance = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workglitem_cyp_balance - workglitem_pyp_balance) / workglitem_cyp_balance) "
             "       END AS period_diff_percent,"
             "       formatMoney(workglitem_cy_balance) AS CY,"
             "       formatMoney(workglitem_py_balance) AS PY,"
             "       formatMoney(workglitem_cy_balance - workglitem_py_balance) AS prior_year_diff,"
             "       CASE WHEN workglitem_cy_balance = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workglitem_cy_balance - workglitem_py_balance) / workglitem_cy_balance) "
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'R')"
             "   AND   (workglitem_subaccnttype_code = 'SI'))"
             " ORDER BY workglitem_account" );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"),
                             q.value("CY"), q.value("PY"),
                             q.value("prior_year_diff"), q.value("prior_year_percent") );

  q.prepare( "SELECT formatMoney(COALESCE(SUM(workglitem_cyp_balance),0)) AS CYP,"
             "       formatMoney(COALESCE(SUM(workglitem_pyp_balance),0)) AS PYP,"
             "       formatMoney(COALESCE(SUM(workglitem_cyp_balance),0) - COALESCE(SUM(workglitem_pyp_balance),0)) AS period_diff,"
             "       CASE WHEN COALESCE(SUM(workglitem_cyp_balance),0) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) / SUM(workglitem_cyp_balance)) "
             "       END AS period_diff_percent,"
             "       formatMoney(COALESCE(SUM(workglitem_cy_balance),0)) AS CY,"
             "       formatMoney(COALESCE(SUM(workglitem_py_balance),0)) AS PY,"
             "       formatMoney(COALESCE(SUM(workglitem_cy_balance),0) - COALESCE(SUM(workglitem_py_balance),0)) AS prior_year_diff,"
             "       CASE WHEN COALESCE(SUM(workglitem_cy_balance),0) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) / SUM(workglitem_cy_balance)) "
             "       END AS prior_year_percent"
             "       FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'R')"
             "   AND   (workglitem_subaccnttype_code = 'SI'))" );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"),
                           q.value("CY"), q.value("PY"),
                           q.value("prior_year_diff"), q.value("prior_year_percent") );
  child->setOpen(TRUE);

  last = 0;
  child = new XListViewItem(root, child, -1, QVariant(tr("Cost of Goods Sold")));

  q.prepare( "SELECT (workglitem_account||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cyp_balance * -1) AS CYP,"
             "       formatMoney(workglitem_pyp_balance * -1) AS PYP,"
             "       formatMoney((workglitem_cyp_balance - workglitem_pyp_balance) * -1) AS period_diff,"
             "       CASE WHEN workglitem_cyp_balance = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((workglitem_cyp_balance - workglitem_pyp_balance) / workglitem_cyp_balance))"
             "       END AS period_diff_percent,"
             "       formatMoney(workglitem_cy_balance * -1) AS CY,"
             "       formatMoney(workglitem_py_balance * -1) AS PY,"
             "       formatMoney((workglitem_cy_balance - workglitem_py_balance) * -1) AS prior_year_diff,"
             "       CASE WHEN workglitem_cy_balance = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((workglitem_cy_balance - workglitem_py_balance) / workglitem_cy_balance))"
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'E')"
             "   AND   (workglitem_subaccnttype_code = 'COGS'))"
             " ORDER BY workglitem_account" );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"),
                             q.value("CY"), q.value("PY"),
                             q.value("prior_year_diff"), q.value("prior_year_percent") );

  q.prepare( "SELECT formatMoney(COALESCE(SUM(workglitem_cyp_balance),0) * -1) AS CYP,"
             "       formatMoney(COALESCE(SUM(workglitem_pyp_balance),0) * -1) AS PYP,"
             "       formatMoney((COALESCE(SUM(workglitem_cyp_balance),0) - COALESCE(SUM(workglitem_pyp_balance),0)) * -1) AS period_diff,"
             "       CASE WHEN COALESCE(SUM(workglitem_cyp_balance),0) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) / SUM(workglitem_cyp_balance)))"
             "       END AS period_diff_percent,"
             "       formatMoney(COALESCE(SUM(workglitem_cy_balance),0) * -1) AS CY,"
             "       formatMoney(COALESCE(SUM(workglitem_py_balance),0) * -1) AS PY,"
             "       formatMoney((COALESCE(SUM(workglitem_cy_balance),0) - COALESCE(SUM(workglitem_py_balance),0)) * -1) AS prior_year_diff,"
             "       CASE WHEN COALESCE(SUM(workglitem_cy_balance),0) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) / SUM(workglitem_cy_balance)))"
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'E')"
             "   AND   (workglitem_subaccnttype_code = 'COGS'))" );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"),
                           q.value("CY"), q.value("PY"),
                           q.value("prior_year_diff"), q.value("prior_year_percent") );
  child->setOpen(TRUE);

  q.prepare( "SELECT formatMoney(SUM(workglitem_cyp_balance)) AS CYP,"
             "       formatMoney(SUM(workglitem_pyp_balance)) AS PYP,"
             "       formatMoney(SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cyp_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) / SUM(workglitem_cyp_balance)) "
             "       END AS period_diff_percent,"
             "       formatMoney(SUM(workglitem_cy_balance)) AS CY,"
             "       formatMoney(SUM(workglitem_py_balance)) AS PY,"
             "       formatMoney(SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) AS prior_year_diff,"
             "       CASE WHEN SUM(workglitem_cy_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) / SUM(workglitem_cy_balance)) "
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND (((workglitem_accnt_type = 'R')"
             "   AND   (workglitem_subaccnttype_code = 'SI'))"
             "     OR ((workglitem_accnt_type = 'E')"
             "     AND (workglitem_subaccnttype_code = 'COGS')))) " );
  q.exec();
  q.first();
  last = new XListViewItem(root, child, -1,
                           QVariant(tr("Margin")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"),
                           q.value("CY"), q.value("PY"),
                           q.value("prior_year_diff"), q.value("prior_year_percent") );

  child = new XListViewItem(root, last, -1, QVariant(tr("Other Income")));
  last = 0;

  q.prepare( "SELECT (workglitem_account||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cyp_balance) AS CYP,"
             "       formatMoney(workglitem_pyp_balance) AS PYP,"
             "       formatMoney(workglitem_cyp_balance - workglitem_pyp_balance) AS period_diff,"
             "       CASE WHEN workglitem_cyp_balance = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workglitem_cyp_balance - workglitem_pyp_balance) / workglitem_cyp_balance) "
             "       END AS period_diff_percent,"
             "       formatMoney(workglitem_cy_balance) AS CY,"
             "       formatMoney(workglitem_py_balance) AS PY,"
             "       formatMoney(workglitem_cy_balance - workglitem_py_balance) AS prior_year_diff,"
             "       CASE WHEN workglitem_cy_balance = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workglitem_cy_balance - workglitem_py_balance) / workglitem_cy_balance) "
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'R')"
             "   AND   (workglitem_subaccnttype_code != 'SI'))"
             " ORDER BY workglitem_account" );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"),
                             q.value("CY"), q.value("PY"),
                             q.value("prior_year_diff"), q.value("prior_year_percent") );

  q.prepare( "SELECT formatMoney(SUM(workglitem_cyp_balance)) AS CYP,"
             "       formatMoney(SUM(workglitem_pyp_balance)) AS PYP,"
             "       formatMoney(SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cyp_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) / SUM(workglitem_cyp_balance)) "
             "       END AS period_diff_percent,"
             "       formatMoney(SUM(workglitem_cy_balance)) AS CY,"
             "       formatMoney(SUM(workglitem_py_balance)) AS PY,"
             "       formatMoney(SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) AS prior_year_diff,"
             "       CASE WHEN SUM(workglitem_cy_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) / SUM(workglitem_cy_balance)) "
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'R')"
             "   AND   (workglitem_subaccnttype_code != 'SI'))" );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"),
                           q.value("CY"), q.value("PY"),
                           q.value("prior_year_diff"), q.value("prior_year_percent") );
  child->setOpen(TRUE);

  q.prepare( "SELECT formatMoney(SUM(workglitem_cyp_balance)) AS CYP,"
             "       formatMoney(SUM(workglitem_pyp_balance)) AS PYP,"
             "       formatMoney(SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cyp_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) / SUM(workglitem_cyp_balance)) "
             "       END AS period_diff_percent,"
             "       formatMoney(SUM(workglitem_cy_balance)) AS CY,"
             "       formatMoney(SUM(workglitem_py_balance)) AS PY,"
             "       formatMoney(SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) AS prior_year_diff,"
             "       CASE WHEN SUM(workglitem_cy_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) / SUM(workglitem_cy_balance)) "
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   ((workglitem_accnt_type = 'R')"
             "       OR (workglitem_accnt_type = 'E')"
             "      AND (workglitem_subaccnttype_code = 'COGS'))) " );
  q.exec();
  q.first();
  last = new XListViewItem(root, child, -1,
                           QVariant(tr("Total Income")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"),
                           q.value("CY"), q.value("PY"),
                           q.value("prior_year_diff"), q.value("prior_year_percent") );

  root->setOpen(TRUE);
  last = 0;

  root = new XListViewItem(_list, root, -1, QVariant(tr("Expenses")));

  q.prepare( "SELECT (workglitem_account||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cyp_balance * -1) AS CYP,"
             "       formatMoney(workglitem_pyp_balance * -1) AS PYP,"
             "       formatMoney((workglitem_cyp_balance - workglitem_pyp_balance) * -1) AS period_diff,"
             "       CASE WHEN workglitem_cyp_balance = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((workglitem_cyp_balance - workglitem_pyp_balance) / workglitem_cyp_balance))"
             "       END AS period_diff_percent,"
             "       formatMoney(workglitem_cy_balance * -1) AS CY,"
             "       formatMoney(workglitem_py_balance * -1) AS PY,"
             "       formatMoney((workglitem_cy_balance - workglitem_py_balance) * -1) AS prior_year_diff,"
             "       CASE WHEN workglitem_cy_balance = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((workglitem_cy_balance - workglitem_py_balance) / workglitem_cy_balance))"
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'E')"
             "   AND   ((workglitem_subaccnttype_code != 'COGS')"
             "       OR (workglitem_subaccnttype_code IS NULL)) )"
             " ORDER BY workglitem_account" );
  q.exec();
  while(q.next())
    last = new XListViewItem(root, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"),
                             q.value("CY"), q.value("PY"),
                             q.value("prior_year_diff"), q.value("prior_year_percent") );

  q.prepare( "SELECT formatMoney(SUM(workglitem_cyp_balance) * -1) AS CYP,"
             "       formatMoney(SUM(workglitem_pyp_balance) * -1) AS PYP,"
             "       formatMoney((SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) * -1) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cyp_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) / SUM(workglitem_cyp_balance)))"
             "       END AS period_diff_percent,"
             "       formatMoney(SUM(workglitem_cy_balance) * -1) AS CY,"
             "       formatMoney(SUM(workglitem_py_balance) * -1) AS PY,"
             "       formatMoney((SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) * -1) AS prior_year_diff,"
             "       CASE WHEN SUM(workglitem_cy_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) / SUM(workglitem_cy_balance)))"
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'E')"
             "   AND   ((workglitem_subaccnttype_code != 'COGS')"
             "       OR (workglitem_subaccnttype_code IS NULL)) )" );
  q.exec();
  q.first();
  last = new XListViewItem(root, last, -1,
                           QVariant(tr("Total Expenses")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"),
                           q.value("CY"), q.value("PY"),
                           q.value("prior_year_diff"), q.value("prior_year_percent") );

  root->setOpen(TRUE);
  
  q.prepare( "SELECT formatMoney(SUM(workglitem_cyp_balance)) AS CYP,"
             "       formatMoney(SUM(workglitem_pyp_balance)) AS PYP,"
             "       formatMoney(SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cyp_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cyp_balance) - SUM(workglitem_pyp_balance)) / SUM(workglitem_cyp_balance)) "
             "       END AS period_diff_percent,"
             "       formatMoney(SUM(workglitem_cy_balance)) AS CY,"
             "       formatMoney(SUM(workglitem_py_balance)) AS PY,"
             "       formatMoney(SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) AS prior_year_diff,"
             "       CASE WHEN SUM(workglitem_cy_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) / SUM(workglitem_cy_balance)) "
             "       END AS prior_year_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   ((workglitem_accnt_type = 'R')"
             "       OR (workglitem_accnt_type = 'E')) )" );
  q.exec();
  q.first();
  last = new XListViewItem(_list, root, -1,
                           QVariant(tr("Profit/Loss")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"),
                           q.value("CY"), q.value("PY"),
                           q.value("prior_year_diff"), q.value("prior_year_percent") );

}


