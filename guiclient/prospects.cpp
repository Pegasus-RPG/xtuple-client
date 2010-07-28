/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "prospects.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "prospect.h"
#include "storedProcErrorLookup.h"

prospects::prospects(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete,   SIGNAL(clicked()),	this, SLOT(sDelete()));
  connect(_edit,     SIGNAL(clicked()),	this, SLOT(sEdit()));
  connect(_new,	     SIGNAL(clicked()),	this, SLOT(sNew()));
  connect(_prospect, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)),
                                        this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view,     SIGNAL(clicked()),	this, SLOT(sView()));

  if (_privileges->check("MaintainProspectMasters"))
  {
    connect(_prospect, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_prospect, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_prospect, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_prospect, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _prospect->addColumn(tr("Number"),  _orderColumn, Qt::AlignCenter, true, "prospect_number" );
  _prospect->addColumn(tr("Name"),    -1,           Qt::AlignLeft,   true, "prospect_name"   );
  _prospect->addColumn(tr("Address"), 175,          Qt::AlignLeft,   true, "addr_line1"   );
  _prospect->addColumn(tr("Phone #"), 100,          Qt::AlignLeft,   true, "cntct_phone"   );

  connect(omfgThis, SIGNAL(prospectsUpdated()), SLOT(sFillList()));

  sFillList();
}

prospects::~prospects()
{
    // no need to delete child widgets, Qt does it all for us
}

void prospects::languageChange()
{
    retranslateUi(this);
}

void prospects::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospects::sEdit()
{
  ParameterList params;
  params.append("prospect_id", _prospect->id());
  params.append("mode", "edit");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospects::sView()
{
  ParameterList params;
  params.append("prospect_id", _prospect->id());
  params.append("mode", "view");

  prospect *newdlg = new prospect();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void prospects::sDelete()
{
  q.prepare("SELECT deleteProspect(:prospect_id) AS result;");
  q.bindValue(":prospect_id", _prospect->id());
  q.exec();
  if (q.first())
  {
    int returnVal = q.value("result").toInt();
    if (returnVal < 0)
    {
        systemError(this, storedProcErrorLookup("deleteProspect", returnVal),
		    __FILE__, __LINE__);
        return;
    }
    omfgThis->sProspectsUpdated();
    omfgThis->sCrmAccountsUpdated(-1);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void prospects::sPopulateMenu(QMenu *pMenu)
{
  QAction *menuItem;

  menuItem = pMenu->addAction("View...", this, SLOT(sView()));

  menuItem = pMenu->addAction("Edit...", this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainProspectMasters"));

  menuItem = pMenu->addAction("Delete", this, SLOT(sDelete()));
  menuItem->setEnabled(_privileges->check("MaintainProspectMasters"));
}

void prospects::sFillList()
{
  _prospect->clear();
  q.prepare( "SELECT prospect_id, prospect_number, prospect_name, addr_line1, cntct_phone "
             "FROM prospect LEFT OUTER JOIN "
	     "     cntct ON (prospect_cntct_id=cntct_id) LEFT OUTER JOIN "
	     "     addr  ON (cntct_addr_id=addr_id) "
             "ORDER BY prospect_number;" );
  q.exec();
  _prospect->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}
