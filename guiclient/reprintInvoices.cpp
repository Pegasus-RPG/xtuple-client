/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "reprintInvoices.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include "mqlutil.h"
#include <openreports.h>

#include "submitAction.h"

reprintInvoices::reprintInvoices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_query, SIGNAL(clicked()), this, SLOT(sQuery()));

  _invoice->addColumn( tr("Invoice #"),    _orderColumn,    Qt::AlignRight, true, "invchead_invcnumber");
  _invoice->addColumn( tr("Doc. Date"),    _dateColumn,     Qt::AlignCenter,true, "invchead_invcdate");
  _invoice->addColumn( tr("Customer"),     -1,              Qt::AlignLeft,  true, "customer");
  _invoice->addColumn( tr("Total Amount"), _bigMoneyColumn, Qt::AlignRight, true, "extprice" );
  _invoice->addColumn( tr("Balance"),      _bigMoneyColumn, Qt::AlignRight, true, "balance" );
  _invoice->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

reprintInvoices::~reprintInvoices()
{
  // no need to delete child widgets, Qt does it all for us
}

void reprintInvoices::languageChange()
{
  retranslateUi(this);
}

void reprintInvoices::sQuery()
{
  MetaSQLQuery mql = mqlLoad("invoices", "detail");
  ParameterList params;
  params.append("invc_pattern", _invoicePattern->text().trimmed());
  if (_dates->allValid())
    _dates->appendValue(params);
  if (_balanceOnly->isChecked())
    params.append("balanceOnly");
  q = mql.toQuery(params);
  _invoice->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void reprintInvoices::sPrint()
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

  foreach (XTreeWidgetItem *cursor, _invoice->selectedItems())
  {
    for (int j = 0; j < _invoiceCopies->numCopies(); j++)
    {
      q.prepare("SELECT findCustomerForm(:cust_id, 'I') AS _reportname;");
      q.bindValue(":cust_id", cursor->altId());
      q.exec();
      if (q.first())
      {
	ParameterList params;
	params.append("invchead_id", cursor->id());
	params.append("showcosts", (_invoiceCopies->showCosts(j) ? "TRUE" : "FALSE") );
	params.append("watermark", _invoiceCopies->watermark(j));

	orReport report(q.value("_reportname").toString(), params);
	if (report.isValid())
	{
	  if (report.print(&printer, setupPrinter))
		 setupPrinter = FALSE;
	      else 
	      {
		report.reportError(this);
		orReport::endMultiPrint(&printer);
		return;
	      }
	}
	else
	  QMessageBox::critical(this, tr("Cannot Find Invoice Form"),
				tr("<p>The Invoice Form '%1' cannot be found. "
                                   "One or more of the selected Invoices "
                                   "cannot be printed until a Customer Form "
                                   "Assignment is updated to remove any "
                                   "references to this Invoice Form or this "
                                   "Invoice Form is created.")
                                  .arg(q.value("_reportname").toString()) );
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return;
      }
    }

    emit finishedPrinting(cursor->id());
  }
  orReport::endMultiPrint(&printer);

  _invoice->clearSelection();
  _close->setText(tr("&Close"));
  _print->setEnabled(FALSE);
}

