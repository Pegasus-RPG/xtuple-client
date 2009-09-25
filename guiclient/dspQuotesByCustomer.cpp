/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQuotesByCustomer.h"

#include <metasql.h>
#include "mqlutil.h"

#include <QVariant>
//#include <QStatusBar>
#include <QWorkspace>
#include <QMessageBox>
#include "salesOrder.h"

/*
 *  Constructs a dspQuotesByCustomer as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspQuotesByCustomer::dspQuotesByCustomer(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_cust, SIGNAL(newId(int)), this, SLOT(sPopulatePo()));
    connect(_selectedPO, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_selectedPO, SIGNAL(toggled(bool)), _poNumber, SLOT(setEnabled(bool)));
    connect(_poNumber, SIGNAL(activated(int)), this, SLOT(sFillList()));
    connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspQuotesByCustomer::~dspQuotesByCustomer()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspQuotesByCustomer::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspQuotesByCustomer::init()
{
//  statusBar()->hide();

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Order Date"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Order Date"));

  _so->addColumn(tr("Quote #"),     _orderColumn, Qt::AlignLeft,   true,  "quhead_number"   );
  _so->addColumn(tr("Quote Date"),  _dateColumn,  Qt::AlignRight,  true,  "quhead_quotedate"  );
  _so->addColumn(tr("Ship-to"),     -1,           Qt::AlignLeft,   true,  "quhead_shiptoname"   );
  _so->addColumn(tr("Cust. P/O #"), 200,          Qt::AlignLeft,   true,  "quhead_custponumber"   );
  
  _cust->setFocus();
  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())  );
}

void dspQuotesByCustomer::sPopulatePo()
{
  _poNumber->clear();

  if ((_cust->isValid()) && (_dates->allValid()))
  {
    q.prepare( "SELECT DISTINCT -2, quhead_custponumber "
               "FROM quhead "
               "WHERE ( (quhead_cust_id=:cust_id)"
               " AND (quhead_quotedate BETWEEN :startDate AND :endDate) ) "
               "ORDER BY quhead_custponumber;" );
    _dates->bindValue(q);
    q.bindValue(":cust_id", _cust->id());
    q.exec();
    _poNumber->populate(q);
  }

  sFillList();
}

void dspQuotesByCustomer::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("Edit..."), this, SLOT(sEditOrder()), 0);
  menuThis->insertItem(tr("View..."), this, SLOT(sViewOrder()), 0);
  
  if (_privileges->check("ConvertQuotes"))
  {
    menuThis->insertSeparator();
    menuThis->insertItem(tr("Convert..."), this, SLOT(sConvert()), 0);
  }

}

void dspQuotesByCustomer::sEditOrder()
{
  if (!checkSitePrivs(_so->id()))
    return;
    
  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", _so->id());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQuotesByCustomer::sViewOrder()
{
  if (!checkSitePrivs(_so->id()))
    return;
    
  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", _so->id());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQuotesByCustomer::sFillList()
{
  _so->clear();

  if ( ( (_allPOs->isChecked()) ||
         ( (_selectedPO->isChecked()) && (_poNumber->currentIndex() != -1) ) ) &&
       (_dates->allValid())  )
  {
    MetaSQLQuery mql = mqlLoad("quotes", "detail");
    ParameterList params;
    _dates->appendValue(params);
    params.append("cust_id", _cust->id());
    params.append("showExpired");
    params.append("customersOnly");
    if (_selectedPO->isChecked())
      params.append("poNumber", _poNumber->currentText());

    q = mql.toQuery(params);
    _so->populate(q);
  }
}

void dspQuotesByCustomer::sConvert()
{
  if ( QMessageBox::information( this, tr("Convert Selected Quote(s)"),
                                 tr("Are you sure that you want to convert the selected Quote(s) to Sales Order(s)?" ),
                                 tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    XSqlQuery check;
    check.prepare( "SELECT quhead_number, cust_creditstatus "
                   "FROM quhead, cust "
                   "WHERE ( (quhead_cust_id=cust_id)"
                   " AND (quhead_id=:quhead_id) );" );

    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");

    int counter = 0;
    int soheadid = -1;

    QList<XTreeWidgetItem*> selected = _so->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        XTreeWidgetItem *cursor = (XTreeWidgetItem*)selected[i];
        check.bindValue(":quhead_id", cursor->id());
        check.exec();
        if (check.first())
        {
	      if ( (check.value("cust_creditstatus").toString() == "H") && (!_privileges->check("CreateSOForHoldCustomer")) )
	      {
	        QMessageBox::warning( this, tr("Cannot Convert Quote"),
					tr( "Quote #%1 is for a Customer that has been placed on a Credit Hold and you do not have\n"
				    "privilege to create Sales Orders for Customers on Credit Hold.  The selected\n"
				    "Customer must be taken off of Credit Hold before you may create convert this Quote." )
				.arg(check.value("quhead_number").toString()) );
	        return;
	      }	

	      if ( (check.value("cust_creditstatus").toString() == "W") && (!_privileges->check("CreateSOForWarnCustomer")) )
	      {
	        QMessageBox::warning( this, tr("Cannot Convert Quote"),
			    	tr( "Quote #%1 is for a Customer that has been placed on a Credit Warning and you do not have\n"
				    "privilege to create Sales Orders for Customers on Credit Warning.  The selected\n"
				    "Customer must be taken off of Credit Warning before you may create convert this Quote." )
				.arg(check.value("quhead_number").toString()) );
	        return;
	      }	
        }
        else
        {
	      systemError( this, tr("A System Error occurred at %1::%2.")
			   .arg(__FILE__)
			   .arg(__LINE__) );
	      continue;
        }

        convert.bindValue(":quhead_id", cursor->id());
        convert.exec();
        if (convert.first())
        { 
	      soheadid = convert.value("sohead_id").toInt();
	      if(soheadid < 0)
	      {
	        QMessageBox::warning( this, tr("Cannot Convert Quote"),
				tr( "Quote #%1 has one or more line items without a warehouse specified.\n"
				    "These line items must be fixed before you may convert this quote." )
				.arg(check.value("quhead_number").toString()) );
	        return;
	      }
	      counter++;
	      omfgThis->sSalesOrdersUpdated(soheadid);
		}
        else
        {
	      systemError( this, tr("A System Error occurred at %1::%2.")
			   .arg(__FILE__)
			   .arg(__LINE__) );
	      return;
        }
	  }
    }

    if (counter)
      omfgThis->sQuotesUpdated(-1);

    if (counter == 1)
      salesOrder::editSalesOrder(soheadid, true);
  }
}

bool dspQuotesByCustomer::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkQuoteSitePrivs(:quheadid) AS result;");
    check.bindValue(":quheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                                       tr("You may not view, edit, or convert this Quote as it references "
                                       "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
