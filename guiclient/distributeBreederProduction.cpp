/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "distributeBreederProduction.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "distributeInventory.h"
#include "changeQtyToDistributeFromBreeder.h"

distributeBreederProduction::distributeBreederProduction(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_changeQty, SIGNAL(clicked()), this, SLOT(sChangeQty()));
  connect(_distribute, SIGNAL(clicked()), this, SLOT(sDistribute()));

  _distrib->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number");
  _distrib->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "descrip");
  _distrib->addColumn(tr("Qty. Per."),   _qtyColumn,  Qt::AlignRight,true, "qtyper");
  _distrib->addColumn(tr("Qty."),        _qtyColumn,  Qt::AlignRight,true, "brddist_qty");
}

distributeBreederProduction::~distributeBreederProduction()
{
  // no need to delete child widgets, Qt does it all for us
}

void distributeBreederProduction::languageChange()
{
  retranslateUi(this);
}

enum SetResponse distributeBreederProduction::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT wo_itemsite_id "
               "FROM wo "
               "WHERE (wo_id=:wo_id);" );
    q.bindValue(":wo_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _item->setItemsiteid(q.value("wo_itemsite_id").toInt());
      _woid = param.toInt();
      _item->setReadOnly(TRUE);
  
      sFillList();
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
      _distrib->setFocus();
  }

  return NoError;
}

void distributeBreederProduction::sDistribute()
{
//  Issue the Breeder Item and Receive the Co-Products/By-Products
  XSqlQuery rollback;
  rollback.prepare("ROLLBACK;");

  q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
  q.prepare("SELECT distributeBreederProduction(:wo_id) AS result;");
  q.bindValue(":wo_id", _woid);
  q.exec();
  if (q.first())
  {
      if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
      {
        rollback.exec();
        QMessageBox::information( this, tr("Distribute Breeder Production"), tr("Transaction Canceled") );
        return;
      }

      q.exec("COMMIT;");
  }
  else
  {
    rollback.exec();
    systemError( this, tr("A System Error occurred at distributeBreederProduction::%1.")
                       .arg(__LINE__) );
  }

  accept();
}

void distributeBreederProduction::sChangeQty()
{
  ParameterList params;
  params.append("brddist_id", _distrib->id());

  changeQtyToDistributeFromBreeder newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void distributeBreederProduction::sFillList()
{
  q.prepare("SELECT brddist_id, item_number,"
            "       (item_descrip1 || ' ' || item_descrip2) AS descrip,"
            "       (brddist_qty / brddist_wo_qty) AS qtyper, brddist_qty,"
            "       'qtyper' AS qtyper_xtnumericrole,"
            "       'qty' AS brddist_qty_xtnumericrole "
            "FROM brddist, itemsite, item "
            "WHERE ( (NOT brddist_posted)"
            " AND (brddist_itemsite_id=itemsite_id)"
            " AND (itemsite_item_id=item_id)"
            " AND (brddist_wo_id=:wo_id) );" );
  q.bindValue(":wo_id", _woid);
  q.exec();
  _distrib->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
