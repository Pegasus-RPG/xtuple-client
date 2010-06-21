/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspIndentedWhereUsed.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "mqlutil.h"
#include "dspInventoryHistoryByItem.h"

dspIndentedWhereUsed::dspIndentedWhereUsed(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  if (_metrics->boolean("AllowInactiveBomItems"))
    _item->setType(ItemLineEdit::cGeneralComponents);
  else
    _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq. #"),               80, Qt::AlignRight, true, "bomwork_seqnumber");
  _bomitem->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  _bomitem->addColumn(tr("Description"),          -1, Qt::AlignLeft,  true, "descrip");
  _bomitem->addColumn(tr("UOM"),          _uomColumn, Qt::AlignCenter,true, "uom_name");
  _bomitem->addColumn(tr("Fxd. Qty."),    _qtyColumn, Qt::AlignRight, true, "bomwork_qtyfxd");
  _bomitem->addColumn(tr("Qty. Per"),     _qtyColumn, Qt::AlignRight, true, "bomwork_qtyper");
  _bomitem->addColumn(tr("Scrap %"),    _prcntColumn, Qt::AlignRight, true, "bomwork_scrap");
  _bomitem->addColumn(tr("Effective"),   _dateColumn, Qt::AlignCenter,true, "bomwork_effective");
  _bomitem->addColumn(tr("Expires"),     _dateColumn, Qt::AlignCenter,true, "bomwork_expires");

  _item->setFocus();
}

dspIndentedWhereUsed::~dspIndentedWhereUsed()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspIndentedWhereUsed::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspIndentedWhereUsed::set(const ParameterList &pParams)
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
    return NoError_Print;
  }

  return NoError;
}

bool dspIndentedWhereUsed::setParams(ParameterList &params)
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Item Number"),
                          tr("You must enter a valid Item Number.") );
    _item->setFocus();
    return false;
  }

  params.append("item_id", _item->id());

  if(_showExpired->isChecked())
    params.append("showExpired");

  if(_showFuture->isChecked())
    params.append("showFuture");

  params.append("byIndented");

  return true;
}

void dspIndentedWhereUsed::sPrint()
{
  q.prepare("SELECT indentedWhereUsed(:item_id) AS result;");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    if (! setParams(params))
      return;

    int worksetid = q.value("result").toInt();
    params.append("bomworkset_id", worksetid);

    orReport report("IndentedWhereUsed", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);

    q.prepare("SELECT deleteBOMWorkset(:bomworkset_id) AS result;");
    q.bindValue(":bomworkset_id", worksetid);
    q.exec();
  }
  else
      QMessageBox::critical( this, tr("Error Executing Report"),
                             tr( "Was unable to create/collect the required information to create this report." ) );
}

void dspIndentedWhereUsed::sViewInventoryHistory()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());
  params.append("run");

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspIndentedWhereUsed::sPopulateMenu(QMenu *menu)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Item Inventory History..."), this, SLOT(sViewInventoryHistory()), 0);
  if (!_privileges->check("ViewInventoryHistory"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspIndentedWhereUsed::sFillList()
{
  q.prepare("SELECT indentedWhereUsed(:item_id) AS workset_id;");
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    MetaSQLQuery mql = mqlLoad("whereUsed", "detail");
    ParameterList params;
    if (! setParams(params))
      return;

    int worksetid = q.value("workset_id").toInt();
    params.append("bomwork_set_id", worksetid);

    q = mql.toQuery(params);
    _bomitem->populate(q, true);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _bomitem->expandAll();

    q.prepare("SELECT deleteBOMWorkset(:bomwork_set_id) AS result;");
    q.bindValue(":bomwork_set_id", worksetid);
    q.exec();
  }
}
