/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspRecurringInvoices.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include  <openreports.h>

#include "getGLDistDate.h"
#include "invoice.h"
#include "mqlutil.h"
#include "storedProcErrorLookup.h"

dspRecurringInvoices::dspRecurringInvoices(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_invchead, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _invchead->addColumn(tr("Invoice #"),    _orderColumn,    Qt::AlignLeft,   true,  "invchead_invcnumber" );
  _invchead->addColumn(tr("Customer"),     -1,              Qt::AlignLeft,   true,  "cust_name" );
  _invchead->addColumn(tr("Ship-to"),      100,             Qt::AlignLeft,   false, "invchead_shipto_name" );
  _invchead->addColumn(tr("Invc. Date"),   _dateColumn,     Qt::AlignCenter, true,  "invchead_invcdate" );
  _invchead->addColumn(tr("Interval"),     100,             Qt::AlignRight,  true,  "invchead_recurring_interval" );
  _invchead->addColumn(tr("Type"),         _orderColumn,    Qt::AlignCenter, true,  "recurring_type" );
  _invchead->addColumn(tr("Until"),        _dateColumn,     Qt::AlignCenter, true,  "invchead_recurring_until" );
  _invchead->addColumn(tr("Amount"),       _bigMoneyColumn, Qt::AlignRight,  true,  "extprice" );
  _invchead->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (_privileges->check("MaintainMiscInvoices"))
  {
    _new->setEnabled(TRUE);
    connect(_invchead, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_invchead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    connect(_invchead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  if (_privileges->check("MaintainMiscInvoices") || _privileges->check("ViewMiscInvoices"))
    connect(_invchead, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));

  connect(omfgThis, SIGNAL(invoicesUpdated(int, bool)), this, SLOT(sFillList()));

  sFillList();
}

dspRecurringInvoices::~dspRecurringInvoices()
{
    // no need to delete child widgets, Qt does it all for us
}

void dspRecurringInvoices::languageChange()
{
    retranslateUi(this);
}

void dspRecurringInvoices::sNew()
{
  invoice::newInvoice(-1);
}

void dspRecurringInvoices::sEdit()
{
  QList<XTreeWidgetItem*> selected = _invchead->selectedItems();
  for (int i = 0; i < selected.size(); i++)
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
        invoice::editInvoice(((XTreeWidgetItem*)(selected[i]))->id());
      
}

void dspRecurringInvoices::sView()
{
  QList<XTreeWidgetItem*> selected = _invchead->selectedItems();
  for (int i = 0; i < selected.size(); i++)
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
        invoice::viewInvoice(((XTreeWidgetItem*)(selected[i]))->id());
}

void dspRecurringInvoices::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainMiscInvoices"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainMiscInvoices")) && (!_privileges->check("ViewMiscInvoices")))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspRecurringInvoices::sFillList()
{
  _invchead->clear();

  MetaSQLQuery mql = mqlLoad("invoices", "detail");
  ParameterList params;
  params.append("recurringOnly");
  params.append("day", tr("Day"));
  params.append("week", tr("Week"));
  params.append("month", tr("Month"));
  params.append("year", tr("Year"));
  params.append("none", tr("None"));
  q = mql.toQuery(params);
  _invchead->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool dspRecurringInvoices::checkSitePrivs(int invcid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkInvoiceSitePrivs(:invcheadid) AS result;");
    check.bindValue(":invcheadid", invcid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view or edit this Invoice as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
