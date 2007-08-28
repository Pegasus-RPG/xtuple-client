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


void copyAssortment::init()
{
  _captive = FALSE;

  _source->setType(cItemAssortments);
  _target->setType(cItemAssortments);
}

enum SetResponse copyAssortment::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _source->setId(param.toInt());
    _source->setEnabled(FALSE);
    _target->setFocus();
  }

  return NoError;
}

void copyAssortment::sCopy()
{
  q.prepare("SELECT copyAssortment(:sourceid, :targetid) AS result;");
  q.bindValue(":sourceid", _source->id());
  q.bindValue(":targetid", _target->id());
  q.exec();
  if (q.first())
  {
    switch (q.value("result").toInt())
    {
      case -1:
        QMessageBox::information( this, tr("Existing Assortment Definition"),
                                  tr( "The selected target Assortment already is already defined.\n"
                                      "You must first delete target Assortment Definition before attempting copy an existing Definition." ) );
        return;

      case -2:
        QMessageBox::information( this, tr("Non-Existent Assortment Definition"),
                                  tr("The selected target Assortment is not defined.") );
        return;

      default:
        omfgThis->sAssortmentsUpdated(_target->id(), TRUE);

        if (_captive)
          done(q.value("result").toInt());
        else
        {
          _source->setId(-1);
          _target->setId(-1);
          _source->setFocus();
        }
    }
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void copyAssortment::sHandleButtons()
{
  _copy->setEnabled(_source->isValid() && _target->isValid());
}

