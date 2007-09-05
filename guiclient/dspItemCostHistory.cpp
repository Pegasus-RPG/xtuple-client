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

#include "dspItemCostHistory.h"

#include <QVariant>
#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspItemCostHistory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspItemCostHistory::dspItemCostHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
  setupUi(this);

  (void)statusBar();

  // signals and slots connections
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_item, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _itemcost->addColumn(tr("Element"), -1,                 Qt::AlignLeft   );
  _itemcost->addColumn(tr("Lower"),   _costColumn,        Qt::AlignCenter );
  _itemcost->addColumn(tr("Type"),    _costColumn,        Qt::AlignCenter );
  _itemcost->addColumn(tr("Time"),    (_dateColumn + 30), Qt::AlignCenter );
  _itemcost->addColumn(tr("User"),    _qtyColumn,         Qt::AlignCenter );
  _itemcost->addColumn(tr("Old"),     _costColumn,        Qt::AlignRight  );
  _itemcost->addColumn(tr("Currency"), _currencyColumn,   Qt::AlignLeft   );
  _itemcost->addColumn(tr("New"),     _costColumn,        Qt::AlignRight  );
  _itemcost->addColumn(tr("Currency"), _currencyColumn,   Qt::AlignLeft   );

  if (omfgThis->singleCurrency())
  {
      _itemcost->hideColumn(6);
      _itemcost->hideColumn(8);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspItemCostHistory::~dspItemCostHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspItemCostHistory::languageChange()
{
  retranslateUi(this);
}

void dspItemCostHistory::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());

  orReport report("ItemCostHistory", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspItemCostHistory::sFillList()
{
  _itemcost->clear();

  if (_item->isValid())
  {
    q.prepare( "SELECT costhist_id, costelem_type, formatBoolYN(costhist_lowlevel),"
               "       CASE WHEN costhist_type='A' THEN :actual"
               "            WHEN costhist_type='S' THEN :standard"
               "            WHEN costhist_type='D' THEN :delete"
               "            WHEN costhist_type='N' THEN :new"
               "       END,"
               "       formatDateTime(costhist_date), getUserName(costhist_user_id),"
               "       formatCost(costhist_oldcost), currConcat(costhist_oldcurr_id), "
	       "       formatCost(costhist_newcost), currConcat(costhist_newcurr_id) "
               "FROM costhist, costelem "
               "WHERE ( (costhist_costelem_id=costelem_id)"
               " AND (costhist_item_id=:item_id) ) "
               "ORDER BY costhist_date, costelem_type;" );
    q.bindValue(":actual", tr("Actual"));
    q.bindValue(":standard", tr("Standard"));
    q.bindValue(":delete", tr("Delete"));
    q.bindValue(":new", tr("New"));
    q.bindValue(":item_id", _item->id());
    q.exec();
    _itemcost->populate(q);
  }
}
