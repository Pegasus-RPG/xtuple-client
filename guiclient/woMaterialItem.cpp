/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "woMaterialItem.h"

#include <metasql.h>
#include "mqlutil.h"

#include <QVariant>
#include <QSqlError>
#include <QMessageBox>
#include <QValidator>
#include "inputManager.h"
#include "errorReporter.h"

woMaterialItem::woMaterialItem(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);
  _bomitemid=-1;
  _wooperid=-1;

  connect(_qtyFxd, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQtyRequired()));
  connect(_qtyPer, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQtyRequired()));
  connect(_scrap, SIGNAL(textChanged(const QString&)), this, SLOT(sUpdateQtyRequired()));
  connect(_uom, SIGNAL(newID(int)), this, SLOT(sUOMChanged()));
  connect(_wo, SIGNAL(valid(bool)), this, SLOT(sEnableSave()));
  connect(_item, SIGNAL(valid(bool)), this, SLOT(sEnableSave()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sItemIdChanged()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _captive = false;

  _wo->setType(cWoOpen | cWoExploded | cWoIssued | cWoReleased);

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _qtyFxd->setValidator(omfgThis->qtyVal());
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

woMaterialItem::~woMaterialItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void woMaterialItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse woMaterialItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _wo->setReadOnly(true);
  }
  
  param = pParams.value("bomitem_id", &valid);
  if (valid)
    _bomitemid=param.toInt();

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _captive = true;

    _item->setId(param.toInt());
    _item->setReadOnly(true);
  }
  
  param = pParams.value("wooper_id", &valid);
  if (valid)
    _wooperid=param.toInt();

  param = pParams.value("qtyFxd", &valid);
  if (valid)
    _qtyFxd->setText(param.toDouble());

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

  param = pParams.value("picklist", &valid);
  if (valid)
    _pickList->setChecked(param.toBool());

  param = pParams.value("showPrice", &valid);
  if (!valid)
  {
    _priceLit->hide();
    _price->hide();
  }

  param = pParams.value("womatl_id", &valid);
  if (valid)
  {
    _captive = true;

    _wo->setEnabled(false);
    _item->setEnabled(false);

    _womatlid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      if (_wo->id() >= 0)
        _wo->setEnabled(false);
      _item->setType(ItemLineEdit::cActive);
      _item->addExtraClause( QString("(itemsite_active)") );  // ItemLineEdit::cActive doesn't compare against the itemsite record
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _wo->setEnabled(false);
      _item->setEnabled(false);
      _qtyFxd->setEnabled(false);
      _qtyPer->setEnabled(false);
      _uom->setEnabled(false);
      _scrap->setEnabled(false);
      _issueMethod->setEnabled(false);
      _notes->setEnabled(false);
      _ref->setEnabled(false);

      _close->setText(tr("&Close"));
      _save->hide();
    }
  }

  return NoError;
}

void woMaterialItem::sEnableSave()
{
  _save->setEnabled(_wo->isValid() && _item->isValid());
}

