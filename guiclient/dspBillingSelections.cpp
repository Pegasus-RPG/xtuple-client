/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspBillingSelections.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
//#include <QStatusBar>
#include <QVariant>
#include <QWorkspace>

#include <openreports.h>
#include "selectOrderForBilling.h"
#include "printInvoices.h"
#include "postBillingSelections.h"

/*
 *  Constructs a dspBillingSelections as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspBillingSelections::dspBillingSelections(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_cobill, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(sCancel()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_postAll, SIGNAL(clicked()), this, SLOT(sPostAll()));
  connect(_cobill, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
  connect(_cobill, SIGNAL(valid(bool)), _cancel, SLOT(setEnabled(bool)));
  connect(_cobill, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

//  statusBar()->hide();
  
  _cobill->addColumn(tr("Order #"),       _orderColumn,  Qt::AlignLeft,   true,  "cohead_number"   );
  _cobill->addColumn(tr("Cust. #"),       _itemColumn,   Qt::AlignLeft,   true,  "cust_number"   );
  _cobill->addColumn(tr("Name"),          -1,            Qt::AlignLeft,   true,  "cust_name"   );
  _cobill->addColumn(tr("Subtotal"),      _priceColumn,  Qt::AlignLeft,   false, "subtotal" );
  _cobill->addColumn(tr("Misc."),         _priceColumn,  Qt::AlignLeft,   false, "cobmisc_misc" ); 
  _cobill->addColumn(tr("Freight"),       _priceColumn,  Qt::AlignLeft,   false, "cobmisc_freight" );
  _cobill->addColumn(tr("Tax"),           _priceColumn,  Qt::AlignLeft,   false, "cobmisc_tax" );
  _cobill->addColumn(tr("Total"),         _priceColumn,  Qt::AlignLeft,   false, "total" );
  _cobill->addColumn(tr("Payment rec'd"), _priceColumn,  Qt::AlignLeft,   false, "cobmisc_payment" );

  if (_privileges->check("PostARDocuments"))
    connect(_cobill, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));

  connect(omfgThis, SIGNAL(billingSelectionUpdated(int, int)), this, SLOT(sFillList()));

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspBillingSelections::~dspBillingSelections()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspBillingSelections::languageChange()
{
    retranslateUi(this);
}

void dspBillingSelections::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
  pMenu->insertItem("Cancel...", this, SLOT(sCancel()), 0);

  menuItem = pMenu->insertItem("Create Invoice", this, SLOT(sPost()), 0);
  if (!_privileges->check("PostARDocuments"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspBillingSelections::sFillList()
{
  q.exec( "SELECT cobmisc_id, cohead_id,"
          "       cohead_number, cust_number, cust_name,"
          "       sum(round(coitem_price*cobill_qty,2)) AS subtotal,"
          "       cobmisc_misc, cobmisc_freight, cobmisc_tax, cobmisc_payment,"
          "       (sum(round(coitem_price*cobill_qty,2))+cobmisc_misc+cobmisc_freight+cobmisc_tax) AS total,"
          "       'curr' AS subtotal_xtnumericrole,"
          "       'curr' AS total_xtnumericrole,"
          "       'curr' AS cobmisc_misc_xtnumericrole,"
          "       'curr' AS cobmisc_freight_xtnumericrole,"
          "       'curr' AS cobmisc_tax_xtnumericrole,"
          "       'curr' AS cobmisc_payment_xtnumericrole "
          "  FROM cobmisc, cohead, cust, coitem, cobill "
          " WHERE((cobmisc_cohead_id=cohead_id)"
          "   AND (cohead_cust_id=cust_id)"
          "   AND (coitem_cohead_id=cohead_id)"
          "   AND (cobill_coitem_id=coitem_id)"
          "   AND (NOT cobmisc_posted)) "
          " GROUP BY cobmisc_id, cohead_id, cohead_number, cust_number,"
          "          cust_name, cobmisc_misc,cobmisc_freight,cobmisc_tax,cobmisc_payment "
          " ORDER BY cohead_number;" );
  _cobill->populate(q, TRUE);
}

void dspBillingSelections::sPostAll()
{
  postBillingSelections newdlg(this, "", TRUE);
  newdlg.exec();
}

void dspBillingSelections::sPost()
{
  int soheadid = _cobill->altId();

  q.prepare("SELECT postBillingSelection(:cobmisc_id) AS result;");
  q.bindValue(":cobmisc_id", _cobill->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();

    if (result == -5)
      QMessageBox::critical( this, tr("Cannot Post one or more Invoices"),
                             tr( "The G/L Account Assignments for the selected Invoice are not configured correctly.\n"
                                 "Because of this, G/L Transactions cannot be posted for this Invoices.\n"
                                 "You must contact your Systems Administrator to have this corrected before you may post this Invoice." ) );
    else if (result < 0)
      systemError( this, tr("A System Error occurred at %1::%2, Error #%3.")
                         .arg(__FILE__)
                         .arg(__LINE__)
                         .arg(q.value("result").toInt()) );

    omfgThis->sInvoicesUpdated(result, TRUE);
    omfgThis->sSalesOrdersUpdated(soheadid);
    omfgThis->sBillingSelectionUpdated(soheadid, TRUE);

    sFillList();
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void dspBillingSelections::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspBillingSelections::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sohead_id", _cobill->altId());

  selectOrderForBilling *newdlg = new selectOrderForBilling();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}


void dspBillingSelections::sCancel()
{
  if ( QMessageBox::critical( this, tr("Cancel Billing"),
                              tr("Are you sure that you want to cancel billing for the selected order?"),
                              tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "SELECT cancelBillingSelection(cobmisc_id) AS result "
               "FROM cobmisc "
               "WHERE (cobmisc_cohead_id=:sohead_id);" );
    q.bindValue(":sohead_id", _cobill->altId());
    q.exec();

    sFillList();
  }
}


void dspBillingSelections::sPrint()
{
  orReport report("BillingSelections");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

