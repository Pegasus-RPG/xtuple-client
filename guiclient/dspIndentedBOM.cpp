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

#include "dspIndentedBOM.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

dspIndentedBOM::dspIndentedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(valid(bool)), _revision, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cKit);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq #"),        80,           Qt::AlignCenter,true, "bomdata_bomwork_seqnumber");
  _bomitem->addColumn(tr("Item Number"),  _itemColumn,  Qt::AlignLeft,  true, "bomdata_item_number");
  _bomitem->addColumn(tr("Description"),  -1,           Qt::AlignLeft,  true, "bomdata_item_descrip1");
  _bomitem->addColumn(tr("UOM"),          _uomColumn,   Qt::AlignCenter,true, "bomdata_uom_name");
  _bomitem->addColumn(tr("Ext.Qty. Per"), _qtyColumn,   Qt::AlignRight, true, "bomdata_qtyper");
  _bomitem->addColumn(tr("Scrap %"),      _prcntColumn, Qt::AlignRight, true, "bomdata_scrap");
  _bomitem->addColumn(tr("Effective"),    _dateColumn,  Qt::AlignCenter,true, "bomdata_effective");
  _bomitem->addColumn(tr("Expires"),      _dateColumn,  Qt::AlignCenter,true, "bomdata_expires");
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
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql("SELECT *,"
                   "      'percent' AS bomdata_scrap_xtnumericrole,"
                   "       'qtyper' AS bomdata_qtyper_xtnumericrole,"
                   "       CASE WHEN COALESCE(bomdata_effective, startOfTime()) <="
                   "                 startOfTime() THEN <? value(\"always\") ?>"
                   "       END AS bomdata_effective_qtdisplayrole,"
                   "       CASE WHEN COALESCE(bomdata_expires, endOfTime()) >="
                   "                 endOfTime() THEN <? value(\"never\") ?>"
                   "       END AS bomdata_expires_qtdisplayrole,"
                   "       CASE WHEN (bomdata_expired) THEN 'expired'"
                   "            WHEN (bomdata_future) THEN 'future'"
                   "       END AS qtforegroundrole,"
                   "       bomdata_bomwork_level - 1 AS xtindentrole "
                   "FROM indentedBOM(<? value(\"item_id\") ?>, "
                   "                 <? value(\"revision_id\") ?>,"
                   "                 <? value(\"expiredDays\") ?>,"
                   "                 <? value(\"futureDays\") ?>) "
                   "WHERE (bomdata_item_id > 0);");

  q = mql.toQuery(params);
  _bomitem->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _bomitem->expandAll();
}
