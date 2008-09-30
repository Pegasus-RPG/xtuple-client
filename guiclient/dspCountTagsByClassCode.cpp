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

#include "dspCountTagsByClassCode.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "countTag.h"
#include "mqlutil.h"

dspCountTagsByClassCode::dspCountTagsByClassCode(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_cnttag, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _classCode->setType(ParameterGroup::ClassCode);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _cnttag->addColumn(tr("Tag #"),               -1, Qt::AlignLeft,  true, "invcnt_tagnumber");
  _cnttag->addColumn(tr("Site"),        _whsColumn, Qt::AlignCenter,true, "warehous_code");
  _cnttag->addColumn(tr("Item"),       _itemColumn, Qt::AlignLeft,  true, "item_number");
  _cnttag->addColumn(tr("Created"),    _dateColumn, Qt::AlignCenter,true, "invcnt_tagdate");
  _cnttag->addColumn(tr("Created By"), _dateColumn, Qt::AlignCenter,true, "creator");
  _cnttag->addColumn(tr("Entered"),    _dateColumn, Qt::AlignCenter,true, "invcnt_cntdate");
  _cnttag->addColumn(tr("Entered By"), _dateColumn, Qt::AlignCenter,true, "counter");
  _cnttag->addColumn(tr("Posted"),     _dateColumn, Qt::AlignCenter,true, "invcnt_postdate");
  _cnttag->addColumn(tr("Posted By"),  _dateColumn, Qt::AlignCenter,true, "poster");
  _cnttag->addColumn(tr("QOH Before"),  _qtyColumn, Qt::AlignRight, true, "qohbefore");
  _cnttag->addColumn(tr("Qty. Counted"),_qtyColumn, Qt::AlignRight, true, "invcnt_qoh_after");
  _cnttag->addColumn(tr("Variance"),    _qtyColumn, Qt::AlignRight, true, "variance");
  _cnttag->addColumn(tr("%"),         _prcntColumn, Qt::AlignRight, true, "percent");

  if (_preferences->boolean("XCheckBox/forgetful"))
    _showUnposted->setChecked(true);
  
  sFillList();
}

dspCountTagsByClassCode::~dspCountTagsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspCountTagsByClassCode::languageChange()
{
  retranslateUi(this);
}

bool dspCountTagsByClassCode::setParams(ParameterList &params)
{
  if (! _dates->allValid())
  {
    _dates->setFocus();
    return false;
  }

  _classCode->appendValue(params);
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_showUnposted->isChecked())
    params.append("showUnposted");

  return true;
}

void dspCountTagsByClassCode::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;
  orReport report("CountTagsByClassCode", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspCountTagsByClassCode::sPopulateMenu(QMenu *pMenu)
{
  pMenu->insertItem(tr("View Count Tag..."), this, SLOT(sView()), 0);
}

void dspCountTagsByClassCode::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("cnttag_id", _cnttag->id());

  countTag newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspCountTagsByClassCode::sFillList()
{
  MetaSQLQuery mql = mqlLoad(":/im/countTags.mql");
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _cnttag->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
