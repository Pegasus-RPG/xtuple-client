/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bomItem.h"

#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <metasql.h>

#include "itemSubstitute.h"
#include "itemCost.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "storedProcErrorLookup.h"
#include "mqlutil.h"

bomItem::bomItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  QButtonGroup* _substituteGroupInt = new QButtonGroup(this);
  _substituteGroupInt->addButton(_noSubstitutes);
  _substituteGroupInt->addButton(_itemDefinedSubstitutes);
  _substituteGroupInt->addButton(_bomDefinedSubstitutes);

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSaveClick()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_item, SIGNAL(typeChanged(const QString&)), this, SLOT(sItemTypeChanged(const QString&)));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sItemIdChanged()));
  connect(_newSubstitution, SIGNAL(clicked()), this, SLOT(sNewSubstitute()));
  connect(_editSubstitution, SIGNAL(clicked()), this, SLOT(sEditSubstitute()));
  connect(_deleteSubstitution, SIGNAL(clicked()), this, SLOT(sDeleteSubstitute()));
  connect(_itemcost, SIGNAL(itemSelectionChanged()), this, SLOT(sCostSelectionChanged()));
  connect(_newCost, SIGNAL(clicked()), this, SLOT(sNewCost()));
  connect(_editCost, SIGNAL(clicked()), this, SLOT(sEditCost()));
  connect(_deleteCost, SIGNAL(clicked()), this, SLOT(sDeleteCost()));
  connect(_char, SIGNAL(activated(int)), this, SLOT(sCharIdChanged()));

  if (_metrics->boolean("AllowInactiveBomItems"))
    _item->setType(ItemLineEdit::cGeneralComponents);
  else
    _item->setType(ItemLineEdit::cGeneralComponents | ItemLineEdit::cActive);

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));

  _qtyFxd->setValidator(omfgThis->qtyVal());
  _qtyPer->setValidator(omfgThis->qtyPerVal());
  _scrap->setValidator(omfgThis->scrapVal());

  _bomitemsub->addColumn(tr("Rank"),        _whsColumn,  Qt::AlignCenter, true, "bomitemsub_rank");
  _bomitemsub->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,  true, "item_number");
  _bomitemsub->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, 
		  "item_descrip1");
  _bomitemsub->addColumn(tr("Ratio"),       _qtyColumn,  Qt::AlignRight, true, "bomitemsub_uomratio");

  _itemcost->addColumn(tr("Element"),     -1,           Qt::AlignLeft,   true, "costelem_type");
  _itemcost->addColumn(tr("Lower"),       _costColumn,  Qt::AlignCenter, true, "itemcost_lowlevel" );
  _itemcost->addColumn(tr("Std. Cost"),   _costColumn,  Qt::AlignRight,  true, "itemcost_stdcost"  );
  _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,true, "baseCurr" );
  _itemcost->addColumn(tr("Posted"),      _dateColumn,  Qt::AlignCenter, true, "itemcost_posted" );
  _itemcost->addColumn(tr("Act. Cost"),   _costColumn,  Qt::AlignRight,  true, "itemcost_actcost"  );
  _itemcost->addColumn(tr("Currency"),    _currencyColumn, Qt::AlignLeft,true, "costCurr" );
  _itemcost->addColumn(tr("Updated"),     _dateColumn,  Qt::AlignCenter, true, "itemcost_updated" );

  if (omfgThis->singleCurrency())
  {
      _itemcost->hideColumn(3);
      _itemcost->hideColumn(6);
  }

  _item->setFocus();
  
  _itemid=0;
  _saved=FALSE;
  adjustSize();
}

bomItem::~bomItem()
{
  // no need to delete child widgets, Qt does it all for us
}

void bomItem::languageChange()
{
  retranslateUi(this);
}

