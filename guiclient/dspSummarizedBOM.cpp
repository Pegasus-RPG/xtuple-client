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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "dspSummarizedBOM.h"

#include <QVariant>
#include <QStatusBar>
#include <parameter.h>
#include "rptSummarizedBOM.h"

/*
 *  Constructs a dspSummarizedBOM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedBOM::dspSummarizedBOM(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_showExpired, SIGNAL(toggled(bool)), _expiredDaysLit, SLOT(setEnabled(bool)));
  connect(_showExpired, SIGNAL(toggled(bool)), _expiredDays, SLOT(setEnabled(bool)));
  connect(_showFuture, SIGNAL(toggled(bool)), _effectiveDaysLit, SLOT(setEnabled(bool)));
  connect(_showFuture, SIGNAL(toggled(bool)), _effectiveDays, SLOT(setEnabled(bool)));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  statusBar()->hide();

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Ext. Qty. Per"), _qtyColumn,   Qt::AlignRight  );
  _bomitem->setIndentation(10);
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), this, SLOT(sFillList()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedBOM::~dspSummarizedBOM()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedBOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspSummarizedBOM::set(const ParameterList &pParams)
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

void dspSummarizedBOM::sPrint()
{
  ParameterList params;
  params.append( "item_id", _item->id() );
  params.append( "print",   TRUE        );

  if (_showExpired->isChecked())
    params.append("expiredDays", _expiredDays->value());

  if (_showFuture->isChecked())
    params.append("futureDays", _effectiveDays->value());

  rptSummarizedBOM newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspSummarizedBOM::sFillList()
{
  _bomitem->clear();

  if (_item->isValid())
  {
    q.prepare("SELECT summarizedBOM(:item_id, :expired, :effective) AS workset_id;");
    q.bindValue(":item_id", _item->id());

    if (_showExpired->isChecked())
      q.bindValue(":expired", _expiredDays->value());
    else
      q.bindValue(":expired", 0);

    if (_showFuture->isChecked())
      q.bindValue(":effective", _effectiveDays->value());
    else
      q.bindValue(":effective", 0);

    q.exec();
    if (q.first())
    {
      int worksetid = q.value("workset_id").toInt();

      QString sql( "SELECT -1, item_number,"
                   "       (item_descrip1 || ' ' || item_descrip2) AS itemdescription,"
                   "       item_invuom,"
                   "       formatQtyPer(SUM(bomwork_qtyper * (1 + bomwork_scrap))) AS f_qtyper,"
                   "       CASE WHEN(bomwork_expires <= CURRENT_DATE) THEN TRUE"
                   "            ELSE FALSE"
                   "       END AS expired,"
                   "       CASE WHEN(bomwork_effective > CURRENT_DATE) THEN TRUE"
                   "            ELSE FALSE"
                   "       END AS future "
                   "FROM bomwork, item "
                   "WHERE ( (bomwork_item_id=item_id)"
                   " AND (bomwork_set_id=:bomwork_set_id) ) "
                   "GROUP BY item_number, item_invuom,"
                   "         item_descrip1, item_descrip2,"
                   "         bomwork_expires, bomwork_effective "
                   "ORDER BY item_number;" );

      q.prepare(sql);
      q.bindValue(":bomwork_set_id", worksetid);
      q.exec();
      XTreeWidgetItem *last = 0;
      while (q.next())
      {
        last = new XTreeWidgetItem(_bomitem, last, -1, q.value("item_number"),
                                   q.value("itemdescription"), q.value("item_invuom"),
                                   q.value("f_qtyper") );
 
        if (q.value("expired").toBool())
          last->setTextColor("red");
        else if (q.value("future").toBool())
          last->setTextColor("blue");
      }
      //_bomitem->closeAll();

      q.prepare("SELECT deleteBOMWorkset(:bomwork_set_id) AS result;");
      q.bindValue(":bomwork_set_id", worksetid);
      q.exec();
    }
  }
}

