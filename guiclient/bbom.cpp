/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bbom.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <openreports.h>

#include "bbomItem.h"

bbom::bbom(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_bbomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

  _costsAbsorbed->setPrecision(omfgThis->percentVal());
  _item->setType(ItemLineEdit::cBreeder);

  _bbomitem->addColumn(tr("Item Number"), _itemColumn,  Qt::AlignLeft, true, "item_number");
  _bbomitem->addColumn(tr("Description"), -1,           Qt::AlignLeft, true, "descrip");
  _bbomitem->addColumn(tr("Type"),        _itemColumn,  Qt::AlignLeft, true, "type");
  _bbomitem->addColumn(tr("UOM"),         _uomColumn,   Qt::AlignCenter,true, "uom_name");
  _bbomitem->addColumn(tr("Qty."),        _qtyColumn,   Qt::AlignRight, true, "bbomitem_qtyper");
  _bbomitem->addColumn(tr("Effective"),   _dateColumn,  Qt::AlignCenter,true, "bbomitem_effective");
  _bbomitem->addColumn(tr("Expires"),     _dateColumn,  Qt::AlignCenter,true, "bbomitem_expires");
  _bbomitem->addColumn(tr("Cost %"),      _prcntColumn, Qt::AlignRight, true, "bbomitem_costabsorb");
  
  connect(omfgThis, SIGNAL(bbomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
bbom::~bbom()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void bbom::languageChange()
{
    retranslateUi(this);
}


enum SetResponse bbom::set(const ParameterList &pParams)
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
      connect(_bbomitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_bbomitem, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
      connect(_bbomitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
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

      _bbomitem->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _new->setEnabled(FALSE);
      _edit->setEnabled(FALSE);
      _delete->setEnabled(FALSE);
      _close->setText(tr("&Close"));

      connect(_bbomitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  return NoError;
}

void bbom::sPrint()
{
  if (!_item->isValid())
  {
    QMessageBox::warning( this, tr("Enter a Valid Item Number"),
                          tr("You must enter a valid Item Number for this report.") );
    _item->setFocus();
    return;
  }

  ParameterList params;
  params.append("item_id", _item->id());

  if(_showExpired->isChecked())
    params.append("showExpired");

  if(_showFuture->isChecked())
    params.append("showFuture");

  orReport report("BreederBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void bbom::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());

  bbomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bbom::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bbomitem_id", _bbomitem->id());

  bbomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bbom::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bbomitem_id", _bbomitem->id());

  bbomItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void bbom::sDelete()
{
  if (QMessageBox::warning( this, tr("Delete Breeder BOM Item"),
                            tr("Are you sure you want to delete this Breeder BOM Item?"),
                            tr("&Delete"), tr("&Cancel"), QString::null, 0, 1) == 0)
  {
    q.prepare( "DELETE FROM bbomitem "
               "WHERE (bbomitem_id=:bbomitem_id);" );
    q.bindValue(":bbomitem_id", _bbomitem->id());
    q. exec();
    omfgThis->sBBOMsUpdated(_item->id(), TRUE);
  }
}

void bbom::sMoveUp()
{
  q.prepare("SELECT moveBbomitemUp(:bbomitem_id);");
  q.bindValue(":bbomitem_id", _bbomitem->id());
  q.exec();
}

void bbom::sMoveDown()
{
  q.prepare("SELECT moveBbomitemDown(:bbomitem_id);");
  q.bindValue(":bbomitem_id", _bbomitem->id());
  q.exec();
}

void bbom::sFillList()
{
  sFillList(_item->id(), FALSE);
}

void bbom::sFillList(int pItemid, bool)
{
  if (_item->isValid() && (pItemid == _item->id()))
  {
    QString sql( "SELECT bbomitem.*, item_number,"
                 "       (item_descrip1 || ' ' || item_descrip2) AS descrip,"
                 "       CASE WHEN (item_type='C') THEN :coProduct"
                 "            WHEN (item_type='Y') THEN :byProduct"
                 "            ELSE :error"
                 "       END AS type,"
                 "       uom_name,"
		 "       'qty' AS bbomitem_qtyper_xtnumericrole,"
		 "       CASE WHEN (COALESCE(bbomitem_effective, startoftime()) = startoftime()) THEN :always"
		 "       END AS bbomitem_effective_qtdisplayrole,"
		 "       CASE WHEN (COALESCE(bbomitem_expires, endoftime()) = endoftime()) THEN :never"
		 "       END AS bbomitem_expires_qtdisplayrole,"
                 "       CASE WHEN (item_type='Y') THEN :na"
                 "       END AS bbomitem_costabsorb_qtdisplayrole,"
		 "       'scrap' AS bbomitem_costabsorb_xtnumericrole "
                 "FROM bbomitem, item, uom "
                 "WHERE ( (bbomitem_item_id=item_id)"
                 " AND (item_inv_uom_id=uom_id)"
                 " AND (bbomitem_parent_item_id=:item_id)" );

    if (!_showExpired->isChecked())
      sql += " AND (bbomitem_expires > CURRENT_DATE)";

    if (!_showFuture->isChecked())
      sql += " AND (bbomitem_effective <= CURRENT_DATE)";

    sql += ") "
           "ORDER BY bbomitem_seqnumber, bbomitem_effective";

    q.prepare(sql);
    q.bindValue(":coProduct", tr("Co-Product"));
    q.bindValue(":byProduct", tr("By-Product"));
    q.bindValue(":always",    tr("Always"));
    q.bindValue(":never",     tr("Never"));
    q.bindValue(":error",     tr("Error"));
    q.bindValue(":na",        tr("N/A"));
    q.bindValue(":item_id",   _item->id());
    q.exec();
    _bbomitem->populate(q);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "SELECT SUM(bbomitem_costabsorb) * 100 AS absorb "
               "FROM bbomitem "
               "WHERE (bbomitem_parent_item_id=:item_id);" );
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      _costsAbsorbed->setDouble(q.value("absorb").toDouble());

      if (q.value("absorb").toDouble() == 1.0)
        _costsAbsorbed->setPaletteForegroundColor(QColor("black"));
      else
        _costsAbsorbed->setPaletteForegroundColor(namedColor("error"));
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
  else if (!_item->isValid())
    _bbomitem->clear();
}

void bbom::sPopulateMenu(QMenu *)
{
}
