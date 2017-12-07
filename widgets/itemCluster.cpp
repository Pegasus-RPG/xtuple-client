/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QCompleter>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QSqlRecord>
#include <QTreeView>
#include <QtScript>

#include <xsqlquery.h>

#include "guiclientinterface.h"
#include "itemcluster.h"
#include "itemAliasList.h"
#include "xcheckbox.h"
#include "xtreewidget.h"
#include "xsqltablemodel.h"

#define DEBUG false

QString buildItemLineEditQuery(const QString, const QStringList, const QString, const unsigned int, bool);
QString buildItemLineEditTitle(const unsigned int, const QString);

QString buildItemLineEditQuery(const QString pPre, const QStringList pClauses, const QString pPost, const unsigned int pType, bool unionAlias)
{
  QStringList clauses = pClauses;
  QString sql;
  if (unionAlias)
  {
    sql = pPre + " FROM ("
                 "   SELECT item_id, item_number, item_descrip1, item_descrip2, "
                 "      item_upccode, item_type, item_fractional, item_config, item_inv_uom_id, "
                 "      item_sold, item_active, NULL AS itemalias_crmacct_id "
                 "   FROM item "
                 "   UNION "
                 "   SELECT item_id, itemalias_number, "
                 "     CASE WHEN LENGTH(itemalias_descrip1) > 1 THEN itemalias_descrip1 ELSE item_descrip1 END, "
                 "     CASE WHEN LENGTH(itemalias_descrip2) > 1 THEN itemalias_descrip1 ELSE item_descrip2 END, "
                 "      item_upccode, item_type, item_fractional, item_config, item_inv_uom_id,"
                 "      item_sold, item_active, itemalias_crmacct_id "
                 "   FROM item "
                 "     JOIN itemalias ON item_id = itemalias_item_id "
                 ") AS item "
                 " JOIN uom ON (uom_id=item_inv_uom_id) ";
  }
  else
  {
    sql = pPre + " FROM item "
                 "      JOIN uom ON (uom_id=item_inv_uom_id)"
                 "      LEFT OUTER JOIN itemalias ON (itemalias_item_id=item_id)"
                 "      LEFT OUTER JOIN crmacct ON (crmacct_id=itemalias_crmacct_id)";
  }

  if (pType & (ItemLineEdit::cLocationControlled | ItemLineEdit::cLotSerialControlled | ItemLineEdit::cDefaultLocation | ItemLineEdit::cActive))
  {
    sql += ", itemsite";
    clauses << "(itemsite_item_id=item_id)";
  }

  if (pType & (ItemLineEdit::cHasBom))
  {
    sql += ", bomitem";
    clauses << "(bomitem_parent_item_id=item_id)";
  }

  if (pType & (ItemLineEdit::cUsedOnBom))
  {
    sql += ", bomitem";
    clauses << "(bomitem_item_id=item_id)";
  }

  if (pType & ItemLineEdit::cAllItemTypes_Mask)
  {
    QStringList types;

    if (pType & ItemLineEdit::cPurchased)
      types << "'P'";

    if (pType & ItemLineEdit::cManufactured)
      types << "'M'";

    if (pType & ItemLineEdit::cPhantom)
      types << "'F'";

    if (pType & ItemLineEdit::cBreeder)
      types << "'B'";

    if (pType & ItemLineEdit::cCoProduct)
      types << "'C'";

    if (pType & ItemLineEdit::cByProduct)
      types << "'Y'";

    if (pType & ItemLineEdit::cReference)
      types << "'R'";

    if (pType & ItemLineEdit::cCosting)
      types << "'S'";

    if (pType & ItemLineEdit::cTooling)
      types << "'T'";

    if (pType & ItemLineEdit::cOutsideProcess)
      types << "'O'";

    if (pType & ItemLineEdit::cPlanning)
      types << "'L'";

    if (pType & ItemLineEdit::cKit)
      types << "'K'";

    if (!types.isEmpty())
      clauses << QString("(item_type IN (" + types.join(",") + "))");
  }

  if (pType & (ItemLineEdit::cLocationControlled | ItemLineEdit::cLotSerialControlled | ItemLineEdit::cDefaultLocation))
  {
    QString sub;
    if (pType & ItemLineEdit::cLocationControlled)
      sub = "(itemsite_loccntrl)";

    if (pType & ItemLineEdit::cLotSerialControlled)
    {
      if (!sub.isEmpty())
        sub += " OR ";
      sub += "(itemsite_controlmethod IN ('L', 'S'))";
    }

    if (pType & ItemLineEdit::cDefaultLocation)
    {
      if (!sub.isEmpty())
        sub += " OR ";
      sub += "(useDefaultLocation(itemsite_id))";
    }

    clauses << QString("( " + sub + " )");
  }

  if (pType & ItemLineEdit::cSold)
    clauses << "(item_sold)";

  if (pType & (ItemLineEdit::cActive | ItemLineEdit::cItemActive))
    clauses << "(item_active)";

  if (!clauses.isEmpty())
    sql += " WHERE (" + clauses.join(" AND ") + ")";

  if (!pPost.isEmpty())
    sql += " " + pPost;

  sql += ";";

  return sql;
}

