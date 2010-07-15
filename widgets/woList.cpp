/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "woList.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QVariant>
#include <QWidget>

#include <parameter.h>

#include "xtreewidget.h"
#include "warehousegroup.h"
#include "wocluster.h"

woList::woList(QWidget * parent, const char * name, bool modal, Qt::WFlags fl)
  : QDialog(parent, fl)
{
  setObjectName(name ? name : "woList");
  setModal(modal);
  setWindowTitle(tr("Work Orders"));

  _woid = -1;
  _type = 0;

  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  QHBoxLayout *topLayout = new QHBoxLayout(0);
  QVBoxLayout *warehouseLayout = new QVBoxLayout(0);
  QVBoxLayout *buttonsLayout = new QVBoxLayout(0);
  QVBoxLayout *listLayout = new QVBoxLayout(0);

  mainLayout->setContentsMargins(5, 5, 5, 5);
  topLayout->setContentsMargins(0, 0, 0, 0);
  warehouseLayout->setContentsMargins(0, 0, 0, 0);
  buttonsLayout->setContentsMargins(0, 0, 0, 0);
  listLayout->setContentsMargins(0, 0, 0, 0);

  mainLayout->setSpacing(5);
  topLayout->setSpacing(7);
  warehouseLayout->setSpacing(0);
  buttonsLayout->setSpacing(5);
  listLayout->setSpacing(0);

  _warehouse = new WarehouseGroup(this, "_warehouse");
  warehouseLayout->addWidget(_warehouse);

  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Preferred);
  warehouseLayout->addItem(spacer);

  topLayout->addLayout(warehouseLayout);

  QSpacerItem* spacer_2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  topLayout->addItem(spacer_2);

  _close = new QPushButton(tr("&Cancel"), this);
  _close->setObjectName("_close");
  buttonsLayout->addWidget(_close);

  _select = new QPushButton(tr("&Select"), this);
  _select->setObjectName("_select");
  _select->setAutoDefault(TRUE);
  _select->setDefault(TRUE);
  buttonsLayout->addWidget(_select);
  topLayout->addLayout(buttonsLayout);
  mainLayout->addLayout(topLayout);

  QLabel *_workOrdersLit = new QLabel(tr("Work Orders:"), this);
  _workOrdersLit->setObjectName("_workOrdersLit");
  listLayout->addWidget(_workOrdersLit);

  _wo = new XTreeWidget(this);
  _wo->setObjectName("_wo");
  listLayout->addWidget( _wo );
  mainLayout->addLayout(listLayout);

  resize( QSize(484, 366).expandedTo(minimumSizeHint()) );

  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _wo, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _warehouse, SIGNAL(updated()), this, SLOT( sFillList() ) );

  setTabOrder(_warehouse, _wo);
  setTabOrder(_wo, _select);
  setTabOrder(_select, _close);
  setTabOrder(_close, _warehouse);
  _wo->setFocus();

  _wo->addColumn(tr("W/O #"),     _orderColumn, Qt::AlignLeft,  true, "wonumber");
  _wo->addColumn(tr("Status"),              40, Qt::AlignCenter,true, "wo_status");
  _wo->addColumn(tr("Whs."),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _wo->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft,  true, "item_number");
  _wo->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "descrip");
}

void woList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _woid = param.toInt();

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  param = pParams.value("woType", &valid);
  if (valid)
    _type = param.toInt();

  param = pParams.value("sql", &valid);
  if (valid)
    _sql = param.toString();

  sFillList();
}

void woList::sClose()
{
  done(_woid);
}

void woList::sSelect()
{
  done(_wo->id());
}

void woList::sFillList()
{
  QString sql;

  if (_sql.length())
  {
    sql = _sql;
    if (_warehouse->isSelected())
      sql = QString("SELECT * FROM (%1) AS dummy WHERE (itemsite_warehous_id=:warehous_id)").arg(sql);
  }
  else
  {
    sql = "SELECT wo_id,"
          "       formatWONumber(wo_id) AS wonumber,"
          "       wo_status, warehous_code, item_number,"
          "       (item_descrip1 || ' ' || item_descrip2) AS descrip "
          "FROM wo, itemsite, warehous, item "
          "WHERE ( (wo_itemsite_id=itemsite_id)"
          " AND (itemsite_warehous_id=warehous_id)"
          " AND (itemsite_item_id=item_id)";

    if (_type != 0)
    {
      QStringList statuslist;

      if (_type & cWoOpen)
        statuslist << "'O'";

      if (_type & cWoExploded)
        statuslist << "'E'";

      if (_type & cWoReleased)
        statuslist << "'R'";

      if (_type & cWoIssued)
        statuslist << "'I'";

      if (_type & cWoClosed)
        statuslist << "'C'";

      sql += "AND (wo_status IN (" + statuslist.join(",") + "))";
    }

    if (_warehouse->isSelected())
      sql += " AND (itemsite_warehous_id=:warehous_id)";

    sql += ") ORDER BY wo_number, wo_subnumber, warehous_code, item_number";
  }

  XSqlQuery wo;
  wo.prepare(sql);
  _warehouse->bindValue(wo);
  wo.exec();
  _wo->populate(wo, _woid);
}
