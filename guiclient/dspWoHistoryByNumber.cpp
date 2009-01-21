/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoHistoryByNumber.h"

#include <QVariant>
#include <QValidator>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMenu>
#include <QMessageBox>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>
#include <parameter.h>
#include "workOrder.h"

#define COST_COL	10

/*
 *  Constructs a dspWoHistoryByNumber as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoHistoryByNumber::dspWoHistoryByNumber(QWidget* parent, const char* name, Qt::WFlags fl)
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

  _wo->addColumn(tr("W/O #"),       _orderColumn,  Qt::AlignLeft,   true,  "wo_number"   );
  _wo->addColumn(tr("Sub. #"),      _uomColumn,    Qt::AlignLeft,   true,  "wo_subnumber"   );
  _wo->addColumn(tr("Item #"),      _itemColumn,   Qt::AlignLeft,   true,  "item_number"   );
  _wo->addColumn(tr("Description"), -1,            Qt::AlignLeft,   true,  "itemdescrip"   );
  _wo->addColumn(tr("Status"),      _statusColumn, Qt::AlignCenter, true,  "wo_status" );
  _wo->addColumn(tr("Site"),        _whsColumn,    Qt::AlignCenter, true,  "warehous_code" );
  _wo->addColumn(tr("Ordered"),     _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyord"  );
  _wo->addColumn(tr("Received"),    _qtyColumn,    Qt::AlignRight,  true,  "wo_qtyrcv"  );
  _wo->addColumn(tr("Start Date"),  _dateColumn,   Qt::AlignCenter, true,  "wo_startdate"  );
  _wo->addColumn(tr("Due Date"),    _dateColumn,   Qt::AlignCenter, true,  "wo_duedate"  );
  _wo->addColumn(tr("Cost"),        _costColumn,   Qt::AlignRight,  true,  "wo_postedvalue" );
  _wo->addColumn(tr("WIP"),         _costColumn,   Qt::AlignRight,  false, "wo_wipvalue" );
  _wo->addColumn(tr("Project"),     _orderColumn,  Qt::AlignLeft,   false, "project" );
  _wo->addColumn(tr("Priority"),    _statusColumn, Qt::AlignCenter, false, "wo_priority" );
  _wo->addColumn(tr("BOM Rev"),     _orderColumn,  Qt::AlignLeft,   false, "bom_rev_number" );
  _wo->addColumn(tr("BOO Rev"),     _orderColumn,  Qt::AlignLeft,   false, "boo_rev_number" );

  sHandleCosts(_showCost->isChecked());

  connect(omfgThis, SIGNAL(workOrdersUpdated(int, bool)), SLOT(sFillList()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoHistoryByNumber::~dspWoHistoryByNumber()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoHistoryByNumber::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspWoHistoryByNumber::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("wo_number", &valid);
  if (valid)
  {
    _woNumber->setText(param.toString());
    sFillList();
  }

  return NoError;
}

void dspWoHistoryByNumber::sPrint()
{
  if(_woNumber->text().length() > 0)
  {
    ParameterList params;

    params.append("woNumber", _woNumber->text());

    if(_showOnlyTopLevel->isChecked())
      params.append("showOnlyTopLevel");
    if(_showCost->isChecked())
      params.append("showCosts");

    orReport report("WOHistoryByNumber", params);
    if(report.isValid())
      report.print();
    else
      report.reportError(this);
  }
  else
  {
    QMessageBox::warning( this, tr("Invalid Work Order Number"),
                      tr( "You must enter a work order number for this report." ) );
    _woNumber->setFocus();
  }
}

void dspWoHistoryByNumber::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByNumber::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wo_id", _wo->id());

  workOrder *newdlg = new workOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspWoHistoryByNumber::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspWoHistoryByNumber::sHandleCosts(bool pShowCosts)
{
  if (pShowCosts)
    _wo->showColumn(COST_COL);
  else
    _wo->hideColumn(COST_COL);
}

void dspWoHistoryByNumber::sFillList()
{
  if (!checkParameters())
    return;
    
  _wo->clear();

  MetaSQLQuery mql = mqlLoad("workOrderHistory", "detail");
  ParameterList params;
  params.append("wo_number", _woNumber->text());
  if (_metrics->boolean("RevControl"))
    params.append("revControl");
  if (_showOnlyTopLevel->isChecked())
    params.append("showOnlyTopLevel");
  q = mql.toQuery(params);
  _wo->populate(q);
}

bool dspWoHistoryByNumber::checkParameters()
{
  return TRUE;
}

