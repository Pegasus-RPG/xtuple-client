/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "warehouseZone.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

warehouseZone::warehouseZone(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_name, SIGNAL(editingFinished()), this, SLOT(sCheck()));
}

warehouseZone::~warehouseZone()
{
  // no need to delete child widgets, Qt does it all for us
}

void warehouseZone::languageChange()
{
  retranslateUi(this);
}

enum SetResponse warehouseZone::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehousid = param.toInt();

  param = pParams.value("whsezone_id", &valid);
  if (valid)
  {
    _whsezoneid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _name->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void warehouseZone::sSave()
{
  XSqlQuery warehouseSave;
  if (_name->text().length() == 0)
  {
    QMessageBox::information( this, tr("No Name Entered"),
                              tr("<p>You must enter a valid name before saving "
                                 "this Site Zone.") );
    _name->setFocus();
    return;
  }

  if (_mode == cNew)
  {
    warehouseSave.prepare("SELECT NEXTVAL('whsezone_whsezone_id_seq') AS whsezone_id");
    warehouseSave.exec();
    if (warehouseSave.first())
      _whsezoneid = warehouseSave.value("whsezone_id").toInt();
    else if (warehouseSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, warehouseSave.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    warehouseSave.prepare( "INSERT INTO whsezone "
               "(whsezone_id, whsezone_warehous_id, whsezone_name, whsezone_descrip) "
               "VALUES "
               "(:whsezone_id, :warehous_id, :whsezone_name, :whsezone_descrip);" );
  }
  else if (_mode == cEdit)
    warehouseSave.prepare( "UPDATE whsezone "
               "SET whsezone_warehous_id=:warehous_id,"
               "    whsezone_name=:whsezone_name, whsezone_descrip=:whsezone_descrip "
               "WHERE (whsezone_id=:whsezone_id);" );

  warehouseSave.bindValue(":whsezone_id", _whsezoneid);
  warehouseSave.bindValue(":warehous_id", _warehousid);
  warehouseSave.bindValue(":whsezone_name", _name->text());
  warehouseSave.bindValue(":whsezone_descrip", _description->text());
  warehouseSave.exec();
  if (warehouseSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, warehouseSave.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  done(_whsezoneid);
}

void warehouseZone::sCheck()
{
  XSqlQuery warehouseCheck;
  _name->setText(_name->text().trimmed());
  if ( (_mode == cNew) && (_name->text().length()) )
  {
    warehouseCheck.prepare( "SELECT whsezone_id "
               "FROM whsezone "
               "WHERE ( (whsezone_warehous_id=:warehous_id)"
               " AND (UPPER(whsezone_name)=UPPER(:whsezone_name)) );" );
    warehouseCheck.bindValue(":warehous_id", _warehousid);
    warehouseCheck.bindValue(":whsezone_name", _name->text());
    warehouseCheck.exec();
    if (warehouseCheck.first())
    {
      _whsezoneid = warehouseCheck.value("whsezone_id").toInt();
      _mode = cEdit;
      populate();

      _name->setEnabled(FALSE);
    }
    else if (warehouseCheck.lastError().type() != QSqlError::NoError)
    {
      systemError(this, warehouseCheck.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void warehouseZone::populate()
{
  XSqlQuery warehousepopulate;
  warehousepopulate.prepare( "SELECT whsezone_warehous_id, whsezone_name, whsezone_descrip "
             "FROM whsezone "
             "WHERE (whsezone_id=:whsezone_id);" );
  warehousepopulate.bindValue(":whsezone_id", _whsezoneid);
  warehousepopulate.exec();
  if (warehousepopulate.first())
  {
    _warehousid = warehousepopulate.value("whsezone_warehous_id").toInt();
    _name->setText(warehousepopulate.value("whsezone_name").toString());
    _description->setText(warehousepopulate.value("whsezone_descrip").toString());
  }
  else if (warehousepopulate.lastError().type() != QSqlError::NoError)
  {
    systemError(this, warehousepopulate.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
