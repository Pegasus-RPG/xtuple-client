/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bboms.h"

#include <QMessageBox>
#include <QSqlError>

#include <parameter.h>

#include "bbom.h"
#include "guiclient.h"

bboms::bboms(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _bbom->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number");
  _bbom->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "descrip");

  connect(omfgThis, SIGNAL(bbomsUpdated(int, bool)), SLOT(sFillList(int, bool)));

  if (_privileges->check("MaintainBBOMs"))
  {
    connect(_bbom, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_bbom, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_bbom, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_bbom, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();

  _searchFor->setFocus();
}

bboms::~bboms()
{
  // no need to delete child widgets, Qt does it all for us
}

void bboms::languageChange()
{
  retranslateUi(this);
}

void bboms::sFillList()
{
  sFillList(-1, TRUE);
}

void bboms::sFillList(int pItemid, bool pLocal)
{
  q.exec("SELECT DISTINCT item_id, item_number,"
	 "                (item_descrip1 || ' ' || item_descrip2) AS descrip "
	 "FROM bbomitem, item "
	 "WHERE (bbomitem_parent_item_id=item_id) "
	 "ORDER BY item_number;" );
  if (pLocal)
    _bbom->populate(q, pItemid);
  else
    _bbom->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void bboms::sDelete()
{
  if (QMessageBox::question( this, tr("Delete Selected Breeder BOM?"),
                             tr( "Are you sure that you want to delete the selected\n"
                                 "Breeder Bill of Materials?" ),
                             QMessageBox::Yes,
                             QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare( "DELETE FROM bbomitem "
               "WHERE (bbomitem_parent_item_id=:item_id);" );
    q.bindValue(":item_id", _bbom->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    sFillList();
  }
}

void bboms::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bboms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _bbom->id());

  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bboms::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _bbom->id());

  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bboms::sSearch( const QString &pTarget )
{
  _bbom->clearSelection();
  int i;
  for (i = 0; i < _bbom->topLevelItemCount(); i++)
  {
    if (_bbom->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }
    
  if (i < _bbom->topLevelItemCount())
  {
    _bbom->setCurrentItem(_bbom->topLevelItem(i));
    _bbom->scrollToItem(_bbom->topLevelItem(i));
  }
}
