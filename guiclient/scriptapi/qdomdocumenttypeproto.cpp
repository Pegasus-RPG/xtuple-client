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

#include "qdomdocumenttypeproto.h"

#include <QDomDocumentType>
#include <QDomNamedNodeMap>
#include <QString>

void setupQDomDocumentTypeProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomDocumentTypeProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomDocumentType*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomDocumentType>(),  proto);

  QScriptValue constructor = engine->newFunction(constructQDomDocumentType,
                                                 proto);
  engine->globalObject().setProperty("QDomDocumentType",  constructor);
}

QScriptValue constructQDomDocumentType(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomDocumentType *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomDocumentType();
  return engine->toScriptValue(obj);
}

QDomDocumentTypeProto::QDomDocumentTypeProto(QObject *parent)
    : QObject(parent)
{
}

QDomNamedNodeMap QDomDocumentTypeProto:: entities()       const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->entities();
  return QDomNamedNodeMap();
}

QString QDomDocumentTypeProto:: internalSubset() const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->internalSubset();
  return QString();
}

QString QDomDocumentTypeProto:: name()           const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->name();
  return QString();
}

int QDomDocumentTypeProto:: nodeType()       const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QDomNamedNodeMap QDomDocumentTypeProto:: notations()      const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->notations();
  return QDomNamedNodeMap();
}

QString QDomDocumentTypeProto:: publicId()       const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->publicId();
  return QString();
}

QString QDomDocumentTypeProto:: systemId()       const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return item->systemId();
  return QString();
}

QString QDomDocumentTypeProto::toString() const
{
  QDomDocumentType *item = qscriptvalue_cast<QDomDocumentType*>(thisObject());
  if (item)
    return QString("QDomDocumentType()");
  return QString("QDomDocumentType(unknown)");
}
