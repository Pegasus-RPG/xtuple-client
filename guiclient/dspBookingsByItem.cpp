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

#include "dspBookingsByItem.h"

#include <QVariant>
#include <QSqlError>
#include <QMessageBox>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

dspBookingsByItem::dspBookingsByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));

  _item->setType(ItemLineEdit::cSold);
  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _soitem->addColumn(tr("S/O #"),            _orderColumn,    Qt::AlignLeft,   true,  "cohead_number"   );
  _soitem->addColumn(tr("Ord. Date"),        _dateColumn,     Qt::AlignCenter, true,  "cohead_orderdate" );
  _soitem->addColumn(tr("Cust. #"),          _itemColumn,     Qt::AlignLeft,   true,  "cust_number"   );
  _soitem->addColumn(tr("Customer"),         -1,              Qt::AlignLeft,   true,  "cust_name"   );
  _soitem->addColumn(tr("Ordered"),          _qtyColumn,      Qt::AlignRight,  true,  "coitem_qtyord"  );
  _soitem->addColumn(tr("Unit Price"),       _priceColumn,    Qt::AlignRight,  true,  "coitem_price"  );
  _soitem->addColumn(tr("Ext. Price"),       _bigMoneyColumn, Qt::AlignRight,  true,  "extprice"  );
  _soitem->addColumn(tr("Currency"),         _currencyColumn, Qt::AlignCenter, true,  "currAbbr" );
  _soitem->addColumn(tr("Base Unit Price"),  _priceColumn,    Qt::AlignRight,  true,  "baseunitprice" );
  _soitem->addColumn(tr("Base Ext. Price"),  _bigMoneyColumn, Qt::AlignRight,  true,  "baseextprice" );
}

dspBookingsByItem::~dspBookingsByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspBookingsByItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspBookingsByItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
    _item->setItemsiteid(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

bool dspBookingsByItem::setParams(ParameterList &params)
{
  if (!_item->isValid() && isVisible())
  {
    QMessageBox::warning( this, tr("Enter Item Number"),
                          tr("Please enter a valid Item Number.") );
    _item->setFocus();
    return false;
  }

  if (!_dates->startDate().isValid() && isVisible())
  {
    QMessageBox::warning( this, tr("Enter Start Date"),
                          tr("Please enter a valid Start Date.") );
    _dates->setFocus();
    return false;
  }

  if (!_dates->endDate().isValid() && isVisible())
  {
    QMessageBox::warning( this, tr("Enter End Date"),
                          tr("Please enter a valid End Date.") );
    _dates->setFocus();
    return false;
  }

  params.append("item_id", _item->id());
  _warehouse->appendValue(params);
  _dates->appendValue(params);
  params.append("orderByOrderdate");

  return true;
}


void dspBookingsByItem::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("BookingsByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspBookingsByItem::sFillList()
{
  MetaSQLQuery mql = mqlLoad("salesOrderItems", "detail");
  ParameterList params;
  if (! setParams(params))
    return;

  q = mql.toQuery(params);
  _soitem->populate(q);
}
