/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoHistoryByItem.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMenu>
#include <openreports.h>
#include <parameter.h>
#include <metasql.h>
#include "mqlutil.h"
#include "workOrder.h"

/*
 *  Constructs a dspWoHistoryByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoHistoryByItem::dspWoHistoryByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showCost, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));
  connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased);
  _item->setDefaultType(ItemLineEdit::cGeneralManufactured);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _wo->addColumn(tr("W/O #"),      -1,            Qt::AlignLeft,   true,  "wonumber"   );
  _wo->addColumn(tr("Status"),     _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  _wo->addColumn(tr("Site"),       _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  _wo->addColumn(tr("Ordered"),    _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  _wo->addColumn(tr("Received"),   _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  _wo->addColumn(tr("Start Date"), _dateColumn,   Qt::AlignCenter, true,  "wo_startdate" );
  _wo->addColumn(tr("Due Date"),   _dateColumn,   Qt::AlignCenter, true,  "wo_duedate" );
  _wo->addColumn(tr("Cost"),       _costColumn,   Qt::AlignRight,  true,  "wo_postedvalue" );

  sHandleCosts(_showCost->isChecked());
  
  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), this, SLOT(sFillList()));

  _item->setFocus();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoHistoryByItem::~dspWoHistoryByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoHistoryByItem::languageChange()
{
  retranslateUi(this);
}

void dspWoHistoryByItem::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());

  if(_showOnlyTopLevel->isChecked())
    params.append("showOnlyTopLevel");

  if(_showCost->isChecked())
    params.append("showCosts");

  orReport report("WOHistoryByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoHistoryByItem::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByItem::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByItem::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspWoHistoryByItem::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
    _wo->showColumn(7);
  else
    _wo->hideColumn(7);

  sFillList();
}

void dspWoHistoryByItem::sFillList()
{
  if (!checkParameters())
    return;

  _wo->clear();
  ParameterList params;
  setParams(params);
  MetaSQLQuery mql = mqlLoad("workOrderHistory", "detail");
  q = mql.toQuery(params);
  _wo->populate(q);
}

void dspWoHistoryByItem::setParams(ParameterList & params)
{
  params.append("woHistoryByItem");
  params.append("item_id", _item->id());
  params.append("startDate", _dates->startDate());
  params.append("endDate", _dates->endDate());
  if (_showOnlyTopLevel->isChecked())
    params.append("showOnlyTopLevel");

  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());
}

bool dspWoHistoryByItem::checkParameters()
{
  if (! _item->isValid())
  {
    QMessageBox::warning(this, tr("Invalid Item"),
                         tr("Enter a valid Item."));
    _item->setFocus();
    return FALSE;
  }

  if (! _dates->startDate().isValid())
  {
    QMessageBox::warning(this, tr("Invalid Start Date"),
                         tr("Enter a valid Start Date."));
    _dates->setFocus();
    return FALSE;
  }

  if (! _dates->endDate().isValid())
  {
    QMessageBox::warning(this, tr("Invalid End Date"),
                         tr("Enter a valid End Date."));
    _dates->setFocus();
    return FALSE;
  }

  return TRUE;
}
