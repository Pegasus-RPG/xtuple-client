/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "vendors.h"

#include <QMessageBox>
#include <QMenu>

#include <openreports.h>
#include "vendor.h"
#include "storedProcErrorLookup.h"

vendors::vendors(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,   SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_new,    SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_print,  SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_vendor, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view,   SIGNAL(clicked()), this, SLOT(sView()));

  _vendor->addColumn(tr("Type"),   _itemColumn, Qt::AlignCenter, true, "vendtype_code");
  _vendor->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft,   true, "vend_number");
  _vendor->addColumn(tr("Name"),   -1,          Qt::AlignLeft,   true, "vend_name");

  connect(omfgThis, SIGNAL(vendorsUpdated()), SLOT(sFillList()));

  if (_privileges->check("MaintainVendors"))
  {
    connect(_vendor, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_vendor, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_vendor, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_vendor, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

vendors::~vendors()
{
  // no need to delete child widgets, Qt does it all for us
}

void vendors::languageChange()
{
  retranslateUi(this);
}

void vendors::sPrint()
{
  orReport report("VendorMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void vendors::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendors::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("vend_id", _vendor->id());
  params.append("showNextPrev");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendors::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("vend_id", _vendor->id());
  params.append("showNextPrev");

  vendor *newdlg = new vendor();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void vendors::sCopy()
{
}

void vendors::sDelete()
{
  QString question = tr("Are you sure that you want to delete this vendor?");
  if (QMessageBox::question(this, tr("Delete Vendor?"),
                              question,
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  q.prepare("SELECT deleteVendor(:vend_id) AS result;");
  q.bindValue(":vend_id", _vendor->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      QMessageBox::critical( this, tr("Cannot Delete Vendor"),
			     storedProcErrorLookup("deleteVendor", result));
      return;
    }
    omfgThis->sVendorsUpdated();
    sFillList();
  }
}

void vendors::sPopulateMenu(QMenu *menuThis)
{
  int intMenuItem;

  intMenuItem = menuThis->insertItem(tr("Edit Vendor..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainVendors"))
    menuThis->setItemEnabled(intMenuItem, FALSE);

  intMenuItem = menuThis->insertItem(tr("View Vendor..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainVendors")) && (!_privileges->check("ViewVendors")))
    menuThis->setItemEnabled(intMenuItem, FALSE);

  intMenuItem = menuThis->insertItem(tr("Delete Vendor..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainVendors"))
    menuThis->setItemEnabled(intMenuItem, FALSE);
}

void vendors::sFillList()
{
  _vendor->populate( "SELECT vend_id, * "
                     "FROM vendinfo, vendtype "
                     "WHERE (vend_vendtype_id=vendtype_id) "
                     "ORDER BY vend_number;" );
}
