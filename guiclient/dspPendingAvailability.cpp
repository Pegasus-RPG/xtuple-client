/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPendingAvailability.h"

#include <QVariant>
#include <QValidator>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>

#include <metasql.h>
#include "mqlutil.h"

/*
 *  Constructs a dspPendingAvailability as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPendingAvailability::dspPendingAvailability(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(privateIdChanged(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_warehouse, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cTooling);
  _item->setDefaultType(ItemLineEdit::cGeneralManufactured);

  _effective->setNullString(tr("Now"));
  _effective->setNullDate(QDate::currentDate());
  _effective->setAllowNullDate(TRUE);
  _effective->setNull();

  _buildDate->setNullString(tr("Latest"));
  _buildDate->setNullDate(omfgThis->endOfTime());
  _buildDate->setAllowNullDate(TRUE);
  _buildDate->setNull();

  _qtyToBuild->setValidator(omfgThis->qtyVal());
  _qtyToBuild->setText("1.0");

  _items->addColumn(tr("#"),            _seqColumn,  Qt::AlignCenter, true,  "bomitem_seqnumber" );
  _items->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _items->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true,  "item_descrip"   );
  _items->addColumn(tr("UOM"),          _uomColumn,  Qt::AlignCenter, true,  "uom_name" );
  _items->addColumn(tr("Pend. Alloc."), _qtyColumn,  Qt::AlignRight,  true,  "pendalloc"  );
  _items->addColumn(tr("Total Alloc."), _qtyColumn,  Qt::AlignRight,  true,  "totalalloc"  );
  _items->addColumn(tr("QOH"),          _qtyColumn,  Qt::AlignRight,  true,  "qoh"  );
  _items->addColumn(tr("Availability"), _qtyColumn,  Qt::AlignRight,  true,  "totalavail"  );

  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPendingAvailability::~dspPendingAvailability()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPendingAvailability::languageChange()
{
  retranslateUi(this);
}

void dspPendingAvailability::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("warehous_id", _warehouse->id());
  params.append("buildQty", _qtyToBuild->toDouble());
  params.append("effectiveDate", _effective->date());
  params.append("buildDate", _buildDate->date());

  if(_showShortages->isChecked())
    params.append("showShortages");

  orReport report("PendingWOMaterialAvailability", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPendingAvailability::sFillList()
{
  _items->clear();

  ParameterList params;
  if (! setParams(params))
    return;
  MetaSQLQuery mql = mqlLoad("pendingAvailability", "detail");
  q = mql.toQuery(params);
  _items->populate(q, true); 
}

bool dspPendingAvailability::setParams(ParameterList &params)
{
  params.append("buildQty", _qtyToBuild->toDouble());
  params.append("buildDate", _buildDate->date());

  params.append("warehous_id", _warehouse->id());

  if (_item->isValid())
    params.append("item_id", _item->id());
  else
    return false;

  if (!(_effective->isNull()))
    params.append("effective",  _effective->date());

  if (_showShortages->isChecked())
    params.append("showShortages");

  return true;
}
