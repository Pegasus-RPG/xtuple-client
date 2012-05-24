/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "customerType.h"
#include "characteristicAssignment.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

customerType::customerType(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _custtypeid = -1;

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(close()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
  
  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft,  true, "char_name");
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft,  true, "charass_value");
  _charass->addColumn(tr("Default"),        _ynColumn,   Qt::AlignCenter,true, "charass_default");
}

customerType::~customerType()
{
  // no need to delete child widgets, Qt does it all for us
}

void customerType::languageChange()
{
    retranslateUi(this);
}

enum SetResponse customerType::set(const ParameterList &pParams)
{
  XSqlQuery customeret;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("custtype_id", &valid);
  if (valid)
  {
    _custtypeid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();
      
      customeret.exec("SELECT NEXTVAL('custtype_custtype_id_seq') AS custtype_id;");
      if (customeret.first())
        _custtypeid = customeret.value("custtype_id").toInt();
      else
      {
        systemError(this, tr("A System Error occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__) );
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _code->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _code->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void customerType::sCheck()
{
  XSqlQuery customerCheck;
  _code->setText(_code->text().trimmed());
  if ((_mode == cNew) && (_code->text().length()))
  {
    customerCheck.prepare( "SELECT custtype_id"
               "  FROM custtype "
               " WHERE((UPPER(custtype_code)=UPPER(:custtype_code))"
               "   AND (custtype_id != :custtype_id));" );
    customerCheck.bindValue(":custtype_code", _code->text());
    customerCheck.bindValue(":custtype_id", _custtypeid);
    customerCheck.exec();
    if (customerCheck.first())
    {
      _custtypeid = customerCheck.value("custtype_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void customerType::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("custtype_id", _custtypeid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerType::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void customerType::sDelete()
{
  XSqlQuery customerDelete;
  customerDelete.prepare( "DELETE FROM charass "
             "WHERE (charass_id=:charass_id);" );
  customerDelete.bindValue(":charass_id", _charass->id());
  customerDelete.exec();
  if (customerDelete.lastError().type() != QSqlError::NoError)
  {
    systemError(this, customerDelete.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void customerType::sFillList()
{
  XSqlQuery customerFillList;
  customerFillList.prepare( "SELECT charass_id, char_name, charass_value, charass_default "
             "FROM charass, char "
             "WHERE ( (charass_target_type='CT')"
             " AND (charass_char_id=char_id)"
             " AND (charass_target_id=:custtype_id) ) "
             "ORDER BY char_order, char_name;" );
  customerFillList.bindValue(":custtype_id", _custtypeid);
  customerFillList.exec();
  _charass->populate(customerFillList);
  if (customerFillList.lastError().type() != QSqlError::NoError)
  {
    systemError(this, customerFillList.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void customerType::sSave()
{
  XSqlQuery customerSave;
  if (_code->text().trimmed().length() == 0)
  {
    QMessageBox::information( this, tr("Invalid Customer Type Code"),
                              tr("You must enter a valid Code for this Customer Type before creating it.")  );
    _code->setFocus();
    return;
  }

  customerSave.prepare("SELECT custtype_id"
            "  FROM custtype"
            " WHERE((custtype_id != :custtype_id)"
            "   AND (custtype_code=:custtype_name))");
  customerSave.bindValue(":custtype_id", _custtypeid);
  customerSave.bindValue(":custtype_name", _code->text().trimmed());
  customerSave.exec();
  if(customerSave.first())
  {
    QMessageBox::critical(this, tr("Cannot Save Customer Type"),
                          tr("You have entered a duplicate Code for this Customer Type. "
                             "Please select a different Code before saving."));
    _code->setFocus();
    return;
  }


  if (_mode == cNew)
  {
    customerSave.prepare( "INSERT INTO custtype "
               "(custtype_id, custtype_code, custtype_descrip, custtype_char) "
               "VALUES "
               "(:custtype_id, :custtype_code, :custtype_descrip, :custtype_char);" );
  }
  else if (_mode == cEdit)
    customerSave.prepare( "UPDATE custtype "
               "SET custtype_code=:custtype_code,"
               "    custtype_descrip=:custtype_descrip, custtype_char=:custtype_char "
               "WHERE (custtype_id=:custtype_id);" );

  customerSave.bindValue(":custtype_id", _custtypeid);
  customerSave.bindValue(":custtype_code", _code->text().trimmed());
  customerSave.bindValue(":custtype_descrip", _description->text().trimmed());
  customerSave.bindValue(":custtype_char",  QVariant(_characteristicGroup->isChecked()));
  customerSave.exec();
  if (customerSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, customerSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_custtypeid);
}

void customerType::populate()
{
  XSqlQuery customerpopulate;
  customerpopulate.prepare( "SELECT custtype_code, custtype_descrip, custtype_char "
              "FROM custtype "
              "WHERE (custtype_id=:custtype_id);" );
  customerpopulate.bindValue(":custtype_id", _custtypeid);
  customerpopulate.exec();
  if (customerpopulate.first())
  {
    _code->setText(customerpopulate.value("custtype_code").toString());
    _description->setText(customerpopulate.value("custtype_descrip").toString());
    _characteristicGroup->setChecked(customerpopulate.value("custtype_char").toBool());
  }
  else if (customerpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, customerpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}
