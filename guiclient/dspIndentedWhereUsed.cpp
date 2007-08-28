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

#include "dspIndentedWhereUsed.h"

#include <qvariant.h>
#include <qstatusbar.h>
#include <qworkspace.h>
#include "dspInventoryHistoryByItem.h"
#include "rptIndentedWhereUsed.h"

/*
 *  Constructs a dspIndentedWhereUsed as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspIndentedWhereUsed::dspIndentedWhereUsed(QWidget* parent, const char* name, Qt::WFlags fl)
    : QMainWindow(parent, name, fl)
{
    setupUi(this);

    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_bomitem, SIGNAL(populateMenu(Q3PopupMenu*,Q3ListViewItem*,int)), this, SLOT(sPopulateMenu(Q3PopupMenu*)));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspIndentedWhereUsed::~dspIndentedWhereUsed()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspIndentedWhereUsed::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <Q3PopupMenu>

void dspIndentedWhereUsed::init()
{
  statusBar()->hide();

  if (_metrics->boolean("AllowInactiveBomItems"))
    _item->setType(ItemLineEdit::cGeneralComponents);
  else
    _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

  _bomitem->setRootIsDecorated(TRUE);
  _bomitem->addColumn(tr("Seq. #"),        80,           Qt::AlignCenter );
  _bomitem->addColumn(tr("Item Number"),   _itemColumn,  Qt::AlignLeft   );
  _bomitem->addColumn(tr("Description"),   -1,           Qt::AlignLeft   );
  _bomitem->addColumn(tr("UOM"),           _uomColumn,   Qt::AlignCenter );
  _bomitem->addColumn(tr("Ext. Qty. Per"), _qtyColumn,   Qt::AlignRight  );
  _bomitem->addColumn(tr("Scrap %"),       _prcntColumn, Qt::AlignRight  );
  _bomitem->addColumn(tr("Effective"),     _dateColumn,  Qt::AlignCenter );
  _bomitem->addColumn(tr("Expires"),       _dateColumn,  Qt::AlignCenter );
  _bomitem->setTreeStepSize(10);

  _item->setFocus();
}

enum SetResponse dspIndentedWhereUsed::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  if (pParams.inList("run"))
  {
    sFillList();
    return NoError_Print;
  }

  return NoError;
}

void dspIndentedWhereUsed::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("print");

  if (_showExpired->isChecked())
    params.append("showExpired");

  if (_showFuture->isChecked())
    params.append("showFuture");
  
  rptIndentedWhereUsed newdlg(this, "", TRUE);
  newdlg.set(params);
}

void dspIndentedWhereUsed::sViewInventoryHistory()
{
  ParameterList params;
  params.append("item_id", _bomitem->altId());
  params.append("run");

  dspInventoryHistoryByItem *newdlg = new dspInventoryHistoryByItem();
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void dspIndentedWhereUsed::sPopulateMenu(Q3PopupMenu *menu)
{
  int menuItem;

  menuItem = menu->insertItem(tr("View Item Inventory History..."), this, SLOT(sViewInventoryHistory()), 0);
  if (!_privleges->check("ViewInventoryHistory"))
    menu->setItemEnabled(menuItem, FALSE);
}

void dspIndentedWhereUsed::sFillList()
{
  _bomitem->clear();

  if (_item->isValid())
  {
    q.prepare("SELECT indentedWhereUsed(:item_id) AS workset_id;");
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      int worksetid = q.value("workset_id").toInt();

      QString sql( "SELECT bomwork_id, item_id, bomwork_parent_id,"
                   "       bomwork_seqnumber, item_number, item_invuom,"
                   "       (item_descrip1 || ' ' || item_descrip2) AS itemdescription,"
                   "       formatQtyPer(bomwork_qtyper) AS qtyper,"
                   "       formatScrap(bomwork_scrap) AS scrap,"
                   "       formatDate(bomwork_effective, 'Always') AS effective,"
                   "       formatDate(bomwork_expires, 'Never') AS expires "
                   "FROM bomwork, item "
                   "WHERE ( (bomwork_item_id=item_id)"
                   " AND (bomwork_set_id=:bomwork_set_id)" );

      if (!_showExpired->isChecked())
        sql += " AND (bomwork_expires > CURRENT_DATE)";

      if (!_showFuture->isChecked())
        sql += " AND (bomwork_effective <= CURRENT_DATE)";

      sql += ") "
             "ORDER BY bomwork_level, item_number DESC;";

      q.prepare(sql);
      q.bindValue(":bomwork_set_id", worksetid);
      q.exec();
      while (q.next())
      {
        if (q.value("bomwork_parent_id").toInt() == -1)
          new XListViewItem( _bomitem, q.value("bomwork_id").toInt(), q.value("item_id").toInt(),
                             q.value("bomwork_seqnumber"), q.value("item_number"),
                             q.value("itemdescription"), q.value("item_invuom"),
                             q.value("qtyper"), q.value("scrap"),
                             q.value("effective"), q.value("expires") );
        else
        {
          XListViewItem *cursor = _bomitem->firstChild();
          if (cursor)
          {
            do
            {
              if (cursor->id() == q.value("bomwork_parent_id").toInt())
              {
                new XListViewItem( cursor, q.value("bomwork_id").toInt(), q.value("item_id").toInt(),
                                   q.value("bomwork_seqnumber"), q.value("item_number"),
                                   q.value("itemdescription"), q.value("item_invuom"),
                                   q.value("qtyper"), q.value("scrap"),
                                   q.value("effective"), q.value("expires") );
                cursor->setOpen(TRUE);

                break;
              }
            }
            while ((cursor = cursor->itemBelow()) != NULL);
          }
        }
      }

      q.prepare("SELECT deleteBOMWorkset(:workset_id) AS result;");
      q.bindValue(":bomwork_set_id", worksetid);
      q.exec();
    }
  }
}
