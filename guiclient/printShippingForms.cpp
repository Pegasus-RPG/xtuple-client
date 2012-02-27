/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printShippingForms.h"

#include <QMessageBox>
#include <QVariant>

#include <openreports.h>

printShippingForms::printShippingForms(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _captive = FALSE;

  if (_preferences->boolean("XCheckBox/forgetful"))
  {
    _printNew->setChecked(true);
    _printDirty->setChecked(true);
  }
}

printShippingForms::~printShippingForms()
{
    // no need to delete child widgets, Qt does it all for us
}

void printShippingForms::languageChange()
{
    retranslateUi(this);
}

enum SetResponse printShippingForms::set(const ParameterList & pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;
  return NoError;
}

void printShippingForms::sPrint()
{
  if (!_printNew->isChecked() && !_printDirty->isChecked())
  {
    QMessageBox::warning( this, tr("Cannot Print Shipping Forms"),
                          tr("You must indicate if you wish to print Shipping Forms for New and/or Changed Shipments.") );
    return;
  }

  QString sql( "SELECT shiphead_id, shipform_report_name AS report_name "
               "FROM shiphead, cohead, shipform "
               "WHERE ( (NOT shiphead_shipped)"
               " AND (shiphead_order_id=cohead_id)"
	       " AND (shiphead_order_type='SO')"
               " AND (shipform_id=COALESCE(shiphead_shipform_id, cohead_shipform_id))"
               " AND (shiphead_sfstatus IN (" );

  if (_printNew->isChecked())
  {
    sql += "'N'";

    if (_printDirty->isChecked())
      sql += ", 'D'";
  }
  else if (_printDirty->isChecked())
    sql += "'D'";
  sql += ")) ) "
         "ORDER BY shiphead_id;";

  XSqlQuery reports;
  reports.exec(sql);
  if (reports.first())
  {
    QPrinter printer(QPrinter::HighResolution);

    bool     setupPrinter = TRUE;
    bool userCanceled = false;
    if (orReport::beginMultiPrint(&printer, userCanceled) == false)
    {
      if(!userCanceled)
        systemError(this, tr("Could not initialize printing system for multiple reports."));
      return;
    }

    do
    {
      for (int i = 0; i < _shippingformCopies->numCopies(); i++ )
      {
        ParameterList params;

        params.append("shiphead_id", reports.value("shiphead_id").toInt());
        params.append("watermark",   _shippingformCopies->watermark(i));

#if 0
        params.append("shipchrg_id", _shipchrg->id());

#endif

        if (_shippingformCopies->showCosts(i))
          params.append("showcosts");

        orReport report(reports.value("report_name").toString(), params);
        if (report.print(&printer, setupPrinter))
          setupPrinter = FALSE;
        else
        {
          report.reportError(this);
	  orReport::endMultiPrint(&printer);
          return;
        }
      }

      XSqlQuery setStatus;
      setStatus.prepare( "UPDATE shiphead "
                         "SET shiphead_sfstatus='P' "
                         "WHERE (shiphead_id=:shiphead_id);" );
      setStatus.bindValue(":shiphead_id", reports.value("shiphead_id").toInt());
      setStatus.exec();
    }
    while (reports.next());
    orReport::endMultiPrint(&printer);

    if (_captive)
      accept();
  }
  else
    QMessageBox::warning( this, tr("Cannot Print Shipping Forms"),
                          tr("There are no New or Changed Shipments for which Shipping Forms should be printed.") );

}