QString buildItemLineEditTitle(const unsigned int pType, const QString pPost)
{
  QString caption;
  unsigned int items = (0xFFFF & pType); // mask so this value only has the individual items

  if (pType & ItemLineEdit::cSold)
    caption += ItemLineEdit::tr("Sold ");

  if (pType & ItemLineEdit::cActive)
    caption = ItemLineEdit::tr("Active ");

  if (pType & ItemLineEdit::cLocationControlled)
    caption += ItemLineEdit::tr("Multiply Located ");
  else if (pType & ItemLineEdit::cLotSerialControlled)
    caption += ItemLineEdit::tr("Lot/Serial # Controlled ");

  else if (items == (ItemLineEdit::cGeneralPurchased | ItemLineEdit::cGeneralManufactured))
    caption += ItemLineEdit::tr("Purchased and Manufactured ");
  else if ((items == ItemLineEdit::cGeneralPurchased) || (items == ItemLineEdit::cPurchased) )
    caption += ItemLineEdit::tr("Purchased ");
  else if ( (items == ItemLineEdit::cGeneralManufactured) || (items == ItemLineEdit::cManufactured) )
    caption += ItemLineEdit::tr("Manufactured ");
  else if (items == ItemLineEdit::cBreeder)
    caption += ItemLineEdit::tr("Breeder ");
  else if (items == (ItemLineEdit::cCoProduct | ItemLineEdit::cByProduct))
    caption += ItemLineEdit::tr("Co-Product and By-Product ");
  else if (items == ItemLineEdit::cCosting)
    caption += ItemLineEdit::tr("Costing ");
  else if (items == ItemLineEdit::cGeneralComponents)
    caption += ItemLineEdit::tr("Component ");
  else if (items == ItemLineEdit::cKitComponents)
    caption += ItemLineEdit::tr("Kit Components ");

  if(!pPost.isEmpty())
    caption += pPost;

  return caption;
}


ItemLineEditDelegate::ItemLineEditDelegate(QObject *parent)
  : QItemDelegate(parent)
{
  _template = qobject_cast<ItemLineEdit *>(parent);
}

ItemLineEditDelegate::~ItemLineEditDelegate()
{
}

QWidget *ItemLineEditDelegate::createEditor(QWidget *parent,
                                            const QStyleOptionViewItem &/*option*/,
                                            const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    ItemLineEdit *widget = new ItemLineEdit(parent);
    // Make editor share certain properties with template
    widget->setType(_template->type());
    QStringList clauses = _template->getExtraClauseList();
    for (int i = 0; i < clauses.size(); i++)
        widget->addExtraClause(clauses.at(i));

    return widget;
}

void ItemLineEditDelegate::setEditorData(QWidget *editor,
                                           const QModelIndex &index) const
{
  const XSqlTableModel *model = qobject_cast<const XSqlTableModel *>(index.model());
  if (model) {
    ItemLineEdit *widget = qobject_cast<ItemLineEdit *>(editor);
    if (widget)
      widget->setItemNumber(model->data(index).toString());
  }
}

void ItemLineEditDelegate::setModelData(QWidget *editor,
                                          QAbstractItemModel *model,
                                          const QModelIndex &index) const
{
  if (!index.isValid())
    return;

  if (model) {
    ItemLineEdit *widget = qobject_cast<ItemLineEdit *>(editor);
    if (widget)
      model->setData(index, widget->itemNumber(), Qt::EditRole);
  }
}

ItemLineEdit::ItemLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "item", "item_id", "item_number", "(item_descrip1 || ' ' || item_descrip2) ", 0, 0, pName)
{
  if (DEBUG)
    qDebug("ItemLineEdit::ItemLineEdit(%p, %s) entered", pParent, pName);
  setObjectName(pName ? pName : "ItemLineEdit");

  setTitles(tr("Item"), tr("Items"));
  setUiName("item");
  setEditPriv("MaintainItemMasters");
  setViewPriv("ViewItemMasters");
  setNewPriv("MaintainItemMasters");

  setAcceptDrops(true);

  _type = cUndefined;
  _defaultType = cUndefined;
  _useQuery = false;
  _useValidationQuery = false;
  _itemNumber = "";
  _uom = "";
  _itemType = "";
  _id = -1;
  _configured = false;
  _fractional = false;
  _delegate = new ItemLineEditDelegate(this);

  connect(_aliasAct, SIGNAL(triggered()), this, SLOT(sAlias()));

  if (_x_preferences && ! _x_preferences->boolean("ClusterButtons"))
    menu()->insertAction(menu()->actions().at(2), _aliasAct);
}

void ItemLineEdit::setItemNumber(const QString& pNumber)
{
  if (DEBUG)
    qDebug("%s::setItemNumber(%s) entered",
           qPrintable(objectName()), qPrintable(pNumber));
  XSqlQuery item;
  bool      found = false;

  _parsed = true;

  if (pNumber == text())
    return;

  if (!pNumber.isEmpty())
  {
    if (_useValidationQuery)
    {
      item.prepare(_validationSql);
      item.bindValue(":item_number", pNumber);
      item.exec();
      if (item.first())
        found = true;
    }
    else if (_useQuery)
    {
      item.prepare(_sql);
      item.exec();
      found = (item.findFirst("item_number", pNumber) != -1);
    }
    else if (pNumber != QString::Null())
    {
      QString pre( "SELECT DISTINCT item_id, item_number, item_descrip1, item_descrip2,"
                   "                uom_name, item_type, item_config, item_fractional, item_upccode");

      QStringList clauses;
      clauses = _extraClauses;
      clauses << "(item_number=:item_number OR item_upccode=:item_number)";

      item.prepare(buildItemLineEditQuery(pre, clauses, QString::null, _type, false));
      item.bindValue(":item_number", pNumber);
      item.exec();

      if (item.size() > 1)
      {
        ParameterList params;
        params.append("search", pNumber);
        params.append("searchNumber");
        params.append("searchUpc");
        sSearch(params);
        return;
      }
      else
        found = item.first();
    }
  }
  if (found)
  {
    _itemNumber = pNumber;
    _uom        = item.value("uom_name").toString();
    _itemType   = item.value("item_type").toString();
    _configured = item.value("item_config").toBool();
    _fractional = item.value("item_fractional").toBool();
    _id         = item.value("item_id").toInt();
    _upc        = item.value("item_upccode").toInt();
    _valid      = true;

    setText(item.value("item_number").toString());

    emit aliasChanged("");
    emit typeChanged(_itemType);
    emit descrip1Changed(item.value("item_descrip1").toString());
    emit descrip2Changed(item.value("item_descrip2").toString());
    emit uomChanged(item.value("uom_name").toString());
    emit configured(item.value("item_config").toBool());
    emit fractional(item.value("item_fractional").toBool());
    emit upcChanged(item.value("item_upccode").toString());

    emit valid(true);
  }
  else
  {
    _itemNumber = "";
    _uom        = "";
    _itemType   = "";
    _id         = -1;
    _valid      = false;
    _upc        = "";

    setText("");

    emit aliasChanged("");
    emit typeChanged("");
    emit descrip1Changed("");
    emit descrip2Changed("");
    emit uomChanged("");
    emit configured(false);
    emit fractional(false);
    emit upcChanged("");

    emit valid(false);
  }
}

