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


void itemOption::init()
{
  _costModifier->setValidator(omfgThis->negMoneyVal());
  _priceModifier->setValidator(omfgThis->negMoneyVal());

  _itematr->addColumn(tr("Name"),    -1,          AlignLeft   );
  _itematr->addColumn(tr("Default"), _itemColumn, AlignCenter );

  _itemchar->setAllowNull(TRUE);
  _itemchar->populate( "SELECT char_id, char_name "
                       "FROM char "
                       "WHERE (char_options) "
                       "ORDER BY char_name; ");
}

enum SetResponse itemOption::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("itemopn_id", &valid);
  if (valid)
  {
    _itemopnid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('itemopn_itemopn_id_seq') AS itemopn_id;");
      if (q.first())
        _itemopnid = q.value("itemopn_id").toInt();
      else
        systemError( this, tr("A System Error occurred at itemOption::%1.")
                           .arg(__LINE__) );
 
      q.prepare( "INSERT INTO itemopn "
                 "(itemopn_id) "
                 "VALUES "
                 "(:itemopn_id);" );
      q.bindValue(":itemopn_id", _itemopnid);
      q.exec();

      _itematr->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _itemchar->setEnabled(FALSE);
      _default->setEnabled(FALSE);
      _optionTypeGroup->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _new->hide();
      _edit->hide();
      _delete->hide();
      _save->hide();
      _close->setText(tr("&Close"));

      _close->setFocus();
    }
  }

  return NoError;
}

void itemOption::sClose()
{
  if ((_mode == cNew) && (_isAttribute->isChecked()))
  {
    q.prepare( "DELETE FROM itemopn "
               "WHERE (itemopn_id=:itemopn_id);"
               "DELETE FROM itematr "
               "WHERE (itematr_itemopn_id=:itemopn_id);" );
    q.bindValue(":itemopn_id", _itemopnid);
    q.exec();
  }

  reject();
}

void itemOption::sSave()
{
  if (!_itemchar->isValid())
  {
    QMessageBox::warning( this, tr("Enter Option Name"),
                          tr( "You must enter a name for the Option\n"
                              "before saving it." ) );
    _itemchar->setFocus();
    return;
  }

  if (_mode == cNew)
    q.prepare( "UPDATE itemopn "
               "SET itemopn_item_id=:itemopn_item_id,"
               "    itemopn_char_id=:itemopn_char_id, itemopn_type=:itemopn_type,"
               "    itemopn_digits=:itemopn_digits, itemopn_minvalue=:itemopn_minvalue,"
               "    itemopn_maxvalue=:itemopn_maxvalue, itemopn_valuemult=:itemopn_valuemult,"
               "    itemopn_manditory=:itemopn_manditory,"
               "    itemopn_suffixpos=:itemopn_suffixpos, itemopn_suffixval=:itemopn_suffixval,"
               "    itemopn_costmod=:itemopn_costmod, itemopn_pricemod=:itemopn_pricemod,"
               "    itemopn_default=:itemopn_default, itemopn_comments=:itemopn_comments "
               "WHERE (itemopn_id=:itemopn_id);" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE itemopn "
               "SET itemopn_char_id=:itemopn_char_id, itemopn_type=:itemopn_type,"
               "    itemopn_digits=:itemopn_digits, itemopn_minvalue=:itemopn_minvalue,"
               "    itemopn_maxvalue=:itemopn_maxvalue, itemopn_valuemult=:itemopn_valuemult,"
               "    itemopn_manditory=:itemopn_manditory,"
               "    itemopn_suffixpos=:itemopn_suffixpos, itemopn_suffixval=:itemopn_suffixval,"
               "    itemopn_costmod=:itemopn_costmod, itemopn_pricemod=:itemopn_pricemod,"
               "    itemopn_default=:itemopn_default, itemopn_comments=:itemopn_comments "
               "WHERE (itemopn_id=:itemopn_id);" );

  q.bindValue(":itemopn_id", _itemopnid);
  q.bindValue(":itemopn_item_id", _item->id());
  q.bindValue(":itemopn_char_id", _itemchar->id());
  
  if (_isOption->isChecked())
  {
    q.bindValue(":itemopn_type", "O");
    q.bindValue(":itemopn_digits", 0);
    q.bindValue(":itemopn_minvalue", 0);
    q.bindValue(":itemopn_maxvalue", 0);
    q.bindValue(":itemopn_valuemult", 0);
    q.bindValue(":itemopn_suffixval", _suffixValue->text());
  }
  else if (_isAttribute->isChecked())
  {
    q.bindValue(":itemopn_type", "A");
    q.bindValue(":itemopn_digits", 0);
    q.bindValue(":itemopn_minvalue", 0);
    q.bindValue(":itemopn_maxvalue", 0);
    q.bindValue(":itemopn_valuemult", 0);
    q.bindValue(":itemopn_suffixval", "");
  }
  else if (_isQuantity->isChecked())
  {
    q.bindValue(":itemopn_type", "Q");
    q.bindValue(":itemopn_digits", _numOfDigits->value());
    q.bindValue(":itemopn_minvalue", _minimumValue->value());
    q.bindValue(":itemopn_maxvalue", _maximumValue->value());
    q.bindValue(":itemopn_valuemult", _multiple->value());
    q.bindValue(":itemopn_suffixval", "");
  }
  
  q.bindValue(":itemopn_manditory", QVariant(_mandatory->isChecked(), 0));
  q.bindValue(":itemopn_default", QVariant(_default->isChecked(), 0));
  q.bindValue(":itemopn_costmod", _costModifier->toDouble());
  q.bindValue(":itemopn_pricemod", _priceModifier->toDouble());
  q.bindValue(":itemopn_suffixpos", _suffixPosition->value());
  q.bindValue(":itemopn_comments", _comments->text());
  q.exec();

  done(_itemopnid);
}

