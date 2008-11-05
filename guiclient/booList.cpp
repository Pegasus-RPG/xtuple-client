/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
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
