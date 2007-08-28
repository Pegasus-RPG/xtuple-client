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


void purchaseOrderType::init()
{
}

enum SetResponse purchaseOrderType::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("potype_id", &valid);
  if (valid)
  {
    _potypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _name->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void purchaseOrderType::sSave()
{
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("Cannot Save Purchase Order Type"),
                              tr("You must enter a valid name for this Purchase Order Type before you may save it.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('potype_potype_id_seq') AS potype_id;");
    if (q.first())
      _potypeid = q.value("potype_id").toInt();
    else
    {
      systemError(this, tr("A System Error occurred at %1::%2.")
                        .arg(__FILE__)
                        .arg(__LINE__) );
      return;
    }
 
    q.prepare( "INSERT INTO potype "
               "( potype_id, potype_name, potype_descrip ) "
               "VALUES "
               "( :potype_id, :potype_name, :potype_descrip );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE potype "
               "SET potype_name=:potype_name, potype_descrip=:potype_descrip "
               "WHERE (potype_id=:potype_id);" );

  q.bindValue(":potype_id", _potypeid);
  q.bindValue(":potype_name", _name->text());
  q.bindValue(":potype_descrip", _description->text());
  q.exec();

  done(_potypeid);
}

void purchaseOrderType::sCheck()
{
  _name->setText(_name->text().stripWhiteSpace());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    q.prepare( "SELECT potype_id "
               "FROM potype "
               "WHERE (UPPER(potype_name)=UPPER(:potype_name));" );
    q.bindValue(":potype_name", _name->text());
    q.exec();
    if (q.first())
    {
      _potypeid = q.value("potype_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
  }
}

void purchaseOrderType::populate()
{
  q.prepare( "SELECT potype_name, potype_descrip "
             "FROM potype "
             "WHERE (potype_id=:potype_id);" );
  q.bindValue(":potype_id", _potypeid);
  q.exec();
  if (q.first())
  {
    _name->setText(q.value("potype_name").toString());
    _description->setText(q.value("potype_descrip").toString());
  }
}

