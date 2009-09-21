/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspQuotesByItem.h"

#include <metasql.h>
#include "mqlutil.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <QWorkspace>
#include "salesOrder.h"

/*
 *  Constructs a dspQuotesByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspQuotesByItem::dspQuotesByItem(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
    connect(_so, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_dates, SIGNAL(updated()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspQuotesByItem::~dspQuotesByItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspQuotesByItem::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void dspQuotesByItem::init()
{
//  statusBar()->hide();

  _dates->setStartNull(tr("Earliest"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Starting Order Date"));
  _dates->setEndNull(tr("Latest"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Ending Order Date"));

  _item->setType(ItemLineEdit::cSold);

  _so->addColumn(tr("Quote #"),    _orderColumn, Qt::AlignLeft,   true,  "quhead_number"   );
  _so->addColumn(tr("Quote Date"), _dateColumn,  Qt::AlignCenter, true,  "quhead_quotedate" );
  _so->addColumn(tr("Customer"),   -1,           Qt::AlignLeft,   true,  "cust_name"   );
  _so->addColumn(tr("Quoted"),     _qtyColumn,   Qt::AlignRight,  true,  "quitem_qtyord"  );

  connect(omfgThis, SIGNAL(salesOrdersUpdated(int, bool)), this, SLOT(sFillList())); 
}

enum SetResponse dspQuotesByItem::set(ParameterList &pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("startDate", &valid);
  if (valid)
    _dates->setStartDate(param.toDate());

  param = pParams.value("endDate", &valid);
  if (valid)
    _dates->setEndDate(param.toDate());

  return NoError;
}

void dspQuotesByItem::sPopulateMenu(QMenu *menuThis)
{
  menuThis->insertItem(tr("Edit..."), this, SLOT(sEditOrder()), 0);
  menuThis->insertItem(tr("View..."), this, SLOT(sViewOrder()), 0);
}

void dspQuotesByItem::sEditOrder()
{
  if (!checkSitePrivs(_so->altId()))
    return;
    
  ParameterList params;
  params.append("mode", "editQuote");
  params.append("quhead_id", _so->altId());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQuotesByItem::sViewOrder()
{
  if (!checkSitePrivs(_so->altId()))
    return;
    
  ParameterList params;
  params.append("mode", "viewQuote");
  params.append("quhead_id", _so->altId());
      
  salesOrder *newdlg = new salesOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspQuotesByItem::sFillList()
{
  _so->clear();

  if ((_item->isValid()) && (_dates->allValid()))
  {
    MetaSQLQuery mql = mqlLoad("quoteItems", "detail");
    ParameterList params;
    _dates->appendValue(params);
    params.append("item_id", _item->id());

    q = mql.toQuery(params);
    _so->populate(q, true);
  }
}

bool dspQuotesByItem::checkSitePrivs(int orderid)
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

