/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "classCode.h"

#include <QVariant>
#include <QMessageBox>

#include <xtClassCode.h>
#include <xtStorableQuery.h>

classCode::classCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  _data = new xtClassCode();

  // signals and slots connections
  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_classCode, SIGNAL(lostFocus()), this, SLOT(sCheck()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

classCode::~classCode()
{
  if(_data)
    delete _data;
}

void classCode::languageChange()
{
  retranslateUi(this);
}

enum SetResponse classCode::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    try
    {
      _data->load(param.toInt());
      populate();
    }
    catch (std::exception e)
    {
      QMessageBox::critical(this, tr("Error"), tr("Could not load classcode. %1").arg(e.what()));
    }
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _classCode->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _buttonBox->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _classCode->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void classCode::sSave()
{
  if (_classCode->text().length() == 0)
  {
    QMessageBox::information( this, tr("No Class Code Entered"),
                              tr("You must enter a valid Class Code before saving this Item Type.") );
    _classCode->setFocus();
    return;
  }

  try
  {
    _data->setCode(_classCode->text());
    _data->setDescription(_description->text());
    _data->save();

    done(_data->getId());
  }
  catch (std::exception e)
  {
    QMessageBox::critical(this, tr("Error"), tr("Could not save classcode. %1").arg(e.what()));
  }
}

void classCode::sCheck()
{
  _classCode->setText(_classCode->text().trimmed());
  if ( (_mode == cNew) && (_classCode->text().length()) )
  {
    try
    {
      xtClassCode ex;
      ex.setCode(_classCode->text());
      
      xtStorableQuery<xtClassCode> sq(&ex);
      sq.exec();
      std::set<xtClassCode*> codes = sq.result();
      if(!codes.empty())
      {
        std::set<xtClassCode*>::const_iterator ci = codes.begin();
        _data->load((*ci)->getId());
        populate();
      }
    }
    catch (std::exception e)
    {
      QMessageBox::critical(this, tr("Error"), tr("Could not load classcode. %1").arg(e.what()));
    }
  }
}

void classCode::populate()
{
  if(_data)
  {
    _classCode->setText(_data->getCode().toString());
    _description->setText(_data->getDescription().toString());
  }
}