void itemOption::sNew()
{
  if (!_itemchar->isValid())
  {
    QMessageBox::warning( this, tr("Enter Option Name"),
                          tr( "You must enter a name for the Option\n"
                              "before saving it." ) );
    _itemchar->setFocus();
    return;
  }

  q.prepare( "UPDATE itemopn "
             "SET itemopn_char_id=:char_id "
             "WHERE (itemopn_id=:itemopn_id);" );
  q.bindValue(":char_id", _itemchar->id());
  q.bindValue(":itemopn_id", _itemopnid);
  q.exec();

  ParameterList params;
  params.append("mode", "new");
  params.append("itemopn_id", _itemopnid);

  itemAttribute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void itemOption::sEdit()
{
  if (!_itemchar->isValid())
  {
    QMessageBox::warning( this, tr("Enter Option Name"),
                          tr( "You must enter a name for the Option\n"
                              "before saving it." ) );
    _itemchar->setFocus();
    return;
  }

  q.prepare( "UPDATE itemopn "
             "SET itemopn_char_id=:char_id "
             "WHERE (itemopn_id=:itemopn_id);" );
  q.bindValue(":char_id", _itemchar->id());
  q.bindValue(":itemopn_id", _itemopnid);
  q.exec();

  ParameterList params;
  params.append("mode", "edit");
  params.append("itematr_id", _itematr->id());

  itemAttribute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void itemOption::sDelete()
{
  q.prepare( "DELETE FROM itematr "
             "WHERE (itematr_id=:itematr_id);" );
  q.bindValue(":itematr_id", _itematr->id());
  q.exec();

  sFillList();
}

void itemOption::sFillList()
{
  q.prepare( "SELECT itematr_id, char_name, "
             "       CASE WHEN (itematr_default) THEN :yes"
             "            ELSE ''"
             "       END "
             "FROM itematr, char "
             "WHERE ( (itematr_char_id=char_id)"
             " AND (itematr_itemopn_id=:itemopn_id) ) "
             "ORDER BY itematr_default DESC, char_name;" );
  q.bindValue(":yes", tr("Yes"));
  q.bindValue(":itemopn_id", _itemopnid);
  q.exec();
  _itematr->populate(q);
}

void itemOption::populate()
{
  q.prepare( "SELECT itemopn_item_id, itemopn_type,"
             "       itemopn_digits, itemopn_minvalue, itemopn_maxvalue, itemopn_valuemult,"
             "       itemopn_char_id, itemopn_default, itemopn_manditory,"
             "       itemopn_suffixpos, itemopn_suffixval,"
             "       formatMoney(itemopn_costmod) AS f_costmod,"
             "       formatMoney(itemopn_pricemod) AS f_pricemod,"
             "       itemopn_comments "
             "FROM itemopn "
             "WHERE (itemopn_id=:itemopn_id);" );
  q.bindValue(":itemopn_id", _itemopnid);
  q.exec();
  if (q.first())
  {
    _item->setId(q.value("itemopn_item_id").toInt());
    _itemchar->setId(q.value("itemopn_char_id").toInt());
    _default->setChecked(q.value("itemopn_default").toBool());
    _suffixPosition->setValue(q.value("itemopn_suffixpos").toInt());
    _costModifier->setText(q.value("f_costmod").toString());
    _priceModifier->setText(q.value("f_pricemod").toString());
    _comments->setText(q.value("itemopn_comments").toString());

    if (q.value("itemopn_type").toString() == "O")
    {
      _isOption->setChecked(TRUE);
      _suffixValue->setText(q.value("itemopn_suffixval").toString());
    }
    else if (q.value("itemopn_type").toString() == "A")
    {
      _isAttribute->setChecked(TRUE);
      _mandatory->setChecked(q.value("itemopn_manditory").toBool());

      sFillList();
    }
    else if (q.value("itemopn_type").toString() == "Q")
    {
      _isQuantity->setChecked(TRUE);
      _mandatory->setChecked(q.value("itemopn_manditory").toBool());
      _numOfDigits->setValue(q.value("itemopn_digits").toInt());
      _minimumValue->setValue(q.value("itemopn_minvalue").toInt());
      _maximumValue->setValue(q.value("itemopn_maxvalue").toInt());
      _multiple->setValue(q.value("itemopn_valuemult").toInt());
    }
  }
}

void itemOption::sHandleDigits(int pNumberOfDigits)
{
  int maxValue = (int)pow(10, pNumberOfDigits);
  _minimumValue->setMaxValue(maxValue);
  _maximumValue->setMaxValue(maxValue);
  _multiple->setMaxValue(maxValue);
}
