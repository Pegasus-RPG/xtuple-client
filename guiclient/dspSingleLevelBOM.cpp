/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSingleLevelBOM.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include <openreports.h>
#include "item.h"

dspSingleLevelBOM::dspSingleLevelBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(valid(bool)), _revision, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                 ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                 ItemLineEdit::cPlanning | ItemLineEdit::cTooling);

  _bomitem->addColumn(tr("#"),                   30,      Qt::AlignCenter,true, "bomitem_seqnumber" );
  _bomitem->addColumn(tr("Item Number"),    _itemColumn,  Qt::AlignLeft,  true, "item_number"   );
  _bomitem->addColumn(tr("Description"),         -1,      Qt::AlignLeft,  true, "itemdescription"   );
  _bomitem->addColumn(tr("Issue UOM"),       _uomColumn,   Qt::AlignCenter,true, "issueuomname" );
  _bomitem->addColumn(tr("Issue Fxd. Qty."),  _qtyColumn,  Qt::AlignRight, true, "bomitem_qtyfxd"  );
  _bomitem->addColumn(tr("Issue Qty. Per"),  _qtyColumn,   Qt::AlignRight, true, "bomitem_qtyper"  );
  _bomitem->addColumn(tr("Inv. UOM"),        _uomColumn,   Qt::AlignCenter,true, "invuomname" );
  _bomitem->addColumn(tr("Inv. Fxd. Qty."),   _qtyColumn,  Qt::AlignRight, true, "invqtyfxd"  );
  _bomitem->addColumn(tr("Inv. Qty. Per"),   _qtyColumn,   Qt::AlignRight, true, "invqtyper"  );
  _bomitem->addColumn(tr("Scrap %"),        _prcntColumn, Qt::AlignRight, true, "bomitem_scrap"  );
  _bomitem->addColumn(tr("Effective"),      _dateColumn,  Qt::AlignCenter,true, "bomitem_effective" );
  _bomitem->addColumn(tr("Expires"),        _dateColumn,  Qt::AlignCenter,true, "bomitem_expires" );
  _bomitem->addColumn(tr("ECN #"),          _itemColumn,  Qt::AlignLeft,  true, "bomitem_ecn"   );
  _bomitem->addColumn(tr("Notes"),          _itemColumn,  Qt::AlignLeft,  false, "bomitem_notes"   );
  _bomitem->addColumn(tr("Reference"),      _itemColumn,  Qt::AlignLeft,  false, "bomitem_ref"   );

  _expiredDaysLit->setEnabled(_showExpired->isChecked());
  _expiredDays->setEnabled(_showExpired->isChecked());
  _effectiveDaysLit->setEnabled(_showFuture->isChecked());
  _effectiveDays->setEnabled(_showFuture->isChecked());

  _item->setFocus();
  _revision->setEnabled(false);
  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspSingleLevelBOM::~dspSingleLevelBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSingleLevelBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSingleLevelBOM::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
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

bool dspSingleLevelBOM::setParams(ParameterList &params)
{
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());

  if (_showExpired->isChecked())
    params.append("expiredDays", _expiredDays->value());

  if (_showFuture->isChecked())
    params.append("effectiveDays", _effectiveDays->value());

  params.append("always", tr("Always"));
  params.append("never",  tr("Never"));

  params.append("bySingleLvl");
  return true;
}

void dspSingleLevelBOM::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  orReport report("SingleLevelBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSingleLevelBOM::sFillList()
{
  if (_item->isValid())
    sFillList(-1, FALSE);
}

void dspSingleLevelBOM::sFillList(int, bool)
{
  if (! _item->isValid())
    return;

  MetaSQLQuery mql = mqlLoad("bom", "detail");
  ParameterList params;
  if (! setParams(params))
    return;

  q = mql.toQuery(params);
  _bomitem->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspSingleLevelBOM::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

}

void dspSingleLevelBOM::sEdit()
{
  item::editItem(_bomitem->id());
}

void dspSingleLevelBOM::sView()
{
  item::viewItem(_bomitem->id());
}
