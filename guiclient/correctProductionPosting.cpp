/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "correctProductionPosting.h"

#include <QVariant>
#include <QMessageBox>

#include "inputManager.h"
#include "distributeInventory.h"
#include "closeWo.h"

correctProductionPosting::correctProductionPosting(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_correct, SIGNAL(clicked()), this, SLOT(sCorrect()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(populate()));

  _captive = FALSE;
  _qtyReceivedCache = 0.0;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoIssued);
  _qty->setValidator(omfgThis->transQtyVal());
  _qtyOrdered->setPrecision(omfgThis->transQtyVal());
  _qtyReceived->setPrecision(omfgThis->transQtyVal());
  _qtyBalance->setPrecision(omfgThis->transQtyVal());

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _backFlush->setChecked(true);
    _backflushOperations->setChecked(true);
  }
  _nonPickItems->setEnabled(_backFlush->isChecked() &&
			    _privileges->check("ChangeNonPickItems"));

  // TODO: unhide as part of implementation of 5847
  _nonPickItems->hide();

  if (!_metrics->boolean("Routings"))
  {
    _backflushOperations->setChecked(FALSE);
    _backflushOperations->hide();
  }
  
  adjustSize();
}

correctProductionPosting::~correctProductionPosting()
{
  // no need to delete child widgets, Qt does it all for us
}

void correctProductionPosting::languageChange()
{
  retranslateUi(this);
}

enum SetResponse correctProductionPosting::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);

    _qty->setFocus();
  }

  return NoError;
}

void correctProductionPosting::sCorrect()
{
  if (_qty->toDouble() > _qtyReceivedCache)
  {
    QMessageBox::warning( this, tr("Cannot Post Correction"),
                          tr( "The Quantity to correct must be less than or equal to the Quantity already Posted." ) );
    _qty->setFocus();
    return;
  }


  q.prepare( "SELECT item_type "
             "FROM wo, itemsite, item "
             "WHERE ( (wo_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (wo_id=:wo_id) );" );
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
  {
    if (q.value("item_type").toString() == "B")
    {
      QMessageBox::critical( this, tr("Cannot Post Correction"),
                             tr( "You may not post a correction to a W/O that manufactures a Breeder Item.\n"
                                 "You must, instead, adjust the Breeder's, Co-Product's and By-Product's QOH." ) );
      return;
    }
    if (q.value("item_type").toString() == "J")
    {
      QMessageBox::critical( this, tr("Cannot Post Correction"),
                             tr( "You may not post a correction to a W/O for a Job Item.\n"
                                 "You must, instead, adjust shipped quantities." ) );
      return;
    }
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT correctProduction(:wo_id, :qty, :backflushMaterials, :backflushOperations) AS result;");
  q.bindValue(":wo_id", _wo->id());
  if (_wo->method() == "A")
    q.bindValue(":qty", _qty->toDouble());
  else
    q.bindValue(":qty", _qty->toDouble() * -1);
  q.bindValue(":backflushMaterials",  QVariant(_backFlush->isChecked()));
  q.bindValue(":backflushOperations", QVariant(_backflushOperations->isChecked()));
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();

    if (result > 0)
    {
      if (distributeInventory::SeriesAdjust(result, this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Correct Production Posting"), tr("Transaction Canceled") );
        return;
      }

      q.exec("COMMIT;");
      omfgThis->sWorkOrdersUpdated(_wo->id(), TRUE);
    }
    else
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred at correctProductionPosting::%1, Work Order ID #2, Error #%3.")
                         .arg(__LINE__)
                         .arg(_wo->id())
                         .arg(result) );
      return;
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at correctProductionPosting::%1, Work Order ID #%2.")
                       .arg(__LINE__)
                       .arg(_wo->id()) );
    return;
  }

  if (_captive)
    accept();
  else
  {
    _wo->setId(-1);
    _qty->clear();
    _close->setText(tr("&Close"));
    _wo->setFocus();
  }
}

void correctProductionPosting::populate()
{
  if (_wo->id() != -1)
  {
    if (_wo->method() == "D")
    {
      _backFlush->setEnabled(false);
      _backFlush->setChecked(false);
      _backflushOperations->setEnabled(false);
      _backflushOperations->setChecked(false);
      _qtyOrderedLit->setText(tr("Qty. to Disassemble:"));
      _qtyReceivedLit->setText(tr("Qty. Disassembled:"));
    }
    else
    {
      _backFlush->setEnabled(true);
      _backflushOperations->setEnabled(true);
    }
    
    _qtyOrdered->setText(_wo->qtyOrdered());
    _qtyReceived->setText(_wo->qtyReceived());
    _qtyBalance->setText(_wo->qtyBalance());

    _qtyReceivedCache = _wo->qtyReceived();
  }
  else
  {
    _qtyOrdered->clear();
    _qtyReceived->clear();
    _qtyBalance->clear();

    _qtyReceivedCache = 0;
  }

}
