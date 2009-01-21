/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspPoReturnsByVendor.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a dspPoReturnsByVendor as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspPoReturnsByVendor::dspPoReturnsByVendor(QWidget* parent, const char* name, Qt::WFlags fl)
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

  _poreject->addColumn(tr("P/O #"),              _orderColumn, Qt::AlignRight,  true,  "poreject_ponumber"  );
  _poreject->addColumn(tr("Vendor"),             _orderColumn, Qt::AlignLeft,   true,  "vend_name"   );
  _poreject->addColumn(tr("Date"),               _dateColumn,  Qt::AlignCenter, true,  "poreject_date" );
  _poreject->addColumn(tr("Vend. Item #"),       _itemColumn,  Qt::AlignLeft,   true,  "poreject_vend_item_number"   );
  _poreject->addColumn(tr("Vendor Description"), -1,           Qt::AlignLeft,   true,  "poreject_vend_item_descrip"   );
  _poreject->addColumn(tr("Qty."),               _qtyColumn,   Qt::AlignRight,  true,  "poreject_qty"  );
  _poreject->addColumn(tr("Reject Code"),        _itemColumn,  Qt::AlignRight,  true,  "rjctcode_code"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspPoReturnsByVendor::~dspPoReturnsByVendor()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspPoReturnsByVendor::languageChange()
{
  retranslateUi(this);
}

void dspPoReturnsByVendor::sPrint()
{
  if(!_dates->allValid()) {
    QMessageBox::warning( this, tr("Enter Valid Dates"),
                      tr( "Please enter a valid Start and End Date." ) );
    _dates->setFocus();
    return;
  }

  ParameterList params;
  _warehouse->appendValue(params);
  _dates->appendValue(params);

  params.append("vend_id", _vendor->id());

  if (_selectedPurchasingAgent->isChecked())
    params.append("agentUsername", _agent->currentText());

  orReport report("RejectedMaterialByVendor", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspPoReturnsByVendor::sFillList()
{
  QString sql( "SELECT poreject_id, poreject_ponumber, vend_name,"
               "       poreject_date, poreject_qty,"
               "       poreject_vend_item_number, poreject_vend_item_descrip,"
               "       rjctcode_code,"
               "       'qty' AS poreject_qty_xtnumericrole "
               "FROM poreject, vend, itemsite, rjctcode "
               "WHERE ( (poreject_posted)"
               " AND (poreject_vend_id=vend_id)"
               " AND (poreject_rjctcode_id=rjctcode_id)"
               " AND (poreject_itemsite_id=itemsite_id)"
               " AND (vend_id=:vend_id)"
               " AND (DATE(poreject_date) BETWEEN :startDate AND :endDate)" );

  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  if (_selectedPurchasingAgent->isChecked())
    sql += " AND (poreject_agent_username=:username)";

  sql += ") "
         "ORDER BY poreject_date DESC;";

  q.prepare(sql);
  _warehouse->bindValue(q);
  _dates->bindValue(q);
  q.bindValue(":vend_id", _vendor->id());
  q.bindValue(":username", _agent->currentText());
  q.exec();
  _poreject->populate(q);
}