void ItemLineEdit::silentSetId(const int pId)
{
  if (DEBUG)
    qDebug("%s::silentSetId(%d) entered",
           qPrintable(objectName()), pId);

  XSqlQuery item;
  bool      found = false;

  _parsed = true;

  if (_useValidationQuery)
  {
    item.prepare(_validationSql);
    item.bindValue(":item_id", pId);
    item.exec();
    if (item.first())
      found = true;
  }
  else if (_useQuery)
  {
    item.prepare(_sql);
    item.exec();
    found = (item.findFirst("item_id", pId) != -1);
  }
  else if (pId != -1)
  {
    QString pre( "SELECT DISTINCT item_number, item_descrip1, item_descrip2,"
                 "                uom_name, item_type, item_config, item_fractional, item_upccode");

    QStringList clauses;
    clauses = _extraClauses;
    clauses << "(item_id=:item_id)";

    item.prepare(buildItemLineEditQuery(pre, clauses, QString::null, _type, false));
    item.bindValue(":item_id", pId);
    item.exec();

    found = item.first();
  }

  if (found)
  {
    if (completer())
    {
      disconnect(this, SIGNAL(textChanged(QString)), this, SLOT(sHandleCompleter()));
      static_cast<QSqlQueryModel* >(completer()->model())->setQuery(QSqlQuery());
    }

    _itemNumber = item.value("item_number").toString();
    _uom        = item.value("uom_name").toString();
    _itemType   = item.value("item_type").toString();
    _configured = item.value("item_config").toBool();
    _fractional = item.value("item_fractional").toBool();
    _upc        = item.value("item_upccode").toString();
    _id         = pId;
    _valid      = true;

    setText(item.value("item_number").toString());
    emit aliasChanged("");
    emit typeChanged(_itemType);
    emit descrip1Changed(item.value("item_descrip1").toString());
    emit descrip2Changed(item.value("item_descrip2").toString());
    emit uomChanged(item.value("uom_name").toString());
    emit configured(item.value("item_config").toBool());
    emit fractional(item.value("item_fractional").toBool());
    emit upcChanged(item.value("item_upccode").toString());

    emit valid(true);

    if (completer())
      connect(this, SIGNAL(textChanged(QString)), this, SLOT(sHandleCompleter()));
  }
  else
  {
    _itemNumber = "";
    _uom        = "";
    _itemType   = "";
    _id         = -1;
    _upc        = "";
    _valid      = false;

    setText("");

    emit aliasChanged("");
    emit typeChanged("");
    emit descrip1Changed("");
    emit descrip2Changed("");
    emit uomChanged("");
    emit configured(false);
    emit fractional(false);
    emit upcChanged("");

    emit valid(false);
  }
}

void ItemLineEdit::setId(int pId)
{
  if (DEBUG) qDebug("%s::setId(%d) entered", qPrintable(objectName()), pId);
  bool changed = (pId != _id);
  silentSetId(pId);
  if (changed)
  {
    emit privateIdChanged(_id);
    emit newId(_id);
  }
}

void ItemLineEdit::setItemsiteid(int pItemsiteid)
{
  if (DEBUG)
    qDebug("%s::setItemsiteId(%d) entered",
           qPrintable(objectName()), pItemsiteid);
  XSqlQuery itemsite;
  itemsite.prepare( "SELECT itemsite_item_id, itemsite_warehous_id "
                    "FROM itemsite "
                    "WHERE (itemsite_id=:itemsite_id);" );
  itemsite.bindValue(":itemsite_id", pItemsiteid);
  itemsite.exec();
  if (itemsite.first())
  {
    setId(itemsite.value("itemsite_item_id").toInt());
    emit warehouseIdChanged(itemsite.value("itemsite_warehous_id").toInt());
  }
  else
  {
    setId(-1);
    emit warehouseIdChanged(-1);
  }
}

void ItemLineEdit::setCRMAcctId(unsigned int pAcct)
{
  _crmacct = pAcct;
}

void ItemLineEdit::sInfo()
{
  ParameterList params;
  params.append("item_id", id());

  _guiClientInterface->openWindow("itemAvailabilityWorkbench", params, parentWidget()->window() , Qt::NonModal, Qt::Window);
}

void ItemLineEdit::sCopy()
{
  ParameterList params;
  params.append("item_id", id());

  QString uiName = "copyItem";

  QWidget* w = 0;
  w = _guiClientInterface->openWindow(uiName, params, parentWidget()->window() , Qt::WindowModal, Qt::Dialog);

  QDialog* newdlg = qobject_cast<QDialog*>(w);
  int id = newdlg->exec();
  if (id != QDialog::Rejected)
  {
    silentSetId(id);
    emit newId(_id);
    emit valid(_id != -1);
  }

  return;
}

