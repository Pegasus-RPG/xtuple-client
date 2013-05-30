/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "contract.h"

#include <QAction>
#include <QCloseEvent>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "crmacctcluster.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include <metasql.h>
#include <parameter.h>
#include "mqlutil.h"

contract::contract(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close,              SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save,               SIGNAL(clicked()), this, SLOT(sSaveClicked()));
  connect(this,               SIGNAL(rejected()), this, SLOT(sRejected()));

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));

  _captive = false;
  _new = false;
}

contract::~contract()
{
    // no need to delete child widgets, Qt does it all for us
}

void contract::languageChange()
{
    retranslateUi(this);
}

enum SetResponse contract::set(const ParameterList &pParams)
{
  XSqlQuery itemet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("contrct_id", &valid);
  if (valid)
  {
    _contrctid = param.toInt();
    _documents->setId(_contrctid);
    populate();
  }

  param = pParams.value("vend_id", &valid);
  if (valid)
  {
    _vendor->setId(param.toInt());
    _vendor->setEnabled(FALSE);
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _new = true;

      itemet.exec("SELECT NEXTVAL('contrct_contrct_id_seq') AS contrct_id;");
      if (itemet.first())
      {
        _contrctid = itemet.value("contrct_id").toInt();
        _documents->setId(_contrctid);
      }
      else if (itemet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, itemet.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      _captive = true;
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _vendor->setEnabled(FALSE);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _vendor->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _number->setEnabled(FALSE);
      _descrip->setEnabled(FALSE);
      _documents->setReadOnly(true);
      _close->setText(tr("&Close"));
      _save->hide();
    }
    if (param.toString() == "copy")
    {
      _mode = cCopy;
      _new = true;
      _captive = true;
//      int contrctidold = _contrctid;

      itemet.exec("SELECT NEXTVAL('contrct_contrct_id_seq') AS contrct_id;");
      if (itemet.first())
        _contrctid = itemet.value("contrct_id").toInt();
      else if (itemet.lastError().type() != QSqlError::NoError)
      {
        systemError(this, itemet.lastError().databaseText(), __FILE__, __LINE__);
        return UndefinedError;
      }
      
      _dates->setStartDate(omfgThis->dbDate());

      sSave();
    }
  }

  return NoError;
}

void contract::sSaveClicked()
{
  _captive = false;
  if (sSave())
    done(_contrctid);
}

