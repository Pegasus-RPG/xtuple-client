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

#include "employees.h"

#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>
#include <metasql.h>

#include "employee.h"
#include "guiclient.h"
#include "storedProcErrorLookup.h"

employees::employees(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));

  _emp->addColumn(tr("Site"),   _whsColumn,  Qt::AlignLeft, true, "warehous_code");
  _emp->addColumn(tr("Code"),   _itemColumn, Qt::AlignLeft, true, "emp_code");
  _emp->addColumn(tr("Number"), -1,          Qt::AlignLeft, true, "emp_number");
  _emp->addColumn(tr("First"),  _itemColumn, Qt::AlignLeft, true, "cntct_first_name");
  _emp->addColumn(tr("Last"),   _itemColumn, Qt::AlignLeft, true, "cntct_last_name");
  

  if (_privileges->check("MaintainEmployees"))
  {
    connect(_emp, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_emp, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_emp, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_emp, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    _new->setEnabled(FALSE);
  }

   sFillList();
}

employees::~employees()
{
  // no need to delete child widgets, Qt does it all for us
}

void employees::languageChange()
{
  retranslateUi(this);
}

void employees::sDelete()
{
  q.prepare( "SELECT deleteEmp(:id) AS result;");
  q.bindValue(":id", _emp->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deleteEmp", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void employees::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("emp_id", _emp->id());

  employee newdlg(this);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void employees::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("emp_id", _emp->id());

  employee newdlg(this);
  newdlg.set(params);
  newdlg.exec();
}

void employees::sFillList()
{
  
  QString sql("SELECT emp_id, warehous_code, emp_code, emp_number, "
              "       cntct_first_name, cntct_last_name "
              "FROM emp "
              "  LEFT OUTER JOIN cntct ON (emp_cntct_id=cntct_id) "
              "  LEFT OUTER JOIN whsinfo ON (emp_warehous_id=warehous_id) "
	     "<? if exists(\"warehouse_id\") ?>"
             "WHERE (warehous_id=<? value(\"warehouse_id\") ?>)"
             "<? endif ?>"
              "ORDER BY emp_code;" );
              
  MetaSQLQuery mql(sql);
  ParameterList params;
  if (!_warehouse->isAll())
    params.append("warehouse_id", _warehouse->id());
  q = mql.toQuery(params);
  _emp->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void employees::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  employee newdlg(this);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void employees::sPrint()
{
  ParameterList params;
  if (!_warehouse->isAll())
    params.append("warehouse_id", _warehouse->id());

  orReport report("EmployeeList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}
