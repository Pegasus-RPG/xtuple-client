/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedBOM.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>
#include "item.h"

#include "storedProcErrorLookup.h"

dspSummarizedBOM::dspSummarizedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(valid(bool)), _revision, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                 ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                 ItemLineEdit::cPlanning | ItemLineEdit::cJob);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "bomdata_item_number");
  _bomitem->addColumn(tr("Description"),          -1, Qt::AlignLeft,  true, "bomdata_itemdescription");
  _bomitem->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "bomdata_uom_name");
  _bomitem->addColumn(tr("Ext. Qty. Per"),_qtyColumn, Qt::AlignRight, true, "bomdata_qtyper");
  _bomitem->setIndentation(10);

  _expiredDaysLit->setEnabled(_showExpired->isChecked());
  _expiredDays->setEnabled(_showExpired->isChecked());
  _effectiveDaysLit->setEnabled(_showFuture->isChecked());
  _effectiveDays->setEnabled(_showFuture->isChecked());
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList()));
  _revision->setEnabled(false);
  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  //If not Revision Control, hide control
  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspSummarizedBOM::~dspSummarizedBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspSummarizedBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSummarizedBOM::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    param = pParams.value("revision_id", &valid);
    if (valid)
      _revision->setId(param.toInt());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Print;
  }

  return NoError;
}

bool dspSummarizedBOM::setParams(ParameterList &params)
{
  if (_item->isValid())
  {
    params.append("item_id", _item->id());
    params.append("revision_id", _revision->id());
  }
  else
    return false;

  if (_showExpired->isChecked())
    params.append("expiredDays", _expiredDays->value());
  else
    params.append("expiredDays", 0);

  if (_showFuture->isChecked())
    params.append("futureDays", _effectiveDays->value());
  else
    params.append("futureDays", 0);

  return true;
}

void dspSummarizedBOM::sPrint()
{
  ParameterList params;
  if (!setParams(params))
    return;

  orReport report("SummarizedBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);

  QString dels("SELECT deleteBOMWorkset(<? value(\"workset_id\") ?>) AS result;");
  MetaSQLQuery delm(dels);
  q = delm.toQuery(params);
  // ignore errors since this is just cleanup of temp records
}

void dspSummarizedBOM::sFillList()
{
  ParameterList params;
  if (!setParams(params))
    return;

  MetaSQLQuery mql(" SELECT item_id AS itemid, *,"
                   "       'qtyper' AS bomdata_qtyper_xtnumericrole,"
                   "       CASE WHEN bomdata_expired THEN 'expired'"
                   "            WHEN bomdata_future  THEN 'future'"
                   "        END AS qtforegroundrole "
                   "FROM summarizedBOM(<? value(\"item_id\") ?>,"
                   "                     <? value(\"revision_id\") ?>,"
                   "                     <? value(\"expiredDays\") ?>,"
                   "                     <? value(\"futureDays\") ?>)"
                   "     JOIN item ON (bomdata_item_number=item_number);" );
  q = mql.toQuery(params);
  _bomitem->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspSummarizedBOM::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
  pMenu->setItemEnabled(menuItem, FALSE);
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspSummarizedBOM::sEdit()
{
  item::editItem(_bomitem->id());
}

void dspSummarizedBOM::sView()
{
  item::viewItem(_bomitem->id());
}
