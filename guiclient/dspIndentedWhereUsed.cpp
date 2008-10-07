/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "dspIndentedWhereUsed.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

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
  _bomitem->addColumn(tr("Ext. Qty. Per"),_qtyColumn, Qt::AlignRight, true, "bomwork_qtyper");
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
    ParameterList params;
    if (! setParams(params))
      return;

    int worksetid = q.value("workset_id").toInt();
    params.append("bomwork_set_id", worksetid);

    MetaSQLQuery mql("SELECT bomwork_id, item_id, bomwork_parent_id,"
                     "       bomworkitemsequence(bomwork_id) AS seqord, "
                     "       bomwork_seqnumber, item_number, uom_name,"
                     "       (item_descrip1 || ' ' || item_descrip2) AS descrip,"
                     "       bomwork_qtyper,"
                     "       bomwork_scrap,"
                     "       bomwork_effective,"
                     "       bomwork_expires,"
                     "       'qtyper' AS bomwork_qtyper_xtnumericrole,"
                     "       'scrap' AS bomwork_scrap_xtnumericrole,"
                     "       CASE WHEN COALESCE(bomwork_effective,startOfTime())=startOfTime() THEN 'Always' END AS bomwork_effective_qtdisplayrole,"
                     "       CASE WHEN COALESCE(bomwork_expires,endOfTime())=endOfTime() THEN 'Never' END AS bomwork_expires_qtdisplayrole,"
                     "       bomwork_level - 1 AS xtindentrole "
                     "FROM bomwork, item, uom "
                     "WHERE ( (bomwork_item_id=item_id)"
                     " AND (item_inv_uom_id=uom_id)"
                     " AND (bomwork_set_id=<? value(\"bomwork_set_id\") ?>)"
                     "<? if exists(\"showExpired\") ?>"
                     " AND (bomwork_expires > CURRENT_DATE)"
                     "<? endif ?>"
                     "<? if exists(\"showFuture\") ?>"
                     " AND (bomwork_effective <= CURRENT_DATE)"
                     "<? endif ?>"
                     ") "
                     "ORDER BY seqord;");
    q = mql.toQuery(params);
    _bomitem->populate(q, true);
    if (q.lastError().type() != QSqlError::None)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    _bomitem->expandAll();

    q.prepare("SELECT deleteBOMWorkset(:workset_id) AS result;");
    q.bindValue(":bomwork_set_id", worksetid);
    q.exec();
  }
}
