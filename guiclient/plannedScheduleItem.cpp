/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "plannedScheduleItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>

plannedScheduleItem::plannedScheduleItem(QWidget * parent, const char * name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _pschheadid = -1;
  _pschitemid = -1;
  _warehousid = -1;

  _item->setType(ItemLineEdit::cActive | ItemLineEdit::cPlanningMPS);

  connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _qty->setValidator(omfgThis->qtyVal());
}

plannedScheduleItem::~plannedScheduleItem()
{
}

void plannedScheduleItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse plannedScheduleItem::set(const ParameterList & pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("pschhead_id", &valid);
  if(valid)
  {
    _pschheadid = param.toInt();

    // TODO: find and set the appropriate warehous_id value.
    q.prepare("SELECT pschhead_warehous_id "
              "  FROM pschhead "
              " WHERE (pschhead_id=:pschhead_id);");
    q.bindValue(":pschhead_id", _pschheadid);
    q.exec();
    if(q.first())
    {
      _warehousid = q.value("pschhead_warehous_id").toInt();
      _item->clearExtraClauseList();
      _item->addExtraClause(QString("(itemsite_warehous_id=%1)")
          .arg(_warehousid));
    }
  }

  param = pParams.value("pschitem_id", &valid);
  if(valid)
  {
    _pschitemid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    QString mode = param.toString();
    if("new" == mode)
      _mode = cNew;
    else if("copy" == mode)
    {
      _mode = cNew;
      _pschitemid=-1;
    }
    else if("edit" == mode)
      _mode = cEdit;
    else if("view" == mode)
    {
      _mode = cView;

      _item->setEnabled(false);
      _date->setEnabled(false);
      _qty->setEnabled(false);

      _save->hide();
    }
  }

  return NoError;
}

void plannedScheduleItem::sSave()
{
  if (!_item->isValid())
  {
    QMessageBox::information( this, tr("Invalid Item"),
                              tr("You must enter a valid Item for this Schedule.") );
    _item->setFocus();
    return;
  }

  if (!_date->isValid())
  {
    QMessageBox::information( this, tr("Invalid Date"),
                              tr("You must enter a valid Date for this Schedule.") );
    _item->setFocus();
    return;
  }

  if (_qty->toDouble() == 0.0)
  {
    QMessageBox::information( this, tr("Invalid Quantity"),
                              tr("You must enter a valid Quantity for this Schedule.") );
    _item->setFocus();
    return;
  }

  if(cNew == _mode)
    q.prepare("INSERT INTO pschitem"
              "      (pschitem_pschhead_id,"
              "       pschitem_linenumber,"
              "       pschitem_itemsite_id,"
              "       pschitem_scheddate, pschitem_qty) "
              "VALUES(:pschhead_id,"
              "       COALESCE((SELECT MAX(pschitem_linenumber) FROM pschitem WHERE pschitem_pschhead_id=:pschhead_id),0)+10,"
              "       (SELECT itemsite_id FROM itemsite WHERE itemsite_item_id=:item_id AND itemsite_warehous_id=:warehous_id),"
              "       :scheddate, :qty);");
  else
    q.prepare("UPDATE pschitem"
              "   SET pschitem_itemsite_id=(SELECT itemsite_id FROM itemsite WHERE itemsite_item_id=:item_id AND itemsite_warehous_id=:warehous_id),"
              "       pschitem_scheddate=:scheddate,"
              "       pschitem_qty=:qty "
              " WHERE (pschitem_id=:pschitem_id); ");

  q.bindValue(":pschhead_id", _pschheadid);
  q.bindValue(":pschitem_id", _pschitemid);
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":scheddate", _date->date());
  q.bindValue(":qty", _qty->toDouble());

  if(!q.exec())
  {
    QMessageBox::warning( this, tr("Error Saving"),
      tr("An Error was encountered while trying to save the record.") );
    return;
  }

  accept();
}


void plannedScheduleItem::populate()
{
  q.prepare("SELECT pschitem_pschhead_id,"
            "       pschitem_itemsite_id,"
            "       pschitem_scheddate,"
            "       pschitem_qty,"
            "       pschhead_warehous_id "
            "  FROM pschhead JOIN pschitem ON (pschitem_pschhead_id=pschhead_id) "
            " WHERE (pschitem_id=:pschitem_id);");
  q.bindValue(":pschitem_id", _pschitemid);
  q.exec();
  if(q.first())
  {
    _pschheadid = q.value("pschitem_pschhead_id").toInt();
    _warehousid = q.value("pschhead_warehous_id").toInt();
    _item->clearExtraClauseList();
    _item->addExtraClause(QString("(itemsite_warehous_id=%1)")
        .arg(_warehousid));

    _item->setItemsiteid(q.value("pschitem_itemsite_id").toInt());
    _date->setDate(q.value("pschitem_scheddate").toDate());
    _qty->setDouble(q.value("pschitem_qty").toDouble());
  }
}

