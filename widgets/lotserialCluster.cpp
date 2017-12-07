/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "lotserialcluster.h"

#include <QMessageBox>
#include <QSqlError>

#include "errorReporter.h"
#include "xsqlquery.h"
#include "xtreewidget.h"

LotserialList::LotserialList(QWidget* pParent, Qt::WindowFlags flags)
  : VirtualList(pParent, flags)
{
  setObjectName("LotserialList");

  _listTab->headerItem()->setText(0, tr("Lot/Serial #"));
  _listTab->headerItem()->setText(1, tr("Item #"));
}

LotserialSearch::LotserialSearch(QWidget* pParent, Qt::WindowFlags flags)
  : VirtualSearch(pParent, flags)
{
  setObjectName("LotserialSearch");

  _listTab->headerItem()->setText(0, tr("Lot/Serial #"));
  _listTab->headerItem()->setText(1, tr("Item #"));
}

LotserialCluster::LotserialCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
  LotserialLineEdit* lsle=new LotserialLineEdit(this, pName);
  addNumberWidget(lsle);

  connect(lsle, SIGNAL(newItemId(int)), this, SIGNAL(newItemId(int)));
}

LotserialLineEdit::LotserialLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "ls", "ls_id", "ls_number", "item_number", "item_id", 0, pName)
{
  _itemid = -1;
  setTitles(tr("Lot/Serial Number"), tr("Lot/Serial Numbers"));
  _query = QString("SELECT ls_id AS id,"
                   "       ls_number AS number,"
                   "       item_id,"
                   "       item_number AS name,"
                   "       ls_notes AS description"
                   "  FROM ls"
                   "  JOIN item ON (ls_item_id=item_id)"
                   " WHERE true ");
  _strict = true;
}

int LotserialCluster::itemId() const
{
  if (_number && _number->inherits("LotserialLineEdit"))
    return ((LotserialLineEdit*)(_number))->itemId();
  return true;
}

void LotserialCluster::setItemId(const int p)
{
  if (_number && _number->inherits("LotserialLineEdit"))
    ((LotserialLineEdit*)(_number))->setItemId(p);
}

bool LotserialCluster::strict() const
{
  return isStrict();
}

void LotserialLineEdit::setItemId(const int itemid)
{
  if (_itemid == itemid)
    return;
  else if (itemid <= 0)
  {
    _itemid = -1;
    _extraClause = "";
  }
  else
  {
    _itemid = itemid;
    _extraClause = QString(" (item_id=%1) ").arg(itemid);
  }

  if (_id != -1)
  {
    XSqlQuery qry;
    qry.prepare("SELECT ls_id "
                "FROM ls "
                "WHERE ((ls_id=:ls_id) "
                " AND (ls_item_id=:item_id)); ");
    qry.bindValue(":ls_id", _id);
    qry.bindValue(":item_id", _itemid);
    qry.exec();
    if (!qry.first())
      VirtualClusterLineEdit::clear();
  }
}

void LotserialLineEdit::clear()
{
  VirtualClusterLineEdit::clear();
}

void LotserialLineEdit::setId(const int pId)
{
  if (pId == -1 || pId == 0)
  {
    clear();
    emit parsed();
  }
  else if (_x_metrics &&_x_metrics->boolean("LotSerialControl"))
  {
    bool changed = (pId != _id);
    silentSetId(pId);
    if (changed)
    {
      emit newId(pId);
      emit valid(_valid);
      XSqlQuery qq;
      qq.prepare("SELECT ls_item_id"
                 "  FROM ls"
                 " WHERE(ls_id=:id);");
      qq.bindValue(":id", id());
      qq.exec();
      if(qq.first())
        emit newItemId(qq.value("ls_item_id").toInt());
    }
  }
}

/* copied from virtualCluster.cpp but with one important difference:
   if a not-strict flag is set then warn the user but don't clear the
   lotserial field
 */
void LotserialLineEdit::sParse()
{
  if (_x_metrics && _x_metrics->boolean("LotSerialControl"))
  {
    if (_completerId)
    {
      int id = _completerId;
      _completerId = 0;
      setId(id);
    }
    else if (! _parsed)
    {
      QString stripped = text().trimmed().toUpper();
      if (stripped.isEmpty())
      {
        _parsed = true;
        setId(-1);
      }
      else
      {
        XSqlQuery numQ;
        numQ.prepare(_query + _numClause +
                      (_extraClause.isEmpty() ? "" : " AND " + _extraClause) +
                      QString("ORDER BY %1 LIMIT 1;").arg(_numColName));
        numQ.bindValue(":number", "^" + stripped);
        numQ.exec();
        if (numQ.first())
        {
          _valid = true;
          setId(numQ.value("id").toInt());
          _name = (numQ.value("name").toString());
          _itemid = (numQ.value("item_id").toInt());
        }
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Getting Lot/Serial Number"),
                                      numQ, __FILE__, __LINE__))
          return;
        else if (_strict || _itemid == -1)
          VirtualClusterLineEdit::clear();
        else if (isVisible() &&
                  QMessageBox::question(this, tr("Lot/Serial # Not Found"),
                  (_itemid > 0 ?
                    tr("This Lot/Serial # was not found for this item.") :
                    tr("This Lot/Serial # was not found.")) +
                  tr(" Are you sure it is correct?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No) == QMessageBox::No)
        {
          VirtualClusterLineEdit::clear();
          return;
        }
        else
        {
          int lsid = -1;

          numQ.prepare("INSERT INTO ls (ls_item_id, ls_number)"
                        "       VALUES (:item_id,   :number)"
                        " RETURNING ls_id;");
          numQ.bindValue(":item_id", _itemid);
          numQ.bindValue(":number",  stripped);
          numQ.exec();
          if (numQ.first())
            lsid = numQ.value("ls_id").toInt();
          else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Inserting Lot/Serial Number"),
                                        numQ, __FILE__, __LINE__))
            return;

          numQ.prepare("INSERT INTO itemloc "
                        "(itemloc_itemsite_id, itemloc_location_id, itemloc_qty, "
                        "itemloc_expiration, itemloc_ls_id) "
                        "VALUES (-1, -1, 0.0, "
                        "endOfTime(), :ls_id);");
          numQ.bindValue(":ls_id", lsid);
          numQ.exec();
          if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Creating Item Location Placeholder"),
                                   numQ, __FILE__, __LINE__))
            return;
          setId(lsid);
        }
      }
    }

    _parsed = true;
    emit parsed();
  }
}

LotserialList* LotserialLineEdit::listFactory()
{
  return new LotserialList(this);
}

LotserialSearch* LotserialLineEdit::searchFactory()
{
  return new LotserialSearch(this);
}

/*
LotserialInfo* LotserialLineEdit::infoFactory()
{
  return new LotserialInfo(this, "LotserialInfo", true);
}
*/