void ItemLineEdit::sHandleCompleter()
{
  if (DEBUG) qDebug("%s::sHandleCompleter() entered", qPrintable(objectName()));
  if (!hasFocus())
    return;

  QString stripped = text().trimmed().toUpper();
  if (stripped.isEmpty())
    return;

  int width = 0;
  QSqlQueryModel* model = static_cast<QSqlQueryModel *>(_completer->model());
  QTreeView * view = static_cast<QTreeView *>(_completer->popup());
  _parsed = true;
  XSqlQuery numQ;

  if (_useQuery)
  {
    numQ.prepare(QString("SELECT *"
                         "  FROM (%1) data"
                         " WHERE (POSITION(:number IN item_number)=1)"
                         " LIMIT 10")
                 .arg(QString(_sql)).remove(";"));
    numQ.bindValue(":number", stripped);
  }
  else
  {
    QString pre( "SELECT DISTINCT item_id, item_number, "
                 "(item_descrip1 || ' ' || item_descrip2) AS itemdescrip, "
                 "item_upccode AS description " );

    QStringList clauses;
    clauses = _extraClauses;
    clauses << "((POSITION(:searchString IN item_number) = 1)"
            " OR (POSITION(:searchString IN item_upccode) = 1))";
    if (_crmacct > 0)
      clauses << QString("(itemalias_crmacct_id IS NULL OR itemalias_crmacct_id = %1)")
                    .arg(_crmacct);
    numQ.prepare(buildItemLineEditQuery(pre, clauses, QString::null, _type, true)
                              .replace(";"," ORDER BY item_number LIMIT 10;"));
    numQ.bindValue(":searchString", QString(text().trimmed().toUpper()));
  }

  numQ.exec();
  if (numQ.first())
  {
    int numberCol = numQ.record().indexOf("item_number");
    int descripCol = numQ.record().indexOf("itemdescrip");
    model->setQuery(numQ);
    _completer->setCompletionPrefix(stripped);
    for (int i = 0; i < model->columnCount(); i++)
    {
      if ( (i == numberCol) ||
           (i == descripCol) )
      {
        view->resizeColumnToContents(i);
        width += view->columnWidth(i);
      }
      else
        view->hideColumn(i);
    }
  }
  else
    model->setQuery(QSqlQuery());

  if (width > 350)
    width = 350;

  QRect rect;
  rect.setHeight(height());
  rect.setWidth(width);
  rect.setBottomLeft(QPoint(0, height() - 2));
  _completer->complete(rect);
  _parsed = false;
}

void ItemLineEdit::sUpdateMenu()
{
  VirtualClusterLineEdit::sUpdateMenu();
  if (_x_privileges)
    _infoAct->setEnabled(_x_privileges->check("ViewItemAvailabilityWorkbench") &&
                         _id != -1);
}

void ItemLineEdit::sList()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  ParameterList params;
  params.append("item_id", _id);

  if (queryUsed())
    params.append("sql", _sql);
  else
    params.append("itemType", _defaultType);

  if (!_extraClauses.isEmpty())
    params.append("extraClauses", _extraClauses);

  itemList* newdlg = listFactory();
  newdlg->set(params);

  int id;
  if ((id = newdlg->exec())!= _id)
    setId(id);

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

void ItemLineEdit::sSearch()
{
  ParameterList params;
  sSearch(params);
}

void ItemLineEdit::sSearch(ParameterList params)
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
  params.append("item_id", _id);

  if (queryUsed())
    params.append("sql", _sql);
  else
    params.append("itemType", _type);

  if (!_extraClauses.isEmpty())
    params.append("extraClauses", _extraClauses);

  itemSearch* newdlg = searchFactory();
  newdlg->set(params);

  QString stripped = text().trimmed();
  if(stripped.length())
  {
    XSqlQuery numQ;
    numQ.prepare(_query + _numClause +
                 (_extraClause.isEmpty() || !_strict ? "" : " AND " + _extraClause) +
                 QString("ORDER BY %1;").arg(_numColName));
    numQ.bindValue(":number", "^" + stripped);
    numQ.exec();
    if (numQ.first())
      newdlg->setQuery(numQ);
  }
  newdlg->setSearchText(text());
  int id;
  if ((id = newdlg->exec()) != QDialog::Rejected)
    setId(id);
  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

itemList* ItemLineEdit::listFactory()
{
  return new itemList(this);
}

itemSearch* ItemLineEdit::searchFactory()
{
  return new itemSearch(this);
}

void ItemLineEdit::sAlias()
{
  ParameterList params;

  if (queryUsed())
    params.append("sql", _sql);
  else
    params.append("itemType", _type);

  if (!_extraClauses.isEmpty())
    params.append("extraClauses", _extraClauses);

  if (_crmacct > 0)
    params.append("crmacct", _crmacct);

  itemAliasList newdlg(parentWidget(), "", true);
  newdlg.set(params);

  int itemaliasid;
  if ((itemaliasid = newdlg.exec()) != QDialog::Rejected)
  {
    XSqlQuery itemalias;
    itemalias.prepare( "SELECT itemalias_number, itemalias_item_id "
                       "FROM itemalias "
                       "WHERE (itemalias_id=:itemalias_id);" );
    itemalias.bindValue(":itemalias_id", itemaliasid);
    itemalias.exec();
    if (itemalias.first())
    {
      setId(itemalias.value("itemalias_item_id").toInt());
      emit aliasChanged(itemalias.value("itemalias_number").toString());
      focusNextPrevChild(true);
    }
  }
}

QString ItemLineEdit::itemNumber()
{
  sParse();
  return _itemNumber;
}

QString ItemLineEdit::uom()
{
  sParse();
  return _uom;
}

QString ItemLineEdit::upc()
{
  sParse();
  return _upc;
}

QString ItemLineEdit::itemType()
{
  sParse();
  return _itemType;
}

bool ItemLineEdit::isConfigured()
{
  sParse();
  return _configured;
}

bool ItemLineEdit::isFractional()
{
    sParse();
    return _fractional;
}

