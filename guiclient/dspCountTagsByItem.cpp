/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCountTagsByItem.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "countTag.h"
#include "mqlutil.h"

dspCountTagsByItem::dspCountTagsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cnttag, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _cnttag->addColumn(tr("Tag #"),               -1, Qt::AlignLeft,  true, "invcnt_tagnumber");
  _cnttag->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _cnttag->addColumn(tr("Item"),       _itemColumn, Qt::AlignLeft,  true, "item_number");
  _cnttag->addColumn(tr("Created"),    _dateColumn, Qt::AlignCenter,true, "invcnt_tagdate");
  _cnttag->addColumn(tr("Created By"), _dateColumn, Qt::AlignCenter,true, "creator");
  _cnttag->addColumn(tr("Entered"),    _dateColumn, Qt::AlignCenter,true, "invcnt_cntdate");
  _cnttag->addColumn(tr("Entered By"), _dateColumn, Qt::AlignCenter,true, "counter");
  _cnttag->addColumn(tr("Posted"),     _dateColumn, Qt::AlignCenter,true, "invcnt_postdate");
  _cnttag->addColumn(tr("Posted By"),  _dateColumn, Qt::AlignCenter,true, "poster");
  _cnttag->addColumn(tr("QOH Before"),  _qtyColumn, Qt::AlignRight, true, "qohbefore");
  _cnttag->addColumn(tr("Qty. Counted"),_qtyColumn, Qt::AlignRight, true, "invcnt_qoh_after");
  _cnttag->addColumn(tr("Variance"),    _qtyColumn, Qt::AlignRight, true, "variance");
  _cnttag->addColumn(tr("%"),         _prcntColumn, Qt::AlignRight, true, "percent");

  if (_preferences->boolean("XCheckBox/forgetful"))
    _showUnposted->setChecked(true);
  
  sFillList();
}

dspCountTagsByItem::~dspCountTagsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCountTagsByItem::languageChange()
{
  retranslateUi(this);
}

bool dspCountTagsByItem::setParams(ParameterList &params)
{
  if (! _item->isValid())
  {
    _item->setFocus();
    return false;
  }
  if (! _dates->allValid())
  {
    _dates->setFocus();
    return false;
  }
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("item_id", _item->id());

  if (_showUnposted->isChecked())
    params.append("showUnposted");

  return true;
}

void dspCountTagsByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  orReport report("CountTagsByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCountTagsByItem::sPopulateMenu(QMenu *pMenu)
{
  pMenu->addAction(tr("View Count Tag..."), this, SLOT(sView()));
}

void dspCountTagsByItem::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cnttag_id", _cnttag->id());

  countTag newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCountTagsByItem::sFillList()
{
  MetaSQLQuery mql = mqlLoad("countTags", "detail");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _cnttag->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
