/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoOperationsByWorkOrder.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QMenu>
#include <openreports.h>
#include "inputManager.h"
#include "woOperation.h"

/*
 *  Constructs a dspWoOperationsByWorkOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoOperationsByWorkOrder::dspWoOperationsByWorkOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_wooper, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_wo, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wooper->addColumn( tr("Seq #"),         _seqColumn,  Qt::AlignCenter, true,  "wooper_seqnumber" );
  _wooper->addColumn( tr("Scheduled"),     _dateColumn, Qt::AlignCenter, true,  "scheduled" );
  _wooper->addColumn( tr("Work Center"),   _itemColumn, Qt::AlignLeft,   true,  "workcenter"   );
  _wooper->addColumn( tr("Std. Oper."),    _itemColumn, Qt::AlignLeft,   true,  "stdoper"   );
  _wooper->addColumn( tr("Description"),   -1,          Qt::AlignLeft,   true,  "descrip"   );
  _wooper->addColumn( tr("Setup Remain."), _itemColumn, Qt::AlignRight,  true,  "setup"  );
  _wooper->addColumn( tr("Run Remain."),   _itemColumn, Qt::AlignRight,  true,  "run"  );
  _wooper->addColumn( tr("Qty. Complete"), _qtyColumn,  Qt::AlignRight,  true,  "wooper_qtyrcv"  );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoOperationsByWorkOrder::~dspWoOperationsByWorkOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoOperationsByWorkOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspWoOperationsByWorkOrder::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
    _wo->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Run;
  }

  return NoError;
}

void dspWoOperationsByWorkOrder::sPrint()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  orReport report("WOOperationsByWorkOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoOperationsByWorkOrder::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Operation..."), this, SLOT(sViewOperation()), 0);
  if ((!_privileges->check("ViewWoOperations")) && (!_privileges->check("MaintainWoOperations")))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Edit Operation..."), this, SLOT(sEditOperation()), 0);
  if (!_privileges->check("MaintainWoOperations"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem(tr("Delete Operation..."), this, SLOT(sDeleteOperation()), 0);
  if (!_privileges->check("MaintainWoOperations"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void dspWoOperationsByWorkOrder::sViewOperation()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoOperationsByWorkOrder::sEditOperation()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("wooper_id", _wooper->id());

  woOperation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void dspWoOperationsByWorkOrder::sDeleteOperation()
{
  if (QMessageBox::critical( this, tr("Delete W/O Operation"),
                             tr( "If you Delete the selected W/O Operation\n"
                                 "you will not be able to post Labor to this Operation\n"
                                 "to this W/O.  Are you sure that you want to delete the\n"
                                 "selected W/O Operation?"),
                                 tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "DELETE FROM wooper "
               "WHERE (wooper_id=:wooper_id);" );
    q.bindValue(":wooper_id", _wooper->id());
    q.exec();

    sFillList();
  }
}

void dspWoOperationsByWorkOrder::sFillList()
{
  q.prepare( "SELECT wooper.*,"
             "       CAST(wooper_scheduled AS DATE) AS scheduled,"
             "       CASE WHEN (wooper_wrkcnt_id <> -1) THEN (SELECT wrkcnt_code FROM wrkcnt WHERE (wrkcnt_id=wooper_wrkcnt_id))"
             "            ELSE 'Any'"
             "       END AS workcenter,"
             "       CASE WHEN (wooper_stdopn_id <> -1) THEN (SELECT stdopn_number FROM stdopn WHERE (stdopn_id=wooper_stdopn_id))"
             "            ELSE ''"
             "       END AS stdoper,"
             "       (wooper_descrip1 || ' ' || wooper_descrip2) AS descrip,"
             "       CASE WHEN (wooper_sucomplete) THEN 0"
             "            ELSE noNeg(wooper_sutime - wooper_suconsumed)"
             "       END AS setup,"
             "       CASE WHEN (wooper_rncomplete) THEN 0"
             "            ELSE noNeg(wooper_rntime - wooper_rnconsumed)"
             "       END AS run,"
             "       '1' AS setup_xtnumericrole,"
             "       '1' AS run_xtnumericrole,"
             "       CASE WHEN (wooper_sucomplete) THEN :complete END AS setup_qtdisplayrole,"
             "       CASE WHEN (wooper_rncomplete) THEN :complete END AS run_qtdisplayrole,"
             "       'qty' AS wooper_qtyrcv_xtnumericrole "
             "FROM wooper "
             "WHERE (wooper_wo_id=:wo_id) "
             "ORDER BY wooper_seqnumber;" );
  q.bindValue(":complete", tr("Complete"));
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  _wooper->populate(q);
}
