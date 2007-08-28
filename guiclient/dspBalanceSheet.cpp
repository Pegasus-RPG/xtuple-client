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

#include "dspBalanceSheet.h"

#include <qvariant.h>
#include "rptBalanceSheet.h"

/*
 *  Constructs a dspBalanceSheet as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBalanceSheet::dspBalanceSheet(QWidget* parent, const char* name, Qt::WFlags fl)
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
dspBalanceSheet::~dspBalanceSheet()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBalanceSheet::languageChange()
{
    retranslateUi(this);
}


void dspBalanceSheet::init()
{
  _period->setType(XComboBox::AccountingPeriods);

  _list->addColumn(tr("Account"),                    -1, Qt::AlignLeft  );
  _list->addColumn(tr("Current Year"),  _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("Prior Year"),    _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("Difference"),    _bigMoneyColumn, Qt::AlignRight );
  _list->addColumn(tr("Percent Diff."), _bigMoneyColumn, Qt::AlignRight );
}

void dspBalanceSheet::sPrint()
{
  ParameterList params;
  params.append("period_id", _period->id());
  params.append("print");
  
  rptBalanceSheet newdlg( this, "", TRUE);
  newdlg.set(params);
}

void dspBalanceSheet::sFillList()
{
  _list->clear();

  q.prepare( "SELECT createGLReport(:periodid) AS result;");
  q.bindValue(":periodid", _period->id());
  q.exec();

  XListViewItem * root = new XListViewItem(_list, 0, -1, QVariant(tr("Assets")));
  XListViewItem * child = new XListViewItem(root, 0, -1, QVariant(tr("Current Assets")));

  q.prepare( "SELECT (workglitem_account ||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cp_ending_trialbal * -1.00) AS CYP,"
             "       formatMoney(workglitem_pp_ending_trialbal * -1.00) AS PYP,"
             "       formatMoney((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) * -1.00) AS period_diff,"
             "       CASE WHEN workglitem_cp_ending_trialbal = 0.00 THEN formatPrcnt(0.00) "
             "       ELSE formatPrcnt((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) / workglitem_cp_ending_trialbal) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'A')"
             "   AND   (workglitem_subaccnttype_code IN ('CA', 'AR', 'IN', 'CAS')))"
             " ORDER BY workglitem_account;");
  q.exec();
  XListViewItem * last = 0;
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"));

  q.prepare( "SELECT formatMoney(SUM(workglitem_cp_ending_trialbal) * -1.00) AS CYP,"
             "       formatMoney(SUM(workglitem_pp_ending_trialbal) * -1.00) AS PYP,"
             "       formatMoney((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) * -1.00) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cp_ending_trialbal) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'A')"
             "   AND   (workglitem_subaccnttype_code IN ('CA', 'AR', 'IN', 'CAS'))) " );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);

  last = 0;
  child = new XListViewItem(root, child, -1, QVariant(tr("Fixed Assets")));

  q.prepare( "select (workglitem_account||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cp_ending_trialbal * -1.00) AS CYP,"
             "       formatMoney(workglitem_pp_ending_trialbal * -1.00) AS PYP,"
             "       formatMoney((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) * -1.00) AS period_diff,"
             "       CASE WHEN workglitem_cp_ending_trialbal = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) / workglitem_cp_ending_trialbal) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'A')"
             "   AND   (workglitem_subaccnttype_code IN ('FA', 'AD')))"
             " ORDER BY workglitem_account " );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"));

  q.prepare( "SELECT formatMoney(COALESCE(SUM(workglitem_cp_ending_trialbal),0) * -1.00) AS CYP,"
             "       formatMoney(COALESCE(SUM(workglitem_pp_ending_trialbal),0) * -1.00) AS PYP,"
             "       formatMoney((COALESCE(SUM(workglitem_cp_ending_trialbal),0) - COALESCE(SUM(workglitem_pp_ending_trialbal),0)) * -1.00) AS period_diff,"
             "       CASE WHEN COALESCE(SUM(workglitem_cp_ending_trialbal),0) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   and   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   and   (workglitem_accnt_type = 'A')"
             "   and   (workglitem_subaccnttype_code IN ('FA', 'AD'))) " );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);
  root->setOpen(TRUE);

  last = 0;
  child = new XListViewItem(root, child, -1, QVariant(tr("Other Assets")));

  q.prepare( "SELECT (workglitem_account||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cp_ending_trialbal * -1) AS CYP,"
             "       formatMoney(workglitem_pp_ending_trialbal * -1) AS PYP,"
             "       formatMoney((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) * -1) AS period_diff,"
             "       CASE WHEN workglitem_cp_ending_trialbal = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) / workglitem_cp_ending_trialbal))"
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'A')"
             "   AND   ((workglitem_subaccnttype_code NOT IN ('CA', 'AR', 'IN', 'CAS', 'FA', 'AD'))"
             "          OR (workglitem_subaccnttype_code IS NULL)) )"
             " ORDER BY workglitem_account " );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"));

  q.prepare( "SELECT formatMoney(SUM(workglitem_cp_ending_trialbal) * -1.00) AS CYP,"
             "       formatMoney(SUM(workglitem_pp_ending_trialbal) * -1.00) AS PYP,"
             "       formatMoney((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) * -1.00) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cp_ending_trialbal) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'A')"
             "   AND   ((workglitem_subaccnttype_code NOT IN ('CA', 'AR', 'IN', 'CAS', 'FA', 'AD'))"
             "           OR (workglitem_subaccnttype_code IS NULL)) ) " );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);

  q.prepare( "select formatMoney((SUM(workglitem_cp_ending_trialbal)) * -1.00) AS CYP,"
             "       formatMoney((SUM(workglitem_pp_ending_trialbal)) * -1.00) AS PYP,"
             "       formatMoney(((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal))) * -1.00) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cp_ending_trialbal) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)) "
             "       END AS period_diff_percent from workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'A')) " );
  q.exec();
  q.first();
  last = new XListViewItem(root, child, -1,
                           QVariant(tr("Total Assets")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);
  root->setOpen(TRUE);

  root = new XListViewItem(_list, root, -1, QVariant(tr("Liabilities")));
  child = new XListViewItem(root, 0, -1, QVariant(tr("Current Liabilities")));

  q.prepare( "SELECT (workglitem_account||'-'|| workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cp_ending_trialbal) AS CYP,"
             "       formatMoney(workglitem_pp_ending_trialbal) AS PYP,"
             "       formatMoney((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal)) AS period_diff,"
             "       CASE WHEN workglitem_cp_ending_trialbal = 0.00 THEN formatPrcnt(0.00) "
             "       ELSE formatPrcnt(((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) / workglitem_cp_ending_trialbal))"
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'L')"
             "   AND   (workglitem_subaccnttype_code IN ('CL', 'AP')))"
             " ORDER BY workglitem_account" );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"));

  q.prepare( "select formatMoney(SUM(workglitem_cp_ending_trialbal)) AS CYP,"
             "       formatMoney(SUM(workglitem_pp_ending_trialbal)) AS PYP,"
             "       formatMoney((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal))) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cp_ending_trialbal) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt(((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)))"
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'L')"
             "   AND   (workglitem_subaccnttype_code IN ('CL', 'AP')))" );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);

  last = 0;
  child = new XListViewItem(root, child, -1, QVariant(tr("Long Term Liabilities")));

  q.prepare( "select (workglitem_account||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cp_ending_trialbal) AS CYP,"
             "       formatMoney(workglitem_pp_ending_trialbal) AS PYP,"
             "       formatMoney(workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) AS period_diff,"
             "       CASE WHEN workglitem_cp_ending_trialbal = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) / workglitem_cp_ending_trialbal) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'L')"
             "   AND   (workglitem_subaccnttype_code = 'LTL'))"
             " ORDER BY workglitem_account" );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"));

  q.prepare( "SELECT formatMoney(SUM(workglitem_cp_ending_trialbal)) AS CYP,"
             "       formatMoney(SUM(workglitem_pp_ending_trialbal)) AS PYP,"
             "       formatMoney(SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cp_ending_trialbal) = 0.00 THEN formatPrcnt(0.00) "
             "       ELSE formatPrcnt((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'L')"
             "   AND   (workglitem_subaccnttype_code = 'LTL')) " );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);

  last = 0;
  child = new XListViewItem(root, child, -1, QVariant(tr("Other Liabilities")));

  q.prepare( "SELECT workglitem_account||'-'||workglitem_account_desc AS account,"
             "       formatMoney(workglitem_cp_ending_trialbal) AS CYP,"
             "       formatMoney(workglitem_pp_ending_trialbal) AS PYP,"
             "       formatMoney(workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) AS period_diff,"
             "       CASE WHEN workglitem_cp_ending_trialbal = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) / workglitem_cp_ending_trialbal) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND (workglitem_accnt_type = 'L')"
             "   AND ((workglitem_subaccnttype_code NOT IN ('CL', 'AP', 'LTL'))"
             "       OR (workglitem_subaccnttype_code IS NULL)) )"
             " ORDER BY workglitem_account" );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"));

  q.prepare( "select formatMoney(SUM(workglitem_cp_ending_trialbal)) AS CYP,"
             "       formatMoney(SUM(workglitem_pp_ending_trialbal)) AS PYP,"
             "       formatMoney(SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cp_ending_trialbal) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'L')"
             "   AND   ((workglitem_subaccnttype_code NOT IN ('CL', 'AP', 'LTL'))"
             "          OR (workglitem_subaccnttype_code IS NULL)) )" );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);

  q.prepare( "select formatMoney(SUM(workglitem_cp_ending_trialbal)) AS CYP,"
             "       formatMoney(SUM(workglitem_pp_ending_trialbal)) AS PYP,"
             "       formatMoney(SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cp_ending_trialbal) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'L')) " );
  q.exec();
  q.first();
  last = new XListViewItem(root, child, -1,
                           QVariant(tr("Total Liabilities")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);
  root->setOpen(TRUE);

  root = new XListViewItem(_list, root, -1, QVariant(tr("Equity")));
  child = new XListViewItem(root, 0, -1, QVariant(tr("Shareholder")));

  q.prepare( "SELECT (workglitem_account||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cp_ending_trialbal) AS CYP,"
             "       formatMoney(workglitem_pp_ending_trialbal) AS PYP,"
             "       formatMoney(workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) AS period_diff,"
             "       CASE WHEN workglitem_cp_ending_trialbal = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) / workglitem_cp_ending_trialbal) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'Q')"
             "   AND   (workglitem_subaccnttype_code = 'EDC'))"
             " ORDER BY workglitem_account" );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"));

  q.prepare( "SELECT formatMoney(SUM(workglitem_cp_ending_trialbal)) AS CYP,"
             "       formatMoney(SUM(workglitem_pp_ending_trialbal)) AS PYP,"
             "       formatMoney(SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cp_ending_trialbal) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'Q')"
             "   AND   (workglitem_subaccnttype_code = 'EDC'))" );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);

  last = 0;
  child = new XListViewItem(root, child, -1, QVariant(tr("Other Equity")));

  q.prepare( "SELECT (workglitem_account||'-'||workglitem_account_desc) AS account,"
             "       formatMoney(workglitem_cp_ending_trialbal) AS CYP,"
             "       formatMoney(workglitem_pp_ending_trialbal) AS PYP,"
             "       formatMoney(workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) AS period_diff,"
             "       CASE WHEN workglitem_cp_ending_trialbal = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workglitem_cp_ending_trialbal - workglitem_pp_ending_trialbal) / workglitem_cp_ending_trialbal) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'Q')"
             "   AND   ((workglitem_subaccnttype_code != 'EDC')"
             "          OR (workglitem_subaccnttype_code IS NULL)))"
             " ORDER BY workglitem_account" );
  q.exec();
  while(q.next())
    last = new XListViewItem(child, last, -1,
                             q.value("account"), q.value("CYP"), q.value("PYP"),
                             q.value("period_diff"), q.value("period_diff_percent"));

  q.prepare( "SELECT formatMoney(SUM(workglitem_cp_ending_trialbal)) AS CYP,"
             "       formatMoney(SUM(workglitem_pp_ending_trialbal)) AS PYP,"
             "       formatMoney(SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cp_ending_trialbal) = 0.00 THEN formatPrcnt(0.00) "
             "       ELSE formatPrcnt((SUM(workglitem_cp_ending_trialbal) - SUM(workglitem_pp_ending_trialbal)) / SUM(workglitem_cp_ending_trialbal)) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   (workglitem_accnt_type = 'Q')"
             "   AND   ((workglitem_subaccnttype_code != 'EDC')"
             "          OR (workglitem_subaccnttype_code IS NULL)))" );
  q.exec();
  q.first();
  last = new XListViewItem(child, last, -1,
                           QVariant(tr("Subtotal")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  child->setOpen(TRUE);

  // Profit/Loss
  q.prepare( "SELECT formatMoney(SUM(workglitem_cy_balance)) AS CYP,"
             "       formatMoney(SUM(workglitem_py_balance)) AS PYP,"
             "       formatMoney(SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) AS period_diff,"
             "       CASE WHEN SUM(workglitem_cy_balance) = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((SUM(workglitem_cy_balance) - SUM(workglitem_py_balance)) / SUM(workglitem_cy_balance)) "
             "       END AS period_diff_percent"
             "  FROM workglitem, workglhead"
             " WHERE ( (workglitem_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER))"
             "   AND   ((workglitem_accnt_type = 'R')"
             "          OR (workglitem_accnt_type = 'E')))" );
  q.exec();
  q.first();
  last = new XListViewItem(root, child, -1,
                           QVariant(tr("Profit/Loss")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));

  q.prepare( "SELECT formatMoney(workgltotaleq_cp_ending_total) AS CYP,"
             "       formatMoney(workgltotaleq_pp_ending_total) AS PYP,"
             "       formatMoney(workgltotaleq_cp_ending_total - workgltotaleq_pp_ending_total) AS period_diff,"
             "       CASE WHEN workgltotaleq_cp_ending_total = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workgltotaleq_cp_ending_total - workgltotaleq_pp_ending_total) / workgltotaleq_cp_ending_total) "
             "       END AS period_diff_percent"
             "  FROM workgltotaleq, workglhead"
             " WHERE ( (workgltotaleq_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER)) )" );
  q.exec();
  q.first();
  last = new XListViewItem(root, last, -1,
                           QVariant(tr("Total Equity")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));
  root->setOpen(TRUE);

  q.prepare( "select formatMoney(workgltotal_cp_ending_total) AS CYP,"
             "       formatMoney(workgltotal_pp_ending_total) AS PYP,"
             "       formatMoney(workgltotal_cp_ending_total - workgltotal_pp_ending_total) AS period_diff,"
             "       CASE WHEN workgltotal_cp_ending_total = 0.00 THEN formatPrcnt(0.00) "
             "            ELSE formatPrcnt((workgltotal_cp_ending_total - workgltotal_pp_ending_total) / workgltotal_cp_ending_total) "
             "       END AS period_diff_percent"
             "  FROM workgltotal, workglhead"
             " WHERE ( (workgltotal_workglhead_id = workglhead_id)"
             "   AND   (workglhead_usr_id = (select usr_id from usr where usr_username=CURRENT_USER)) )" );
  q.exec();
  q.first();
  root = new XListViewItem(_list, root, -1,
                           QVariant(tr("Total Liabilities & Equity")), q.value("CYP"), q.value("PYP"),
                           q.value("period_diff"), q.value("period_diff_percent"));

}

