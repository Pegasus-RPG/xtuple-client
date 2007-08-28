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


void assortments::init()
{
  statusBar()->hide();
  
  _ass->addColumn(tr("Item Number"), _itemColumn, AlignLeft );
  _ass->addColumn(tr("Description"), -1,          AlignLeft );
  
  connect(omfgThis, SIGNAL(assortmentsUpdated(int, bool)), SLOT(sFillList(int, bool)));
  
  if (_privleges->check("MaintainBOMs"))
  {
    connect(_ass, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ass, SIGNAL(valid(bool)), _copy, SLOT(setEnabled(bool)));
    connect(_ass, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

    connect(_ass, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_ass, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList(-1, FALSE);
}

void assortments::sCopy()
{
  ParameterList params;
  params.append("item_id", _ass->id());

  copyAssortment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void assortments::sDelete()
{
  if (QMessageBox::critical( this, tr("Delete Assortment Definition"),
                             tr("Are you sure that you want to delete the definition for the selected Assortment?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1) == 0)
  {
    q.prepare( "DELETE FROM assitem "
               "WHERE (assitem_parent_item_id=:item_id);"

               "DELETE FROM asshead "
               "WHERE (asshead_item_id=:item_id);" );
    q.bindValue(":item_id", _ass->id());
    q.exec();

    omfgThis->sAssortmentsUpdated(-1, TRUE);
  }
}

void assortments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _ass->id());

  assortment *newdlg = new assortment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void assortments::sFillList( int pItemid, bool pLocal )
{
  QString sql;

  sql = "SELECT DISTINCT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) "
        "FROM item, assitem "
        "WHERE ( (assitem_parent_item_id=item_id)";

  if (!_showInactive->isChecked())
    sql += " AND (item_active)";

  sql += ") "
         "ORDER BY item_number;";

  if ((pItemid != -1) && (pLocal))
    _ass->populate(sql, pItemid);
  else
    _ass->populate(sql);
}

void assortments::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  assortment *newdlg = new assortment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void assortments::sPrint()
{
  ParameterList params;
  params.append( "item_id", _ass->id() );
  params.append( "print",   TRUE       );

  rptSingleLevelBOM newdlg(this, "", TRUE);
  newdlg.set(params);
}

void assortments::sSearch( const QString &pTarget )
{
  QListViewItem *target;
    
  if (_ass->selectedItem())
    _ass->setSelected(_ass->selectedItem(), FALSE);
    
  _ass->clearSelection();
    
  target = _ass->firstChild();
  while ((target != NULL) && (pTarget.upper() != target->text(0).left(pTarget.length())))
    target = target->nextSibling();
    
  if (target != NULL)
  {
    _ass->setSelected(target, TRUE);
    _ass->ensureItemVisible(target);
  }
}

void assortments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _ass->id());

  assortment *newdlg = new assortment();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void assortments::sFillList()
{
  sFillList(-1, TRUE);
}
