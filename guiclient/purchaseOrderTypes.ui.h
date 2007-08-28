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


void purchaseOrderTypes::init()
{
  statusBar()->hide();
  
  if (_privleges->check("MaintainClassCodes"))
  {
    connect(_potype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_potype, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_potype, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_potype, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
  }

  _potype->addColumn(tr("Name"),        _itemColumn, AlignLeft );
  _potype->addColumn(tr("Description"), -1,          AlignLeft );

  sFillList();
}

void purchaseOrderTypes::sFillList()
{
  _potype->populate( "SELECT potype_id, potype_name, potype_descrip "
                     "FROM potype "
                     "ORDER BY potype_name;" );
}

void purchaseOrderTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  purchaseOrderType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void purchaseOrderTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("potype_id", _potype->id());

  purchaseOrderType newdlg(this, "", TRUE);
  newdlg.set(params);
  
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void purchaseOrderTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("potype_id", _potype->id());

  purchaseOrderType newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void purchaseOrderTypes::sDelete()
{
  q.prepare( "DELETE FROM potype "
             "WHERE (potype_id=:potype_id);" );
  q.bindValue(":potype_id", _potype->id());
  q.exec();

  sFillList();
}

void purchaseOrderTypes::sPopulateMenu(QPopupMenu *)
{
}

void purchaseOrderTypes::sPrint()
{
  orReport report("PurchaseOrderTypes");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