enum SetResponse bomItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("bomitem_id", &valid);
  if (valid)
  {
    _bomitemid = param.toInt();
    populate();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();

  param = pParams.value("revision_id", &valid);
  if (valid)
    _revisionid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      QString issueMethod = _metrics->value("DefaultWomatlIssueMethod");
      if (issueMethod == "S")
        _issueMethod->setCurrentIndex(0);
      else if (issueMethod == "L")
        _issueMethod->setCurrentIndex(1);
      else if (issueMethod == "M")
        _issueMethod->setCurrentIndex(2);

      q.exec("SELECT NEXTVAL('bomitem_bomitem_id_seq') AS bomitem_id");
      if (q.first())
        _bomitemid = q.value("bomitem_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
  
      //Set up configuration tab if parent item is configured or kit
      q.prepare("SELECT item_config, item_type "
                "FROM item "
                "WHERE (item_id=:item_id); ");
      q.bindValue(":item_id", _itemid);
      q.exec();
      if (q.first())
      {
        if (q.value("item_config").toBool())
          _char->populate(QString( "SELECT -1 AS charass_char_id, '' AS char_name "
                                   "UNION "
                                   "SELECT DISTINCT charass_char_id, char_name "
                                   "FROM charass, char "
                                   "WHERE ((charass_char_id=char_id) "
                                   "AND (charass_target_type='I') "
                                   "AND (charass_target_id= %1)) "
                                   "ORDER BY char_name; ").arg(_itemid));
        else
          _tab->removeTab(_tab->indexOf(_configurationTab));
          
        if (q.value("item_type").toString() == "K")
        {
          if (_metrics->boolean("AllowInactiveBomItems"))
            _item->setType(ItemLineEdit::cKitComponents);
          else
            _item->setType(ItemLineEdit::cKitComponents | ItemLineEdit::cActive);
        }
      }

      connect(_bomDefinedCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleBomitemCost()));
      _item->setFocus();
    }
    else if (param.toString() == "replace")
    {
      _mode = cReplace;

      _item->setId(-1);
      _dates->setStartDate(omfgThis->dbDate());
      _item->setFocus();

      _sourceBomitemid = _bomitemid;
      q.exec("SELECT NEXTVAL('bomitem_bomitem_id_seq') AS bomitem_id");
      if (q.first())
        _bomitemid = q.value("bomitem_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(TRUE);
      connect(_bomDefinedCosts, SIGNAL(toggled(bool)), this, SLOT(sHandleBomitemCost()));
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;

      _sourceBomitemid = _bomitemid;
      q.exec("SELECT NEXTVAL('bomitem_bomitem_id_seq') AS bomitem_id");
      if (q.first())
        _bomitemid = q.value("bomitem_id").toInt();
      else if (q.lastError().type() != QSqlError::NoError)
      {
	systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	return UndefinedError;
      }

      _dates->setStartDate(omfgThis->dbDate());
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _qtyFxd->setEnabled(FALSE);
      _qtyPer->setEnabled(FALSE);
      _scrap->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _createWo->setEnabled(FALSE);
      _issueMethod->setEnabled(FALSE);
      _uom->setEnabled(FALSE);
      _comments->setReadOnly(TRUE);
      _ecn->setEnabled(FALSE);
      _substituteGroup->setEnabled(FALSE);
      _notes->setEnabled(FALSE);
      _ref->setEnabled(FALSE);
      _buttonBox->setStandardButtons(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  // Check the parent item type and if it is a Kit then change some widgets
  q.prepare("SELECT item_type "
            "FROM item "
            "WHERE (item_id=:item_id); ");
  q.bindValue(":item_id", _itemid);
  q.exec();
  if (q.first() && (q.value("item_type").toString() == "K"))
  {
    _createWo->setChecked(false);
    _createWo->setEnabled(false);
    _issueMethod->setCurrentIndex(0);
    _issueMethod->setEnabled(false);
    _tab->setTabEnabled(_tab->indexOf(_substitutionsTab), false);
  }
  
  return NoError;
}

void bomItem::sSave()
{
  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_qtyPer->toDouble() == 0.0 && _qtyFxd->toDouble() == 0.0, _qtyPer,
                          tr("You must enter a Quantity Per value before saving this BOM Item."))
         << GuiErrorCheck(_scrap->text().length() == 0, _scrap,
                          tr("You must enter a Scrap value before saving this BOM Item."))
         << GuiErrorCheck(_dates->endDate() < _dates->startDate(), _dates,
                          tr("The expiration date cannot be earlier than the effective date."))
     ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Bill of Material Item"), errors))
    return;

  // Check the component item type and if it is a Reference then issue a warning
  XSqlQuery itemtype;
  itemtype.prepare("SELECT item_type FROM item WHERE (item_id=:item_id); ");
  itemtype.bindValue(":item_id", _item->id());
  itemtype.exec();
  if (itemtype.first() && itemtype.value("item_type").toString() == "R")
  {
    int answer = QMessageBox::question(this, tr("Reference Item"),
                            tr("<p>Adding a Reference Item to a Bill of Material "
                               "may cause W/O variances. <p> "
                               "OK to continue? "),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No);
    if (answer == QMessageBox::No)
      return;
  }

  if (_mode == cNew && !_saved)
    q.prepare( "SELECT createBOMItem( :bomitem_id, :parent_item_id, :component_item_id, :issueMethod,"
               "                      :bomitem_uom_id, :qtyFxd, :qtyPer, :scrap,"
               "                      :effective, :expires,"
               "                      :createWo, -1, :scheduledWithBooItem,"
               "                      :ecn, :subtype, :revision_id,"
               "                      :char_id, :value, :notes, :ref ) AS result;" );
  else if ( (_mode == cCopy) || (_mode == cReplace) )
    q.prepare( "SELECT createBOMItem( :bomitem_id, :parent_item_id, :component_item_id,"
               "                      bomitem_seqnumber, :issueMethod,"
               "                      :bomitem_uom_id, :qtyFxd, :qtyPer, :scrap,"
               "                      :effective, :expires,"
               "                      :createWo, -1, :scheduledWithBooItem,"
               "                      :ecn, :subtype, :revision_id,"
               "                      :char_id, :value, :notes, :ref ) AS result "
               "FROM bomitem "
               "WHERE (bomitem_id=:sourceBomitem_id);" );
  else if (_mode == cEdit  || _saved)
    q.prepare( "UPDATE bomitem "
               "SET bomitem_qtyfxd=:qtyFxd, bomitem_qtyper=:qtyPer, bomitem_scrap=:scrap,"
               "    bomitem_effective=:effective, bomitem_expires=:expires,"
               "    bomitem_createwo=:createWo, bomitem_issuemethod=:issueMethod,"
               "    bomitem_uom_id=:bomitem_uom_id,"
               "    bomitem_ecn=:ecn, bomitem_moddate=CURRENT_DATE, bomitem_subtype=:subtype, "
               "    bomitem_char_id=:char_id, bomitem_value=:value, bomitem_notes=:notes, "
               "    bomitem_ref=:ref "
               "WHERE (bomitem_id=:bomitem_id);" );
  else
//  ToDo
    return;

  q.bindValue(":bomitem_id", _bomitemid);
  q.bindValue(":sourceBomitem_id", _sourceBomitemid);
  q.bindValue(":parent_item_id", _itemid);
  q.bindValue(":revision_id", _revisionid);
  q.bindValue(":component_item_id", _item->id());
  q.bindValue(":bomitem_uom_id", _uom->id());
  q.bindValue(":qtyFxd", _qtyFxd->toDouble());
  q.bindValue(":qtyPer", _qtyPer->toDouble());
  q.bindValue(":scrap", (_scrap->toDouble() / 100));
  q.bindValue(":effective", _dates->startDate());
  q.bindValue(":expires", _dates->endDate());
  q.bindValue(":ecn", _ecn->text());
  q.bindValue(":notes",	_notes->toPlainText());
  q.bindValue(":ref",   _ref->toPlainText());

  q.bindValue(":createWo", QVariant(_createWo->isChecked()));

  if (_issueMethod->currentIndex() == 0)
    q.bindValue(":issueMethod", "S");
  if (_issueMethod->currentIndex() == 1)
    q.bindValue(":issueMethod", "L");
  if (_issueMethod->currentIndex() == 2)
    q.bindValue(":issueMethod", "M");

  if (_noSubstitutes->isChecked())
    q.bindValue(":subtype", "N");
  else if (_itemDefinedSubstitutes->isChecked())
    q.bindValue(":subtype", "I");
  else if (_bomDefinedSubstitutes->isChecked())
    q.bindValue(":subtype", "B");
  q.bindValue(":revision_id", _revisionid);
  
  if (_char->id() != -1)
  {
    q.bindValue(":char_id", _char->id());
    q.bindValue(":value", _value->currentText());
  }

  q.bindValue(":configType", "N");
  q.bindValue(":configId", -1);
  q.bindValue(":configFlag", QVariant(FALSE));

  q.exec();

  if ((_mode == cNew) || (_mode == cReplace) || (_mode == cCopy))
  {
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("createBOMItem", result),
		    __FILE__, __LINE__);
        _item->setFocus();
	return;
      }

      if (_mode == cReplace)
      {
        XSqlQuery replace;
        replace.prepare( "UPDATE bomitem "
                         "SET bomitem_expires=:bomitem_expires "
                         "WHERE (bomitem_id=:bomitem_id)" );
        replace.bindValue(":bomitem_expires", _dates->startDate());
        replace.bindValue(":bomitem_id", _sourceBomitemid);
        replace.exec();
        if (replace.lastError().type() != QSqlError::NoError)
        {
          systemError(this, replace.lastError().databaseText(), __FILE__, __LINE__);
          return;
        }
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  omfgThis->sBOMsUpdated(_itemid, TRUE);
  
  emit saved(_bomitemid);

  _saved=TRUE;
}

void bomItem::sSaveClick()
{
  sSave();
  if (_saved)
    done(_bomitemid);
}

void bomItem::sClose()
{
  if (_mode == cNew)
  {
    q.prepare( "DELETE FROM bomitemsub "
               "WHERE (bomitemsub_bomitem_id=:bomitem_id);"
               "DELETE FROM bomitemcost "
               "WHERE (bomitemcost_bomitem_id=:bomitem_id);" );
    q.bindValue("bomitem_id", _bomitemid);
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  reject();
}

void bomItem::sItemTypeChanged(const QString &type)
{
  if (type == "M")
    _createWo->setEnabled(TRUE);
  else
  {
    _createWo->setEnabled(FALSE);
    _createWo->setChecked(FALSE);
  }
}

void bomItem::populate()
{
  q.prepare( "SELECT bomitem.*, item_config, item_type "
             "FROM bomitem JOIN item ON (item_id=bomitem_parent_item_id) "
             "WHERE (bomitem_id=:bomitem_id);" );
  q.bindValue(":bomitem_id", _bomitemid);
  q.exec();
  if (q.first())
  {
    _itemid = q.value("bomitem_parent_item_id").toInt();
    _item->setId(q.value("bomitem_item_id").toInt());
    _uom->setId(q.value("bomitem_uom_id").toInt());
    _notes->setText(q.value("bomitem_notes").toString());
    _ref->setText(q.value("bomitem_ref").toString());

    if (q.value("bomitem_issuemethod").toString() == "S")
      _issueMethod->setCurrentIndex(0);
    else if (q.value("bomitem_issuemethod").toString() == "L")
      _issueMethod->setCurrentIndex(1);
    else if (q.value("bomitem_issuemethod").toString() == "M")
      _issueMethod->setCurrentIndex(2);

    if (q.value("item_type").toString() == "M" || q.value("item_type").toString() == "F")
      _createWo->setChecked(q.value("bomitem_createwo").toBool());

    _dates->setStartDate(q.value("bomitem_effective").toDate());
    _dates->setEndDate(q.value("bomitem_expires").toDate());
    _qtyFxd->setDouble(q.value("bomitem_qtyfxd").toDouble());
    _qtyPer->setDouble(q.value("bomitem_qtyper").toDouble());
    _scrap->setDouble(q.value("bomitem_scrap").toDouble() * 100);

    if (_mode != cCopy)
      _ecn->setText(q.value("bomitem_ecn").toString());

    _comments->setId(_bomitemid);

    if (q.value("item_type").toString() == "M" || 
        q.value("item_type").toString() == "F" )
      _createWo->setChecked(q.value("bomitem_createwo").toBool());
      
    if (q.value("item_config").toBool())
    {
      _char->populate(QString( "SELECT -1 AS charass_char_id, '' AS char_name "
                               "UNION "
                               "SELECT DISTINCT charass_char_id, char_name "
                               "FROM charass, char "
                               "WHERE ((charass_char_id=char_id) "
                               "AND (charass_target_type='I') "
                               "AND (charass_target_id= %1)) "
                               "ORDER BY char_name; ").arg(_itemid));
      _char->setId(q.value("bomitem_char_id").toInt());
      sCharIdChanged();
      _value->setText(q.value("bomitem_value").toString());
    }
    else
      _tab->removeTab(_tab->indexOf(_configurationTab));

    if (q.value("bomitem_subtype").toString() == "I")
      _itemDefinedSubstitutes->setChecked(true);
    else if (q.value("bomitem_subtype").toString() == "B")
      _bomDefinedSubstitutes->setChecked(true);
    else
      _noSubstitutes->setChecked(true);
    sFillSubstituteList();
    sFillCostList();

  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void bomItem::sNewSubstitute()
{
  sSave();
  ParameterList params;
  params.append("mode", "new");
  params.append("bomitem_id", _bomitemid);

  if (_mode == cNew)
    params.append("bomitem_item_id", _item->id());

  itemSubstitute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSubstituteList();
}

void bomItem::sEditSubstitute()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bomitemsub_id", _bomitemsub->id());

  itemSubstitute newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillSubstituteList();
}

void bomItem::sDeleteSubstitute()
{
  q.prepare( "DELETE FROM bomitemsub "
             "WHERE (bomitemsub_id=:bomitemsub_id);" );
  q.bindValue(":bomitemsub_id", _bomitemsub->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillSubstituteList();
}

void bomItem::sFillSubstituteList()
{
  q.prepare( "SELECT bomitemsub.*, item_number, item_descrip1,"
             "       'ratio' AS bomitemsub_uomratio_xtnumericrole "
             "FROM bomitemsub, item "
             "WHERE ( (bomitemsub_item_id=item_id)"
             " AND (bomitemsub_bomitem_id=:bomitem_id) ) "
             "ORDER BY bomitemsub_rank, item_number" );
  q.bindValue(":bomitem_id", _bomitemid);
  q.exec();
  _bomitemsub->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void bomItem::sItemIdChanged()
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
              "  JOIN uom ON (itemuomconv_to_uom_id=uom_id),"
              "  itemuom, uomtype "
              " WHERE((itemuomconv_from_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id) "
              "   AND (itemuom_itemuomconv_id=itemuomconv_id) "
              "   AND (uomtype_id=itemuom_uomtype_id) "
              "   AND (uomtype_name='MaterialIssue'))"
              " UNION "
              "SELECT uom_id, uom_name"
              "  FROM item"
              "  JOIN itemuomconv ON (itemuomconv_item_id=item_id)"
              "  JOIN uom ON (itemuomconv_from_uom_id=uom_id),"
              "  itemuom, uomtype "
              " WHERE((itemuomconv_to_uom_id=item_inv_uom_id)"
              "   AND (item_id=:item_id) "
              "   AND (itemuom_itemuomconv_id=itemuomconv_id) "
              "   AND (uomtype_id=itemuom_uomtype_id) "
              "   AND (uomtype_name='MaterialIssue'))"
              " ORDER BY uom_name;");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  _uom->populate(uom);
  uom.prepare("SELECT item_inv_uom_id, item_type "
              "  FROM item"
              " WHERE(item_id=:item_id);");
  uom.bindValue(":item_id", _item->id());
  uom.exec();
  if(uom.first())
  {
    _uom->setId(uom.value("item_inv_uom_id").toInt());

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
  }
}

void bomItem::sCharIdChanged()
{
  XSqlQuery charass;
  charass.prepare("SELECT charass_id, charass_value "
            "FROM charass "
            "WHERE ((charass_target_type='I') "
            "AND (charass_target_id=:item_id) "
            "AND (charass_char_id=:char_id) "
            "AND (COALESCE(charass_value,'')!='')); ");
  charass.bindValue(":item_id", _itemid);
  charass.bindValue(":char_id", _char->id());
  charass.exec();
  _value->populate(charass);
}

void bomItem::sFillCostList()
{
  if (_item->isValid() && _bomitemid > 0)
  {
    _itemcost->clear();
    double standardCost = 0.0;
    double actualCostBase = 0.0;
    double actualCostLocal = 0.0;

    MetaSQLQuery mql = mqlLoad("itemCost", "list");

    ParameterList params;
    params.append("error", tr("!ERROR!"));
    params.append("never", tr("Never"));
    params.append("bomitem_id", _bomitemid);

    XSqlQuery qry = mql.toQuery(params);
    if (qry.first())
    {
      _bomDefinedCosts->setChecked(true);
      if (_privileges->check("CreateCosts"))
        _newCost->setEnabled(true);
    }
    else
    {
      params.append("item_id", _item->id());
      qry = mql.toQuery(params);
      _bomDefinedCosts->setChecked(false);
      _newCost->setEnabled(false);
    }

    _itemcost->populate(qry, true);
    if (qry.lastError().type() != QSqlError::NoError)
    {
      systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    bool multipleCurrencies = false;
    int firstCurrency = 0;
    bool baseKnown = true;
    if (qry.first())
    {
      firstCurrency = qry.value("itemcost_curr_id").toInt();
      do
      {
        standardCost += qry.value("itemcost_stdcost").toDouble();
        if (qry.value("itemcost_actcost").isNull())
            baseKnown = false;
        else
            actualCostBase += qry.value("actcostBase").toDouble();
        actualCostLocal += qry.value("itemcost_actcost").toDouble();
        if (! multipleCurrencies &&
            qry.value("itemcost_curr_id").toInt() != firstCurrency)
            multipleCurrencies = true;
      }
      while (qry.next());
    }

    XSqlQuery convert;
    double actualCost = 0;
    if (multipleCurrencies)
    {
        actualCost = actualCostBase;
        convert.prepare("SELECT currConcat(baseCurrId()) AS baseConcat, "
                        "       currConcat(baseCurrId()) AS currConcat;");
    }
    else
    {
        actualCost = actualCostLocal;
        baseKnown = true; // not necessarily but we can trust actualCost
        convert.prepare("SELECT currConcat(baseCurrId()) AS baseConcat, "
                        "	currConcat(:firstCurrency) AS currConcat;" );
        convert.bindValue(":firstCurrency", firstCurrency);
    }
    convert.exec();
    if (convert.first())
        new XTreeWidgetItem(_itemcost,
                            _itemcost->topLevelItem(_itemcost->topLevelItemCount() - 1), -1,
                            QVariant(tr("Totals")),
                            "",
                            formatCost(standardCost),
                            convert.value("baseConcat"),
                            "",
                            baseKnown ? formatCost(actualCost) : tr("?????"),
                            convert.value("currConcat"));
    else if (convert.lastError().type() != QSqlError::NoError)
        systemError(this, convert.lastError().databaseText(), __FILE__, __LINE__);

  }
  else
    _itemcost->clear();
}

void bomItem::sHandleBomitemCost()
{
  XSqlQuery qry;
  qry.prepare("SELECT toggleBomitemCost(:bomitem_id, :enabled);");
  qry.bindValue(":bomitem_id", _bomitemid);
  qry.bindValue(":enabled", _bomDefinedCosts->isChecked());
  qry.exec();
  if (qry.lastError().type() != QSqlError::NoError)
    systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
  sFillCostList();
}

void bomItem::sCostSelectionChanged()
{
  bool yes = (_itemcost->id() != -1);

  if (_privileges->check("EnterActualCosts"))
    _editCost->setEnabled(yes);

  if (_privileges->check("DeleteCosts"))
    _deleteCost->setEnabled(yes);
}

void bomItem::sNewCost()
{
  ParameterList params;
  params.append("bomitem_id", _bomitemid);
  params.append("mode", "new");

  if (_mode == cNew)
    params.append("bomitem_item_id", _item->id());

  itemCost newdlg(this, "", TRUE);
  if (newdlg.set(params) == NoError && newdlg.exec())
    sFillCostList();
}

void bomItem::sEditCost()
{
  ParameterList params;
  params.append("bomitemcost_id", _itemcost->id());
  params.append("mode", "edit");

  itemCost newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec())
    sFillCostList();
}

void bomItem::sDeleteCost()
{
  q.prepare( "DELETE FROM bomitemcost WHERE (bomitemcost_id=:bomitemcost_id);" );
  q.bindValue(":bomitemcost_id", _itemcost->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

  sFillCostList();
}

