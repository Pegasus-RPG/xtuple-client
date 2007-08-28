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

#include "postAPCheck.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a postAPCheck as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postAPCheck::postAPCheck(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sHandleBankAccount(int)));
    connect(_apchk, SIGNAL(notNull(bool)), _post, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
postAPCheck::~postAPCheck()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postAPCheck::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QSqlError>

void postAPCheck::init()
{
  _captive = FALSE;

  _apchk->setAllowNull(TRUE);

  _bankaccnt->setType(XComboBox::APBankAccounts);
}

enum SetResponse postAPCheck::set(ParameterList &pParams)
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

void postAPCheck::sPost()
{
  q.prepare( "SELECT apchk_posted FROM apchk WHERE (apchk_id=:apchk_id); ");
  q.bindValue(":apchk_id", _apchk->id());
  q.exec();
  if(q.first() && q.value("apchk_posted").toBool())
  {
    QMessageBox::information( this, tr("Check Already Posted"),
          tr("The selected A/P Check has already been posted.") );
    return;
  }

  q.prepare( "SELECT apchk_bankaccnt_id, postAPCheck(apchk_id) AS result "
             "FROM apchk "
             "WHERE ((apchk_id=:apchk_id)"
             " AND  (NOT apchk_posted) );" );
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
  else if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

}

void postAPCheck::sHandleBankAccount(int pBankaccntid)
{
  q.prepare( "SELECT apchk_id, (TEXT(apchk_number) || '-' || vend_name) "
             "FROM apchk, vend "
             "WHERE ( (apchk_vend_id=vend_id)"
             " AND (NOT apchk_void)"
             " AND (NOT apchk_posted)"
             " AND (apchk_printed)"
             " AND (apchk_bankaccnt_id=:bankaccnt_id) ) "
             "ORDER BY apchk_number;" );
  q.bindValue(":bankaccnt_id", pBankaccntid);
  q.exec();
  _apchk->populate(q);
  _apchk->setNull();
}

void postAPCheck::populate(int pApchkid)
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

