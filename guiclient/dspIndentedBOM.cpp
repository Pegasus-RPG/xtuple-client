/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspIndentedBOM.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include <openreports.h>
#include "item.h"

dspIndentedBOM::dspIndentedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(valid(bool)), _revision, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                 ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                 ItemLineEdit::cPlanning | ItemLineEdit::cTooling);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq #"),        80,           Qt::AlignCenter,true, "bomdata_bomwork_seqnumber");
  _bomitem->addColumn(tr("Item Number"),  _itemColumn,  Qt::AlignLeft,  true, "bomdata_item_number");
  _bomitem->addColumn(tr("Description"),  -1,           Qt::AlignLeft,  true, "bomdata_itemdescription");
  _bomitem->addColumn(tr("UOM"),          _uomColumn,   Qt::AlignCenter,true, "bomdata_uom_name");
  _bomitem->addColumn(tr("Qty. Req."),    _qtyColumn,   Qt::AlignRight, true, "bomdata_qtyreq");
  _bomitem->addColumn(tr("Scrap %"),      _prcntColumn, Qt::AlignRight, true, "bomdata_scrap");
  _bomitem->addColumn(tr("Effective"),    _dateColumn,  Qt::AlignCenter,true, "bomdata_effective");
  _bomitem->addColumn(tr("Expires"),      _dateColumn,  Qt::AlignCenter,true, "bomdata_expires");
  _bomitem->addColumn(tr("Notes"),        _itemColumn,  Qt::AlignCenter,false, "bomdata_notes");
  _bomitem->addColumn(tr("Reference"),    _itemColumn,  Qt::AlignCenter,false, "bomdata_ref");
  _bomitem->setIndentation(10);

  _expiredDaysLit->setEnabled(_showExpired->isChecked());
  _expiredDays->setEnabled(_showExpired->isChecked());
  _effectiveDaysLit->setEnabled(_showFuture->isChecked());
  _effectiveDays->setEnabled(_showFuture->isChecked());

  _item->setFocus();
  _revision->setEnabled(false);
  _revision->setMode(RevisionLineEdit::View);
  _revision->setType("BOM");

  //If not Revision Control, hide control
  _revision->setVisible(_metrics->boolean("RevControl"));
}

dspIndentedBOM::~dspIndentedBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspIndentedBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspIndentedBOM::set(const ParameterList &pParams)
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
    return NoError_Run;
  }

  return NoError;
}

bool dspIndentedBOM::setParams(ParameterList &params)
{
  if(!_item->isValid())
  {
    QMessageBox::warning(this, tr("Invalid Item"),
      tr("You must specify a valid item.") );
    return false;
  }

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

  params.append("always", tr("Always"));
  params.append("never",  tr("Never"));

  params.append("byIndented");

  return true;
}

void dspIndentedBOM::sPrint()
{

  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("IndentedBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspIndentedBOM::sFillList()
{
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
  _bomitem->expandAll();
}

void dspIndentedBOM::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;
  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemMasters"))
  pMenu->setItemEnabled(menuItem, FALSE);
  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
}

void dspIndentedBOM::sEdit()
{
  item::editItem(_bomitem->id());
}

void dspIndentedBOM::sView()
{
  item::viewItem(_bomitem->id());
}