void ItemLineEdit::sParse()
{
  if (DEBUG)
    qDebug("%s::sParse() entered with parsed %d, text [%s], "
           "_useValidationQuery %d, _useQuery %d",
           qPrintable(objectName()), _parsed, qPrintable(text()),
           _useValidationQuery, _useQuery);
  if (_completerId)
  {
    int id = _completerId;
    _completerId = 0;
    setId(id);
  }
  else if (!_parsed)
  {
    QString stripped = text().trimmed().toUpper();
    _parsed = true;

    if (stripped.isEmpty())
    {
      setId(-1);
      return;
    }
    else if (_useValidationQuery)
    {
      XSqlQuery item;
      if (completer())
        item.prepare("SELECT item_id"
                     "  FROM item"
                     " WHERE ((POSITION(:searchString IN item_number) = 1)"
                     "     OR (POSITION(:searchString IN item_upccode) = 1));");
      else
        item.prepare("SELECT item_id"
                     "  FROM item"
                     " WHERE (item_number = :searchString"
                     "     OR item_upccode = :searchString);");

      item.bindValue(":searchString", stripped);
      item.exec();
      while (item.next())
      {
        int itemid = item.value("item_id").toInt();
        XSqlQuery oneq;
        oneq.prepare(_validationSql);
        oneq.bindValue(":item_id", itemid);
        oneq.exec();
        if (oneq.size() > 1)
        {
          ParameterList params;
          params.append("search", stripped);
          params.append("searchNumber");
          params.append("searchUpc");
          sSearch(params);
          return;
        }
        else if (oneq.first())
        {
          setId(itemid);
          return;
        }
      }
    }
    else if (_useQuery)
    {
      XSqlQuery item(_sql);
      while (item.next())
      {
        if (item.value("item_number").toString().startsWith(stripped) ||
            item.value("item_upccode").toString().startsWith(stripped))
        {
          setId(item.value("item_id").toInt());
          return;
        }
      }
    }
    else
    {
      XSqlQuery item;

      QString pre( "SELECT DISTINCT item_id, item_number AS number, "
                   "(item_descrip1 || ' ' || item_descrip2) AS name, "
                   "item_upccode AS description " );

      QStringList clauses;
      // first check item number
      clauses = _extraClauses;
      clauses << "(POSITION(:searchString IN item_number) = 1)";
      if (_crmacct > 0)
        clauses << QString("(itemalias_crmacct_id IS NULL OR itemalias_crmacct_id = %1)")
                      .arg(_crmacct);
      item.prepare(buildItemLineEditQuery(pre, clauses, QString::null, _type, true)
                               .replace(";"," ORDER BY item_number LIMIT 1;"));
      item.bindValue(":searchString", stripped);
      item.exec();
      if (item.first())
      {
        setId(item.value("item_id").toInt());
        if(_itemNumber != item.value("number").toString())
          emit aliasChanged(item.value("number").toString());
        return;
      }
      // item number not found, check upccode
      clauses = _extraClauses;
      clauses << "(POSITION(:searchString IN item_upccode) = 1)";
      item.prepare(buildItemLineEditQuery(pre, clauses, QString::null, _type, true)
                   .replace(";"," ORDER BY item_number LIMIT 1;"));
      item.bindValue(":searchString", stripped);
      item.exec();
      if (item.first())
      {
        setId(item.value("item_id").toInt());
        return;
      }
      if (_x_metrics && _x_metrics->boolean("AutoItemSearch"))
      {
        // nothing found, start search
        ParameterList params;
        params.append("search", stripped);
        params.append("searchNumber");
        params.append("searchUpc");
        params.append("searchAlias");
        sSearch(params);
        return;
      }
    }
    setId(-1);
  }
}

void ItemLineEdit::addExtraClause(const QString & pClause)
{
  _extraClauses << pClause;
}

ItemCluster::ItemCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
  setObjectName(pName);
  addNumberWidget(new ItemLineEdit(this, pName));
  setLabel(tr("Item Number:"));

//  Create the component Widgets
  QLabel *_uomLit = new QLabel(tr("UOM:"), this);
  _uomLit->setObjectName("_uomLit");
  _uomLit->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  _uom = new QLabel(this);
  _uom->setObjectName("_uom");
  _uom->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  _uom->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  _uom->setMinimumWidth(50);

  _grid->addWidget(_uomLit, 0, 2);
  _grid->addWidget(_uom, 0, 3);

  _descrip2 = new QLabel(this);
  _descrip2->setObjectName("_descrip2");
  _descrip2->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
  _descrip2->setMaximumWidth(300);
  _grid->addWidget(_descrip2, 3, 1, 1, -1);
  setDescriptionVisible(true);

//  Make some internal connections
  ItemLineEdit* itemNumber = static_cast<ItemLineEdit* >(_number);
  connect(itemNumber, SIGNAL(aliasChanged(const QString &)), this, SIGNAL(aliasChanged(const QString &)));
  connect(itemNumber, SIGNAL(privateIdChanged(int)), this, SIGNAL(privateIdChanged(int)));
  connect(itemNumber, SIGNAL(uomChanged(const QString &)), this, SIGNAL(uomChanged(const QString &)));
  connect(itemNumber, SIGNAL(descrip1Changed(const QString &)), this, SIGNAL(descrip1Changed(const QString &)));
  connect(itemNumber, SIGNAL(descrip2Changed(const QString &)), this, SIGNAL(descrip2Changed(const QString &)));
  connect(itemNumber, SIGNAL(warehouseIdChanged(int)), this, SIGNAL(warehouseIdChanged(int)));
  connect(itemNumber, SIGNAL(typeChanged(const QString &)), this, SIGNAL(typeChanged(const QString &)));
  connect(itemNumber, SIGNAL(upcChanged(const QString &)), this, SIGNAL(upcChanged(const QString &)));
  connect(itemNumber, SIGNAL(configured(bool)), this, SIGNAL(configured(bool)));
  connect(itemNumber, SIGNAL(fractional(bool)), this, SIGNAL(fractional(bool)));

  connect(itemNumber, SIGNAL(uomChanged(const QString &)), _uom, SLOT(setText(const QString &)));
  connect(itemNumber, SIGNAL(descrip1Changed(const QString &)), _description, SLOT(setText(const QString &)));
  connect(itemNumber, SIGNAL(descrip2Changed(const QString &)), _descrip2, SLOT(setText(const QString &)));
}

