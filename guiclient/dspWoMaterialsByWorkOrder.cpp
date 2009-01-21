/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoMaterialsByWorkOrder.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QMenu>
#include <openreports.h>
#include "inputManager.h"
#include "woMaterialItem.h"

/*
 *  Constructs a dspWoMaterialsByWorkOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspWoMaterialsByWorkOrder::dspWoMaterialsByWorkOrder(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_womatl, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));

  _wo->setType(cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _womatl->addColumn(tr("Component Item"),  _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  _womatl->addColumn(tr("Oper. #"),         _dateColumn,  Qt::AlignCenter, true,  "wooperseq" );
  _womatl->addColumn(tr("Iss. Meth.") ,     _orderColumn, Qt::AlignCenter, true,  "issuemethod" );
  _womatl->addColumn(tr("Iss. UOM") ,       _uomColumn,   Qt::AlignLeft,   true,  "uom_name"   );
  _womatl->addColumn(tr("Qty. Per"),        _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyper"  );
  _womatl->addColumn(tr("Scrap %"),         _prcntColumn, Qt::AlignRight,  true,  "womatl_scrap"  );
  _womatl->addColumn(tr("Required"),        _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyreq"  );
  _womatl->addColumn(tr("Issued"),          _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyiss"  );
  _womatl->addColumn(tr("Scrapped"),        _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtywipscrap"  );
  _womatl->addColumn(tr("Balance"),         _qtyColumn,   Qt::AlignRight,  true,  "balance"  );
  _womatl->addColumn(tr("Due Date"),        _dateColumn,  Qt::AlignCenter, true,  "womatl_duedate" );
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspWoMaterialsByWorkOrder::~dspWoMaterialsByWorkOrder()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspWoMaterialsByWorkOrder::languageChange()
{
  retranslateUi(this);
}

enum SetResponse dspWoMaterialsByWorkOrder::set(const ParameterList &pParams)
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

void dspWoMaterialsByWorkOrder::sPrint()
{
  ParameterList params;
  params.append("wo_id", _wo->id());

  orReport report("WOMaterialRequirementsByWorkOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoMaterialsByWorkOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View Requirement..."), this, SLOT(sView()), 0);
}

void dspWoMaterialsByWorkOrder::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("womatl_id", _womatl->id());
  
  woMaterialItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void dspWoMaterialsByWorkOrder::sFillList()
{
  if (!checkParameters())
    return;

  _womatl->clear();

  if (_wo->isValid())
  {
    q.prepare( "SELECT womatl.*, item_number, formatwooperseq(womatl_wooper_id) AS wooperseq,"
               "       CASE WHEN (womatl_issuemethod = 'S') THEN :push"
               "            WHEN (womatl_issuemethod = 'L') THEN :pull"
               "            WHEN (womatl_issuemethod = 'M') THEN :mixed"
               "            ELSE :error"
               "       END AS issuemethod,"
               "       uom_name,"
               "       noNeg(womatl_qtyreq - womatl_qtyiss) AS balance,"
               "       CASE WHEN (womatl_duedate <= CURRENT_DATE) THEN 'error' END AS womatl_duedate_qtforegroundrole,"
               "       'qtyper' AS womatl_qtyper_xtnumericrole,"
               "       'percent' AS womatl_scrap_xtnumericrole,"
               "       'qty' AS womatl_qtyreq_xtnumericrole,"
               "       'qty' AS womatl_qtyiss_xtnumericrole,"
               "       'qty' AS womatl_qtywipscrap_xtnumericrole,"
               "       'qty' AS balance_xtnumericrole,"
               "       0 AS balance_xttotalrole "
               "FROM wo, womatl, itemsite, item, uom "
               "WHERE ( (womatl_wo_id=wo_id)"
               " AND (womatl_uom_id=uom_id)"
               " AND (womatl_itemsite_id=itemsite_id)"
               " AND (itemsite_item_id=item_id)"
               " AND (wo_id=:wo_id) ) "
               "ORDER BY wooperseq, item_number;" );
    q.bindValue(":push", tr("Push"));
    q.bindValue(":pull", tr("Pull"));
    q.bindValue(":mixed", tr("Mixed"));
    q.bindValue(":error", tr("Error"));
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    _womatl->populate(q);
  }
}

bool dspWoMaterialsByWorkOrder::checkParameters()
{
  return TRUE;
}