void woMaterialItem::sSave()
{
  XSqlQuery woSave;
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
    woSave.prepare( "SELECT component.itemsite_id AS itemsiteid, "
                    "component.itemsite_costmethod AS costmethod "
               "FROM wo, itemsite AS parent, itemsite AS component "
               "WHERE ( (parent.itemsite_warehous_id=component.itemsite_warehous_id)"
               " AND (parent.itemsite_id=wo_itemsite_id)"
               " AND (component.itemsite_item_id=:item_id)"
               " AND (wo_id=:wo_id) );" );
    woSave.bindValue(":item_id", _item->id());
    woSave.bindValue(":wo_id", _wo->id());
    woSave.exec();
    if (woSave.first())
    {
      if (woSave.value("costmethod")=="J")
      {
        QMessageBox::warning( this, tr("Cannot Create W/O Material Requirement"),
                            tr( "A W/O Material Requirement cannot be created for the selected\n"
                                "Work Order/Item as the selected Item is Job Costed,\n"
                                "and Work Order Component Items may not be Job Costed." ));
        _item->setId(-1);
        _item->setFocus();
        return;
      }
    }
    else
    {
      QMessageBox::warning( this, tr("Cannot Create W/O Material Requirement"),
                            tr( "A W/O Material Requirement cannot be created for the selected\n"
                                "Work Order/Item as the selected Item does not exist in the warehouse\n"
                                "that the selected Work Order does." ));
      _item->setId(-1);
      _item->setFocus();
      return;
    }

    int itemsiteid = woSave.value("itemsiteid").toInt();

    woSave.prepare("WITH RECURSIVE _wo AS ( "
                   " SELECT itemsite_item_id, wo_ordtype, wo_ordid "
                   "   FROM wo "
                   "   JOIN itemsite ON wo_itemsite_id=itemsite_id "
                   "  WHERE wo_id=:wo_id "
                   " UNION "
                   " SELECT parent.itemsite_item_id, wo.wo_ordtype, wo.wo_ordid "
                   "   FROM _wo "
                   "   JOIN wo ON _wo.wo_ordtype='W' "
                   "          AND _wo.wo_ordid=wo.wo_id "
                   "   JOIN itemsite parent ON wo.wo_itemsite_id=parent.itemsite_id "
                   ") "
                   "SELECT BOOL_OR(itemsite_item_id=:item_id) AS recursive "
                   "  FROM _wo;");
    woSave.bindValue(":wo_id", _wo->id());
    woSave.bindValue(":item_id", _item->id());
    woSave.exec();
    if (woSave.first() && woSave.value("recursive").toBool())
    {
      QMessageBox::warning(this, tr("Cannot Create W/O Material Requirement"),
                           tr("Cannot add an Item as a Material to itself."));
      _item->setId(-1);
      _item->setFocus();
      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Checking Parents"),
                                  woSave, __FILE__, __LINE__))
      return;

    woSave.prepare("SELECT createWoMaterial(:wo_id, :itemsite_id, :issueMethod,"
                   "                        :uom_id, :qtyFxd, :qtyPer,"
                   "                        :scrap, :bomitem_id, :notes,"
                   "                        :ref, :wooper_id, :picklist,"
                   "                        :price) AS womatlid;");
    woSave.bindValue(":wo_id", _wo->id());
    woSave.bindValue(":itemsite_id", itemsiteid);
    woSave.bindValue(":issueMethod", issueMethod);
    woSave.bindValue(":qtyFxd", _qtyFxd->toDouble());
    woSave.bindValue(":qtyPer", _qtyPer->toDouble());
    woSave.bindValue(":uom_id", _uom->id());
    woSave.bindValue(":scrap", (_scrap->toDouble() / 100));
    woSave.bindValue(":bomitem_id", _bomitemid);
    woSave.bindValue(":notes", _notes->toPlainText());
    woSave.bindValue(":ref",   _ref->toPlainText());
    woSave.bindValue(":wooper_id", _wooperid);
    woSave.bindValue(":picklist", QVariant(_pickList->isChecked()));
    woSave.bindValue(":price", _price->baseValue());
    woSave.exec();
    if (woSave.first())
    {
      _womatlid = woSave.value("womatlid").toInt();
    }
    else if (woSave.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving"),
                           woSave, __FILE__, __LINE__);
      return;
    }
  }
  else if (_mode == cEdit)
  {
    woSave.prepare( "UPDATE womatl "
               "SET womatl_qtyfxd=:qtyFxd,"
               "    womatl_qtyper=:qtyPer,"
               "    womatl_scrap=:scrap,"
               "    womatl_issuemethod=:issueMethod,"
               "    womatl_uom_id=:uom_id,"
               "    womatl_qtyreq=:qtyReq,"
               "    womatl_notes=:notes,"
               "    womatl_ref=:ref,"
               "    womatl_picklist=:picklist,"
               "    womatl_price=:price "
               "WHERE (womatl_id=:womatl_id);" );
    woSave.bindValue(":womatl_id", _womatlid);
    woSave.bindValue(":issueMethod", issueMethod);
    woSave.bindValue(":qtyFxd", _qtyFxd->toDouble());
    woSave.bindValue(":qtyPer", _qtyPer->toDouble());
    woSave.bindValue(":qtyReq", _qtyRequired->toDouble());
    woSave.bindValue(":uom_id", _uom->id());
    woSave.bindValue(":scrap", (_scrap->toDouble() / 100));
    woSave.bindValue(":notes", _notes->toPlainText());
    woSave.bindValue(":ref",   _ref->toPlainText());
    woSave.bindValue(":picklist", QVariant(_pickList->isChecked()));
    woSave.bindValue(":price", _price->baseValue());
    woSave.exec();
    if (woSave.lastError().type() != QSqlError::NoError)
    {
      ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving"),
                           woSave, __FILE__, __LINE__);
      return;
    }
  }

  omfgThis->sWorkOrderMaterialsUpdated(_wo->id(), _womatlid, true);

  if (_captive)
    done(_womatlid);
  else
  {
    _item->setId(-1);
    _qtyFxd->clear();
    _qtyPer->clear();
    _qtyRequired->clear();
    _scrap->clear();
    _item->setFocus();
    _notes->clear();
    _ref->clear();
    _pickList->setChecked(false);
  }
}

