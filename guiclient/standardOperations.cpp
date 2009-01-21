/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "standardOperations.h"

#include <qvariant.h>
#include <qmessagebox.h>
//#include <qstatusbar.h>
#include <parameter.h>
#include <openreports.h>
#include "standardOperation.h"

/*
 *  Constructs a standardOperations as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
standardOperations::standardOperations(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_stdopn, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
standardOperations::~standardOperations()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void standardOperations::languageChange()
{
    retranslateUi(this);
}


void standardOperations::init()
{
//  statusBar()->hide();
  
  if (_privileges->check("MaintainStandardOperations"))
  {
    connect(_stdopn, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_stdopn, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_stdopn, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_stdopn, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _stdopn->addColumn(tr("Number"),      _itemColumn, Qt::AlignLeft,   true,  "stdopn_number" );
  _stdopn->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "descrip" );

  sFillList();

}

void standardOperations::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void standardOperations::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("stdopn_id", _stdopn->id());

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  int stdopnid = newdlg.exec();

  if (stdopnid != XDialog::Rejected)
    sFillList(stdopnid);
}

void standardOperations::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("stdopn_id", _stdopn->id());

  standardOperation newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void standardOperations::sDelete()
{
  XSqlQuery booitemid;
  booitemid.prepare( "SELECT booitem_id "
                   "FROM booitem "
                   "WHERE (booitem_stdopn_id=:stdopn_id) "
                   "LIMIT 1;" );
  booitemid.bindValue(":stdopn_id", _stdopn->id());
  booitemid.exec();
  if (booitemid.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Standard Operation"),
                           tr( "The selected Standard Operation cannot be deleted as it is currently in use in one or more\n"
                               "Bill of Operations items.  You must remove the selected Standard Operation's use from all \n"
                               "Bills of Operations before you may delete it." ) );
    return;
  }

  XSqlQuery wooperid;
  wooperid.prepare( "SELECT wooper_id "
                    "FROM wooper "
                    "WHERE (wooper_stdopn_id=:stdopn_id) "
                    "LIMIT 1;" );
  wooperid.bindValue(":stdopn_id", _stdopn->id());
  wooperid.exec();
  if (wooperid.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Standard Operation"),
                           tr( "The selected Standard Operation cannot be deleted as it is currently in use in one or more\n"
                               "Work Order Operations.  You must remove close all Work Orders whose Operations use the selected\n"
                               "Standard Operation before you may delete it." ) );
    return;
  }

  XSqlQuery deleteOperation;
  deleteOperation.prepare( "DELETE FROM stdopn "
                           "WHERE (stdopn_id=:stdopn_id);" );
  deleteOperation.bindValue(":stdopn_id", _stdopn->id());
  deleteOperation.exec();
  sFillList();
}

void standardOperations::sFillList()
{
  sFillList(-1);
}

void standardOperations::sFillList(int pStdopnid)
{
  _stdopn->populate( "SELECT stdopn_id, stdopn_number, (stdopn_descrip1 || ' ' || stdopn_descrip2) AS descrip "
                     "FROM stdopn "
                     "ORDER BY stdopn_number;", pStdopnid );
}

void standardOperations::sPrint()
{
  orReport report("StdOperationsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

