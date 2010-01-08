/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspValidLocationsByItem.h"

#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include <openreports.h>
#include <parameter.h>

dspValidLocationsByItem::dspValidLocationsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_location, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _location->addColumn(tr("Site"),        _whsColumn,   Qt::AlignCenter, true,  "warehous_code" );
  _location->addColumn(tr("Location"),    _itemColumn,  Qt::AlignLeft,   true,  "locationname"   );
  _location->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "locationdescrip"   );
  _location->addColumn(tr("Restricted"),  _orderColumn, Qt::AlignCenter, true,  "location_restrict"  );
  _location->addColumn(tr("Netable"),     _orderColumn, Qt::AlignCenter, true,  "location_netable" );

  _item->setFocus();
}

dspValidLocationsByItem::~dspValidLocationsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspValidLocationsByItem::languageChange()
{
  retranslateUi(this);
}

void dspValidLocationsByItem::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  _warehouse->appendValue(params);

  orReport report("ValidLocationsByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspValidLocationsByItem::sPopulateMenu(QMenu *)
{
}

void dspValidLocationsByItem::sFillList()
{
  QString sql = "SELECT location_id, warehous_code,"
                "       formatLocationName(location_id) AS locationname,"
                "       firstLine(location_descrip) AS locationdescrip,"
                "       location_restrict, location_netable "
                "FROM itemsite, location, warehous "
                "WHERE ( (validLocation(location_id, itemsite_id))"
                " AND ( (itemsite_loccntrl) OR (itemsite_location_id=location_id) )"
                " AND (itemsite_item_id=:item_id)"
                " AND (itemsite_warehous_id=warehous_id)";

  if (_warehouse->isSelected())
    sql += " AND (warehous_id=:warehous_id)";

  sql += ") "
         "ORDER BY warehous_code, locationname;";

  q.prepare(sql);
  q.bindValue(":item_id", _item->id());
  _warehouse->bindValue(q);
  q.exec();
  _location->populate(q);
}

