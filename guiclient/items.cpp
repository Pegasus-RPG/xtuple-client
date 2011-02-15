/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "items.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>
#include <metasql.h>

#include "characteristic.h"
#include "copyItem.h"
#include "item.h"
#include "storedProcErrorLookup.h"
#include "parameterwidget.h"

items::items(QWidget* parent, const char*, Qt::WFlags fl)
  : display(parent, "items", fl)
{
  setWindowTitle(tr("Items"));
  setReportName("Items");
  setMetaSQLOptions("items", "detail");
  setNewVisible(true);
  setSearchVisible(true);
  setQueryOnStartEnabled(true);
  setParameterWidgetVisible(true);

  QString qryType = QString( "SELECT  1, '%1' UNION "
                             "SELECT  2, '%2' UNION "
                             "SELECT  3, '%3'")
      .arg(tr("Buy Items"))
      .arg(tr("Make Items"))
      .arg(tr("Sold Items"));

  parameterWidget()->appendComboBox(tr("Class Code"), "classcode_id", XComboBox::ClassCodes);
  parameterWidget()->append(tr("Class Code Pattern"), "classcode_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Freight Class"), "freightclass_id", XComboBox::FreightClasses);
  parameterWidget()->append(tr("Freight Class Pattern"), "freightclass_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item Number Pattern"), "item_number_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Item Description"), "item_descrip_pattern", ParameterWidget::Text);
  parameterWidget()->appendComboBox(tr("Item Group"), "itemgrp_id", XComboBox::ItemGroups);
  parameterWidget()->appendComboBox(tr("Item Types"), "item_types", qryType);
  parameterWidget()->appendComboBox(tr("Product Category"), "prodcat_id", XComboBox::ProductCategories);
  parameterWidget()->append(tr("Product Category Pattern"), "prodcat_pattern", ParameterWidget::Text);
  parameterWidget()->append(tr("Show Inactive"), "showInactive", ParameterWidget::Exists);

  list()->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft   , true, "item_number" );
  list()->addColumn(tr("Active"),      _ynColumn,   Qt::AlignCenter , true, "item_active" );
  list()->addColumn(tr("Description"), -1,          Qt::AlignLeft   , true, "item_descrip" );
  list()->addColumn(tr("Class Code"),  _dateColumn, Qt::AlignLeft , true, "classcode_code");
  list()->addColumn(tr("Type"),        _itemColumn, Qt::AlignLeft , true, "item_type");
  list()->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft , true, "uom_name");
  list()->addColumn(tr("Product Category"),  _itemColumn, Qt::AlignLeft , false, "prodcat_code");
  list()->addColumn(tr("Freight Class"),  _itemColumn, Qt::AlignLeft , false, "freightclass_code");
  
  if (_privileges->check("MaintainItemMasters"))
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sEdit()));
  else
  {
    newAction()->setEnabled(FALSE);
    connect(list(), SIGNAL(itemSelected(int)), this, SLOT(sView()));
  }

  // Add columns and parameters for characteristics
  QString column;
  QString name;
  XSqlQuery chars;
  chars.exec("SELECT char_id, char_name, char_type "
             "FROM char "
             "WHERE (char_items) "
             "ORDER BY char_name;");
  while (chars.next())
  {
    characteristic::CharacteristicType chartype = (characteristic::CharacteristicType)chars.value("char_type").toInt();
    column = QString("char%1").arg(chars.value("char_id").toString());
    name = chars.value("char_name").toString();
    list()->addColumn(name, -1, Qt::AlignLeft , false, column );
    if (chartype == characteristic::Text)
    {
      _charidstext.append(chars.value("char_id").toInt());
      parameterWidget()->append(name, column, ParameterWidget::Text);
    }
    else if (chartype == characteristic::List)
    {
      _charidslist.append(chars.value("char_id").toInt());
      QString sql =   QString("SELECT charopt_value, charopt_value "
                              "FROM charopt "
                              "WHERE (charopt_char_id=%1);")
          .arg(chars.value("char_id").toInt());
      parameterWidget()->append(name, column, ParameterWidget::Multiselect, QVariant(), false, sql);
    }
    else if (chartype == characteristic::Date)
    {
      _charidsdate.append(chars.value("char_id").toInt());
      parameterWidget()->append(name + tr(" Start Date"), column + "startDate", ParameterWidget::Date);
      parameterWidget()->append(name + tr(" End Date"), column + "endDate", ParameterWidget::Date);
    }
  }

  connect(omfgThis, SIGNAL(itemsUpdated(int, bool)), this, SLOT(sFillList()));
}


