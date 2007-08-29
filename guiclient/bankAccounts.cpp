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

#include "bankAccounts.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <openreports.h>
#include "bankAccount.h"

/*
 *  Constructs a bankAccounts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
bankAccounts::bankAccounts(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_bankaccnt, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_bankaccnt, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_serviceCharge, SIGNAL(clicked()), this, SLOT(sPostServiceCharge()));
    connect(_adjustment, SIGNAL(clicked()), this, SLOT(sPostAdjustment()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
bankAccounts::~bankAccounts()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void bankAccounts::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void bankAccounts::init()
{
  statusBar()->hide();
  
  _bankaccnt->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft   );
  _bankaccnt->addColumn(tr("Description"), -1,          Qt::AlignLeft   );
  _bankaccnt->addColumn(tr("Type"),        _dateColumn, Qt::AlignCenter );
  _bankaccnt->addColumn(tr("A/P"),         _ynColumn,   Qt::AlignCenter );
  _bankaccnt->addColumn(tr("A/R"),         _ynColumn,   Qt::AlignCenter );
  _bankaccnt->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignCenter );

  if (omfgThis->singleCurrency())
      _bankaccnt->hideColumn(5);

  if (_privleges->check("MaintainBankAccounts"))
  {
    connect(_bankaccnt, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_bankaccnt, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_bankaccnt, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_bankaccnt, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

void bankAccounts::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  bankAccount newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void bankAccounts::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bankaccnt_id", _bankaccnt->id());

  bankAccount newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void bankAccounts::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bankaccnt_id", _bankaccnt->id());

  bankAccount newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bankAccounts::sFillList()
{
  q.prepare( "SELECT bankaccnt_id, bankaccnt_name, bankaccnt_descrip,"
             "       CASE WHEN (bankaccnt_type='K') THEN :checking"
             "            WHEN (bankaccnt_type='C') THEN :cash"
             "            ELSE '?'"
             "       END,"
             "       formatBoolYN(bankaccnt_ap), formatBoolYN(bankaccnt_ar), "
	     "       currConcat(bankaccnt_curr_id) "
             "FROM bankaccnt "
             "ORDER BY bankaccnt_name;" );
  q.bindValue(":checking", tr("Checking"));
  q.bindValue(":cash", tr("Cash"));
  q.exec();
  _bankaccnt->populate(q);
}

void bankAccounts::sDelete()
{
  q.prepare( "SELECT cashrcpt_id "
             "FROM cashrcpt "
             "WHERE (cashrcpt_bankaccnt_id=:bankaccnt_id) "
             "LIMIT 1;" );
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Bank Account"),
                           tr("The selected Bank Account cannot be deleted as Cash Receipts have been posted against it.\n") );
    return;
  }

  q.prepare( "SELECT apchk_id "
             "FROM apchk "
             "WHERE (apchk_bankaccnt_id=:bankaccnt_id) "
             "LIMIT 1;" );
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Bank Account"),
                           tr("The selected Bank Account cannot be delete as A/P Checks have been posted against it.\n") );
    return;
  }

  q.prepare( "DELETE FROM bankaccnt "
             "WHERE (bankaccnt_id=:bankaccnt_id);" );
  q.bindValue(":bankaccnt_id", _bankaccnt->id());
  q.exec();
  sFillList();
}

void bankAccounts::sPopulateMenu( QMenu * )
{
}

void bankAccounts::sPrint()
{
  orReport report("BankAccountsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void bankAccounts::sPostServiceCharge()
{
}

void bankAccounts::sPostAdjustment()
{
}