void ItemCluster::addNumberWidget(VirtualClusterLineEdit* pNumberWidget)
{
  VirtualClusterLineEdit *matchType = qobject_cast<VirtualClusterLineEdit *>(pNumberWidget);

  if(matchType == 0)
    return;

  _number = matchType;

  if (! _number)
    return;

  _grid->addWidget(_number, 0, 1);
  setFocusProxy(pNumberWidget);

  connect(_number,      SIGNAL(newId(int)),     this,    SIGNAL(newId(int)));
  connect(_number,      SIGNAL(parsed()),       this,    SLOT(sRefresh()));
  connect(_number,      SIGNAL(valid(bool)),    this,    SIGNAL(valid(bool)));
}

void ItemCluster::setDescriptionVisible(const bool p)
{
  _description->setVisible(p);
  _descrip2->setVisible(p);
}

void ItemCluster::setReadOnly(const bool pReadOnly)
{
  if (pReadOnly)
    _number->setEnabled(false);
  else
    _number->setEnabled(true);
}

void ItemCluster::setEnabled(const bool pEnabled)
{
  setReadOnly(!pEnabled);
}

void ItemCluster::setDisabled(bool pDisabled)
{
  setReadOnly(pDisabled);
}

void ItemCluster::setId(const int pId, const QString&)
{
  _number->setId(pId);
}

void ItemCluster::setItemNumber(QString pNumber)
{
  static_cast<ItemLineEdit* >(_number)->setItemNumber(pNumber);
}

void ItemCluster::setItemsiteid(int intPItemsiteid)
{
  static_cast<ItemLineEdit* >(_number)->setItemsiteid(intPItemsiteid);
}

void ItemCluster::setCRMAcctId(unsigned int intPCRMAcctid)
{
  static_cast<ItemLineEdit* >(_number)->setCRMAcctId(intPCRMAcctid);
}

void ItemCluster::setOrientation(Qt::Orientation orientation)
{
  _descrip2->setVisible(orientation == Qt::Vertical);
  VirtualCluster::setOrientation(orientation);
}

////////////////////////////////////////////////////////////////////////

itemList::itemList(QWidget* pParent, Qt::WindowFlags pFlags ) :
    VirtualList(pParent, pFlags)
{
  setObjectName( "itemList" );
  setAttribute(Qt::WA_DeleteOnClose);
  setMinimumWidth(600);

  _itemid = -1;
  _itemType = ItemLineEdit::cUndefined;
  _useQuery = false;

  setWindowTitle(tr("Item List"));

  _showInactive = new QCheckBox(tr("Show &Inactive Items"), this);
  _showInactive->setObjectName("_showInactive");
  _dialogLyt->insertWidget(1, _showInactive );
  _showMake = new QCheckBox(tr("&Make Items Only"), this);
  _showMake->setObjectName("_showMake");
  _showBuy = new QCheckBox(tr("&Buy Items Only"), this);
  _showBuy->setObjectName("_showBuy");
  _dialogLyt->insertWidget(2, _showMake );
  _dialogLyt->insertWidget(3, _showBuy );

  connect( _showInactive, SIGNAL( clicked() ), this, SLOT( sFillList() ) );
  connect( _showMake, SIGNAL( clicked() ), this, SLOT( sFillList() ) );
  connect( _showBuy, SIGNAL( clicked() ), this, SLOT( sFillList() ) );

  _listTab->setColumnCount(0);
  _listTab->addColumn(tr("Item Number"), 100, Qt::AlignLeft, true);
  _listTab->addColumn(tr("Description"),  -1, Qt::AlignLeft, true);
  _listTab->addColumn(tr("Bar Code"),    100, Qt::AlignLeft, true);
}

void itemList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();
  else
    _itemid = -1;

  param = pParams.value("sql", &valid);
  if (valid)
  {
    _sql = param.toString();
    _useQuery = true;
  }
  else
    _useQuery = false;

  param = pParams.value("itemType", &valid);
  if (valid)
  {
    _itemType = param.toUInt();
    setWindowTitle(buildItemLineEditTitle(_itemType, tr("Items")));
    _showMake->setChecked(_itemType & ItemLineEdit::cGeneralManufactured);
    _showBuy->setChecked(_itemType & ItemLineEdit::cGeneralPurchased);
  }
  else
  {
    _itemType = ItemLineEdit::cUndefined;
    _showMake->hide();
    _showBuy->hide();
  }

  param = pParams.value("extraClauses", &valid);
  if (valid)
    _extraClauses = param.toStringList();

  _showInactive->setChecked(false);
  _showInactive->setEnabled(!(_itemType & ItemLineEdit::cActive));
  if(!_showInactive->isEnabled())
    _showInactive->hide();

  param = pParams.value("caption", &valid);
  if (valid)
    setWindowTitle(param.toString());
  sFillList();
}

void itemList::sSearch(const QString &pTarget)
{
  if (_listTab->currentItem())
    _listTab->setCurrentItem(0);

  _listTab->clearSelection();

  int i;
  for (i = 0; i < _listTab->topLevelItemCount(); i++)
    if (_listTab->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive) ||
        _listTab->topLevelItem(i)->text(1).startsWith(pTarget, Qt::CaseInsensitive) ||
        _listTab->topLevelItem(i)->text(2).startsWith(pTarget, Qt::CaseInsensitive))
      break;

  if (i < _listTab->topLevelItemCount())
  {
    _listTab->setCurrentItem(_listTab->topLevelItem(i));
    _listTab->scrollToItem(_listTab->topLevelItem(i));
  }
}