void woMaterialItem::sUpdateQtyRequired()
{
  XSqlQuery qtyreq;
  qtyreq.prepare("SELECT roundQty(itemuomfractionalbyuom(:item_id, :uom_id),"
                  "              (:qtyfxd + :qtyper * :qtyord) * (1 + :scrap)) AS qtyreq;");
  qtyreq.bindValue(":item_id", _item->id());
  qtyreq.bindValue(":uom_id", _uom->id());
  qtyreq.bindValue(":qtyord", _wo->qtyOrdered());
  qtyreq.bindValue(":qtyfxd", _qtyFxd->toDouble());
  qtyreq.bindValue(":qtyper", _qtyPer->toDouble());
  qtyreq.bindValue(":scrap", (_scrap->toDouble() / 100.0));
  qtyreq.exec();
  if(qtyreq.first())
    _qtyRequired->setDouble(qtyreq.value("qtyreq").toDouble());
}

void woMaterialItem::populate()
{
  XSqlQuery wopopulate;
  wopopulate.prepare( "SELECT womatl.*, itemsite_item_id,"
             "                womatl_scrap * 100 AS scrap "
             "FROM womatl JOIN itemsite ON (womatl_itemsite_id=itemsite_id) "
             "WHERE (womatl_id=:womatl_id);" );
  wopopulate.bindValue(":womatl_id", _womatlid);
  wopopulate.exec();
  if (wopopulate.first())
  {
    _wo->setId(wopopulate.value("womatl_wo_id").toInt());
    _item->setId(wopopulate.value("itemsite_item_id").toInt());
    _qtyFxd->setDouble(wopopulate.value("womatl_qtyfxd").toDouble());
    _qtyPer->setDouble(wopopulate.value("womatl_qtyper").toDouble());
    _uom->setId(wopopulate.value("womatl_uom_id").toInt());
    _scrap->setDouble(wopopulate.value("scrap").toDouble());
    _notes->setText(wopopulate.value("womatl_notes").toString());
    _ref->setText(wopopulate.value("womatl_ref").toString());
    _pickList->setChecked(wopopulate.value("womatl_picklist").toBool());
    _price->setBaseValue(wopopulate.value("womatl_price").toDouble());

    if (wopopulate.value("womatl_issuemethod").toString() == "S")
      _issueMethod->setCurrentIndex(0);
    else if (wopopulate.value("womatl_issuemethod").toString() == "L")
      _issueMethod->setCurrentIndex(1);
    else if (wopopulate.value("womatl_issuemethod").toString() == "M")
      _issueMethod->setCurrentIndex(2);
  }
}

