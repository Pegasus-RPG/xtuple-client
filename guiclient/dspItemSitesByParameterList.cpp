/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspItemSitesByParameterList.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include <QMenu>
#include <openreports.h>
#include <metasql.h>

#include "mqlutil.h"
#include "createCountTagsByItem.h"
#include "dspInventoryAvailabilityByItem.h"
#include "itemSite.h"

/*
 *  Constructs a dspItemSitesByParameterList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemSitesByParameterList::dspItemSitesByParameterList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_itemsite, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _parameter->setType(ParameterGroup::ClassCode);

  _itemsite->addColumn(tr("Site"),          _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  _itemsite->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _itemsite->addColumn(tr("Description"),   -1,           Qt::AlignLeft,   true,  "description"   );
  _itemsite->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  _itemsite->addColumn(tr("QOH"),           _qtyColumn,   Qt::AlignRight,  true,  "itemsite_qtyonhand"  );
  _itemsite->addColumn(tr("Loc. Cntrl."),   _dateColumn,  Qt::AlignCenter, true,  "loccntrl" );
  _itemsite->addColumn(tr("Cntrl. Meth."),  _dateColumn,  Qt::AlignCenter, true,  "controlmethod" );
  _itemsite->addColumn(tr("Sold Ranking"),  _dateColumn,  Qt::AlignCenter, true,  "soldranking" );
  _itemsite->addColumn(tr("ABC Class"),     _dateColumn,  Qt::AlignCenter, true,  "itemsite_abcclass" );
  _itemsite->addColumn(tr("Cycle Cnt."),    _dateColumn,  Qt::AlignCenter, true,  "itemsite_cyclecountfreq" );
  _itemsite->addColumn(tr("Last Cnt'd"),    _dateColumn,  Qt::AlignCenter, true,  "datelastcount" );
  _itemsite->addColumn(tr("Last Used"),     _dateColumn,  Qt::AlignCenter, true,  "datelastused" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemSitesByParameterList::~dspItemSitesByParameterList()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemSitesByParameterList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspItemSitesByParameterList::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("warehous_id", &valid);
  if (valid)
    _warehouse->setId(param.toInt());

  _showInactive->setChecked(pParams.inList("showInactive"));

  param = pParams.value("classcode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("classcode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("classcode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ClassCode);

  param = pParams.value("plancode_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("plancode_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::PlannerCode);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("plancode", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::PlannerCode);

  param = pParams.value("itemgrp_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("itemgrp_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ItemGroup);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("itemgrp", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::ItemGroup);

  param = pParams.value("costcat_id", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CostCategory);
    _parameter->setId(param.toInt());
  }

  param = pParams.value("costcat_pattern", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::CostCategory);
    _parameter->setPattern(param.toString());
  }

  param = pParams.value("costcat", &valid);
  if (valid)
    _parameter->setType(ParameterGroup::CostCategory);

  switch (_parameter->type())
  {
    case ParameterGroup::ClassCode:
      setWindowTitle(tr("Item Sites by Class Code"));
      break;

    case ParameterGroup::PlannerCode:
      setWindowTitle(tr("Item Sites by Planner Code"));
      break;

    case ParameterGroup::ItemGroup:
      setWindowTitle(tr("Item Sites by Item Group"));
      break;

    case ParameterGroup::CostCategory:
      setWindowTitle(tr("Item Sites by Cost Category"));
      break;

    default:
      break;
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspItemSitesByParameterList::sPrint()
{
  ParameterList params;
  _parameter->appendValue(params);
  _warehouse->appendValue(params);

  if(_showInactive->isChecked())
    params.append("showInactive");

  orReport report("ItemSitesByParameterList", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemSitesByParameterList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByParameterList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsite_id", _itemsite->id());

  itemSite newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByParameterList::sInventoryAvailability()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  params.append("run");
  params.append("byLeadTime");

  dspInventoryAvailabilityByItem *newdlg = new dspInventoryAvailabilityByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspItemSitesByParameterList::sIssueCountTag()
{
  ParameterList params;
  params.append("itemsite_id", _itemsite->id());
  
  createCountTagsByItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspItemSitesByParameterList::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Item Site..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainItemSites")) && (!_privileges->check("ViewItemSites")))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Item Site..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemSites"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("View Inventory Availability..."), this, SLOT(sInventoryAvailability()), 0);
  if (!_privileges->check("ViewInventoryAvailability"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Issue Count Tag..."), this, SLOT(sIssueCountTag()), 0);
  if (!_privileges->check("IssueCountTags"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspItemSitesByParameterList::sFillList()
{
  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("itemSites", "detail");
  q = mql.toQuery(params);
  _itemsite->populate(q, true);  
}

bool dspItemSitesByParameterList::setParams(ParameterList &params)
  {
  params.append("byParameterList");  

  params.append("regular", tr("Regular"));
  params.append("none", tr("None"));
  params.append("lot", tr("Lot #"));
  params.append("serial", tr("Serial #"));
  params.append("na", tr("N/A"));
  
  if (_showInactive->isChecked())
    params.append("showInactive");  
  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());

  if (_parameter->isSelected())
    params.append("byParameterId");
  else if (_parameter->isPattern())
    params.append("byParameterPattern");
  _parameter->appendValue(params);
  return true;
}