void itemList::sFillList()
{
  _listTab->clear();
  if (_useQuery)
  {
    _listTab->populate(_sql, _itemid);
  }
  else
  {
      QString pre;
      QString post;
      if(_x_preferences && _x_preferences->boolean("ListNumericItemNumbersFirst"))
      {
        pre = "SELECT DISTINCT item_id, item_number,  "
              "(item_descrip1 || ' ' || item_descrip2) AS itemdescrip, item_upccode, isNumeric(item_number) ";
        post = "ORDER BY isNumeric(item_number) DESC, item_number, item_upccode;";
      }
      else
      {
        pre =  "SELECT DISTINCT item_id, item_number,"
               "(item_descrip1 || ' ' || item_descrip2) AS itemdescrip, item_upccode ";
        post = "ORDER BY item_number;";
      }

      QStringList clauses;
      clauses = _extraClauses;
      if(!(_itemType & ItemLineEdit::cActive) && !_showInactive->isChecked())
        clauses << "(item_active)";

      if (_showMake->isChecked())
            _itemType = (_itemType | ItemLineEdit::cGeneralManufactured);
      else if (_itemType & ItemLineEdit::cGeneralManufactured)
            _itemType = (_itemType ^ ItemLineEdit::cGeneralManufactured);

      if (_showBuy->isChecked())
            _itemType = (_itemType | ItemLineEdit::cGeneralPurchased);
      else if (_itemType & ItemLineEdit::cGeneralPurchased)
            _itemType = (_itemType ^ ItemLineEdit::cGeneralPurchased);

      setWindowTitle(buildItemLineEditTitle(_itemType, tr("Items")));

      _listTab->populate(buildItemLineEditQuery(pre, clauses, post, _itemType, false), _itemid);
  }
}

void itemList::reject()
{
  done(_itemid);
}

void itemList::showEvent(QShowEvent *e)
{
  QDialog::showEvent(e);
}

//////////////////////////////////////////////////////////////////////

itemSearch::itemSearch(QWidget* pParent, Qt::WindowFlags pFlags)
    : VirtualSearch(pParent, pFlags)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setObjectName( "itemSearch" );
  setMinimumWidth(600);

  _itemid = -1;
  _itemType = ItemLineEdit::cUndefined;
  _useQuery = false;

  setWindowTitle( tr( "Search for Item" ) );

  _searchName->setText(tr("Search through Description 1"));
  _searchDescrip->show();
  _searchDescrip->setText(tr("Search through Description 2"));

  _searchUpc = new XCheckBox(tr("Search through Bar Code"), this);
  _searchUpc->setChecked( true );
  _searchUpc->setObjectName("_searchUpc");
  selectorsLyt->addWidget(_searchUpc, 3, 0);

  _searchAlias = new XCheckBox(tr("Search through Alias Number"), this);
  _searchAlias->setChecked( true );
  _searchAlias->setObjectName("_searchAlias");
  selectorsLyt->addWidget(_searchAlias, 4, 0);

  _showInactive = new XCheckBox(tr("Show &Inactive Items"), this);
  _showInactive->setObjectName("_showInactive");
  selectorsLyt->addWidget(_showInactive, 5, 0);

  // signals and slots connections
  connect( _showInactive, SIGNAL( clicked() ), this, SLOT( sFillList() ) );
  connect( _searchName, SIGNAL(clicked() ), this, SLOT( sFillList() ) );
  connect( _searchUpc, SIGNAL( clicked() ), this, SLOT( sFillList() ) );
  connect( _searchAlias, SIGNAL( clicked() ), this, SLOT( sFillList() ) );

  _listTab->setColumnCount(0);
  _listTab->addColumn(tr("Item Number"),  100,  Qt::AlignLeft,  true, "item_number" );
  _listTab->addColumn(tr("Description"),   -1,  Qt::AlignLeft,  true, "itemdescrip" );
  _listTab->addColumn(tr("Bar Code"),     100,  Qt::AlignLeft,  true, "item_upccode" );
  _listTab->addColumn(tr("Active"),        50,  Qt::AlignLeft,  true, "item_active" );
  _listTab->addColumn(tr("Alias Number"), 100,  Qt::AlignLeft,  true, "itemalias_number" );
  _listTab->addColumn(tr("Account"),      100,  Qt::AlignLeft,  true, "crmacct_name" );
}

void itemSearch::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();
  else
    _itemid = -1;

  param = pParams.value("sql", &valid);
  if (valid)
  {
    _sql = param.toString();
    _useQuery = true;
  }
  else
    _useQuery = false;

  param = pParams.value("itemType", &valid);
  if (valid)
  {
    _itemType = param.toUInt();
    setWindowTitle(buildItemLineEditTitle(_itemType, tr("Items")));
  }
  else
    _itemType = ItemLineEdit::cUndefined;

  param = pParams.value("extraClauses", &valid);
  if (valid)
    _extraClauses = param.toStringList();

  _showInactive->setChecked(false);
  _showInactive->setEnabled(!(_itemType & ItemLineEdit::cActive));

  param = pParams.value("caption", &valid);
  if (valid)
    setWindowTitle(param.toString());

  param = pParams.value("search", &valid);
  if (valid)
    _search->setText(param.toString());

  param = pParams.value("searchNumber", &valid);
  if (valid)
    _searchNumber->setChecked(true);

  param = pParams.value("searchUpc", &valid);
  if (valid)
    _searchUpc->setChecked(true);

  param = pParams.value("searchAlias", &valid);
  if (valid)
    _searchAlias->setChecked(true);

  sFillList();
}

