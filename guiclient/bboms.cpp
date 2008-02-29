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
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
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

#include "bboms.h"

#include <QVariant>
#include <QMessageBox>
#include <QStatusBar>
#include <parameter.h>
#include <QWorkspace>
#include "bbom.h"
#include "guiclient.h"

/*
 *  Constructs a bboms as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
bboms::bboms(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_bbom, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
bboms::~bboms()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void bboms::languageChange()
{
    retranslateUi(this);
}


void bboms::init()
{
  statusBar()->hide();
  
  _bbom->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft );
  _bbom->addColumn(tr("Description"), -1,          Qt::AlignLeft );

  connect(omfgThis, SIGNAL(bbomsUpdated(int, bool)), SLOT(sFillList(int, bool)));

  if (_privileges->check("MaintainBBOMs"))
  {
    connect(_bbom, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_bbom, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_bbom, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_bbom, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();

  _searchFor->setFocus();
}

void bboms::sFillList()
{
  sFillList(-1, TRUE);
}

void bboms::sFillList(int pItemid, bool pLocal)
{
  q.exec( "SELECT DISTINCT item_id, item_number, (item_descrip1 || ' ' || item_descrip2) "
                 "FROM bbomitem, item "
                 "WHERE (bbomitem_parent_item_id=item_id) "
                 "ORDER BY item_number;" );
  if (pLocal)
    _bbom->populate(q, pItemid);
  else
    _bbom->populate(q);

}

void bboms::sDelete()
{
  if ( QMessageBox::warning( this, tr("Delete Selected Breeder BOM?"),
                             tr( "Are you sure that you want to delete the selected\n"
                                 "Breeder Bill of Materials?" ),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0)
  {
    q.prepare( "DELETE FROM bbomitem "
               "WHERE (bbomitem_parent_item_id=:item_id);" );
    q.bindValue(":item_id", _bbom->id());
    q.exec();
    sFillList();
  }
}

void bboms::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bboms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("item_id", _bbom->id());

  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bboms::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("item_id", _bbom->id());

  bbom *newdlg = new bbom();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void bboms::sSearch( const QString &pTarget )
{
  _bbom->clearSelection();
  int i;
  for (i = 0; i < _bbom->topLevelItemCount(); i++)
  {
    if (_bbom->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }
    
  if (i < _bbom->topLevelItemCount())
  {
    _bbom->setCurrentItem(_bbom->topLevelItem(i));
    _bbom->scrollToItem(_bbom->topLevelItem(i));
  }
}