bool contract::sSave()
{
  XSqlQuery itemSave;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(!_vendor->isValid(), _vendor,
                          tr( "You must select a Vendor before you may save this Contract." ) )
         << GuiErrorCheck(_dates->endDate() < _dates->startDate(), _dates,
                          tr("The expiration date cannot be earlier than the effective date.") )
         << GuiErrorCheck(_number->isNull(), _number,
                          tr( "You must enter a Contract Number before you may save this Contract." ) )
         << GuiErrorCheck(_descrip->isNull(), _descrip,
                          tr( "You must enter a Description before you may save this Contract." ) )
     ;

  /* TODO - need this?
  itemSave.prepare( "SELECT count(*) AS numberOfOverlaps "
                    "FROM contrct "
                    "WHERE (contrct_vend_id = :contrct_vend_id)"
                    "  AND (contrct_id != :contrct_id)"
                    "  AND ( (contrct_effective BETWEEN :contrct_effective AND :contrct_expires OR"
                    "         contrct_expires BETWEEN :contrct_effective AND :contrct_expires)"
                    "   OR   (contrct_effective <= :contrct_effective AND"
                    "         contrct_expires   >= :contrct_expires) );" );
  itemSave.bindValue(":contrct_id", _contrctid);
  itemSave.bindValue(":contrct_vend_id", _vendor->id());
  itemSave.bindValue(":contrct_effective", _dates->startDate());
  itemSave.bindValue(":contrct_expires", _dates->endDate());
  itemSave.exec();
  if (itemSave.first())
  {
    if (itemSave.value("numberOfOverlaps").toInt() > 0)
    {
      errors << GuiErrorCheck(true, _dates,
                              tr("The date range overlaps with another date range.\n"
                                 "Please check the values of these dates."));
    }
  }
  else if (itemSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }
  */

  if(_mode == cNew || _mode == cCopy)
  {
    itemSave.prepare( "SELECT contrct_id "
                      "  FROM contrct "
                      " WHERE ((contrct_vend_id=:vend_id) "
                      "   AND (contrct_number=:contrct_number));" );
    itemSave.bindValue(":vend_id", _vendor->id());
    itemSave.bindValue(":contrct_number", _number->text());
    itemSave.exec();
    if(itemSave.first())
    {
      errors << GuiErrorCheck(true, _vendor,
                              tr("A Contract already exists for the Vendor,\n"
                                 "Contract Number you have specified."));
    }
    else if (itemSave.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
      return false;
    }
  }
  
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Contract"), errors))
    return false;

  if (_mode == cNew || _mode == cCopy)
    itemSave.prepare( "INSERT INTO contrct "
               "( contrct_id, contrct_vend_id,"
               "  contrct_number, contrct_descrip,"
               "  contrct_effective, contrct_expires,"
               "  contrct_note ) "
               "VALUES "
               "( :contrct_id, :contrct_vend_id,"
               "  :contrct_number, :contrct_descrip,"
               "  :contrct_effective, :contrct_expires,"
               "  :contrct_note );" );
  if (_mode == cEdit)
    itemSave.prepare( "UPDATE contrct "
               "SET contrct_number=:contrct_number,"
               "    contrct_descrip=:contrct_descrip,"
               "    contrct_effective=:contrct_effective,"
               "    contrct_expires=:contrct_expires,"
               "    contrct_note=:contrct_note "
               "WHERE (contrct_id=:contrct_id);" );

  itemSave.bindValue(":contrct_id", _contrctid);
  itemSave.bindValue(":contrct_vend_id", _vendor->id());
  itemSave.bindValue(":contrct_effective", _dates->startDate());
  itemSave.bindValue(":contrct_expires", _dates->endDate());
  itemSave.bindValue(":contrct_number", _number->text());
  itemSave.bindValue(":contrct_descrip", _descrip->text());
  itemSave.bindValue(":contrct_note", _notes->toPlainText());
  itemSave.exec();
  if (itemSave.lastError().type() != QSqlError::NoError)
  {
    systemError(this, itemSave.lastError().databaseText(), __FILE__, __LINE__);
    return false;
  }

  if (_captive)
  {
    if (_mode != cCopy)
    {
      _vendor->setEnabled(FALSE);
    }
    _mode = cEdit;
    _captive = false;
  }
    
  return true;
}

void contract::populate()
{
  XSqlQuery contrctQ;
  contrctQ.prepare( "SELECT * "
                    "FROM contrct "
                    "WHERE (contrct_id=:contrct_id);" );
  contrctQ.bindValue(":contrct_id", _contrctid);
  contrctQ.exec();
  if (contrctQ.first())
  {
    _vendor->setId(contrctQ.value("contrct_vend_id").toInt());
    _dates->setStartDate(contrctQ.value("contrct_effective").toDate());
    _dates->setEndDate(contrctQ.value("contrct_expires").toDate());
    _number->setText(contrctQ.value("contrct_number").toString());
    _descrip->setText(contrctQ.value("contrct_descrip").toString());
    _notes->setText(contrctQ.value("contrct_note").toString());
  }
  else if (contrctQ.lastError().type() != QSqlError::NoError)
  {
    systemError(this, contrctQ.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void contract::sRejected()
{
  XSqlQuery itemRejected;
  if (_new)
  {
    itemRejected.prepare( "DELETE FROM contrct "
               "WHERE (contrct_id=:contrct_id);" );
    itemRejected.bindValue(":contrct_id", _contrctid);
    itemRejected.exec();
    if (itemRejected.lastError().type() != QSqlError::NoError)
    {
      systemError(this, itemRejected.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}