void itemSearch::sFillList()
{
  _search->setText(_search->text().trimmed().toUpper());
  if (_search->text().length() == 0)
    return;

  QString sql;

  if (_useQuery)
  {
    QStringList clauses;
    if (!_showInactive->isChecked())
      clauses << "(item_active)";

    QStringList subClauses;
    if (_searchNumber->isChecked())
      subClauses << "(item_number ~* :searchString)";

    if (_searchName->isChecked())
      subClauses << "(item_descrip1 ~* :searchString)";

    if (_searchDescrip->isChecked())
      subClauses << "(item_descrip2 ~* :searchString)";

    if (_searchUpc->isChecked())
      subClauses << "(item_upccode ~* :searchString)";

    if (_searchAlias->isChecked())
      subClauses << "(itemalias_number ~* :searchString)";

    if(!subClauses.isEmpty())
      clauses << QString("( " + subClauses.join(" OR ") + " )");

    sql = "SELECT * FROM (" + _sql + ") AS dummy WHERE (" +
          clauses.join(" AND ") + ");" ;
  }
  else
  {
    if ( (!_searchNumber->isChecked()) &&
         (!_searchName->isChecked()) &&
         (!_searchDescrip->isChecked()) &&
         (!_searchUpc->isChecked()) &&
         (!_searchAlias->isChecked()) )
    {
      _listTab->clear();
      return;
    }

    QString pre;
    QString post;
    if(_x_preferences && _x_preferences->boolean("ListNumericItemNumbersFirst"))
    {
      pre =  "SELECT DISTINCT ON (toNumeric(item_number, 999999999999999), item_number)"
             "       item_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, item_upccode,"
             "       item_active, itemalias_number, crmacct_name";
      post = "ORDER BY toNumeric(item_number, 999999999999999), item_number";
    }
    else
    {
      pre =  "SELECT DISTINCT"
             "       item_id, item_number, (item_descrip1 || ' ' || item_descrip2) AS itemdescrip, item_upccode,"
             "       item_active, itemalias_number, crmacct_name";
      post = "ORDER BY item_number";
    }

    QStringList clauses;
    clauses = _extraClauses;
    if(!(_itemType & ItemLineEdit::cActive) && !_showInactive->isChecked())
      clauses << "(item_active)";

    QStringList subClauses;
    if (_searchNumber->isChecked())
      subClauses << "(item_number ~* :searchString)";

    if (_searchName->isChecked())
      subClauses << "(item_descrip1 ~* :searchString)";

    if (_searchDescrip->isChecked())
      subClauses << "(item_descrip2 ~* :searchString)";

    if (_searchUpc->isChecked())
      subClauses << "(item_upccode ~* :searchString)";

    if (_searchAlias->isChecked())
      subClauses << "(itemalias_number ~* :searchString)";

    if(!subClauses.isEmpty())
      clauses << QString("( " + subClauses.join(" OR ") + " )");

    sql = buildItemLineEditQuery(pre, clauses, post, _itemType, false);
  }

  XSqlQuery search;
  search.prepare(sql);
  search.bindValue(":searchString", _search->text());
  search.exec();
  _listTab->populate(search, _itemid);
}

// script api //////////////////////////////////////////////////////////////////

void setupItemLineEdit(QScriptEngine *engine)
{
  // cannot use engine->newQMetaObject because cActive goes negative
  QScriptValue::PropertyFlags ro = QScriptValue::ReadOnly | QScriptValue::Undeletable;
  QScriptValue widget = engine->globalObject().property("ItemLineEdit");
  if (! widget.isObject()) {
    widget = engine->newObject();
    engine->globalObject().setProperty("ItemLineEdit", widget, ro);
  }

  widget.setProperty("cUndefined",          QScriptValue(engine, ItemLineEdit::cUndefined),          ro);
  widget.setProperty("cPurchased",          QScriptValue(engine, ItemLineEdit::cPurchased),          ro);
  widget.setProperty("cManufactured",       QScriptValue(engine, ItemLineEdit::cManufactured),       ro);
  widget.setProperty("cPhantom",            QScriptValue(engine, ItemLineEdit::cPhantom),            ro);
  widget.setProperty("cBreeder",            QScriptValue(engine, ItemLineEdit::cBreeder),            ro);
  widget.setProperty("cCoProduct",          QScriptValue(engine, ItemLineEdit::cCoProduct),          ro);
  widget.setProperty("cByProduct",          QScriptValue(engine, ItemLineEdit::cByProduct),          ro);
  widget.setProperty("cReference",          QScriptValue(engine, ItemLineEdit::cReference),          ro);
  widget.setProperty("cCosting",            QScriptValue(engine, ItemLineEdit::cCosting),            ro);
  widget.setProperty("cTooling",            QScriptValue(engine, ItemLineEdit::cTooling),            ro);
  widget.setProperty("cOutsideProcess",     QScriptValue(engine, ItemLineEdit::cOutsideProcess),     ro);
  widget.setProperty("cPlanning",           QScriptValue(engine, ItemLineEdit::cPlanning),           ro);
  widget.setProperty("cKit",                QScriptValue(engine, ItemLineEdit::cKit),                ro);
  widget.setProperty("cAllItemTypes_Mask",  QScriptValue(engine, ItemLineEdit::cAllItemTypes_Mask),  ro);
  widget.setProperty("cPlanningMRP",        QScriptValue(engine, ItemLineEdit::cPlanningMRP),        ro);
  widget.setProperty("cPlanningMPS",        QScriptValue(engine, ItemLineEdit::cPlanningMPS),        ro);
  widget.setProperty("cPlanningNone",       QScriptValue(engine, ItemLineEdit::cPlanningNone),       ro);
  widget.setProperty("cPlanningAny",        QScriptValue(engine, ItemLineEdit::cPlanningAny),        ro);
  widget.setProperty("cItemActive",         QScriptValue(engine, ItemLineEdit::cItemActive),         ro);
  widget.setProperty("cSold",               QScriptValue(engine, ItemLineEdit::cSold),               ro);
  widget.setProperty("cLocationControlled", QScriptValue(engine, ItemLineEdit::cLocationControlled), ro);
  widget.setProperty("cLotSerialControlled",QScriptValue(engine, ItemLineEdit::cLotSerialControlled),ro);
  widget.setProperty("cDefaultLocation",    QScriptValue(engine, ItemLineEdit::cDefaultLocation),    ro);
  widget.setProperty("cActive",             QScriptValue(engine, ItemLineEdit::cActive),             ro);
  widget.setProperty("cGeneralManufactured",QScriptValue(engine, ItemLineEdit::cGeneralManufactured),ro);
  widget.setProperty("cGeneralPurchased",   QScriptValue(engine, ItemLineEdit::cGeneralPurchased),   ro);
  widget.setProperty("cGeneralComponents",  QScriptValue(engine, ItemLineEdit::cGeneralComponents),  ro);
  widget.setProperty("cGeneralInventory",   QScriptValue(engine, ItemLineEdit::cGeneralInventory),   ro);
  widget.setProperty("cKitComponents",      QScriptValue(engine, ItemLineEdit::cKitComponents),      ro);
}
