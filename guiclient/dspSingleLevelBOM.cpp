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

#include "dspSingleLevelBOM.h"

#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

dspSingleLevelBOM::dspSingleLevelBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(valid(bool)), _revision, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cKit);

  _bomitem->addColumn(tr("#"),                   30,      Qt::AlignCenter,true, "bomitem_seqnumber" );
  _bomitem->addColumn(tr("Item Number"),    _itemColumn,  Qt::AlignLeft,  true, "item_number"   );
  _bomitem->addColumn(tr("Description"),         -1,      Qt::AlignLeft,  true, "itemdescription"   );
  _bomitem->addColumn(tr("UOM"),            _uomColumn,   Qt::AlignCenter,true, "uom_name" );
  _bomitem->addColumn(tr("Qty. Per"),       _qtyColumn,   Qt::AlignRight, true, "qtyper"  );
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

  MetaSQLQuery mql(
               "SELECT bomitem.*, item_number, uom_name,"
               "       (item_descrip1 || ' ' || item_descrip2) AS itemdescription,"
               "       itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper) AS qtyper,"
               "       'qtyper' AS qtyper_xtnumericrole,"
               "       'percent' AS bomitem_scrap_xtnumericrole,"
               "       CASE WHEN COALESCE(bomitem_effective, startOfTime()) <= startOfTime() THEN <? value(\"always\") ?> END AS bomitem_effective_qtdisplayrole,"
               "       CASE WHEN COALESCE(bomitem_expires, endOfTime()) <= endOfTime() THEN <? value(\"never\") ?> END AS bomitem_expires_qtdisplayrole,"
               "       CASE WHEN (bomitem_expires < CURRENT_DATE) THEN 'expired'"
               "            WHEN (bomitem_effective >= CURRENT_DATE) THEN 'future'"
               "            WHEN (item_type='M') THEN 'altemphasis'"
               "       END AS qtforegroundrole "
               "FROM bomitem(<? value(\"item_id\" ?>,<? value(\"revision_id\" ?>), item, uom "
               "WHERE ( (bomitem_item_id=item_id)"
               " AND (item_inv_uom_id=uom_id)"
               "<? if exists(\"expiredDays\") ?>"
               " AND (bomitem_expires > (CURRENT_DATE - <? value(\"expiredDays\") ?>))"
               "<? else ?>"
               " AND (bomitem_expires > CURRENT_DATE)"
               "<? endif ?>"
               "<? if exists(\"effectiveDays\") ?>"
               " AND (bomitem_effective <= (CURRENT_DATE + <? value(\"effectiveDays\" ?>))"
               "<? else ?>"
               " AND (bomitem_effective <= CURRENT_DATE)"
               "<? endif ?>"
               ") "
               "ORDER BY bomitem_seqnumber, bomitem_effective;" );

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
