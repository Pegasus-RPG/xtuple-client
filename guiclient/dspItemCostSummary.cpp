/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemCostSummary.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>

#include <metasql.h>
#include "mqlutil.h"

#include "dspItemCostDetail.h"
#include "itemCost.h"

/*
 *  Constructs a dspItemCostSummary as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemCostSummary::dspItemCostSummary(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_itemcost, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));

  _itemcost->addColumn(tr("Element"),   -1,           Qt::AlignLeft,   true,  "costelem_type"   );
  _itemcost->addColumn(tr("Lower"),     _costColumn,  Qt::AlignCenter, true,  "lowlevel" );
  _itemcost->addColumn(tr("Std. Cost"), _costColumn,  Qt::AlignRight,  true,  "itemcost_stdcost"  );
  _itemcost->addColumn(tr("Posted"),    _dateColumn,  Qt::AlignCenter, true,  "itemcost_posted" );
  _itemcost->addColumn(tr("Act. Cost"), _costColumn,  Qt::AlignRight,  true,  "itemcost_actcost"  );
  _itemcost->addColumn(tr("Updated"),   _dateColumn,  Qt::AlignCenter, true,  "itemcost_updated" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemCostSummary::~dspItemCostSummary()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemCostSummary::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspItemCostSummary::set(const ParameterList &pParams)
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

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspItemCostSummary::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  orReport report("ItemCostSummary", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostSummary::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (pSelected->text(1) == "Yes")
  {
    pMenu->insertItem(tr("View Costing Detail..."), this, SLOT(sViewDetail()), 0);
    pMenu->insertSeparator();
  }
}

void dspItemCostSummary::sViewDetail()
{
  ParameterList params;
  params.append("itemcost_id", _itemcost->id());
  params.append("run");

  dspItemCostDetail *newdlg = new dspItemCostDetail();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemCostSummary::sFillList()
{
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

bool dspItemCostSummary::setParams(ParameterList &params)
{
  params.append("bySummrey");

  params.append("item_id", _item->id());
  params.append("never", tr("Never")); 
  return true;
}
