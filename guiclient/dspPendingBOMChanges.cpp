/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPendingBOMChanges.h"

#include <QMenu>

#include <openreports.h>
#include <parameter.h>

#include "bomItem.h"

dspPendingBOMChanges::dspPendingBOMChanges(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_revision, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_cutoff, SIGNAL(newDate(const QDate&)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _item->setType(ItemLineEdit::cGeneralManufactured);

  _cutoff->setNullString(tr("Latest"));
  _cutoff->setNullDate(omfgThis->endOfTime().addDays(-1));
  _cutoff->setAllowNullDate(TRUE);
  _cutoff->setNull();

  _bomitem->addColumn(tr("Date"),        _dateColumn,  Qt::AlignCenter, true,  "actiondate" );
  _bomitem->addColumn(tr("Action"),      _itemColumn,  Qt::AlignCenter, true,  "action" );
  _bomitem->addColumn(tr("Seq #"),       40,           Qt::AlignCenter, true,  "bomitem_seqnumber"  );
  _bomitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _bomitem->addColumn(tr("Description"), -1,           Qt::AlignCenter, true,  "description" );
  _bomitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  _bomitem->addColumn(tr("Qty. Per"),    _qtyColumn,   Qt::AlignRight,  true,  "qtyper"  );
  _bomitem->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight,  true,  "bomitem_scrap"  );
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  //If not Revision Control, hide control
  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspPendingBOMChanges::~dspPendingBOMChanges()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspPendingBOMChanges::languageChange()
{
  retranslateUi(this);
}

void dspPendingBOMChanges::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());
  params.append("cutOffDate", _cutoff->date());

  orReport report("PendingBOMChanges", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPendingBOMChanges::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit BOM Item..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainBOMs"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View BOM Item..."), this, SLOT(sView()), 0);
  if ( (!_privileges->check("MaintainBOMs")) && (!_privileges->check("ViewBOMs")) )
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspPendingBOMChanges::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bomitem_id", _bomitem->id());

  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspPendingBOMChanges::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bomitem_id", _bomitem->id());

  bomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspPendingBOMChanges::sFillList()
{
  sFillList(-1, FALSE);
}

void dspPendingBOMChanges::sFillList(int, bool)
{
  if ((_item->isValid()) && (_cutoff->isValid()))
  {
    q.prepare( "SELECT bomitem_id, actiondate, action,"
               "       bomitem_seqnumber, item_number, description,"
               "       uom_name, qtyper,"
               "       bomitem_scrap, actiondate,"
               "       'qtyper' AS qtyper_xtnumericrole,"
               "       'percent' AS bomitem_scrap_xtnumericrole "
               "FROM ( "
               "SELECT bomitem_id, :effective AS action,"
               "       bomitem_seqnumber, item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       uom_name, itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper) AS qtyper,"
               "       bomitem_scrap, bomitem_effective AS actiondate "
               "FROM bomitem(:item_id,:revision_id), item, uom "
               "WHERE ( (bomitem_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (bomitem_effective BETWEEN CURRENT_DATE AND :cutOffDate) ) "
               "UNION "
               "SELECT bomitem_id, :expires AS action, "
               "       bomitem_seqnumber, item_number, (item_descrip1 || ' ' || item_descrip2) AS description,"
               "       uom_name, itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper) AS qtyper,"
               "       bomitem_scrap, bomitem_expires AS actiondate "
               "FROM bomitem(:item_id,:revision_id), item, uom "
               "WHERE ( (bomitem_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               " AND (bomitem_expires BETWEEN CURRENT_DATE AND :cutOffDate) ) "
               "    ) AS data "
               "ORDER BY action, actiondate, bomitem_seqnumber;" );
    q.bindValue(":effective", tr("Effective"));
    q.bindValue(":expires", tr("Expires"));
    q.bindValue(":item_id", _item->id());
    q.bindValue(":revision_id", _revision->id());
    q.bindValue(":cutOffDate", _cutoff->date());
    q.exec();
    _bomitem->populate(q);
  }
  else
    _bomitem->clear();
}
