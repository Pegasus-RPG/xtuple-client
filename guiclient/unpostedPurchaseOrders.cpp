/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "unpostedPurchaseOrders.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <QWorkspace>

#include <metasql.h>
#include <parameter.h>

#include "purchaseOrder.h"
#include "printPurchaseOrder.h"
#include "guiclient.h"
#include "storedProcErrorLookup.h"

unpostedPurchaseOrders::unpostedPurchaseOrders(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_showUnreleased, SIGNAL(toggled(bool)), this,	SLOT(sFillList()));
  connect(_showOpen, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_delete,	SIGNAL(clicked()),	this,	SLOT(sDelete()));
  connect(_edit,	SIGNAL(clicked()),	this,	SLOT(sEdit()));
  connect(_new,         SIGNAL(clicked()),	this,	SLOT(sNew()));
  connect(_pohead,	SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)),
                                                this,	SLOT(sPopulateMenu(QMenu*, QTreeWidgetItem *)));
  connect(_pohead,	SIGNAL(valid(bool)),	this,	SLOT(sHandleButtons()));
  connect(_release,	SIGNAL(clicked()),	this,	SLOT(sRelease()));
  connect(_print,	SIGNAL(clicked()),	this,	SLOT(sPrint()));
  connect(_view,	SIGNAL(clicked()),	this,	SLOT(sView()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_next, SIGNAL(clicked()), this, SLOT(sSearchNext()));

  connect(omfgThis,	SIGNAL(purchaseOrdersUpdated(int, bool)),
                                              this,	SLOT(sFillList()));

  _pohead->addColumn(tr("P/O #"),     _orderColumn, Qt::AlignLeft,   true, "pohead_number" );
  _pohead->addColumn(tr("Vendor"),    -1,           Qt::AlignLeft,   true, "vend_name"   );
  _pohead->addColumn(tr("Due Date"),  _dateColumn,  Qt::AlignCenter, true, "min_duedate" );
  _pohead->addColumn(tr("Status"),    _ynColumn,    Qt::AlignCenter, true, "pohead_status" );
  _pohead->addColumn(tr("Printed"),   _ynColumn,    Qt::AlignCenter, true, "pohead_printed");

  _pohead->setSelectionMode(QAbstractItemView::ExtendedSelection);

  _showUnreleased->setChecked(FALSE);

  if (_privileges->check("MaintainPurchaseOrders"))
    _new->setEnabled(TRUE);

  sFillList();
}

unpostedPurchaseOrders::~unpostedPurchaseOrders()
{
  // no need to delete child widgets, Qt does it all for us
}

void unpostedPurchaseOrders::languageChange()
{
    retranslateUi(this);
}

void unpostedPurchaseOrders::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrder *newdlg = new purchaseOrder();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void unpostedPurchaseOrders::sEdit()
{
  QList<XTreeWidgetItem*> list = _pohead->selectedItems();
  bool done = false;
  for (int i = 0; i < list.size(); i++)
  {
    if (((list[i]->text(3) == "U" && _privileges->check("MaintainPurchaseOrders")) ||
	(list[i]->text(3) == "O" && _privileges->check("MaintainPostedPurchaseOrders"))) &&
        (checkSitePrivs(((XTreeWidgetItem*)(list[i]))->id())))
    {
      ParameterList params;
      params.append("mode", "edit");
      params.append("pohead_id", ((XTreeWidgetItem*)(list[i]))->id());

      purchaseOrder *newdlg = new purchaseOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      done = true;
      break;
    }
  }
  if (done)
    omfgThis->sPurchaseOrdersUpdated(-1, TRUE);
  else
    QMessageBox::information(this, tr("Nothing To Edit"),
			     tr("<p>There were no selected Purchase Orders "
				"that you could edit."),
			     QMessageBox::Ok|QMessageBox::Default);
}

void unpostedPurchaseOrders::sView()
{
  QList<XTreeWidgetItem*> list = _pohead->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(list[i]))->id()))
    {

      ParameterList params;
      params.append("mode", "view");
      params.append("pohead_id", ((XTreeWidgetItem*)(list[i]))->id());

      purchaseOrder *newdlg = new purchaseOrder();
      newdlg->set(params);
      omfgThis->handleNewWindow(newdlg);
      break;
    }
  }
}

