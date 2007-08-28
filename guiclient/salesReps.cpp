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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "salesReps.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <parameter.h>
#include <openreports.h>
#include "salesRep.h"
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a salesReps as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
salesReps::salesReps(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_salesrep, SIGNAL(populateMenu(Q3PopupMenu *, Q3ListViewItem *, int)), this, SLOT(sPopulateMenu(Q3PopupMenu*)));
    connect(_salesrep, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_salesrep, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
salesReps::~salesReps()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void salesReps::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void salesReps::init()
{
  statusBar()->hide();
  
  if (_privleges->check("MaintainSalesReps"))
  {
    connect(_salesrep, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_salesrep, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_salesrep, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_salesrep, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _salesrep->addColumn(tr("Number"), 70,  Qt::AlignLeft   );
  _salesrep->addColumn(tr("Name"),   -1,  Qt::AlignLeft   );
  _salesrep->addColumn(tr("Active"), 50,  Qt::AlignCenter );

  sFillList();
}

void salesReps::sDelete()
{
  q.prepare( "SELECT cust_id "
             "FROM cust "
             "WHERE (cust_salesrep_id=:salesrep_id) "
             "LIMIT 1;" );
  q.bindValue(":salesrep_id", _salesrep->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Selected Sales Rep."),
                           tr( "The selected Sales Rep. cannot be deleted as he/she is still assigned\n"
                               "to one or more Customers.  You must reassign different Sales Reps. to all\n"
                               "Customers to which the selected Sales Rep. is assigned before you may\n"
                               "delete the selected Sales Rep." ) );
    return;
  }

  q.prepare( "SELECT shipto_id "
             "FROM shipto "
             "WHERE (shipto_salesrep_id=:salesrep_id) "
             "LIMIT 1;" );
  q.bindValue(":salesrep_id", _salesrep->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Selected Sales Rep."),
                           tr( "The selected Sales Rep. cannot be deleted as he/she is still assigned\n"
                               "to one or more Ship-tos.  You must reassign different Sales Reps. to all\n"
                               "Ship-tos to which the selected Sales Rep. is assigned before you may\n"
                               "delete the selected Sales Rep." ) );
    return;
  }

  q.prepare( "SELECT aropen_id "
             "FROM aropen "
             "WHERE (aropen_salesrep_id=:salesrep_id) "

             "UNION SELECT cohead_id "
             "FROM cohead "
             "WHERE (cohead_salesrep_id=:salesrep_id) "
 
             "UNION SELECT cmhead_id "
             "FROM cmhead "
             "WHERE (cmhead_salesrep_id=:salesrep_id) "

             "UNION SELECT cohist_id "
             "FROM cohist "
             "WHERE (cohist_salesrep_id=:salesrep_id) "

             "LIMIT 1;" );
  q.bindValue(":salesrep_id", _salesrep->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::critical( this, tr("Cannot Delete Selected Sales Rep."),
                           tr( "The selected Sales Rep. cannot be deleted as there has been sales history\n"
                               "recorded against him/her.  You may edit and set the selected Sales Rep's\n"
                               "active status to inactive." ) );
    return;
  }

  q.prepare( "DELETE FROM salesrep "
             "WHERE (salesrep_id=:salesrep_id);" );
  q.bindValue(":salesrep_id", _salesrep->id());
  q.exec();

  sFillList();
}

void salesReps::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void salesReps::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("salesrep_id", _salesrep->id());

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void salesReps::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("salesrep_id", _salesrep->id());

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void salesReps::sFillList()
{
  _salesrep->populate( "SELECT salesrep_id, salesrep_number, salesrep_name, formatBoolYN(salesrep_active) "
                       "FROM salesrep "
                       "ORDER BY salesrep_number;" );
}

void salesReps::sPopulateMenu( Q3PopupMenu * menu )
{
  int menuItem;

  menuItem = menu->insertItem(tr("Edit Sales Rep..."), this, SLOT(sEdit()), 0);
  if (!_privleges->check("MaintainSalesReps"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Sales Rep..."), this, SLOT(sView()), 0);
  if ((!_privleges->check("MaintainSalesReps")) && (!_privleges->check("ViewSalesReps")))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Delete Sales Rep..."), this, SLOT(sDelete()), 0);
  if (!_privleges->check("MaintainSalesReps"))
    menu->setItemEnabled(menuItem, FALSE);
}

void salesReps::sPrint()
{
  orReport report("SalesRepsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

