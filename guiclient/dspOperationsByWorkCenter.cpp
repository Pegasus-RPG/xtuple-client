/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspOperationsByWorkCenter.h"

#include <QVariant>
#include <QWorkspace>
#include <QMenu>
#include <parameter.h>
#include <openreports.h>
#include "boo.h"
#include "booItem.h"

/*
 *  Constructs a dspOperationsByWorkCenter as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspOperationsByWorkCenter::dspOperationsByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wrkcnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_booitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _wrkcnt->populate( "SELECT wrkcnt_id, wrkcnt_code "
                     "FROM wrkcnt "
                     "ORDER BY wrkcnt_code;" );
	
  _booitem->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"   );
  _booitem->addColumn(tr("Seq #"),       _seqColumn,  Qt::AlignCenter, true,  "booitem_seqnumber" );
  _booitem->addColumn(tr("Std. Oper."),  _itemColumn, Qt::AlignLeft,   true,  "stdopn_number"   );
  _booitem->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "boodescrip"   );

  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), this, SLOT(sFillList()));
	
  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspOperationsByWorkCenter::~dspOperationsByWorkCenter()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspOperationsByWorkCenter::languageChange()
{
    retranslateUi(this);
}

void dspOperationsByWorkCenter::sPrint()
{
  ParameterList params;
  params.append("wrkcnt_id", _wrkcnt->id());

  orReport report("OperationsByWorkCenter", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspOperationsByWorkCenter::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Operation..."), this, SLOT(sViewOperation()), 0);
  if (!_privileges->check("ViewBOOs"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("View Bill of Operations..."), this, SLOT(sViewBOO()), 0);
  if (!_privileges->check("ViewBOOs"))
    pMenu->setItemEnabled(menuItem, FALSE);

  pMenu->insertSeparator();

  menuItem = pMenu->insertItem(tr("Edit Operation..."), this, SLOT(sEditOperation()), 0);
  if (!_privileges->check("MaintainBOOs"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Bill of Operations..."), this, SLOT(sEditBOO()), 0);
  if (!_privileges->check("MaintainBOOs"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspOperationsByWorkCenter::sViewOperation()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("booitem_id", _booitem->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspOperationsByWorkCenter::sEditOperation()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("booitem_id", _booitem->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspOperationsByWorkCenter::sViewBOO()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _booitem->altId());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspOperationsByWorkCenter::sEditBOO()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _booitem->altId());

  boo *newdlg = new boo();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspOperationsByWorkCenter::sFillList()
{
  q.prepare( "SELECT wrkcnt_descrip, warehous_code "
             "FROM wrkcnt, warehous "
             "WHERE ( (wrkcnt_warehous_id=warehous_id)"
             " AND (wrkcnt_id=:wrkcnt_id) );" );
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  if (q.first())
  {
    _description->setText(q.value("wrkcnt_descrip").toString());
    _warehouse->setText(q.value("warehous_code").toString());
  }

  q.prepare( "SELECT booitem_id, item_id,"
             "       item_number, booitem_seqnumber,"
             "       stdopn_number, (booitem_descrip1 || ' ' || booitem_descrip2) AS boodescrip "
             "FROM booitem JOIN item ON (booitem_item_id=item_id) "
             " LEFT OUTER JOIN stdopn ON (booitem_stdopn_id=stdopn_id) "
             "WHERE (booitem_wrkcnt_id=:wrkcnt_id) "
             "ORDER BY item_number, booitem_seqnumber;" );
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  _booitem->populate(q, TRUE );
}
