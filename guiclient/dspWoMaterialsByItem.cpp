/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoMaterialsByItem.h"

#include <QVariant>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>
#include "mqlutil.h"
#include "inputManager.h"

/*
 *  Constructs a dspWoMaterialsByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoMaterialsByItem::dspWoMaterialsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  omfgThis->inputManager()->notify(cBCItem, this, _item, SLOT(setItemid(int)));
  omfgThis->inputManager()->notify(cBCItemSite, this, _item, SLOT(setItemsiteid(int)));

  _manufacturing = false;
  if (_metrics->value("Application") == "Standard")
  {
    XSqlQuery xtmfg;
    xtmfg.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtmfg'");
    if (xtmfg.first())
      _manufacturing = true;
  }

  _womatl->addColumn(tr("W/O #"),         _orderColumn, Qt::AlignLeft,   true,  "wonumber"   );
  _womatl->addColumn(tr("Parent Item #"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  if (_manufacturing)
    _womatl->addColumn(tr("Oper. #"),       _dateColumn,  Qt::AlignCenter, true,  "wooperseq" );
  _womatl->addColumn(tr("Iss. Meth."),    _dateColumn,  Qt::AlignCenter, true,  "issuemethod" );
  _womatl->addColumn(tr("Iss. UOM"),      _uomColumn,   Qt::AlignLeft,   true,  "uom_name"   );
  _womatl->addColumn(tr("Fxd. Qty."),     _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyfxd"  );
  _womatl->addColumn(tr("Qty. Per"),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyper"  );
  _womatl->addColumn(tr("Scrap %"),       _prcntColumn, Qt::AlignRight,  true,  "womatl_scrap"  );
  _womatl->addColumn(tr("Required"),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyreq"  );
  _womatl->addColumn(tr("Issued"),        _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyiss"  );
  _womatl->addColumn(tr("Scrapped"),      _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtywipscrap"  );
  _womatl->addColumn(tr("Balance"),       _qtyColumn,   Qt::AlignRight,  true,  "balance"  );
  _womatl->addColumn(tr("Due Date"),      _dateColumn,  Qt::AlignCenter, true,  "womatl_duedate" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoMaterialsByItem::~dspWoMaterialsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoMaterialsByItem::languageChange()
{
  retranslateUi(this);
}

void dspWoMaterialsByItem::setParams(ParameterList & params)
{
  params.append("item_id", _item->id());
  _warehouse->appendValue(params);
  params.append("push", tr("Push"));
  params.append("pull", tr("Pull"));
  params.append("mixed", tr("Mixed"));
  params.append("error", tr("Error"));
  if (_manufacturing)
      params.append("Manufacturing");
}

void dspWoMaterialsByItem::sPrint()
{
  if (!_item->isValid())
    return;
    
  ParameterList params;
  setParams(params);
  params.append("includeFormatted");

  orReport report("WOMaterialRequirementsByComponentItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoMaterialsByItem::sFillList()
{
  if (!_item->isValid())
    return;
    
  MetaSQLQuery mql = mqlLoad("workOrderMaterial", "detail");
  ParameterList params;
  setParams(params);

  q = mql.toQuery(params);
  _womatl->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