void items::sPopulateMenu(QMenu * pMenu, QTreeWidgetItem *, int)
{
  QAction *menuItem;

  menuItem = pMenu->addAction(tr("Edit..."), this, SLOT(sEdit()));
  menuItem->setEnabled(_privileges->check("MaintainItemMasters"));

  menuItem = pMenu->addAction(tr("View..."), this, SLOT(sView()));

  QAction *tmpaction = pMenu->addAction(tr("Delete..."));
  connect(tmpaction, SIGNAL(triggered()), this, SLOT(sDelete()));
  tmpaction->setEnabled(_privileges->check("DeleteItemMasters"));
  tmpaction->setObjectName("items.popup.delete");

  pMenu->addSeparator();

  menuItem = pMenu->addAction(tr("Copy..."), this, SLOT(sCopy()));
  menuItem->setEnabled(_privileges->check("MaintainItemMasters"));
}

void items::sNew()
{
  item::newItem();
}

void items::sEdit()
{
  item::editItem(list()->id());
}

void items::sView()
{
  item::viewItem(list()->id());
}

void items::sDelete()
{
  if (QMessageBox::information( this, tr("Delete Item"),
                                tr( "Are you sure that you want to delete the Item?"),
                                tr("&Delete"), tr("&Cancel"), 0, 0, 1 ) == 0  )
  {
    XSqlQuery qry;
    qry.prepare("SELECT deleteItem(:item_id) AS returnVal;");
    qry.bindValue(":item_id", list()->id());
    qry.exec();
    if (qry.first())
    {
      int returnVal = qry.value("returnVal").toInt();
      if (returnVal < 0)
      {
        systemError(this, storedProcErrorLookup("deleteItem", returnVal), __FILE__, __LINE__);
        return;
      }
      sFillList();
    }
    else if (qry.lastError().type() != QSqlError::NoError)
    {
      systemError(this, qry.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }   
}

bool items::setParams(ParameterList &params)
{
  if (!display::setParams(params))
    return false;

  // Handle characteristics
  QVariant param;
  bool valid;
  QString column;
  QStringList clauses;

  // Put together the list of text and date based charids used to build joins
  QList<QVariant> charids = _charidstext;
  for (int i = 0; i < _charidslist.count(); i++)
    charids.append(_charidslist.at(i));
  for (int i = 0; i < _charidsdate.count(); i++)
    charids.append(_charidsdate.at(i));
  params.append("char_id_list", charids);

  // Handle text based sections of clause
  for (int i = 0; i < _charidstext.count(); i++)
  {
    column = QString("char%1").arg(_charidstext.at(i).toString());
    param = params.value(column, &valid);
    if (valid)
      clauses.append(QString("charass_alias%1.charass_value ~* '%2'").arg(_charidstext.at(i).toString()).arg(param.toString()));
  }
  // Handle text based sections of clause
  for (int i = 0; i < _charidslist.count(); i++)
  {
    column = QString("char%1").arg(_charidslist.at(i).toString());
    param = params.value(column, &valid);
    if (valid)
    {
      QStringList list = param.toStringList();
      clauses.append(QString("charass_alias%1.charass_value IN ('%2') ").arg(_charidslist.at(i).toString()).arg(list.join("','")));
    }
  }
  // Handle date based sections of clause
  for (int i = 0; i < _charidsdate.count(); i++)
  {
    // Look for start date
    column = QString("char%1startDate").arg(_charidsdate.at(i).toString());
    param = params.value(column, &valid);
    if (valid)
      clauses.append(QString("charass_alias%1.charass_value::date >= '%2'").arg(_charidsdate.at(i).toString()).arg(param.toString()));

    // Look for end date
    column = QString("char%1endDate").arg(_charidsdate.at(i).toString());
    param = params.value(column, &valid);
    if (valid)
      clauses.append(QString("charass_alias%1.charass_value::date <= '%2'").arg(_charidsdate.at(i).toString()).arg(param.toString()));
  }
  if (clauses.count())
    params.append("charClause", clauses.join(" AND ").prepend(" AND "));

  return true;
}

void items::sCopy()
{
  ParameterList params;
  params.append("item_id", list()->id());

  copyItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}
