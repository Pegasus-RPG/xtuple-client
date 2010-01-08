/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoDeliveryDateVariancesByVendor.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspPoDeliveryDateVariancesByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPoDeliveryDateVariancesByVendor::dspPoDeliveryDateVariancesByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_selectedPurchasingAgent, SIGNAL(toggled(bool)), _agent, SLOT(setEnabled(bool)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_vendor, SIGNAL(valid(bool)), _query, SLOT(setEnabled(bool)));

  _agent->setType(XComboBox::Agent);
  _agent->setText(omfgThis->username());
  
  _porecv->addColumn(tr("P/O #"),              _orderColumn, Qt::AlignRight,  true,  "porecv_ponumber"  );
  _porecv->addColumn(tr("Vendor"),             _orderColumn, Qt::AlignLeft,   true,  "vend_name"   );
  _porecv->addColumn(tr("Date"),               _dateColumn,  Qt::AlignCenter, true,  "receivedate" );
  _porecv->addColumn(tr("Vend. Item #"),       _itemColumn,  Qt::AlignLeft,   true,  "venditemnumber"   );
  _porecv->addColumn(tr("Vendor Description"), -1,           Qt::AlignLeft,   true,  "venditemdescrip"   );
  _porecv->addColumn(tr("Qty."),               _qtyColumn,   Qt::AlignRight,  true,  "porecv_qty"  );
  _porecv->addColumn(tr("Due Date"),           _dateColumn,  Qt::AlignRight,  true,  "porecv_duedate"  );
  _porecv->addColumn(tr("Recv. Date"),         _dateColumn,  Qt::AlignRight,  true,  "porecv_date"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPoDeliveryDateVariancesByVendor::~dspPoDeliveryDateVariancesByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPoDeliveryDateVariancesByVendor::languageChange()
{
  retranslateUi(this);
}

void dspPoDeliveryDateVariancesByVendor::sPrint()
{
  ParameterList params;
  params.append("vend_id", _vendor->id());

  _warehouse->appendValue(params);
  _dates->appendValue(params);

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  orReport report("DeliveryDateVariancesByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoDeliveryDateVariancesByVendor::sFillList()
{
  QString sql( "SELECT porecv_id, porecv_ponumber, vend_name,"
               "       DATE(porecv_date) AS receivedate,"
               "       firstLine(porecv_vend_item_number) AS venditemnumber,"
               "       firstLine(porecv_vend_item_descrip) AS venditemdescrip,"
               "       porecv_qty, porecv_duedate, porecv_date,"
               "       'qty' AS porecv_qty_xtnumericrole "
               "FROM porecv, vend "
               "WHERE ( (porecv_vend_id=vend_id)"
               " AND (vend_id=:vend_id)"
               " AND (DATE(porecv_date) BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (porecv_itemsite_id in (SELECT itemsite_id FROM itemsite WHERE (itemsite_warehous_id=:warehous_id)))";

  if (_selectedPurchasingAgent->isChecked())
    sql += " AND (porecv_agent_username=:username)";

  sql += ") "
         "ORDER BY porecv_date DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":vend_id", _vendor->id());
  q.bindValue(":username", _agent->currentText());
  q.exec();
  _porecv->populate(q);
}
