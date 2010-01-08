/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspCostedSingleLevelBOM.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "dspItemCostSummary.h"
#include "maintainItemCosts.h"

dspCostedSingleLevelBOM::dspCostedSingleLevelBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  QButtonGroup* _costsGroupInt = new QButtonGroup(this);
  _costsGroupInt->addButton(_useStandardCosts);
  _costsGroupInt->addButton(_useActualCosts);

  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_costsGroupInt, SIGNAL(buttonClicked(int)), this, SLOT(sFillList()));
  connect(_item,          SIGNAL(newId(int)),         this, SLOT(sFillList()));
  connect(_print,         SIGNAL(clicked()),          this, SLOT(sPrint()));
  connect(_revision,      SIGNAL(newId(int)),         this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                 ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                 ItemLineEdit::cPlanning | ItemLineEdit::cJob |
				 ItemLineEdit::cTooling);

  _bomitem->addColumn(tr("#"),          _itemColumn, Qt::AlignLeft,  true, "bomdata_bomwork_seqnumber");
  _bomitem->addColumn(tr("Item Number"),_itemColumn, Qt::AlignLeft,  true, "bomdata_item_number");
  _bomitem->addColumn(tr("Description"),         -1, Qt::AlignLeft,  true, "bomdata_itemdescription");
  _bomitem->addColumn(tr("UOM"),         _uomColumn, Qt::AlignCenter,true, "bomdata_uom_name");
  _bomitem->addColumn(tr("Batch Sz."),   _qtyColumn, Qt::AlignRight, true, "bomdata_batchsize");
  _bomitem->addColumn(tr("Fxd. Qty."),   _qtyColumn, Qt::AlignRight, true, "bomdata_qtyfxd");
  _bomitem->addColumn(tr("Qty. Per"),    _qtyColumn, Qt::AlignRight, true, "bomdata_qtyper");
  _bomitem->addColumn(tr("Scrap %"),   _prcntColumn, Qt::AlignRight, true, "bomdata_scrap");
  _bomitem->addColumn(tr("Effective"),  _dateColumn, Qt::AlignCenter,true, "bomdata_effective");
  _bomitem->addColumn(tr("Expires"),    _dateColumn, Qt::AlignCenter,true, "bomdata_expires");
  _bomitem->addColumn(tr("Unit Cost"),  _costColumn, Qt::AlignRight, true, "unitcost");
  _bomitem->addColumn(tr("Ext. Cost"),  _priceColumn,Qt::AlignRight, true, "extendedcost");

  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList(int, bool)));

  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspCostedSingleLevelBOM::~dspCostedSingleLevelBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCostedSingleLevelBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspCostedSingleLevelBOM::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
    param = pParams.value("revision_id", &valid);
    if (valid)
      _revision->setId(param.toInt());
  }

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspCostedSingleLevelBOM::setParams(ParameterList &params)
{
  if (! _item->isValid())
  {
    QMessageBox::critical(this, tr("Need Item Number"),
                          tr("You must select an Item to see its BOM."));
    _item->setFocus();
    return false;
  }

  params.append("item_id",     _item->id());
  params.append("revision_id", _revision->id());

  if(_useStandardCosts->isChecked())
    params.append("useStandardCosts");

  if(_useActualCosts->isChecked())
    params.append("useActualCosts");

  params.append("always", tr("Always"));
  params.append("never",  tr("Never"));

  return true;
}

void dspCostedSingleLevelBOM::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("CostedSingleLevelBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCostedSingleLevelBOM::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected)
{
  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("Maintain Item Costs..."), this, SLOT(sMaintainItemCosts()), 0);

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    pMenu->insertItem(tr("View Item Costing..."), this, SLOT(sViewItemCosting()), 0);
}

void dspCostedSingleLevelBOM::sMaintainItemCosts()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());

  maintainItemCosts *newdlg = new maintainItemCosts();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedSingleLevelBOM::sViewItemCosting()
{
  ParameterList params;
  params.append( "item_id", _bomitem->altId() );
  params.append( "run",     TRUE              );

  dspItemCostSummary *newdlg = new dspItemCostSummary();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspCostedSingleLevelBOM::sFillList(int pItemid, bool)
{
  if (pItemid != _item->id())
    return;
  sFillList();
}

void dspCostedSingleLevelBOM::sFillList()
{
  MetaSQLQuery mql("SELECT bomdata_bomitem_id AS id,"
                   "       CASE WHEN (bomdata_bomitem_id = -1) THEN -1 "
                   "            ELSE (bomdata_item_id) END AS altid, "
                   "       *, "
                   "<? if exists(\"useStandardCosts\") ?>"
                   "       bomdata_stdunitcost AS unitcost,"
                   "       bomdata_stdextendedcost AS extendedcost,"
                   "<? elseif exists(\"useActualCosts\") ?>"
                   "       bomdata_actunitcost AS unitcost,"
                   "       bomdata_actextendedcost AS extendedcost,"
                   "<? endif ?>"
                   "       'qty' AS bomdata_batchsize_xtnumericrole,"
                   "       'qty' AS bomdata_qtyfxd_xtnumericrole,"
                   "       'qtyper' AS bomdata_qtyper_xtnumericrole,"
                   "       'percent' AS bomdata_scrap_xtnumericrole,"
                   "       'cost' AS unitcost_xtnumericrole,"
                   "       'cost' AS extendedcost_xtnumericrole,"
                   "       CASE WHEN COALESCE(bomdata_effective, startOfTime()) <= startOfTime() THEN <? value(\"always\") ?> END AS bomdata_effective_qtdisplayrole,"
                   "       CASE WHEN COALESCE(bomdata_expires, endOfTime()) <= endOfTime() THEN <? value(\"never\") ?> END AS bomdata_expires_qtdisplayrole,"
                   "       CASE WHEN bomdata_expired THEN 'expired'"
                   "            WHEN bomdata_future  THEN 'future'"
                   "       END AS qtforegroundrole,"
                   "       0 AS extendedcost_xttotalrole "
                   "FROM singlelevelbom(<? value(\"item_id\") ?>,"
                   "                    <? value(\"revision_id\") ?>,0,0);");

  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _bomitem->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }


  q.prepare( "SELECT formatCost(actcost(:item_id)) AS actual,"
             "       formatCost(stdcost(:item_id)) AS standard;" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    XTreeWidgetItem *last = new XTreeWidgetItem(_bomitem, -1, -1, tr("Actual Cost"), "");
    last->setText(11, q.value("actual").toString());

    last = new XTreeWidgetItem(_bomitem, last, -1, -1, tr("Standard Cost"), "" );
    last->setText(11, q.value("standard").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
