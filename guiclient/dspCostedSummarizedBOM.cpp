/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCostedSummarizedBOM.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

dspCostedSummarizedBOM::dspCostedSummarizedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  QButtonGroup* _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralManufactured);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Item Number"),  _itemColumn, Qt::AlignLeft,  true, "bomdata_item_number");
  _bomitem->addColumn(tr("Description"),           -1, Qt::AlignLeft,  true, "bomdata_itemdescription");
  _bomitem->addColumn(tr("UOM"),           _uomColumn, Qt::AlignCenter,true, "bomdata_uom_name");
  _bomitem->addColumn(tr("Ext. Qty. Per"),_qtyColumn, Qt::AlignRight, true, "bomdata_qtyper");
  _bomitem->addColumn(tr("Unit Cost"),    _costColumn, Qt::AlignRight, true, "unitcost");
  _bomitem->addColumn(tr("Ext. Cost"),    _priceColumn, Qt::AlignRight, true, "extendedcost");
  _bomitem->setIndentation(10);

  _expiredDaysLit->setEnabled(_showExpired->isChecked());
  _expiredDays->setEnabled(_showExpired->isChecked());
  _effectiveDaysLit->setEnabled(_showFuture->isChecked());
  _effectiveDays->setEnabled(_showFuture->isChecked());

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));

  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspCostedSummarizedBOM::~dspCostedSummarizedBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCostedSummarizedBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspCostedSummarizedBOM::set(const ParameterList &pParams)
{
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

bool dspCostedSummarizedBOM::setParams(ParameterList &params)
{
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());

  if(_showExpired->isChecked())
    params.append("expiredDays", _expiredDays->value());
  else
    params.append("expiredDays", 0);

  if(_showFuture->isChecked())
    params.append("futureDays", _effectiveDays->value());
  else
    params.append("futureDays", 0);

  if (_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if (_useActualCosts->isChecked())
    params.append("useActualCosts");

  return true;
}

void dspCostedSummarizedBOM::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("CostedSummarizedBOM", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCostedSummarizedBOM::sFillList()
{
  if (! _item->isValid())
    return;

  MetaSQLQuery mql("SELECT -1, *,"
                   "<? if exists(\"useActualCosts\") ?>"
                   "       bomdata_actunitcost AS unitcost,"
                   "       bomdata_actextendedcost AS extendedcost,"
                   "<? elseif exists(\"useStandardCosts\") ?>"
                   "       bomdata_stdunitcost AS unitcost,"
                   "       bomdata_stdextendedcost AS extendedcost,"
                   "<? endif ?>"
                   "       'qtyper' AS bomdata_qtyper_xtnumericrole,"
                   "       'cost' AS unitcost_xtnumericrole,"
                   "       'cost' AS extendedcost_xtnumericrole,"
                   "       CASE WHEN COALESCE(bomdata_effective, startOfTime()) <= startOfTime() THEN <? value(\"always\") ?> END AS bomdata_effective_qtdisplayrole,"
                   "       CASE WHEN COALESCE(bomdata_expires, endOfTime()) <= endOfTime() THEN <? value(\"never\") ?> END AS bomdata_expires_qtdisplayrole,"
                   "       CASE WHEN bomdata_expired THEN 'expired'"
                   "            WHEN bomdata_future  THEN 'future'"
                   "       END AS qtforegroundrole "
                   "FROM summarizedBOM(<? value(\"item_id\") ?>,"
                   "                   <? value(\"revision_id\") ?>,"
                   "                   <? value(\"expiredDays\") ?>,"
                   "                   <? value(\"futureDays\") ?>);");

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
  for (int i = 0; i < _bomitem->topLevelItemCount(); i++)
    _bomitem->collapseItem(_bomitem->topLevelItem(i));
}

void dspCostedSummarizedBOM::sFillList(int pid, bool)
{
  if (pid == _item->id())
    sFillList();
}
