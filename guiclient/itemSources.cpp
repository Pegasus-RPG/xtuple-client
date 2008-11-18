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
 * The Original Code is xTuple ERP: PostBooks Edition 
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
 * Powered by xTuple ERP: PostBooks Edition
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

#include "itemSources.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QMenu>
#include <metasql.h>
#include <parameter.h>
#include <openreports.h>
#include "itemSource.h"

/*
 *  Constructs a itemSources as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
itemSources::itemSources(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_itemsrc, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_searchFor, SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));
    connect(_showInactive, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
itemSources::~itemSources()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void itemSources::languageChange()
{
    retranslateUi(this);
}


void itemSources::init()
{
//  statusBar()->hide();
  
  _itemsrc->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft, true, "item_number" );
  _itemsrc->addColumn(tr("Description"), -1,          Qt::AlignLeft, true, "item_descrip" );
  _itemsrc->addColumn(tr("Vendor"),      _itemColumn, Qt::AlignLeft, true, "vend_name" );
  _itemsrc->addColumn(tr("Vendor Item"), _itemColumn, Qt::AlignLeft, true, "itemsrc_vend_item_number" );
  _itemsrc->addColumn(tr("Manufacturer"), _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_name" );
  _itemsrc->addColumn(tr("Manuf. Item#"), _itemColumn, Qt::AlignLeft, true, "itemsrc_manuf_item_number" );

  if (_privileges->check("MaintainItemSources"))
  {
    connect(_itemsrc, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_itemsrc, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_itemsrc, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
  _searchFor->setFocus();
}

void itemSources::sPrint()
{
  orReport report("ItemSourceMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void itemSources::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void itemSources::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void itemSources::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("itemsrc_id", _itemsrc->id());

  itemSource newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void itemSources::sDelete()
{
  q.prepare("SELECT poitem_id, itemsrc_active "
            "FROM poitem, itemsrc "
            "WHERE ((poitem_itemsrc_id=:itemsrc_id) "
            "AND (itemsrc_id=:itemsrc_id)); ");
  q.bindValue(":itemsrc_id", _itemsrc->id());
  q.exec();
  if (q.first())
  {
    if (q.value("itemsrc_active").toBool())
    {
      if (QMessageBox::question( this, tr("Delete Item Source"),
                                    tr( "This item source is used by existing purchase order records"
                                    " and may not be deleted.  Would you like to deactivate it instead?"),
                                    tr("&Ok"), tr("&Cancel"), 0, 0, 1 ) == 0  )
      {
        q.prepare( "UPDATE itemsrc SET "
                   "  itemsrc_active=false "
                   "WHERE (itemsrc_id=:itemsrc_id);" );
        q.bindValue(":itemsrc_id", _itemsrc->id());
        q.exec();

        sFillList();
      }
    }
    else
      QMessageBox::critical( this, tr("Delete Item Source"), tr("This item source is used by existing "
                          "purchase order records and may not be deleted."));
    return;
  }
            
  q.prepare( "SELECT item_number "
             "FROM itemsrc, item "
             "WHERE ( (itemsrc_item_id=item_id)"
             " AND (itemsrc_id=:itemsrc_id) );" );
  q.bindValue(":itemsrc_id", _itemsrc->id());
  q.exec();
  if (q.first())
  {
    if (QMessageBox::information( this, tr("Delete Item Source"),
                                  tr( "Are you sure that you want to delete the Item Source for %1?")
                                  .arg(q.value("item_number").toString()),
                                  tr("&Delete"), tr("&Cancel"), 0, 0, 1 ) == 0  )
    {
      q.prepare( "DELETE FROM itemsrc "
                 "WHERE (itemsrc_id=:itemsrc_id);"
                 "DELETE FROM itemsrcp "
                 "WHERE (itemsrcp_itemsrc_id=:itemsrc_id);" );
      q.bindValue(":itemsrc_id", _itemsrc->id());
      q.exec();

      sFillList();
    }
  }
}

void itemSources::sPopulateMenu(QMenu *menuThis)
{
  int intMenuItem;

  intMenuItem = menuThis->insertItem(tr("Edit Item Source..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainItemSource"))
    menuThis->setItemEnabled(intMenuItem, FALSE);

  intMenuItem = menuThis->insertItem(tr("View Item Source..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainItemSource")) && (!_privileges->check("ViewItemSource")))
    menuThis->setItemEnabled(intMenuItem, FALSE);

  intMenuItem = menuThis->insertItem(tr("Delete Item Source..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainItemSource"))
    menuThis->setItemEnabled(intMenuItem, FALSE);
}

void itemSources::sFillList()
{
  QString sql( "SELECT itemsrc_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS item_descrip,"
               "       vend_name, itemsrc_vend_item_number, itemsrc_manuf_name, "
               "       itemsrc_manuf_item_number "
               "FROM item, vend, itemsrc "
               "WHERE ( (itemsrc_item_id=item_id)"
               " AND (itemsrc_vend_id=vend_id)"
               "<? if exists(\"onlyShowActive\") ?>"
               " AND (itemsrc_active)"
               "<? endif ?>"
               ") ORDER BY item_number, vend_name;" );
               
  ParameterList params;
  if (!_showInactive->isChecked())
    params.append("onlyShowActive");

  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);

  if (q.first())
    _itemsrc->populate(q);
  else
    _itemsrc->clear();
}

void itemSources::sSearch( const QString &pTarget )
{
  _itemsrc->clearSelection();
  int i;
  for (i = 0; i < _itemsrc->topLevelItemCount(); i++)
  {
    if ( (_itemsrc->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(1).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(2).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(3).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(4).startsWith(pTarget, Qt::CaseInsensitive)) ||
         (_itemsrc->topLevelItem(i)->text(5).startsWith(pTarget, Qt::CaseInsensitive)) )
      break;
  }

  if (i < _itemsrc->topLevelItemCount())
  {
    _itemsrc->setCurrentItem(_itemsrc->topLevelItem(i));
    _itemsrc->scrollToItem(_itemsrc->topLevelItem(i));
  }
}

