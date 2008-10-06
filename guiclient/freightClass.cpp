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

#include "freightClass.h"

#include <qvariant.h>
#include <qmessagebox.h>

/*
 *  Constructs a freightClass as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
freightClass::freightClass(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_freightClass, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
freightClass::~freightClass()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void freightClass::languageChange()
{
    retranslateUi(this);
}


void freightClass::init()
{
}

enum SetResponse freightClass::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("freightclass_id", &valid);
  if (valid)
  {
    _freightclassid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _freightClass->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _freightClass->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void freightClass::sSave()
{
  if (_freightClass->text().length() == 0)
  {
    QMessageBox::information( this, tr("No Freight Class Entered"),
                              tr("You must enter a valid Freight Class before saving this Item Type.") );
    _freightClass->setFocus();
    return;
  }

  if (_mode == cEdit)
    q.prepare( "UPDATE freightclass "
               "SET freightclass_code=:freightclass_code, freightclass_descrip=:freightclass_descrip "
               "WHERE (freightclass_id=:freightclass_id);" );
  else if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('freightclass_freightclass_id_seq') AS freightclass_id");
    if (q.first())
      _freightclassid = q.value("freightclass_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    q.prepare( "INSERT INTO freightclass "
               "( freightclass_id, freightclass_code, freightclass_descrip ) "
               "VALUES "
               "( :freightclass_id, :freightclass_code, :freightclass_descrip );" );
  }

  q.bindValue(":freightclass_id", _freightclassid);
  q.bindValue(":freightclass_code", _freightClass->text());
  q.bindValue(":freightclass_descrip", _description->text());
  q.exec();

  done(_freightclassid);
}

void freightClass::sCheck()
{
  _freightClass->setText(_freightClass->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_freightClass->text().length()) )
  {
    q.prepare( "SELECT freightclass_id "
               "FROM freightclass "
               "WHERE (UPPER(freightclass_code)=UPPER(:freightclass_code));" );
    q.bindValue(":freightclass_code", _freightClass->text());
    q.exec();
    if (q.first())
    {
      _freightclassid = q.value("freightclass_id").toInt();
      _mode = cEdit;
      populate();

      _freightClass->setEnabled(FALSE);
    }
  }
}

void freightClass::populate()
{
  q.prepare( "SELECT * "
             "FROM freightclass "
             "WHERE (freightclass_id=:freightclass_id);" );
  q.bindValue(":freightclass_id", _freightclassid);
  q.exec();
  if (q.first())
  {
    _freightClass->setText(q.value("freightclass_code"));
    _description->setText(q.value("freightclass_descrip"));
  }
}

