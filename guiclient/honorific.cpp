/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "honorific.h"

#include <QSqlError>
#include <QMessageBox>
#include <QValidator>
#include <QVariant>

/*
 *  Constructs a honorific as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
honorific::honorific(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
    connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(_code, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
honorific::~honorific()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void honorific::languageChange()
{
    retranslateUi(this);
}

enum SetResponse honorific::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("hnfc_id", &valid);
  if (valid)
  {
    _honorificid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _code->setFocus();
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
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void honorific::sCheck()
{
  XSqlQuery honorificCheck;
  _code->setText(_code->text().trimmed());
  if ( (_mode == cNew) && (_code->text().length()) )
  {
    honorificCheck.prepare( "SELECT hnfc_id "
               "FROM hnfc "
               "WHERE (UPPER(hnfc_code)=UPPER(:hnfc_code));" );
    honorificCheck.bindValue(":hnfc_code", _code->text());
    honorificCheck.exec();
    if (honorificCheck.first())
    {
      _honorificid = honorificCheck.value("hnfc_id").toInt();
      _mode = cEdit;
      populate();

      _code->setEnabled(FALSE);
    }
  }
}

void honorific::sSave()
{
  XSqlQuery honorificSave;
  if (_code->text().length() == 0)
  {
      QMessageBox::warning( this, tr("Cannot Save Title"),
                            tr("You must enter a valid Title.") );
      return;
  }
  
  if (_mode == cEdit)
  {
    honorificSave.prepare( "SELECT hnfc_id "
               "FROM hnfc "
               "WHERE ( (hnfc_id<>:hnfc_id)"
               " AND (UPPER(hnfc_code)=UPPER(:hnfc_code)) );");
    honorificSave.bindValue(":hnfc_id", _honorificid);
  }
  else
  {
    honorificSave.prepare( "SELECT hnfc_id "
               "FROM hnfc "
               "WHERE (hnfc_code=:hnfc_code);");
  }
  honorificSave.bindValue(":hnfc_code", _code->text().trimmed());
  honorificSave.exec();
  if (honorificSave.first())
  {
    QMessageBox::critical( this, tr("Cannot Create Title"),
			   tr( "A Title with the entered code already exists."
			       "You may not create a Title with this code." ) );
    _code->setFocus();
    return;
  }
  else if (honorificSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, honorificSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (_mode == cNew)
  {
    honorificSave.exec("SELECT NEXTVAL('hnfc_hnfc_id_seq') AS _hnfc_id");
    if (honorificSave.first())
      _honorificid = honorificSave.value("_hnfc_id").toInt();
    else if (honorificSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, honorificSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    honorificSave.prepare( "INSERT INTO hnfc "
               "( hnfc_id, hnfc_code ) "
               "VALUES "
               "( :hnfc_id, :hnfc_code );" );
  }
  else if (_mode == cEdit)
  {
    honorificSave.prepare( "SELECT hnfc_id "
               "FROM hnfc "
               "WHERE ( (UPPER(hnfc_code)=UPPER(:hnfc_code))"
               " AND (hnfc_id<>:hnfc_id) );" );
    honorificSave.bindValue(":hnfc_id", _honorificid);
    honorificSave.bindValue(":hnfc_code", _code->text());
    honorificSave.exec();
    if (honorificSave.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Title"),
                            tr("You may not rename this Title with the entered value as it is in use by another Title.") );
      return;
    }
    else if (honorificSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, honorificSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    honorificSave.prepare( "UPDATE hnfc "
               "SET hnfc_code=:hnfc_code "
               "WHERE (hnfc_id=:hnfc_id);" );
  }

  honorificSave.bindValue(":hnfc_id", _honorificid);
  honorificSave.bindValue(":hnfc_code", _code->text());
  honorificSave.exec();
  if (honorificSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, honorificSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_honorificid);
}

void honorific::populate()
{
  XSqlQuery honorificpopulate;
  honorificpopulate.prepare( "SELECT hnfc_code "
             "FROM hnfc "
             "WHERE (hnfc_id=:hnfc_id);" );
  honorificpopulate.bindValue(":hnfc_id", _honorificid);
  honorificpopulate.exec();
  if (honorificpopulate.first())
  {
    _code->setText(honorificpopulate.value("hnfc_code").toString());
  }
  else if (honorificpopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, honorificpopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
