/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoPriceVariancesByItem.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <parameter.h>
#include <openreports.h>

/*
 *  Constructs a dspPoPriceVariancesByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPoPriceVariancesByItem::dspPoPriceVariancesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_item, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));

  _item->setType(ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured);
  _item->setDefaultType(ItemLineEdit::cGeneralPurchased);

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  
  _porecv->addColumn(tr("P/O #"),                _orderColumn,    Qt::AlignRight,  true,  "porecv_ponumber"  );
  _porecv->addColumn(tr("Vendor"),               _orderColumn,    Qt::AlignLeft,   true,  "vend_name"   );
  _porecv->addColumn(tr("Date"),                 _dateColumn,     Qt::AlignCenter, true,  "receivedate" );
  _porecv->addColumn(tr("Vend. Item #"),         _itemColumn,     Qt::AlignLeft,   true,  "venditemnumber"   );
  _porecv->addColumn(tr("Vendor Description"),   -1,              Qt::AlignLeft,   true,  "venditemdescrip"   );
  _porecv->addColumn(tr("Qty."),                 _qtyColumn,      Qt::AlignRight,  true,  "porecv_qty"  );
  _porecv->addColumn(tr("Purchase Cost"),        _priceColumn,    Qt::AlignRight,  true,  "porecv_purchcost"  );
  _porecv->addColumn(tr("Vouchered Cost"),       _priceColumn,    Qt::AlignRight,  true,  "vouchercost"  );
  _porecv->addColumn(tr("Std. Cost at Receipt"), _priceColumn,    Qt::AlignRight,  true,  "porecv_recvcost"  );
  _porecv->addColumn(tr("Currency"),             _currencyColumn, Qt::AlignRight,  true,  "currAbbr"  );

  if (omfgThis->singleCurrency())
      _porecv->hideColumn(9);
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPoPriceVariancesByItem::~dspPoPriceVariancesByItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPoPriceVariancesByItem::languageChange()
{
  retranslateUi(this);
}

void dspPoPriceVariancesByItem::sPrint()
{
  if (!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                          tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  params.append("item_id", _item->id());

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  orReport report("PurchasePriceVariancesByItem", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoPriceVariancesByItem::sFillList()
{
  QString sql( "SELECT porecv_id, porecv_ponumber, vend_name,"
               "       DATE(porecv_date) AS receivedate,"
               "       firstLine(porecv_vend_item_number) AS venditemnumber,"
               "       firstLine(porecv_vend_item_descrip) AS venditemdescrip,"
               "       porecv_qty, porecv_purchcost, porecv_recvcost,"
               "       currToCurr(vohead_curr_id, porecv_curr_id, SUM(vodist_amount) / vodist_qty, vohead_docdate) AS vouchercost,"
               "       currConcat(vohead_curr_id) AS currAbbr,"
               "       'qty' AS porecv_qty_xtnumericrole,"
               "       'purchprice' AS porecv_purchcost_xtnumericrole,"
               "       'purchprice' AS vouchercost_xtnumericrole,"
               "       'purchprice' AS porecv_recvcost_xtnumericrole "
               "FROM vend, itemsite, porecv" 
               "     LEFT OUTER JOIN"
               "     ( vodist JOIN vohead"
               "       ON (vodist_vohead_id=vohead_id and vohead_posted)"
               "     ) ON ( (vodist_poitem_id=porecv_poitem_id) AND (vodist_vohead_id=porecv_vohead_id) ) "
               "WHERE ( (porecv_vend_id=vend_id)"
               " AND (porecv_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=:item_id)"
               " AND (DATE(porecv_date) BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_selectedPurchasingAgent->isChecked())
    sql += " AND (porecv_agent_username=:username)";

  sql += ") "
         "GROUP BY porecv_id, porecv_ponumber, vend_name, porecv_date, porecv_vend_item_number,"
         "         porecv_vend_item_descrip, porecv_qty, porecv_purchcost, porecv_recvcost,"
         "         vodist_qty, vohead_curr_id, porecv_curr_id, vohead_docdate "
         "ORDER BY porecv_date DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":item_id", _item->id());
  q.bindValue(":username", _agent->currentText());
  q.exec();
  _porecv->populate(q);
}
