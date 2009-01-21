/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "openReturnAuthorizations.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QWorkspace>
#include <QSqlError>
#include <openreports.h>
#include <metasql.h>
#include "returnAuthorization.h"
#include "openReturnAuthorizations.h"
#include "printRaForm.h"
//#include "deliverSalesOrder.h"

/*
 *  Constructs a openReturnAuthorizations as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
openReturnAuthorizations::openReturnAuthorizations(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_ra, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_ra, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sFillList()));
  connect(_expired, SIGNAL(clicked()), this, SLOT(sFillList()));

//  statusBar()->hide();
  
  _ra->addColumn(tr("R/A #"),            _orderColumn, Qt::AlignLeft,   true,  "rahead_number"   );
  _ra->addColumn(tr("Cust. #"),          _orderColumn, Qt::AlignLeft,   true,  "customer"   );
  _ra->addColumn(tr("Customer"),         -1,           Qt::AlignLeft,   true,  "rahead_billtoname"   );
  _ra->addColumn(tr("Disposition"),      _itemColumn,  Qt::AlignLeft,   true,  "disposition"   );
  _ra->addColumn(tr("Created"),          _dateColumn,  Qt::AlignCenter, true,  "rahead_authdate" );
  _ra->addColumn(tr("Expires"),          _dateColumn,  Qt::AlignCenter, true,  "rahead_expiredate" );
  
  if (_privileges->check("MaintainReturns"))
  {
    connect(_ra, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ra, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ra, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_ra, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  connect(omfgThis, SIGNAL(returnAuthorizationsUpdated()), this, SLOT(sFillList()));

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
openReturnAuthorizations::~openReturnAuthorizations()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void openReturnAuthorizations::languageChange()
{
    retranslateUi(this);
}

void openReturnAuthorizations::sPrint()
{
  ParameterList params;
  _warehouse->appendValue(params);

  if(_expired->isChecked())
    params.append("showExpired");

  orReport report("ListOpenReturnAuthorizations", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void openReturnAuthorizations::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openReturnAuthorizations::sEdit()
{
  if (!checkSitePrivs(_ra->id()))
    return;
    
  ParameterList params;
  params.append("mode", "edit");
  params.append("rahead_id", _ra->id());

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openReturnAuthorizations::sView()
{  
  if (!checkSitePrivs(_ra->id()))
    return;
    
  ParameterList params;
  params.append("mode", "view");
  params.append("rahead_id", _ra->id());

  returnAuthorization *newdlg = new returnAuthorization();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void openReturnAuthorizations::sDelete()
{
  if (!checkSitePrivs(_ra->id()))
    return;
    
  if ( QMessageBox::warning( this, tr("Delete Return Authorization?"),
                             tr("Are you sure that you want to completely delete the selected Return Authorization?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0 )
  {
	q.prepare("DELETE FROM rahead WHERE (rahead_id=:rahead_id);");
    q.bindValue(":rahead_id", _ra->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    }
  }
  omfgThis->sReturnAuthorizationsUpdated();
}

void openReturnAuthorizations::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainReturns"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainReturns"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  /*
  if (_metrics->boolean("EnableBatchManager"))
  {
    menuItem = pMenu->insertItem(tr("Schedule S/O for Email Delivery..."), this, SLOT(sDeliver()), 0);
  }*/

  menuItem = pMenu->insertItem(tr("Print Return Authorization Form..."), this, SLOT(sPrintForms()), 0); 

}


void openReturnAuthorizations::sFillList()
{
  QString sql( "SELECT DISTINCT rahead_id, rahead_number,"
               "       COALESCE(cust_number, :undefined) AS customer,"
               "       rahead_billtoname, "
               "       CASE "
               "       WHEN rahead_disposition = 'C' THEN :credit "
               "       WHEN rahead_disposition = 'R' THEN :return "
               "       WHEN rahead_disposition = 'P' THEN :replace "
               "       WHEN rahead_disposition = 'V' THEN :service "
               "       WHEN rahead_disposition = 'M' THEN :substitute "
               "       END AS disposition, "
               "       rahead_authdate, rahead_expiredate "
               "FROM rahead LEFT OUTER JOIN cust ON (rahead_cust_id=cust_id) "
               "     LEFT OUTER JOIN raitem JOIN itemsite ON (raitem_itemsite_id=itemsite_id) "
               "     ON (raitem_rahead_id=rahead_id) "
               "WHERE (((raitem_status = 'O') OR (raitem_status IS NULL)) ");

  if (!_expired->isChecked())
    sql += " AND (COALESCE(rahead_expiredate,current_date)>=current_date)";
  if (_warehouse->isSelected())
    sql += " AND (itemsite_warehous_id=:warehous_id)";

  sql += " ) "
         "GROUP BY rahead_id, rahead_number, cust_number, rahead_billtoname,"
         "         rahead_disposition, rahead_authdate, rahead_expiredate "
         "ORDER BY rahead_number ";

  q.prepare(sql);
  _warehouse->bindValue(q);
  q.bindValue(":undefined", tr("Undefined"));
  q.bindValue(":credit", tr("Credit"));
  q.bindValue(":return", tr("Return"));
  q.bindValue(":replace", tr("Replace"));
  q.bindValue(":service", tr("Service"));
  q.bindValue(":substitute", tr("Substitute"));
  q.exec();

  _ra->populate(q);
  _ra->setDragString("raheadid=");
}

void openReturnAuthorizations::sPrintForms()
{
  if (!checkSitePrivs(_ra->id()))
    return;
    
  ParameterList params;
  params.append("rahead_id", _ra->id());

  printRaForm newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

bool openReturnAuthorizations::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkRASitePrivs(:raheadid) AS result;");
    check.bindValue(":raheadid", orderid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("You may not view or edit this Return Authorization as it references "
                                 "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}
