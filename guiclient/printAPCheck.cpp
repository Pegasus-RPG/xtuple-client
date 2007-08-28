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

#include "printAPCheck.h"

#include <qvariant.h>
#include <openreports.h>
#include <qmessagebox.h>

/*
 *  Constructs a printAPCheck as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printAPCheck::printAPCheck(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));
    connect(_apchk, SIGNAL(notNull(bool)), _print, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
printAPCheck::~printAPCheck()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printAPCheck::languageChange()
{
    retranslateUi(this);
}


void printAPCheck::init()
{
  _captive = FALSE;

  _apchk->setAllowNull(TRUE);

  _bankaccnt->setType(XComboBox::APBankAccounts);
}

enum SetResponse printAPCheck::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("apchk_id", &valid);
  if (valid)
  {
    populate(param.toInt());
    _bankaccnt->setEnabled(FALSE);
    _apchk->setEnabled(FALSE);
  }

  return NoError;
}

void printAPCheck::sPrint()
{
  q.prepare( "SELECT apchk_printed, report_name, bankaccnt_id "
             "FROM apchk, bankaccnt, form, report "
             "WHERE ( (apchk_bankaccnt_id=bankaccnt_id)"
             " AND (bankaccnt_check_form_id=form_id)"
             " AND (form_report_id=report_id)"
             " AND (apchk_id=:apchk_id) );" );
  q.bindValue(":apchk_id", _apchk->id());
  q.exec();
  if (q.first())
  {
    if(q.value("apchk_printed").toBool())
    {
      QMessageBox::information( this, tr("Check Already Printed"),
          tr("The selected A/P Check has already been printed.") );
      return;
    }

    ParameterList params;

    params.append("apchk_id", _apchk->id());

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();

    omfgThis->sAPChecksUpdated(q.value("bankaccnt_id").toInt(), _apchk->id(), TRUE);
  }
  else
  {
    QMessageBox::critical( this, tr("Cannot Print Check"),
                           tr( "The selected A/P Check cannot be printed as the Bank Account that it is to draw upon\n"
                               "does not have a valid Check Format assigned to it.  Please assign a valid Check Format\n"
                               "to this Bank Account before attempting to print this A/P Check." ) );
    return;
  }

  if ( QMessageBox::information( this, tr("Check Printed"),
                                 tr("Was the selected Check printed sucessfully?"),
                                 tr("&Yes"), tr("&No"), QString::null, 1, 0 ) == 0 )
  {
    q.prepare( "SELECT apchk_bankaccnt_id, markAPCheckAsPrinted(apchk_id) AS result "
               "FROM apchk "
               "WHERE (apchk_id=:apchk_id);" );
    q.bindValue(":apchk_id", _apchk->id());
    q.exec();
    if (q.first())
    {
      omfgThis->sAPChecksUpdated(q.value("apchk_bankaccnt_id").toInt(), _apchk->id(), TRUE);

      if (_captive)
        accept();
      else
      {
        sHandleBankAccount(_bankaccnt->id());
        _close->setText(tr("&Close"));
      }
    }
  }
  else if ( QMessageBox::information( this, tr("Mark Check as Voided"),
                                      tr("Would you like to mark the selected Check as Void and create a replacement check?"),
                                      tr("&Yes"), tr("&No"), QString::null, 1, 0 ) == 0)
  {
    q.prepare("SELECT voidAPCheck(:apchk_id) AS result;");
    q.bindValue(":apchk_id", _apchk->id());
    q.exec();

    q.prepare( "SELECT apchk_bankaccnt_id, replaceVoidedAPCheck(apchk_id) AS result "
               "FROM apchk "
               "WHERE (apchk_id=:apchk_id);" );
    q.bindValue(":apchk_id", _apchk->id());
    q.exec();
    if (q.first())
    {
      omfgThis->sAPChecksUpdated(q.value("apchk_bankaccnt_id").toInt(), _apchk->id(), TRUE);

      sHandleBankAccount(_bankaccnt->id());
      _print->setFocus();
    }
    else
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
  }
}

void printAPCheck::sHandleBankAccount(int pBankaccntid)
{
  q.prepare( "SELECT apchk_id, (TEXT(apchk_number) || '-' || vend_name) "
             "FROM apchk, vend "
             "WHERE ( (apchk_vend_id=vend_id)"
             " AND (NOT apchk_void)"
             " AND (NOT apchk_printed)"
             " AND (NOT apchk_posted)"
             " AND (apchk_bankaccnt_id=:bankaccnt_id) ) "
             "ORDER BY apchk_number;" );
  q.bindValue(":bankaccnt_id", pBankaccntid);
  q.exec();
  _apchk->populate(q);
  _apchk->setNull();
}

void printAPCheck::populate(int pApchkid)
{
  q.prepare( "SELECT apchk_bankaccnt_id "
             "FROM apchk "
             "WHERE (apchk_id=:apchk_id);" );
  q.bindValue(":apchk_id", pApchkid);
  q.exec();
  if (q.first())
  {
    _bankaccnt->setId(q.value("apchk_bankaccnt_id").toInt());
    _apchk->setId(pApchkid);
  }
//  ToDo
}

