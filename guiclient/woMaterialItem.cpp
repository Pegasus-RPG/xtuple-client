/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "woMaterialItem.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "inputManager.h"

/*
 *  Constructs a woMaterialItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
woMaterialItem::woMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  _bomitemid=-1;
  _wooperid=-1;


  // signals and slots connections
  connect(_qtyPer, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQtyRequired()));
  connect(_scrap, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQtyRequired()));
  connect(_item, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sItemIdChanged()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _captive = FALSE;

  _wo->setType(cWoOpen | cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _qtyPer->setValidator(omfgThis->qtyPerVal());
  _scrap->setValidator(omfgThis->scrapVal());
  _qtyRequired->setPrecision(omfgThis->qtyVal());

  QString issueMethod = _metrics->value("DefaultWomatlIssueMethod");
  if (issueMethod == "S")
    _issueMethod->setCurrentIndex(0);
  else if (issueMethod == "L")
    _issueMethod->setCurrentIndex(1);
  else if (issueMethod == "M")
    _issueMethod->setCurrentIndex(2);
}

/*
 *  Destroys the object and frees any allocated resources
 */
woMaterialItem::~woMaterialItem()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void woMaterialItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse woMaterialItem::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);

    _item->setFocus();
  }
  
  param = pParams.value("bomitem_id", &valid);
  if (valid)
    _bomitemid=param.toInt();

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }
  
  param = pParams.value("wooper_id", &valid);
  if (valid)
    _wooperid=param.toInt();

  param = pParams.value("qtyPer", &valid);
  if (valid)
    _qtyPer->setText(param.toDouble());

  param = pParams.value("uom_id", &valid);
  if (valid)
    _uom->setId(param.toInt());

  param = pParams.value("scrap", &valid);
  if (valid)
    _scrap->setText(param.toDouble());

  param = pParams.value("issueMethod", &valid);
  if (valid)
  {
    if (param.toString() == "push")
      _issueMethod->setCurrentIndex(0);
    else if (param.toString() == "pull")
      _issueMethod->setCurrentIndex(1);
    else if (param.toString() == "mixed")
      _issueMethod->setCurrentIndex(2);
  }
  
  param = pParams.value("notes", &valid);
  if (valid)
    _notes->setText(param.toString());
  
  param = pParams.value("reference", &valid);
  if (valid)
    _ref->setText(param.toString());

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _wo->setEnabled(FALSE);
    _item->setEnabled(FALSE);

    _womatlid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _wo->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _wo->setEnabled(FALSE);
      _item->setEnabled(FALSE);
      _qtyPer->setEnabled(FALSE);
      _uom->setEnabled(FALSE);
      _scrap->setEnabled(FALSE);
      _issueMethod->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _ref->setEnabled(FALSE);

      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void woMaterialItem::sSave()
{
  QString issueMethod;

  switch (_issueMethod->currentIndex())
  {
    case 0:
      issueMethod = 'S';
      break;

    case 1:
      issueMethod = 'L';
      break;

    case 2:
      issueMethod = 'M';
      break;
  }

  if (_mode == cNew)
  {
    q.prepare( "SELECT component.itemsite_id AS itemsiteid "
               "FROM wo, itemsite AS parent, itemsite AS component "
               "WHERE ( (parent.itemsite_warehous_id=component.itemsite_warehous_id)"
               " AND (parent.itemsite_id=wo_itemsite_id)"
               " AND (component.itemsite_item_id=:item_id)"
               " AND (wo_id=:wo_id) );" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (!q.first())
    {
      QMessageBox::warning( this, tr("Cannot Create W/O Material Requirement"),
                            tr( "A W/O Material Requirement cannot be created for the selected\n"
                                "Work Order/Item as the selected Item does not exist in the warehouse\n"
                                "that the selected Work Order does." ));
      _item->setId(-1);
      _item->setFocus();
      return;
    }

    int itemsiteid = q.value("itemsiteid").toInt();

    q.prepare("SELECT createWoMaterial(:wo_id, :itemsite_id, :issueMethod, :uom_id, :qtyPer, :scrap, :bomitem_id, :notes, :ref) AS womatlid;");
    q.bindValue(":wo_id", _wo->id());
    q.bindValue(":itemsite_id", itemsiteid);
    q.bindValue(":issueMethod", issueMethod);
    q.bindValue(":qtyPer", _qtyPer->toDouble());
    q.bindValue(":uom_id", _uom->id());
    q.bindValue(":scrap", (_scrap->toDouble() / 100));
    q.bindValue(":bomitem_id", _bomitemid);
    q.bindValue(":notes", _notes->text());
    q.bindValue(":ref", _ref->text());
    q.bindValue(":wooper_id", _wooperid);
    q.exec();
    if (q.first())
    {
      _womatlid = q.value("womatlid").toInt();
      
      q.prepare("UPDATE womatl SET womatl_wooper_id=:wooper_id WHERE womatl_id=:womatl_id");
      q.bindValue(":wooper_id",_wooperid);
      q.bindValue(":womatl_id",_womatlid);
      q.exec();
    }
    
//  ToDo
  }
  else if (_mode == cEdit)
  {
    q.prepare( "UPDATE womatl "
               "SET womatl_qtyper=:qtyPer, womatl_scrap=:scrap, womatl_issuemethod=:issueMethod,"
               "    womatl_uom_id=:uom_id,"
               "    womatl_qtyreq=(:qtyPer * (1 + :scrap) * wo_qtyord), womatl_notes=:notes, womatl_ref=:ref "
               "FROM wo "
               "WHERE ( (womatl_wo_id=wo_id)"
               " AND (womatl_id=:womatl_id) );" );
    q.bindValue(":womatl_id", _womatlid);
    q.bindValue(":issueMethod", issueMethod);
    q.bindValue(":qtyPer", _qtyPer->toDouble());
    q.bindValue(":uom_id", _uom->id());
    q.bindValue(":scrap", (_scrap->toDouble() / 100));
    q.bindValue(":notes", _notes->text());
    q.bindValue(":ref", _ref->text());
    q.exec();
  }

  omfgThis->sWorkOrderMaterialsUpdated(_wo->id(), _womatlid, TRUE);

  if (_captive)
    done(_womatlid);
  else
  {
    _item->setId(-1);
    _qtyPer->clear();
    _qtyRequired->clear();
    _scrap->clear();
    _item->setFocus();
    _notes->clear();
    _ref->clear();
  }
}

void woMaterialItem::sUpdateQtyRequired()
{
  _qtyRequired->setText(_wo->qtyOrdered() * (_qtyPer->toDouble() * (1 + (_scrap->toDouble() / 100))));
}

void woMaterialItem::populate()
{
  q.prepare( "SELECT womatl_wo_id, itemsite_item_id,"
             "       womatl_qtyper AS qtyper,"
             "       womatl_uom_id,"
             "       womatl_scrap * 100 AS scrap,"
             "       womatl_issuemethod, womatl_notes, womatl_ref "
             "FROM womatl, itemsite "
             "WHERE ( (womatl_itemsite_id=itemsite_id)"
             " AND (womatl_id=:womatl_id) );" );
  q.bindValue(":womatl_id", _womatlid);
  q.exec();
  if (q.first())
  {
    _wo->setId(q.value("womatl_wo_id").toInt());
    _item->setId(q.value("itemsite_item_id").toInt());
    _qtyPer->setText(q.value("qtyper").toDouble());
    _uom->setId(q.value("womatl_uom_id").toInt());
    _scrap->setText(q.value("scrap").toDouble());
    _notes->setText(q.value("womatl_notes").toString());
    _ref->setText(q.value("womatl_ref").toString());

    if (q.value("womatl_issuemethod").toString() == "S")
      _issueMethod->setCurrentIndex(0);
    else if (q.value("womatl_issuemethod").toString() == "L")
      _issueMethod->setCurrentIndex(1);
    else if (q.value("womatl_issuemethod").toString() == "M")
      _issueMethod->setCurrentIndex(2);
  }
}

void woMaterialItem::sItemIdChanged()
{
  XSqlQuery uom;
  uom.prepare("SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN uom ON (item_inv_uom_id=uom_id)"
              " WHERE(item_id=:item_id)"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_to_uom_id=uom_id)"
              " WHERE((itemuomconv_from_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_from_uom_id=uom_id)"
              " WHERE((itemuomconv_to_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id))"
              " ORDER BY uom_name;");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  _uom->populate(uom);
  uom.prepare("SELECT item_inv_uom_id"
              "  FROM item"
              " WHERE(item_id=:item_id);");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  if(uom.first())
    _uom->setId(uom.value("item_inv_uom_id").toInt());
}

