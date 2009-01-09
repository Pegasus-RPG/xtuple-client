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

#include "xdatawidgetmapperproto.h"
#include "xdatawidgetmapper.h"

void setupXDataWidgetMapperProto(QScriptEngine *engine)
{
  QScriptValue mapperproto = engine->newQObject(new XDataWidgetMapperProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<XDataWidgetMapper*>(), mapperproto);
}

XDataWidgetMapperProto::XDataWidgetMapperProto(QObject *parent)
  : QObject(parent)
{
}

/* compiler errors (confusion between int and char*)
void XDataWidgetMapperProto::addMapping(QWidget *widget, int section)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, section);
}
*/

/* compiler errors (confusion between int and char*)
void XDataWidgetMapperProto::addMapping(QWidget *widget, int section, const QByteArray &propertyName)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, section, propertyName);
}
*/

void XDataWidgetMapperProto::clearMapping()
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->clearMapping();
}

int XDataWidgetMapperProto::currentIndex() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->currentIndex();
  return -1;
}

QAbstractItemDelegate *XDataWidgetMapperProto::itemDelegate() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->itemDelegate();
  return 0;
}

QByteArray XDataWidgetMapperProto::mappedPropertyName(QWidget *widget) const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedPropertyName(widget);
  return QByteArray();
}

int XDataWidgetMapperProto::mappedSection(QWidget *widget) const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedSection(widget);
  return -1;
}

QWidget *XDataWidgetMapperProto::mappedWidgetAt(int section) const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedWidgetAt(section);
  return 0;
}

QAbstractItemModel *XDataWidgetMapperProto::model() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->model();
  return 0;
}

int XDataWidgetMapperProto::orientation() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return (int)(item->orientation());
  return 0;
}

void XDataWidgetMapperProto::removeMapping(QWidget *widget)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->removeMapping(widget);
}

QModelIndex XDataWidgetMapperProto::rootIndex() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->rootIndex();
  return QModelIndex();
}

void XDataWidgetMapperProto::setItemDelegate(QAbstractItemDelegate *delegate)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setItemDelegate(delegate);
}

void XDataWidgetMapperProto::setModel(QAbstractItemModel *model)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setModel(model);
}

void XDataWidgetMapperProto::setOrientation(int orientation)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setOrientation((Qt::Orientation)orientation);
}

void XDataWidgetMapperProto::setRootIndex(const QModelIndex &index)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setRootIndex(index);
}

void XDataWidgetMapperProto::setSubmitPolicy(int policy)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->setSubmitPolicy((QDataWidgetMapper::SubmitPolicy)policy);
}

int XDataWidgetMapperProto::submitPolicy() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->submitPolicy();
  return -1;
}

QByteArray XDataWidgetMapperProto::mappedDefaultName(QWidget *widget)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return item->mappedDefaultName(widget);
  return QByteArray();
}

void XDataWidgetMapperProto::addMapping(QWidget *widget, QString fieldName)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, fieldName);
}

void XDataWidgetMapperProto::addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, fieldName, propertyName);
}

void XDataWidgetMapperProto::addMapping(QWidget *widget, QString fieldName, const QByteArray &propertyName, const QByteArray &defaultName)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->addMapping(widget, fieldName, propertyName, defaultName);
}

void XDataWidgetMapperProto::removeDefault(QWidget *widget)
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    item->removeDefault(widget);
}

QString XDataWidgetMapperProto::toString() const
{
  XDataWidgetMapper *item = qscriptvalue_cast<XDataWidgetMapper*>(thisObject());
  if (item)
    return QString("XDataWidgetMapper(%1)").arg(item->objectName());
  return QString("XDataWidgetMapper(unknown)");
}
