/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspStandardOperationsByWorkCenter.h"

#include <QMenu>

#include <openreports.h>
#include <parameter.h>

#include "standardOperation.h"

dspStandardOperationsByWorkCenter::dspStandardOperationsByWorkCenter(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wrkcnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_stdopn, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));

  _stdopn->addColumn(tr("Std. Oper. #"), _itemColumn, Qt::AlignLeft,   true,  "stdopn_number"  );
  _stdopn->addColumn(tr("Description"),  -1,          Qt::AlignLeft,   true,  "stdopndescrip"  );

  sFillList();
}

dspStandardOperationsByWorkCenter::~dspStandardOperationsByWorkCenter()
{
  // no need to delete child widgets, Qt does it all for us
}

void dspStandardOperationsByWorkCenter::languageChange()
{
  retranslateUi(this);
}

void dspStandardOperationsByWorkCenter::sPrint()
{
  ParameterList params;
  params.append("wrkcnt_id", _wrkcnt->id());

  orReport report("StandardOperationsByWorkCenter", params);

  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspStandardOperationsByWorkCenter::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  pMenu->insertItem(tr("View Standard Operation..."), this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem(tr("Edit Standard Operation..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainStandardOperations"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspStandardOperationsByWorkCenter::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdopn_id", _stdopn->id());

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspStandardOperationsByWorkCenter::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdopn_id", _stdopn->id());

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspStandardOperationsByWorkCenter::sFillList()
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
//  ToDo

  q.prepare( "SELECT stdopn_id, stdopn_number,"
             "       (stdopn_descrip1 || ' ' || stdopn_descrip2) AS stdopndescrip "
             "FROM stdopn "
             "WHERE (stdopn_wrkcnt_id=:wrkcnt_id) "
             "ORDER BY stdopn_number;" );
  q.bindValue(":wrkcnt_id", _wrkcnt->id());
  q.exec();
  _stdopn->populate(q);
}
