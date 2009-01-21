/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSequencedBOM.h"

#include <QVariant>

#include <openreports.h>

dspSequencedBOM::dspSequencedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_revision, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_item, SIGNAL(valid(bool)), _revision, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured  | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cKit);

  _bomitem->addColumn(tr("BOO Seq. #"),  _qtyColumn,   Qt::AlignCenter, true,  "seqnumber" );
  _bomitem->addColumn(tr("BOM Seq. #"),  _qtyColumn,   Qt::AlignCenter, true,  "bomitem_seqnumber" );
  _bomitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _bomitem->addColumn(tr("Description"), -1,           Qt::AlignLeft,   true,  "itemdescrip"   );
  _bomitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter, true,  "uom_name" );
  _bomitem->addColumn(tr("Qty."),        _qtyColumn,   Qt::AlignRight,  true,  "qtyper"  );
  _bomitem->addColumn(tr("Scrap %"),     _prcntColumn, Qt::AlignRight,  true,  "bomitem_scrap"  );
  _bomitem->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter, true,  "bomitem_effective" );
  _bomitem->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter, true,  "bomitem_expires" );

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), SLOT(sFillList(int, bool)));
  _revision->setEnabled(false);
  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  //If not Revision Control, hide control
  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspSequencedBOM::~dspSequencedBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSequencedBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSequencedBOM::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspSequencedBOM::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());

  if (_showExpired->isChecked())
    params.append("showExpired");

  if (_showFuture->isChecked())
    params.append("showFuture");

  orReport report("SequencedBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSequencedBOM::sFillList()
{
  if (_item->isValid())
    sFillList(_item->id(), FALSE);
  else
    _bomitem->clear();
}

void dspSequencedBOM::sFillList(int pItemid, bool)
{
  if (pItemid == _item->id())
  {
    _bomitem->clear();

    QString sql( "SELECT bomitem_id, TEXT(booitem_seqnumber) AS seqnumber, bomitem_seqnumber, item_number,"
                 "       (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
                 "       itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper) AS qtyper,"
                 "       bomitem_scrap, bomitem_effective, bomitem_expires,"
                 "       'qtyper' AS qtyper_xtnumericrole,"
                 "       'percent' AS bomitem_scrap_xtnumericrole,"
                 "       CASE WHEN COALESCE(bomitem_effective, startOfTime()) <= startOfTime() THEN :always"
                 "       END AS bomitem_effective_qtdisplayrole,"
                 "       CASE WHEN COALESCE(bomitem_expires, endOfTime()) >= endOfTime() THEN :never"
                 "       END AS bomitem_expires_qtdisplayrole,"
                 "       CASE WHEN(bomitem_effective > CURRENT_DATE) THEN 'future'"
                 "       END AS bomitem_effective_qtforegroundrole,"
                 "       CASE WHEN(bomitem_expires <= CURRENT_DATE) THEN 'expired'"
                 "       END AS bomitem_expires_qtforegroundrole "
				 "FROM bomitem(:item_id,:revision_id), booitem(:item_id), item, uom "
                 "WHERE ( (bomitem_item_id=item_id)"
                 " AND (item_inv_uom_id=uom_id)"
                 " AND (bomitem_booitem_seq_id=booitem_seq_id)" );

    if (!_showExpired->isChecked())
      sql += " AND (bomitem_expires>CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (bomitem_effective<=CURRENT_DATE)";

    sql += " ) "
           "UNION SELECT bomitem_id, '' AS seqnumber, bomitem_seqnumber, item_number,"
           "             (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, uom_name,"
           "             itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper) AS qtyper,"
           "             bomitem_scrap, bomitem_effective, bomitem_expires,"
           "            'qtyper' AS qtyper_xtnumericrole,"
           "            'percent' AS bomitem_scrap_xtnumericrole,"
           "            CASE WHEN COALESCE(bomitem_effective, startOfTime()) <= startOfTime() THEN :always"
           "            END AS bomitem_effective_qtdisplayrole,"
           "            CASE WHEN COALESCE(bomitem_expires, endOfTime()) >= endOfTime() THEN :never"
           "            END AS bomitem_expires_qtdisplayrole,"
           "            CASE WHEN(bomitem_effective > CURRENT_DATE) THEN 'future'"
           "            END AS bomitem_effective_qtforegroundrole,"
           "            CASE WHEN(bomitem_expires <= CURRENT_DATE) THEN 'expired'"
           "            END AS bomitem_expires_qtforegroundrole "
           "FROM bomitem(:item_id,:revision_id), item, uom "
           "WHERE ( (bomitem_item_id=item_id)"
           " AND (item_inv_uom_id=uom_id)"
           " AND (bomitem_parent_item_id=:item_id)"
           " AND (bomitem_booitem_seq_id=-1)";

    if (!_showExpired->isChecked())
      sql += " AND (bomitem_expires>CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (bomitem_effective<=CURRENT_DATE)";

    sql += " ) "
           "ORDER BY seqnumber, bomitem_seqnumber, bomitem_effective;";

    q.prepare(sql);
    q.bindValue(":always", tr("Always"));
    q.bindValue(":never", tr("Never"));
    q.bindValue(":item_id", _item->id());
    q.bindValue(":revision_id", _revision->id());
    q.exec();
    _bomitem->populate(q);
  }
}
