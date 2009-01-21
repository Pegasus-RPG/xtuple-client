/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "scrapWoMaterialFromWIP.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include "inputManager.h"
#include "distributeInventory.h"
#include "returnWoMaterialItem.h"

scrapWoMaterialFromWIP::scrapWoMaterialFromWIP(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_qty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleButtons()));
  connect(_scrap,              SIGNAL(clicked()), this, SLOT(sScrap()));
  connect(_scrapComponent, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_scrapTopLevel,  SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
  connect(_topLevelQty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleButtons()));
  connect(_topLevelQty, SIGNAL(textChanged(const QString&)), this, SLOT(sHandleButtons()));
  connect(_wo,     SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_womatl, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

  _captive = FALSE;
  _fromWOTC = FALSE;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoIssued);
  //_womatl->setType(WomatlCluster::Push);
  _qty->setValidator(omfgThis->qtyVal());
  _topLevelQty->setValidator(omfgThis->qtyVal());
  _qtyScrappedFromWIP->setPrecision(omfgThis->qtyVal());
}

scrapWoMaterialFromWIP::~scrapWoMaterialFromWIP()
{
  // no need to delete child widgets, Qt does it all for us
}

void scrapWoMaterialFromWIP::languageChange()
{
  retranslateUi(this);
}

enum SetResponse scrapWoMaterialFromWIP::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _womatl->setId(param.toInt());
    _qty->setFocus();
  }

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setEnabled(false);
  }

  param = pParams.value("allowTopLevel", &valid);
  if (valid)
    _scrapTopLevel->setEnabled(param.toBool());

  param = pParams.value("fromWOTC", &valid);
  if (valid)
    _fromWOTC = TRUE;

  param = pParams.value("wooper_id", &valid);
  if (valid)
    _womatl->setWooperid(param.toInt());

  return NoError;
}

void scrapWoMaterialFromWIP::sScrap()
{
  if (_scrapComponent->isChecked() && _qty->toDouble() <= 0)
  {
    QMessageBox::critical( this, tr("Cannot Scrap from WIP"),
                           tr("You must enter a quantity of the selected W/O Material to Scrap." ) );
    _qty->setFocus();
    return;
  }
  else if (_scrapTopLevel->isChecked() && _topLevelQty->toDouble() <= 0)
  {
    QMessageBox::critical( this, tr("Cannot Scrap from WIP"),
                           tr("You must enter a quantity of the W/O Top Level Item to Scrap." ) );
    _topLevelQty->setFocus();
    return;
  }

  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations

  if (_scrapComponent->isChecked())
  {
    q.prepare("SELECT scrapWoMaterial(:womatl_id, :qty, :issueRepl) AS result;");
    q.bindValue(":womatl_id", _womatl->id());
    q.bindValue(":qty", _qty->toDouble());
  }
  else if (_scrapTopLevel->isChecked())
  {
    q.prepare("SELECT scrapTopLevel(:wo_id, :qty, :issueRepl) AS result;");
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":qty",   _topLevelQty->toDouble());
  }
  //q.bindValue(":issueRepl", QVariant(_fromWOTC, 0));
  q.bindValue(":issueRepl", QVariant(false, 0));

  q.exec();
  if (q.first())
  {
    if (q.value("result").toInt() < 0)
    {
      rollback.exec();
      systemError( this, tr("A System Error occurred scrapping material for "
			    "Work Order ID #%1, Error #%2.")
			   .arg(_wo->id())
			   .arg(q.value("result").toInt()),
		   __FILE__, __LINE__);
       return;
    }
    else
    {
      // scrapWoMaterial() returns womatlid, not itemlocSeries
      if (_scrapTopLevel->isChecked())
      {
        if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Scrap Work Order Material"), tr("Transaction Canceled") );
          return;
        }
      }

      q.exec("COMMIT;");

      if (_captive)
        accept();
      else
      {
        _qty->clear();
        _wo->setId(_wo->id());
        _womatl->setFocus();
      }
    }
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred scrapping material for "
			  "Work Order ID #%1\n\n%2")
			  .arg(_wo->id())
			  .arg(q.lastError().databaseText()),
		 __FILE__, __LINE__ );
    return;
  }
}

void scrapWoMaterialFromWIP::sHandleButtons()
{
  if (_wo->id() != -1 && _wo->method() == "D")
  {
    QMessageBox::critical( this, windowTitle(),
                      tr("Posting of scrap against disassembly work orders is not supported.") );
    _wo->setId(-1);
    _wo->setFocus();
    return;
  }
  
  _scrap->setEnabled(_wo->isValid() && (
                     (_scrapTopLevel->isChecked() && _topLevelQty->toDouble()) ||
		     (_scrapComponent->isChecked() && _qty->toDouble() &&
		      _womatl->isValid()) ));
}