void unpostedPurchaseOrders::sDelete()
{
  if ( QMessageBox::question(this, tr("Delete Selected Purchase Orders"),
                             tr("<p>Are you sure that you want to delete the "
			        "selected Purchase Orders?" ),
                             QMessageBox::Yes,
                             QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("SELECT deletePo(:pohead_id) AS result;");

    QList<XTreeWidgetItem*> list = _pohead->selectedItems();
    bool done = false;
    for (int i = 0; i < list.size(); i++)
    {
      if (checkSitePrivs(((XTreeWidgetItem*)(list[i]))->id()))
      {
        if (list[i]->text(3) == "U")
        {
          if (list[i]->altId() != -1)
		  {
            QString question = tr("<p>The Purchase Order that you selected to delete was created "
            "to satisfy Sales Order demand. If you delete the selected "
            "Purchase Order then the Sales Order demand will remain but "
            "the Purchase Order to relieve that demand will not. Are you "
            "sure that you want to delete the selected Purchase Order?" );
            if (QMessageBox::question(this, tr("Delete Purchase Order?"),
                                      question,
                                      QMessageBox::Yes,
                                      QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
			  continue;
		  }
          q.bindValue(":pohead_id", ((XTreeWidgetItem*)(list[i]))->id());
          q.exec();
          if (q.first() && ! q.value("result").toBool())
              systemError(this, tr("<p>Only Unposted Purchase Orders may be "
                                   "deleted. Check the status of Purchase Order "
                                   "%1. If it is 'U' then contact your system "
                                   "Administrator.").arg(list[i]->text(0)),
                          __FILE__, __LINE__);
          else if (q.lastError().type() != QSqlError::NoError)
            systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
          else
            done = true;
        }
      }
    }
    if (done)
      omfgThis->sPurchaseOrdersUpdated(-1, TRUE);
    else
      QMessageBox::information(this, tr("Nothing To Delete"),
			       tr("<p>There were no selected Purchase Orders "
				  "that could be deleted."),
			       QMessageBox::Ok|QMessageBox::Default);
  }
}

void unpostedPurchaseOrders::sPrint()
{
  QList<XTreeWidgetItem*> list = _pohead->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    if (checkSitePrivs(((XTreeWidgetItem*)(list[i]))->id()))
    {
      ParameterList params;
      params.append("pohead_id", ((XTreeWidgetItem*)(list[i]))->id());

      printPurchaseOrder newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
      break;
    }
  }
  sFillList();
}

void unpostedPurchaseOrders::sRelease()
{
  if ( QMessageBox::warning( this, tr("Release Selected Purchase Orders"),
                             tr("<p>Are you sure that you want to release "
			        "the selected Purchase Orders?" ),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    q.prepare("SELECT releasePurchaseOrder(:pohead_id) AS result;");

    QList<XTreeWidgetItem*> list = _pohead->selectedItems();
    bool done = false;
    for (int i = 0; i < list.size(); i++)
    {
      if ((list[i]->text(3) == "U")
        && (_privileges->check("ReleasePurchaseOrders"))
        && (checkSitePrivs(((XTreeWidgetItem*)(list[i]))->id())))
      {
        q.bindValue(":pohead_id", ((XTreeWidgetItem*)(list[i]))->id());
        q.exec();
        if (q.first())
        {
          int result = q.value("result").toInt();
          if (result < 0)
            systemError(this, storedProcErrorLookup("releasePurchaseOrder", result),
                        __FILE__, __LINE__);
          else
			done = true;
		}
        else if (q.lastError().type() != QSqlError::NoError)
          systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      }
    }
    if (done)
      omfgThis->sPurchaseOrdersUpdated(-1, TRUE);
    else
      QMessageBox::information(this, tr("Nothing To Post"),
			       tr("<p>There were no selected Purchase Orders "
				  "to be posted."),
			       QMessageBox::Ok|QMessageBox::Default);
  }
}

void unpostedPurchaseOrders::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pItem)
{
  int menuItem;

  bool canMaintain = (pItem->text(3) == "U" && _privileges->check("MaintainPurchaseOrders")) ||
		     (pItem->text(3) == "O" && _privileges->check("MaintainPostedPurchaseOrders"));

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEdit()), 0);
  pMenu->setItemEnabled(menuItem, canMaintain);

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  pMenu->setItemEnabled(menuItem, canMaintain || _privileges->check("ViewPurchaseOrders"));

  menuItem = pMenu->insertItem(tr("Delete..."), this, SLOT(sDelete()), 0);
  pMenu->setItemEnabled(menuItem, (pItem->text(3) == "U" && _privileges->check("MaintainPurchaseOrders")));

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Print..."), this, SLOT(sPrint()), 0);
  pMenu->setItemEnabled(menuItem, canMaintain);

  menuItem = pMenu->insertItem(tr("Release..."), this, SLOT(sRelease()), 0);
  pMenu->setItemEnabled(menuItem, _privileges->check("ReleasePurchaseOrders") &&
				  pItem->text(3) == "U");
}

