/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "changeQtyToDistributeFromBreeder.h"

#include <QSqlError>
#include <QValidator>
#include <QVariant>

changeQtyToDistributeFromBreeder::changeQtyToDistributeFromBreeder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_actualQtyToDistribute, SIGNAL(lostFocus()), this, SLOT(sUpdateQtyPer()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _actualQtyPer->setPrecision(omfgThis->qtyPerVal());
  _openWoQty->setPrecision(omfgThis->qtyVal());
  _standardQtyPer->setPrecision(omfgThis->qtyPerVal());
  _standardQtyToDistribute->setPrecision(omfgThis->qtyVal());
  _actualQtyToDistribute->setValidator(omfgThis->qtyVal());
}

changeQtyToDistributeFromBreeder::~changeQtyToDistributeFromBreeder()
{
  // no need to delete child widgets, Qt does it all for us
}

void changeQtyToDistributeFromBreeder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse changeQtyToDistributeFromBreeder::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("brddist_id", &valid);
  if (valid)
  {
    _brddistid = param.toInt();

    XSqlQuery brddist;
    brddist.prepare( "SELECT brddist_wo_qty, brddist_wo_qty,"
                     "       brddist_stdqtyper,"
                     "       brddist_stdqtyper * brddist_wo_qty AS stdqty,"
                     "       (brddist_qty / brddist_wo_qty) AS actqtyper,"
                     "       brddist_qty "
                     "FROM brddist "
                     "WHERE (brddist_id=:brddist_id);" );
    brddist.bindValue(":brddist_id", param.toInt());
    brddist.exec();
    if (brddist.first())
    {
      _cachedOpenWoQty = brddist.value("brddist_qty").toDouble();
      _openWoQty->setDouble(brddist.value("brddist_wo_qty").toDouble());
      _standardQtyPer->setDouble(brddist.value("brddist_stdqtyper").toDouble());
      _standardQtyToDistribute->setDouble(brddist.value("stdqty").toDouble());
      _actualQtyPer->setDouble(brddist.value("actqtyper").toDouble());
      _actualQtyToDistribute->setDouble(brddist.value("brddist_qty").toDouble());
    }
    else if (brddist.lastError().type() != QSqlError::NoError)
    {
      systemError(this, brddist.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  return NoError;
}

void changeQtyToDistributeFromBreeder::sUpdateQtyPer()
{
  _actualQtyPer->setDouble(_actualQtyToDistribute->toDouble() / _cachedOpenWoQty);
}

void changeQtyToDistributeFromBreeder::sSave()
{
  XSqlQuery changeQty;
  changeQty.prepare( "UPDATE brddist "
                     "SET brddist_qty=:qty "
                     "WHERE (brddist_id=:brddist_id);" );
  changeQty.bindValue(":qty", _actualQtyToDistribute->toDouble());
  changeQty.bindValue(":brddist_id", _brddistid);
  changeQty.exec();
  if (changeQty.lastError().type() != QSqlError::NoError)
  {
    systemError(this, changeQty.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}
