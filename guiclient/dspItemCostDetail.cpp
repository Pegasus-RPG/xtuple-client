/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemCostDetail.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "bomItem.h"
#include "mqlutil.h"

dspItemCostDetail::dspItemCostDetail(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  QButtonGroup* _costTypeGroupInt = new QButtonGroup(this);
  _costTypeGroupInt->addButton(_standardCosts);
  _costTypeGroupInt->addButton(_actualCosts);

  connect(_costTypeGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulate()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_costType, SIGNAL(newID(int)), this, SLOT(sFillList()));

  _bom->addColumn(tr("#"),               _seqColumn,   Qt::AlignCenter,true, "seqnumber");
  _bom->addColumn(tr("Item Number"),     _itemColumn,  Qt::AlignLeft,  true, "item_number");
  _bom->addColumn(tr("Description"),     -1,           Qt::AlignLeft,  true, "itemdescrip");
  _bom->addColumn(tr("UOM"),             _uomColumn,   Qt::AlignCenter,true, "uom_name");
  _bom->addColumn(tr("Qty. Per"),        _qtyColumn,   Qt::AlignRight, true, "qtyper");
  _bom->addColumn(tr("Scrap/Absorb. %"), _itemColumn,  Qt::AlignRight, true, "scrap" );
  _bom->addColumn(tr("Unit Cost"),       _costColumn,  Qt::AlignRight, true, "cost");
  _bom->addColumn(tr("Ext'd Cost"),      _moneyColumn, Qt::AlignRight, true, "extendedcost");

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
}

dspItemCostDetail::~dspItemCostDetail()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemCostDetail::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspItemCostDetail::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  param = pParams.value("costtype", &valid);
  if (valid)
  {
    for (int cursor = 0; cursor < _costType->count(); cursor++)
    {
      if (_costType->text(cursor) == param.toString())
      {
        _costType->setCurrentIndex(cursor);
        break;
      }
    }
  }

  param = pParams.value("itemcost_id", &valid);
  if (valid)
  {
    q.prepare( "SELECT itemcost_item_id, itemcost_costelem_id "
               "FROM itemcost "
               "WHERE (itemcost_id=:itemcost_id)" );
    q.bindValue(":itemcost_id", param.toInt());
    q.exec();
    if (q.first())
    {
      _item->setId(q.value("itemcost_item_id").toInt());
      _item->setReadOnly(TRUE);
      _costType->setId(q.value("itemcost_costelem_id").toInt());
      _costType->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return UndefinedError;
    }
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspItemCostDetail::sPopulate()
{
  q.prepare( "SELECT costelem_id, costelem_type, costelem_type "
             "FROM costelem, itemcost "
             "WHERE ( (itemcost_costelem_id=costelem_id)"
             " AND (itemcost_lowlevel)"
             " AND (itemcost_item_id=:item_id) );" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  _costType->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool dspItemCostDetail::setParams(ParameterList &params)
{
  params.append("item_id", _item->id());
  params.append("costelem_id", _costType->id());

  if (_standardCosts->isChecked())
    params.append("standardCost");
  else
    params.append("actualCost");

  if ( (_item->itemType() == "M") ||
       (_item->itemType() == "F") ||
       (_item->itemType() == "B") ||
       (_item->itemType() == "T") ||
       (_item->itemType() == "Y") ||
       (_item->itemType() == "R") ||
       (_item->itemType() == "O") ||
       (_item->itemType() == "P") )
    params.append("useBOM");
  else if (_item->itemType() == "C")
    params.append("useBBOM");

  if (_metrics->value("Application") != "PostBooks")
    params.append("includeRevisionControl");

  return true;
}

void dspItemCostDetail::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("ItemCostDetail", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostDetail::sFillList()
{
  sFillList(-1, TRUE);
}

void dspItemCostDetail::sFillList(int pItemid, bool pLocale)
{
  if (! pLocale && (pItemid != _item->id()))
    return;

  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("itemcostdetail", "detail");

  q = mql.toQuery(params);
  _bom->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
