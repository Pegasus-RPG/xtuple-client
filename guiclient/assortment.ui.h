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


void assortment::init()
{
  statusBar()->hide();

  _item->setType(cItemAssortments);

  _assitem->addColumn(tr("#"),            _seqColumn,   AlignCenter );
  _assitem->addColumn(tr("Item Number"),  _itemColumn,  AlignLeft   );
  _assitem->addColumn(tr("Description"),  -1,           AlignLeft   );
  _assitem->addColumn(tr("UOM"),          _uomColumn,   AlignCenter );
  _assitem->addColumn(tr("Qty. Per"),     _qtyColumn,   AlignRight  );
  _assitem->addColumn(tr("Effective"),    _dateColumn,  AlignCenter );
  _assitem->addColumn(tr("Expires"),      _dateColumn,  AlignCenter );
  _assitem->setDragString("assid=");
  _assitem->setAltDragString("itemid=");
}

enum SetResponse assortment::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if ( (param.toString() == "new") || (param.toString() == "edit") )
    {
      connect(_assitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_assitem, SIGNAL(valid(bool)), _expire, SLOT(setEnabled(bool)));
      connect(_assitem, SIGNAL(valid(bool)), _moveUp, SLOT(setEnabled(bool)));
      connect(_assitem, SIGNAL(valid(bool)), _moveDown, SLOT(setEnabled(bool)));
      connect(_assitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_assitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }

    if (param.toString() == "new")
    {
      _mode = cNew;
      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(TRUE);
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _item->setReadOnly(TRUE);
      _documentNum->setEnabled(FALSE);
      _revision->setEnabled(FALSE);
      _revisionDate->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _edit->setEnabled(FALSE);
      _expire->setEnabled(FALSE);
      _moveUp->setEnabled(FALSE);
      _moveDown->setEnabled(FALSE);

      connect(_assitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  return NoError;
}

void assortment::sSave()
{
  q.prepare( "SELECT asshead_id "
             "FROM asshead "
             "WHERE (asshead_item_id=:item_id);" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  if (q.first())
  {
    int assheadid = q.value("asshead_id").toInt();

    q.prepare( "UPDATE asshead "
               "SET asshead_docnum=:asshead_docnum,"
               "    asshead_revision=:asshead_revision, asshead_revisiondate=:asshead_revisiondate "
               "WHERE (asshead_id=:asshead_id);" );
    q.bindValue(":asshead_id", assheadid);
  }
  else
  {
    q.prepare( "INSERT INTO asshead "
               "( asshead_item_id, asshead_docnum,"
               "  asshead_revision, asshead_revisiondate ) "
               "VALUES "
               "( :asshead_item_id, :asshead_docnum,"
               "  :asshead_revision, :asshead_revisiondate ) " );
    q.bindValue(":asshead_item_id", _item->id());
  }

  q.bindValue(":asshead_docnum", _documentNum->text());
  q.bindValue(":asshead_revision", _revision->text());
  q.bindValue(":asshead_revisiondate", _revisionDate->date());
  q.exec();

  close();
}

void assortment::sPrint()
{
  ParameterList params;
  params.append("item_id", _item->id());
  params.append("print");

  if (_showExpired->isChecked())
    params.append("expiredDays", 999);

  if (_showFuture->isChecked())
    params.append("futureDays", 999);

#if 0
  rptSingleLevelBOM newdlg(this, "", TRUE);
  newdlg.set(params);
#endif
}

void assortment::sPopulateMenu(QPopupMenu *menuThis)
{
  menuThis->insertItem(tr("View"), this, SLOT(sView()), 0);

  if ((_mode == cNew) || (_mode == cEdit))
  {
    menuThis->insertItem(tr("Edit"), this, SLOT(sEdit()), 0);
    menuThis->insertItem(tr("Expire"), this, SLOT(sExpire()), 0);
    menuThis->insertItem(tr("Replace"), this, SLOT(sReplace()), 0);

    menuThis->insertSeparator();

    menuThis->insertItem(tr("Move Up"),   this, SLOT(sMoveUp()), 0);
    menuThis->insertItem(tr("Move Down"), this, SLOT(sMoveDown()), 0);
  }
}

void assortment::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());

  assortmentItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void assortment::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("assitem_id", _assitem->id());

  assortmentItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void assortment::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("assitem_id", _assitem->id());

  assortmentItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void assortment::sExpire()
{
  q.prepare( "UPDATE assitem "
             "SET assitem_expires=CURRENT_DATE "
             "WHERE (assitem_id=:assitem_id);" );
  q.bindValue(":assitem_id", _assitem->id());
  q.exec();
  sFillList();
}

void assortment::sReplace()
{
  ParameterList params;
  params.append("mode", "replace");
  params.append("assitem_id", _assitem->id());

  assortmentItem newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != QDialog::Rejected)
    sFillList();
}

void assortment::sMoveUp()
{
  q.prepare("SELECT moveAssitemUp(:assitem_id) AS result;");
  q.bindValue(":assitem_id", _assitem->id());
  q.exec();
  sFillList();
}

void assortment::sMoveDown()
{
  q.prepare("SELECT moveAssitemDown(:assitem_id) AS result;");
  q.bindValue(":assitem_id", _assitem->id());
  q.exec();
  sFillList();
}

void assortment::sFillList()
{
  sFillList(_item->id(), TRUE);
}

void assortment::sFillList(int pItemid, bool)
{
  if (_item->isValid() && (pItemid == _item->id()))
  {
    q.prepare( "SELECT asshead_docnum, asshead_revision,"
               "       asshead_revisiondate "
               "FROM asshead "
               "WHERE (asshead_item_id=:item_id);" );
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      _documentNum->setText(q.value("asshead_docnum").toString());
      _revision->setText(q.value("asshead_revision").toString());
      _revisionDate->setDate(q.value("asshead_revisiondate").toDate());
    }
    else
    {
      _documentNum->clear();
      _revision->clear();
      _revisionDate->clear();
    }

    QString sql( "SELECT assitem_id, item_id, assitem_seqnumber,"
                 "       item_number, (item_descrip1 || ' ' || item_descrip2) AS item_description,"
                 "       item_invuom,"
                 "       formatQtyPer(assitem_qtyper) AS f_qtyper,"
                 "       formatDate(assitem_effective, :always) AS f_effective,"
                 "       formatDate(assitem_expires, :never) AS f_expires "
                 "FROM assitem, item "
                 "WHERE ((assitem_item_id=item_id)"
                 " AND (assitem_parent_item_id=:item_id)" );

    if (!_showExpired->isChecked())
      sql += " AND (assitem_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (assitem_effective <= CURRENT_DATE)";

    sql += ") "
           "ORDER BY assitem_seqnumber, assitem_effective";

    int assitemid = _assitem->id();
    _assitem->clear();

    XListViewItem *selected = 0;

    q.prepare(sql);
    q.bindValue(":always", tr("Always"));
    q.bindValue(":never", tr("Never"));
    q.bindValue(":item_id", _item->id());
    q.exec();
    while (q.next())
    {
      XListViewItem *last = new XListViewItem( _assitem, _assitem->lastItem(),
                                               q.value("assitem_id").toInt(), q.value("item_id").toInt(),
                                               q.value("assitem_seqnumber"), q.value("item_number"),
                                               q.value("item_description"), q.value("item_invuom"),
                                               q.value("f_qtyper"), q.value("f_effective"),
                                               q.value("f_expires") );
      if (q.value("config").toBool())
        last->setColor("blue");

      if (q.value("assitem_id").toInt() == assitemid)
        selected = last;
    }

    if (selected)
    {
      _assitem->setSelected(selected, TRUE);
      _assitem->ensureItemVisible(selected);
    }
  }
  else if (!_item->isValid())
  {
    _documentNum->clear();
    _revision->clear();
    _revisionDate->clear();
    
    _assitem->clear();
  }
}

