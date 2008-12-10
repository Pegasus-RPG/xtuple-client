/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
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
  params.append("revControl", _metrics->boolean("RevControl"));
  if (_showOnlyTopLevel->isChecked())
    params.append("showOnlyTopLevel");
  q = mql.toQuery(params);
  _wo->populate(q);
}

bool dspWoHistoryByNumber::checkParameters()
{
  return TRUE;
}

