/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "quotes.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>
#include <metasql.h>

#include "failedPostList.h"
#include "mqlutil.h"
#include "salesOrder.h"
#include "storedProcErrorLookup.h"
#include "customer.h"

quotes::quotes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);
  
  _cust->hide();

  connect(_quote, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_convert, SIGNAL(clicked()), this, SLOT(sConvert()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  _quote->addColumn(tr("Quote #"),    _orderColumn, Qt::AlignRight, true, "quhead_number");
  _quote->addColumn(tr("Customer"),   -1,           Qt::AlignLeft,  true, "quhead_billtoname");
  _quote->addColumn(tr("P/O Number"), -1,           Qt::AlignLeft,  true, "quhead_custponumber");
  _quote->addColumn(tr("Quote Date"), _dateColumn,  Qt::AlignCenter,true, "quhead_quotedate");
  _quote->addColumn(tr("Expires"),    _dateColumn,  Qt::AlignCenter,true, "quhead_expire");
  _quote->setSelectionMode(QAbstractItemView::ExtendedSelection);

  if (_privileges->check("PrintQuotes"))
    connect(_quote, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  if (_privileges->check("ConvertQuotes"))
    connect(_quote, SIGNAL(valid(bool)), _convert, SLOT(setEnabled(bool)));

  if (_privileges->check("MaintainQuotes"))
  {
    connect(_quote, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_quote, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_quote, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_quote, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(quotesUpdated(int, bool)), this, SLOT(sFillList()));

  if (_preferences->boolean("XCheckBox/forgetful"))
    _showProspects->setChecked(true);
}

quotes::~quotes()
{
  // no need to delete child widgets, Qt does it all for us
}

void quotes::languageChange()
{
  retranslateUi(this);
}

enum SetResponse quotes::set(const ParameterList& pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool	   valid;
  
  param = pParams.value("run", &valid);
  if (valid)
  {
    connect(_showProspects, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
    sFillList();
  }

  return NoError;
}

void quotes::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Print..."), this, SLOT(sPrint()), 0);
  if (!_privileges->check("PrintQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Convert..."), this, SLOT(sConvert()), 0);
  if (!_privileges->check("ConvertQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainQuotes"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void quotes::sPrint()
{
  QPrinter printer(QPrinter::HighResolution);
  bool setupPrinter = TRUE;
  q.prepare( "SELECT findCustomerForm(quhead_cust_id, 'Q') AS reportname "
             "FROM quhead "
             "WHERE (quhead_id=:quheadid); " );
  bool userCanceled = false;
  QList<XTreeWidgetItem*> selected = _quote->selectedItems();

  if (selected.size() > 0 &&
      orReport::beginMultiPrint(&printer, userCanceled) == false)
  {
    if(!userCanceled)
      systemError(this, tr("<p>Could not initialize printing system for "
			   "multiple reports."));
    return;
  }
  QList<int> printedQuotes;
  for (int i = 0; i < selected.size(); i++)
  {
    int quheadid = ((XTreeWidgetItem*)(selected[i]))->id();
    q.bindValue(":quheadid", quheadid);
    q.exec();
    if (q.first())
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        ParameterList params;
        params.append("quhead_id", quheadid);

        orReport report(q.value("reportname").toString(), params);
        if (report.isValid() && report.print(&printer, setupPrinter))
        {
          setupPrinter = FALSE;
          printedQuotes.append(quheadid);
        }
        else
        {
          report.reportError(this);
          break;
        }
      }
      else if (q.lastError().type() != QSqlError::NoError)
      {
        systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
        break;
      }
    }
  }
  if (selected.size() > 0)
    orReport::endMultiPrint(&printer);

  for (int i = 0; i < printedQuotes.size(); i++)
    emit finishedPrinting(printedQuotes.at(i));
}

void quotes::sConvert()
{
  if (QMessageBox::question(this, tr("Convert Selected Quote(s)"),
			    tr("<p>Are you sure that you want to convert "
			       "the selected Quote(s) to Sales Order(s)?"),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    XSqlQuery convert;
    convert.prepare("SELECT convertQuote(:quhead_id) AS sohead_id;");

    XSqlQuery prospectq;
    prospectq.prepare("SELECT convertProspectToCustomer(quhead_cust_id) AS result "
		      "FROM quhead "
		      "WHERE (quhead_id=:quhead_id);");

    bool tryagain = false;
    QList<int> converted;
    do {
      int soheadid = -1;
      QList<XTreeWidgetItem*> selected = _quote->selectedItems();
      QList<XTreeWidgetItem*> notConverted;

      for (int i = 0; i < selected.size(); i++)
      {
        if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
        {
          int quheadid = ((XTreeWidgetItem*)(selected[i]))->id();
          convert.bindValue(":quhead_id", quheadid);
          convert.exec();
          if (convert.first())
          {
            soheadid = convert.value("sohead_id").toInt();
            if (soheadid == -3)
            {
              if ((_metrics->value("DefaultSalesRep").toInt() > 0) &&
                  (_metrics->value("DefaultTerms").toInt() > 0) &&
                  (_metrics->value("DefaultCustType").toInt() > 0) && 
                  (_metrics->value("DefaultShipFormId").toInt() > 0)  && 
                  (_privileges->check("MaintainCustomerMasters"))) 
                {
                  if (QMessageBox::question(this, tr("Quote for Prospect"),
                                tr("<p>This Quote is for a Prospect, not "
                               "a Customer. Do you want to convert "
                               "the Prospect to a Customer using global "
                               "default values?"),
                          QMessageBox::Yes,
                          QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
                  {
                    prospectq.bindValue(":quhead_id", quheadid);
                    prospectq.exec();
                    if (prospectq.first())
                    {
                      int result = prospectq.value("result").toInt();
                      if (result < 0)
                      {
                        systemError(this,
                                    storedProcErrorLookup("convertProspectToCustomer",
                                    result), __FILE__, __LINE__);
                        notConverted.append(selected[i]);
                        continue;
                      }
                      convert.exec();
                      if (convert.first())
                      {
                        soheadid = convert.value("sohead_id").toInt();
                        if (soheadid < 0)
                        {
                          QMessageBox::warning(this, tr("Cannot Convert Quote"),
                                  storedProcErrorLookup("convertQuote", soheadid)
                                  .arg(selected[i] ? selected[i]->text(0) : ""));
                          notConverted.append(selected[i]);
                          continue;
                        }
                      }
                    }
                    else if (prospectq.lastError().type() != QSqlError::NoError)
                    {
                      systemError(this, prospectq.lastError().databaseText(),
                              __FILE__, __LINE__);
                      notConverted.append(selected[i]);
                      continue;
                    }
                  }
                  else
                  {
                    QMessageBox::information(this, tr("Quote for Prospect"),
                                tr("<p>The prospect must be manually "
                                   "converted to customer from either the "
                                   "CRM Account or Customer windows before "
                                   "coverting this quote."));
                    notConverted.append(selected[i]);
                    continue;
                  }
              }
              else
              {
                QMessageBox::information(this, tr("Quote for Prospect"),
                            tr("<p>The prospect must be manually "
                               "converted to customer from either the "
                               "CRM Account or Customer windows before "
                               "coverting this quote."));
                notConverted.append(selected[i]);
                continue;
              }
            }
            else if (soheadid < 0)
            {
              QMessageBox::warning(this, tr("Cannot Convert Quote"),
                      storedProcErrorLookup("convertQuote", soheadid)
                      .arg(selected[i] ? selected[i]->text(0) : ""));
              notConverted.append(selected[i]);
              continue;
            }
            converted << quheadid;
            omfgThis->sSalesOrdersUpdated(soheadid);

            salesOrder::editSalesOrder(soheadid, true);
          }
          else if (convert.lastError().type() != QSqlError::NoError)
          {
            notConverted.append(selected[i]);
            systemError(this, convert.lastError().databaseText(), __FILE__, __LINE__);
            continue;
          }
        }
      }

      if (notConverted.size() > 0)
      {
	failedPostList newdlg(this, "", true);
	newdlg.setLabel(tr("<p>The following Quotes could not be converted."));
	newdlg.sSetList(notConverted, _quote->headerItem(), _quote->header());
	tryagain = (newdlg.exec() == XDialog::Accepted);
	selected = notConverted;
	notConverted.clear();
      }
    } while (tryagain);
    for (int i = 0; i < converted.size(); i++)
    {
      omfgThis->sQuotesUpdated(converted[i]);
    }
  } // if user wants to convert
}

void quotes::sNew()
{
  ParameterList params;
  params.append("mode", "newQuote");
  if (_cust->isValid())
    params.append("cust_id", _cust->id());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void quotes::sEdit()
{
  QList<XTreeWidgetItem*> selected = _quote->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      ParameterList params;
      params.append("mode", "editQuote");
      params.append("quhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
    
      salesOrder *newdlg = new salesOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      break;
    }
  }
}

void quotes::sView()
{
  QList<XTreeWidgetItem*> selected = _quote->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
    {
      ParameterList params;
      params.append("mode", "viewQuote");
      params.append("quhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
      
      salesOrder *newdlg = new salesOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      break;
    }
  }
}

void quotes::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Selected Quotes"),
                            tr("<p>Are you sure that you want to delete the "
			       "selected Quotes?" ),
			    QMessageBox::Yes,
			    QMessageBox::No | QMessageBox::Default)
			    == QMessageBox::Yes)
  {
    q.prepare("SELECT deleteQuote(:quhead_id) AS result;");

    int counter = 0;
    QList<XTreeWidgetItem*> selected = _quote->selectedItems();
    for (int i = 0; i < selected.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(selected[i]))->id()))
      {
        q.bindValue(":quhead_id", ((XTreeWidgetItem*)(selected[i]))->id());
        q.exec();
        if (q.first())
        {
          int result = q.value("result").toInt();
          if (result < 0)
          {
            systemError(this, storedProcErrorLookup("deleteQuote", result),
                        __FILE__, __LINE__);
            continue;
          }
          counter++;
        }
        else if (q.lastError().type() != QSqlError::NoError)
        {
          systemError(this, tr("A System Error occurred deleting Quote #%1\n%2.")
                             .arg(selected[i]->text(0))
                             .arg(q.lastError().databaseText()), __FILE__, __LINE__);
          continue;
        }
      }
    }

    if (counter)
      omfgThis->sQuotesUpdated(-1);
  }
}

void quotes::sFillList()
{
  MetaSQLQuery mql = mqlLoad("quotes", "detail");
  ParameterList params;
  if (_cust->isValid())
    params.append("cust_id", _cust->id());
  if (_warehouse->isSelected())
    params.append("warehous_id", _warehouse->id());
  if (! _showProspects->isChecked())
    params.append("customersOnly");
  if ( _showExpired->isChecked())
    params.append("showExpired");
  q = mql.toQuery(params);
  _quote->clear();
  _quote->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool quotes::checkSitePrivs(int orderid)
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
