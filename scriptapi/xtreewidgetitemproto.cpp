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

#include "xtreewidgetitemproto.h"
#include "xtreewidget.h"

void setupXTreeWidgetItemProto(QScriptEngine *engine)
{
  QScriptValue itemproto = engine->newQObject(new XTreeWidgetItemProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<XTreeWidgetItem*>(), itemproto);
}

XTreeWidgetItemProto::XTreeWidgetItemProto(QObject *parent)
  : QObject(parent)
{
}

int XTreeWidgetItemProto::altId() const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->altId();
  return -1;
}

int XTreeWidgetItemProto::id()    const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->id();
  return -1;
}

void XTreeWidgetItemProto::setAltId(const int pId)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->setAltId(pId);
}

void XTreeWidgetItemProto::setId(const int pId)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->setId(pId);
}

XTreeWidgetItem *XTreeWidgetItemProto::child(int idx) const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->child(idx);
  return 0;
}    

int XTreeWidgetItemProto::id(const QString p)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->id(p);
  return -1;
}

QVariant XTreeWidgetItemProto::rawValue(const QString pName)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->rawValue(pName);
  return QVariant();
}

void XTreeWidgetItemProto::setText(int pColumn, const QVariant & pVariant)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    item->setText(pColumn, pVariant.toString());
}

void XTreeWidgetItemProto::setTextColor(int column, const QColor & color)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    item->setTextColor(column, color);
}

void XTreeWidgetItemProto::setTextColor(const QColor &pColor)
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    item->setTextColor(pColor);
}

QString XTreeWidgetItemProto::text(int p) const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->text(p);
  return QString();
}

QString XTreeWidgetItemProto::text(const QString &pColumn) const
{
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
    return item->text(pColumn);
  return QString();
}

QString XTreeWidgetItemProto::toString() const
{
  QString returnVal = QString("XTreeWidgetItem");
  XTreeWidgetItem *item = qscriptvalue_cast<XTreeWidgetItem*>(thisObject());
  if (item)
  {
    QStringList data;
    for (int i = 0; i < item->columnCount(); i++)
      data << item->text(i);
    returnVal += QString("(id = %1, altId = %2, data = [%3])")
      .arg(item->id()).arg(item->altId()).arg(data.join(", "));
  }
  else
    returnVal += "(unknown)";

  return returnVal;
}
