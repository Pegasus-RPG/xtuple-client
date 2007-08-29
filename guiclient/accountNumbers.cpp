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

#include "accountNumbers.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
#include <openreports.h>
#include "accountNumber.h"

/*
 *  Constructs a accountNumbers as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
accountNumbers::accountNumbers(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));

  sBuildList();

  connect(omfgThis, SIGNAL(configureGLUpdated()), this, SLOT(sBuildList()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
accountNumbers::~accountNumbers()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void accountNumbers::languageChange()
{
  retranslateUi(this);
}

void accountNumbers::sDelete()
{
  q.prepare("SELECT deleteAccount(:accnt_id) AS result;");
  q.bindValue(":accnt_id", _account->id());
  q.exec();
  if (q.first())
  {
    switch (q.value("result").toInt())
    {
      case -1:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more Cost Categories.\n"
                                   "You must reassign these Cost Category assignments before you may delete the selected G/L Account." ) );
        return;

      case -2:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more Sales Account Assignment.\n"
                                   "You must reassign these Sales Account Assignments before you may delete the selected G/L Account." ) );
        return;

      case -3:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more Customer A/R Account assignments.\n"
                                   "You must reassign these Customer A/R Account assignments before you may delete the selected G/L Account." ) );
        return;

      case -4:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used as the default Account one or more Warehouses.\n"
                                   "You must reassign the default Account for these Warehouses before you may delete the selected G/L Account." ) );
        return;

      case -5:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more Bank Accounts.\n"
                                   "You must reassign these Bank Accounts before you may delete the selected G/L Account." ) );
        return;

      case -6:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more Expense Categories.\n"
                                   "You must reassign these Expense Categories before you may delete the selected G/L Account." ) );
        return;

      case -7:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more Tax Codes.\n"
                                   "You must reassign these Tax Codes before you may delete the selected G/L Account." ) );
        return;

      case -8:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more Standard Journals.\n"
                                   "You must reassign these Standard Journal Items before you may delete the selected G/L Account." ) );
        return;

      case -9:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more Customer A/P Account assignments.\n"
                                   "You must reassign these Customer A/P Account assignments before you may delete the selected G/L Account." ) );
        return;

      case -10:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more Currency definition.\n"
                                   "You must reassign these Currency definitions before you may delete the selected G/L Account." ) );
        return;

      case -11:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr( "The selected G/L Account cannot be deleted as it is currently used in one or more A/R Open Items.\n"
                                   "You must reassign these Currency definitions before you may delete the selected G/L Account." ) );
        return;

      case -99:
        QMessageBox::critical( this, tr("Cannot Delete G/L Account"),
                               tr("The selected G/L Account cannot be deleted as there have been G/L Transactions posted aginst it.") );
        return;

      default:
//  ToDo

      case 0:
        sFillList();
        break;
    }
  }
//  ToDo
}

void accountNumbers::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  accountNumber newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void accountNumbers::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("accnt_id", _account->id());

  accountNumber newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void accountNumbers::sPrint()
{
  orReport report("AccountNumberMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void accountNumbers::sFillList()
{
  QString sql("SELECT accnt_id, ");

  if (_metrics->value("GLCompanySize").toInt() > 0)
    sql += "accnt_company, ";

  if (_metrics->value("GLProfitSize").toInt() > 0)
    sql += "accnt_profit, ";

  sql += "accnt_number, ";

  if (_metrics->value("GLSubaccountSize").toInt() > 0)
    sql += "accnt_sub, ";

  sql += " accnt_descrip,"
         " accnt_type "
         "FROM accnt "
         "ORDER BY accnt_number, accnt_sub, accnt_profit;";

  _account->populate(sql);
}

void accountNumbers::sBuildList()
{
  _account->setColumnCount(0);

  if (_metrics->value("GLCompanySize").toInt() > 0)
    _account->addColumn(tr("Company"), 50, Qt::AlignCenter);

  if (_metrics->value("GLProfitSize").toInt() > 0)
    _account->addColumn(tr("Profit"), 50, Qt::AlignCenter);

  _account->addColumn(tr("Account Number"), 100, Qt::AlignCenter);

  if (_metrics->value("GLSubaccountSize").toInt() > 0)
    _account->addColumn(tr("Sub."), 50, Qt::AlignCenter);

  _account->addColumn(tr("Description"), -1, Qt::AlignLeft);
  _account->addColumn(tr("Type"), _ynColumn, Qt::AlignCenter);

  sFillList();
}
