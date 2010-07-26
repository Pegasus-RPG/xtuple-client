/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspWoMaterialsByWorkOrder.h"

#include <QVariant>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <QSqlError>

#include <metasql.h>
#include <openreports.h>
#include "mqlutil.h"
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

  _manufacturing = false;
  if (_metrics->value("Application") == "Standard")
  {
    XSqlQuery xtmfg;
    xtmfg.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtmfg'");
    if (xtmfg.first())
      _manufacturing = true;
  }

  _womatl->addColumn(tr("Component Item"),  _itemColumn,  Qt::AlignLeft,   true,  "item_number"   );
  if (_manufacturing)
    _womatl->addColumn(tr("Oper. #"),         _dateColumn,  Qt::AlignCenter, true,  "wooperseq" );
  _womatl->addColumn(tr("Iss. Meth.") ,     _orderColumn, Qt::AlignCenter, true,  "issuemethod" );
  _womatl->addColumn(tr("Iss. UOM") ,       _uomColumn,   Qt::AlignLeft,   true,  "uom_name"   );
  _womatl->addColumn(tr("Fxd. Qty."),       _qtyColumn,   Qt::AlignRight,  true,  "womatl_qtyfxd"  );
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
  XWidget::set(pParams);
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

void dspWoMaterialsByWorkOrder::setParams(ParameterList & params)
{
  params.append("wo_id", _wo->id());
  params.append("push", tr("Push"));
  params.append("pull", tr("Pull"));
  params.append("mixed", tr("Mixed"));
  params.append("error", tr("Error"));
  if (_manufacturing)
      params.append("Manufacturing");
}

void dspWoMaterialsByWorkOrder::sPrint()
{
  if (!_wo->isValid())
    return;
    
  ParameterList params;
  setParams(params);
  params.append("includeFormatted");

  orReport report("WOMaterialRequirementsByWorkOrder", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspWoMaterialsByWorkOrder::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("View Requirement..."), this, SLOT(sView()));
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
  if (!_wo->isValid())
    return;
    
  MetaSQLQuery mql = mqlLoad("workOrderMaterial", "detail");
  ParameterList params;
  setParams(params);

  q = mql.toQuery(params);
  _womatl->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

