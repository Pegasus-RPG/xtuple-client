/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemCostHistory.h"

#include <QMessageBox>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include <parameter.h>

dspItemCostHistory::dspItemCostHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _itemcost->addColumn(tr("Element"),              -1, Qt::AlignLeft,  true, "costelem_type");
  _itemcost->addColumn(tr("Lower"),       _costColumn, Qt::AlignCenter,true, "lowlevel");
  _itemcost->addColumn(tr("Type"),        _costColumn, Qt::AlignLeft,  true, "type");
  _itemcost->addColumn(tr("Time"),    _timeDateColumn, Qt::AlignCenter,true, "costhist_date");
  _itemcost->addColumn(tr("User"),         _qtyColumn, Qt::AlignCenter,true, "username");
  _itemcost->addColumn(tr("Old"),         _costColumn, Qt::AlignRight, true, "costhist_oldcost");
  _itemcost->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "oldcurr");
  _itemcost->addColumn(tr("New"),         _costColumn, Qt::AlignRight, true, "costhist_newcost");
  _itemcost->addColumn(tr("Currency"),_currencyColumn, Qt::AlignLeft,  true, "newcurr");

  if (omfgThis->singleCurrency())
  {
      _itemcost->hideColumn("oldcurr");
      _itemcost->hideColumn("newcurr");
  }
}

dspItemCostHistory::~dspItemCostHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspItemCostHistory::languageChange()
{
  retranslateUi(this);
}

void dspItemCostHistory::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  orReport report("ItemCostHistory", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostHistory::sFillList()
{
  _itemcost->clear();

  if (_item->isValid())
  {
    MetaSQLQuery mql = mqlLoad("itemCost", "detail");

    ParameterList params;
    if (! setParams(params))
      return;

   q = mql.toQuery(params);
   _itemcost->populate(q, true);
  }
}

bool dspItemCostHistory::setParams(ParameterList &params)
{
  params.append("byHistory");

  params.append("actual", tr("Actual"));
  params.append("standard", tr("Standard"));
  params.append("delete", tr("Delete"));
  params.append("new", tr("New"));

  params.append("item_id", _item->id());
 
  return true;
}
