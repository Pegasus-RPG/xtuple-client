/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "poType.h"

#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"

poType::poType(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_save,  SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
}

poType::~poType()
{
  // no need to delete child widgets, Qt does it all for us
}

void poType::languageChange()
{
  retranslateUi(this);
}

enum SetResponse poType::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  XSqlQuery poType;
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

      poType.exec("SELECT NEXTVAL('potype_potype_id_seq') AS potype_id;");
      if (poType.first())
        _potypeid = poType.value("potype_id").toInt();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _typeCode->setEnabled(false);
      _typeDescr->setEnabled(false);
      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void poType::sClose()
{
  XSqlQuery typeClose;
  if (_mode == cNew)
  {
    typeClose.prepare( "SELECT deletepotype(:potype_id);" );
    typeClose.bindValue(":potype_id", _potypeid);
    typeClose.exec();

    ErrorReporter::error(QtCriticalMsg, this, tr("Error Deleting Purchase Order Type"),
                              typeClose, __FILE__, __LINE__);
  }

  close();
}

void poType::sSave()
{
  XSqlQuery typeSave;
  XSqlQuery defaultSave;
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_typeCode->text().trimmed().isEmpty(), _typeCode,
                          tr("You must enter a Code for this PO Type before you may save it."));
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Purchase Order Type"), errors))
    return;

  if (_mode == cNew)
    typeSave.prepare( "INSERT INTO potype "
               "(potype_id, potype_code, potype_descr, potype_active, potype_default ) "
               "VALUES "
               "(:potype_id, :potype_typeCode, :potype_typeDescr, :potype_active, :potype_default);" );
  else
    typeSave.prepare( "UPDATE potype "
               "SET potype_code=:potype_typeCode, potype_descr=:potype_typeDescr, potype_active=:potype_active, "
               "    potype_default=:potype_default  "
               "WHERE (potype_id=:potype_id);" );

  typeSave.bindValue(":potype_id", _potypeid);
  typeSave.bindValue(":potype_typeCode", _typeCode->text());
  typeSave.bindValue(":potype_typeDescr", _typeDescr->text());
  typeSave.bindValue(":potype_active", QVariant(_active->isChecked()));
  typeSave.bindValue(":potype_default", QVariant(_default->isChecked()));
  typeSave.exec();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Purchase Order Type"),
                                typeSave, __FILE__, __LINE__))
  {
    return;
  }

  if (_default->isChecked())
  {
    defaultSave.prepare("UPDATE potype SET potype_default = false "
                        " WHERE potype_id <> :potype_id; " );
    defaultSave.bindValue(":potype_id", _potypeid);
    defaultSave.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Updating Purchase Order defaults"),
                                typeSave, __FILE__, __LINE__))
    {
      return;
    }
  }

  close();
}

void poType::populate()
{
  XSqlQuery typepopulate;
  typepopulate.prepare( "SELECT * FROM potype "
             "WHERE (potype_id=:potype_id);" );
  typepopulate.bindValue(":potype_id", _potypeid);
  typepopulate.exec();
  if (typepopulate.first())
  {
    _typeCode->setText(typepopulate.value("potype_code").toString());
    _typeDescr->setText(typepopulate.value("potype_descr").toString());
    _active->setChecked(typepopulate.value("potype_active").toBool());
    _default->setChecked(typepopulate.value("potype_default").toBool());
  }
}