void woMaterialItem::sItemIdChanged()
{
  sPopulateUOM();

  XSqlQuery uom;
  uom.prepare("SELECT item_inv_uom_id, item_type, item_picklist "
              "  FROM item"
              " WHERE(item_id=:item_id);");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  if(uom.first())
  {
    _invuomid = uom.value("item_inv_uom_id").toInt();
    _uom->setId(_invuomid);
    if (uom.value("item_type").toString() != "T" && uom.value("item_type").toString() != "R")
    {
      if (_qtyPer->text().length() == 0)
      {
        _qtyFxd->setDouble(0.0);
        _qtyPer->setDouble(1.0);
      }
    }
    else
    {
      if (_qtyPer->text().length() == 0)
      {
        _qtyFxd->setDouble(1.0);
        _qtyPer->setDouble(0.0);
      }
    }
    
    if (_scrap->text().length() == 0)
      _scrap->setDouble(0.0);
    
    _pickList->setChecked(uom.value("item_picklist").toBool());
  }
  else
  {
    _qtyFxd->clear();
    _qtyPer->clear();
    _qtyRequired->clear();
    _scrap->clear();
    _pickList->setChecked(false);
  }
}

void woMaterialItem::sPopulateUOM()
{
  // Get list of active, valid MaterialIssue UOMs
  MetaSQLQuery muom = mqlLoad("uoms", "item");
  
  ParameterList params;
  params.append("uomtype", "MaterialIssue");
  params.append("item_id", _item->id());
  
  // Include Global UOMs
  if (_privileges->check("MaintainUOMs"))
  {
    params.append("includeGlobal", true);
    params.append("global", tr("-Global"));
  }
  
  // Also have to factor UOMs previously used on WO Material now inactive
  if (_womatlid != -1)
  {
    XSqlQuery wouom;
    wouom.prepare("SELECT womatl_uom_id "
                  "  FROM womatl"
                  " WHERE(womatl_id=:womatl_id);");
    wouom.bindValue(":womatl_id", _womatlid);
    wouom.exec();
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting WoMatl UOMs"),
                             wouom, __FILE__, __LINE__))
      return;
    else if (wouom.first())
      params.append("uom_id", wouom.value("womatl_uom_id"));
  }
  
  XSqlQuery uom = muom.toQuery(params);
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting UOM"),
                           uom, __FILE__, __LINE__))
    return;
  
  int saveuomid = _uom->id();
  disconnect(_uom, SIGNAL(newID(int)), this, SLOT(sUOMChanged()));
  _uom->populate(uom);
  _uom->setId(saveuomid);
  connect(_uom, SIGNAL(newID(int)), this, SLOT(sUOMChanged()));
}

void woMaterialItem::sUOMChanged()
{
  // Check for Global UOM Conversion that must be setup for Item
  if (_uom->code() == "G")
  {
    if (QMessageBox::question(this, tr("Use Global UOM?"),
                              tr("<p>This Global UOM Conversion is not setup for this Item."
                                 "<p>Do you want to add this UOM conversion to this Item?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
    {
      // create itemuomconv and itemuom
      XSqlQuery adduom;
      adduom.prepare("SELECT createItemUomConv(:item_id, :uom_id, :uom_type) AS result;");
      adduom.bindValue(":item_id", _item->id());
      adduom.bindValue(":uom_id", _uom->id());
      adduom.bindValue(":uom_type", "MaterialIssue");
      adduom.exec();
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Creating Item UOM Conv"),
                               adduom, __FILE__, __LINE__))
        return;

      // repopulate uom combobox
      int saveuomid = _uom->id();
      disconnect(_uom, SIGNAL(newID(int)), this, SLOT(sUOMChanged()));
      sPopulateUOM();
      _uom->setId(saveuomid);
      connect(_uom, SIGNAL(newID(int)), this, SLOT(sUOMChanged()));
    }
    else
    {
      _uom->setId(_invuomid);
    }
  }
}
