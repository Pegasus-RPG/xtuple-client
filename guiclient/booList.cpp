/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "booList.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>

#include "boo.h"
#include "copyBOO.h"

booList::booList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_boo, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));

  _boo->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number");
  _boo->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "descrip");
  
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), SLOT(sFillList(int, bool)));

  if (_privileges->check("MaintainBOOs"))
  {
    connect(_boo, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_boo, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_boo, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));

    connect(_boo, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_boo, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }
  
  sFillList();

  _searchFor->setFocus();
}

booList::~booList()
{
  // no need to delete child widgets, Qt does it all for us
}

void booList::languageChange()
{
  retranslateUi(this);
}

void booList::sCopy()
{
  ParameterList params;
  params.append("item_id", _boo->id());

  copyBOO newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void booList::sDelete()
{

  q.prepare("SELECT booitem_id "
            "FROM booitem "
            "WHERE ((booitem_item_id=:item_id) "
            "   AND (booitem_rev_id > -1));");
  q.bindValue(":item_id",_boo->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical(  this, tr("Delete Bill of Operations"),
                                tr("<p>The selected Bill of Operations has "
                                   "revision control records and may not be "
                                   "deleted."));
	return;
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  if (  QMessageBox::question(  this, tr("Delete Bill of Operations"),
                                tr("<p>Are you sure that you want to delete "
                                   "the selected BOO?"),
				  QMessageBox::Yes,
				  QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)

  {
    q.prepare( "DELETE FROM boohead "
               "WHERE (boohead_item_id=:item_id);"
               "DELETE FROM booitem "
               "WHERE (booitem_item_id=:item_id);" );
    q.bindValue(":item_id", _boo->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    omfgThis->sBOOsUpdated(_boo->id(), TRUE);
  }
}

void booList::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void booList::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _boo->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void booList::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _boo->id());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void booList::sFillList( int pItemid, bool pLocal )
{
  QString sql(  "SELECT DISTINCT item_id,"
                " CASE WHEN "
                "  COALESCE(booitem_rev_id, -1)=-1 THEN "
                "   0 "
                " ELSE 1 "
                " END AS revcontrol, "
                " item_number,"
                "(item_descrip1 || ' ' || item_descrip2) AS descrip "
                "FROM item "
                "  LEFT OUTER JOIN booitem ON (item_id=booitem_item_id) "
                "  LEFT OUTER JOIN boohead ON (item_id=boohead_item_id) "
                "WHERE (((booitem_id IS NOT NULL) "
                "OR (boohead_id IS NOT NULL)) ");

  if (!_showInactive->isChecked())
    sql += " AND (item_active)";

  sql += ") "
         "ORDER BY item_number;";

  if ((pItemid != -1) && (pLocal))
    _boo->populate(sql, TRUE, pItemid);
  else
    _boo->populate(sql, TRUE);

  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void booList::sFillList()
{
  sFillList(-1, TRUE);
}

void booList::sPrint()
{
  ParameterList params;
  params.append("item_id", _boo->id());

  orReport report("BillOfOperations", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void booList::sSearch( const QString & pTarget )
{
  _boo->clearSelection();
  int i;
  for (i = 0; i < _boo->topLevelItemCount(); i++)
  {
    if (_boo->topLevelItem(i)->text(0).contains(pTarget, Qt::CaseInsensitive))
      break;
  }
    
  if (i < _boo->topLevelItemCount())
  {
    _boo->setCurrentItem(_boo->topLevelItem(i));
    _boo->scrollToItem(_boo->topLevelItem(i));
  }
}

void booList::sPopulateMenu( QMenu *, QTreeWidgetItem * )
{
}

void booList::sHandleButtons()
{
  if (_boo->altId() == 0)
    _delete->setEnabled(TRUE);
  else
    _delete->setEnabled(FALSE);
}