void unpostedPurchaseOrders::sHandleButtons()
{
  // see if the selected items list contains both unposted and open/posted items
  bool unposted = false;
  bool open = false;
  QList<XTreeWidgetItem*> list = _pohead->selectedItems();
  for (int i = 0; i < list.size(); i++)
  {
    if (! unposted && list[i]->text(3) == "U")
      unposted = true;
    if (! open && list[i]->text(3) == "O")
      open = true;
    if (open && unposted)
      break;
  }

  _delete->setEnabled(unposted && _privileges->check("MaintainPurchaseOrders"));
  _edit->setEnabled((unposted && _privileges->check("MaintainPurchaseOrders")) ||
		    (open && _privileges->check("MaintainPostedPurchaseOrders")));
  _release->setEnabled(unposted && _privileges->check("ReleasePurchaseOrders"));
  _print->setEnabled(_privileges->check("PrintPurchaseOrders"));

  if ((unposted && _privileges->check("MaintainPurchaseOrders")) ||
      (open && _privileges->check("MaintainPostedPurchaseOrders")))
  {
    disconnect(_pohead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    connect(_pohead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    disconnect(_pohead, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_pohead, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
}

void unpostedPurchaseOrders::sFillList()
{
  _pohead->clear();

  ParameterList params;
  if (_showUnreleased->isChecked() && _showOpen->isChecked() )
    params.append("showBoth");
  else if (_showUnreleased->isChecked())
    params.append("showUnreleased");
  else if (_showOpen->isChecked())
    params.append("showOpen");
  else
    params.append("shownothing");

  QString sql( "SELECT pohead_id, COALESCE(pohead_cohead_id, -1) AS pohead_cohead_id,"
               "       pohead_number, vend_name,"
               "       MIN(poitem_duedate) AS min_duedate, "
               "       pohead_status, pohead_printed "
               "FROM vend, pohead LEFT OUTER JOIN "
               "     poitem ON (poitem_pohead_id=pohead_id) "
               "WHERE ( (pohead_vend_id=vend_id)"
               "<? if exists(\"showUnreleased\") ?> "
               "  AND (pohead_status ='U') "
               "<? endif ?> "
               "<? if exists(\"showOpen\") ?>"
               "  AND (pohead_status='O' )"
               "<? endif ?> "
               "<? if exists(\"showBoth\") ?> "
               "  AND (pohead_status IN ('U', 'O') ) "
               "<? endif ?> "
               "<? if exists(\"shownothing\") ?> "
               "  AND (pohead_status NOT IN ('U', 'O', 'C')) "
               "<? endif ?> "
               ") "
               "GROUP BY pohead_number, pohead_id, pohead_cohead_id,"
			   "         vend_name, pohead_status, pohead_printed "
               "ORDER BY pohead_number;" );
  MetaSQLQuery mql(sql);
  XSqlQuery r = mql.toQuery(params);
  if (r.lastError().type() != QSqlError::NoError)
  {
    systemError(this, r.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  _pohead->populate(r, true);
}

bool unpostedPurchaseOrders::checkSitePrivs(int orderid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkPOSitePrivs(:poheadid) AS result;");
    check.bindValue(":poheadid", orderid);
    check.exec();
    if (check.first())
    {
    if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("<p>You may not view or edit this Purchase "
                                 "Order as it references a Site for which you "
                                 "have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}


void unpostedPurchaseOrders::sSearch( const QString &pTarget )
{
  _pohead->clearSelection();
  int i;
  for (i = 0; i < _pohead->topLevelItemCount(); i++)
  {
    if ( _pohead->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive) ||
         _pohead->topLevelItem(i)->text(1).contains(pTarget, Qt::CaseInsensitive) )
      break;
  }

  if (i < _pohead->topLevelItemCount())
  {
    _pohead->setCurrentItem(_pohead->topLevelItem(i));
    _pohead->scrollToItem(_pohead->topLevelItem(i));
  }
}

void unpostedPurchaseOrders::sSearchNext()
{
  QString target = _searchFor->text();
  int i;
  int currentIndex = _pohead->indexOfTopLevelItem(_pohead->currentItem()) + 1;
  if(currentIndex < 0 || currentIndex > _pohead->topLevelItemCount())
    currentIndex = 0;
  for (i = currentIndex; i < _pohead->topLevelItemCount(); i++)
  {
    if ( _pohead->topLevelItem(i)->text(0).startsWith(target, Qt::CaseInsensitive) ||
         _pohead->topLevelItem(i)->text(1).contains(target, Qt::CaseInsensitive) )
      break;
  }

  if (i < _pohead->topLevelItemCount())
  {
    _pohead->setCurrentItem(_pohead->topLevelItem(i));
    _pohead->scrollToItem(_pohead->topLevelItem(i));
  }
}
