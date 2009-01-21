/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoHistoryByClassCode.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMenu>
#include <openreports.h>
#include "workOrder.h"

/*
 *  Constructs a dspWoHistoryByClassCode as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoHistoryByClassCode::dspWoHistoryByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wo, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showCost, SIGNAL(toggled(bool)), this, SLOT(sHandleCosts(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ParameterGroup::ClassCode);

  _wo->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft,   true,  "wonumber"   );
  _wo->addColumn(tr("Item #"),      _itemColumn,   Qt::AlignLeft,   true,  "item_number"   );
  _wo->addColumn(tr("Description"), -1,            Qt::AlignLeft,   true,  "itemdescrip"   );
  _wo->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  _wo->addColumn(tr("Site"),        _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  _wo->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  _wo->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  _wo->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignRight,  true,  "wo_startdate"  );
  _wo->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignRight,  true,  "wo_duedate"  );
  _wo->addColumn(tr("Cost"),        _costColumn,   Qt::AlignRight,  true,  "wo_postedvalue" );

  sHandleCosts(_showCost->isChecked());
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoHistoryByClassCode::~dspWoHistoryByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoHistoryByClassCode::languageChange()
{
  retranslateUi(this);
}

void dspWoHistoryByClassCode::sPrint()
{
  ParameterList params;

  _classCode->appendValue(params);
  _warehouse->appendValue(params);

  if(_topLevel->isChecked())
    params.append("showOnlyTopLevel");

  if(_showCost->isChecked())
    params.append("showCosts");

  orReport report("WOHistoryByClassCode", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoHistoryByClassCode::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByClassCode::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByClassCode::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspWoHistoryByClassCode::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
    _wo->showColumn(9);
  else
    _wo->hideColumn(9);
}

void dspWoHistoryByClassCode::sFillList()
{
  if (!checkParameters())
    return;

  QString sql( "SELECT wo_id, formatWoNumber(wo_id) AS wonumber, item_number,"
               "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip,"
               "       wo_status, warehous_code,"
               "       wo_qtyord, wo_qtyrcv, wo_postedvalue,"
               "       wo_startdate,wo_duedate,"
               "       'qty' AS wo_qtyord_xtnumericrole,"
               "       'qty' AS wo_qtyrcv_xtnumericrole,"
               "       'cost' AS wo_postedvalue_xtnumericrole "
               "FROM wo, itemsite, warehous, item "
               "WHERE ((wo_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (itemsite_warehous_id=warehous_id)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id)";
  else if (_classCode->isPattern())
    sql += " AND (item_classcode_id IN (SELECT classcode_id FROM classcode WHERE (classcode_code ~ :classcode_pattern)))";

  if (_topLevel->isChecked())
    sql += " AND ( (wo_ordtype<>'W') OR (wo_ordtype IS NULL) )";

  sql += ") "
         "ORDER BY item_number;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _classCode->bindValue(q);
  q.exec();
  _wo->populate(q);
}

bool dspWoHistoryByClassCode::checkParameters()
{
  return TRUE;
}
