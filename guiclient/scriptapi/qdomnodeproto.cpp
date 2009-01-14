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

#include "qdomnodeproto.h"

#include <QDomAttr>
#include <QDomCDATASection>
#include <QDomCharacterData>
#include <QDomComment>
#include <QDomDocument>
#include <QDomDocumentFragment>
#include <QDomDocumentType>
#include <QDomElement>
#include <QDomEntity>
#include <QDomEntityReference>
#include <QDomImplementation>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomNotation>
#include <QDomProcessingInstruction>
#include <QDomText>

void setupQDomNodeProto(QScriptEngine *engine)
{
  QScriptValue proto = engine->newQObject(new QDomNodeProto(engine));
  engine->setDefaultPrototype(qMetaTypeId<QDomNode*>(), proto);
  engine->setDefaultPrototype(qMetaTypeId<QDomNode>(), proto);

  QScriptValue constructor = engine->newFunction(constructQDomNode,
                                                 proto);
  engine->globalObject().setProperty("QDomNode",  constructor);
}

QScriptValue constructQDomNode(QScriptContext * /*context*/,
                                    QScriptEngine  *engine)
{
  QDomNode *obj = 0;
  // if (context->argumentCount() > 0)
  // else
    obj = new QDomNode();
  return engine->toScriptValue(obj);
}

QDomNodeProto::QDomNodeProto(QObject *parent)
    : QObject(parent)
{
}

QDomNode QDomNodeProto:: appendChild(const QDomNode& newChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->appendChild(newChild);
  return QDomNode();
}

QDomNamedNodeMap QDomNodeProto:: attributes() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->attributes();
  return QDomNamedNodeMap();
}

QDomNodeList QDomNodeProto:: childNodes() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->childNodes();
  return QDomNodeList();
}

void QDomNodeProto:: clear()
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->clear();
}

QDomNode QDomNodeProto:: cloneNode(bool deep) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->cloneNode(deep);
  return QDomNode();
}

int QDomNodeProto:: columnNumber() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->columnNumber();
  return 0;
}

QDomNode QDomNodeProto:: firstChild() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->firstChild();
  return QDomNode();
}

QDomElement QDomNodeProto:: firstChildElement(const QString &tagName) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->firstChildElement(tagName);
  return QDomElement();
}

bool QDomNodeProto:: hasAttributes() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->hasAttributes();
  return false;
}

bool QDomNodeProto:: hasChildNodes() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->hasChildNodes();
  return false;
}

QDomNode QDomNodeProto:: insertAfter(const QDomNode& newChild, const QDomNode& refChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->insertAfter(newChild, refChild);
  return QDomNode();
}

QDomNode QDomNodeProto:: insertBefore(const QDomNode& newChild, const QDomNode& refChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->insertBefore(newChild, refChild);
  return QDomNode();
}

bool QDomNodeProto:: isAttr() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isAttr();
  return false;
}

bool QDomNodeProto:: isCDATASection() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isCDATASection();
  return false;
}

bool QDomNodeProto:: isCharacterData() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isCharacterData();
  return false;
}

bool QDomNodeProto:: isComment() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isComment();
  return false;
}

bool QDomNodeProto:: isDocument() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isDocument();
  return false;
}

bool QDomNodeProto:: isDocumentFragment() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isDocumentFragment();
  return false;
}

bool QDomNodeProto:: isDocumentType() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isDocumentType();
  return false;
}

bool QDomNodeProto:: isElement() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isElement();
  return false;
}

bool QDomNodeProto:: isEntity() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isEntity();
  return false;
}

bool QDomNodeProto:: isEntityReference() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isEntityReference();
  return false;
}

bool QDomNodeProto:: isNotation() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isNotation();
  return false;
}

bool QDomNodeProto:: isNull() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isNull();
  return false;
}

bool QDomNodeProto:: isProcessingInstruction() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isProcessingInstruction();
  return false;
}

bool QDomNodeProto:: isSupported(const QString& feature, const QString& version) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isSupported(feature, version);
  return false;
}

bool QDomNodeProto:: isText() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->isText();
  return false;
}

QDomNode QDomNodeProto:: lastChild() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->lastChild();
  return QDomNode();
}

QDomElement QDomNodeProto:: lastChildElement(const QString &tagName) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->lastChildElement(tagName);
  return QDomElement();
}

int QDomNodeProto:: lineNumber() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->lineNumber();
  return 0;
}

QString QDomNodeProto:: localName() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->localName();
  return QString();
}

QDomNode QDomNodeProto:: namedItem(const QString& name) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->namedItem(name);
  return QDomNode();
}

QString QDomNodeProto:: namespaceURI() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->namespaceURI();
  return QString();
}

QDomNode QDomNodeProto:: nextSibling() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nextSibling();
  return QDomNode();
}

QDomElement QDomNodeProto:: nextSiblingElement(const QString &taName) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nextSiblingElement(taName);
  return QDomElement();
}

QString QDomNodeProto:: nodeName() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nodeName();
  return QString();
}

int QDomNodeProto::nodeType() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nodeType();
  return 0;
}

QString QDomNodeProto:: nodeValue() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->nodeValue();
  return QString();
}

void QDomNodeProto:: normalize()
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->normalize();
}

QDomDocument QDomNodeProto:: ownerDocument() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->ownerDocument();
  return QDomDocument();
}

QDomNode QDomNodeProto:: parentNode() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->parentNode();
  return QDomNode();
}

QString QDomNodeProto:: prefix() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->prefix();
  return QString();
}

QDomNode QDomNodeProto:: previousSibling() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->previousSibling();
  return QDomNode();
}

QDomElement QDomNodeProto:: previousSiblingElement(const QString &tagName) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->previousSiblingElement(tagName);
  return QDomElement();
}

QDomNode QDomNodeProto:: removeChild(const QDomNode& oldChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->removeChild(oldChild);
  return QDomNode();
}

QDomNode QDomNodeProto:: replaceChild(const QDomNode& newChild, const QDomNode& oldChild)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->replaceChild(newChild, oldChild);
  return QDomNode();
}

void QDomNodeProto:: save(QTextStream& stream, int intarg) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->save(stream, intarg);
}

void QDomNodeProto:: save(QTextStream& stream, int intarg, int policy) const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->save(stream, intarg, (QDomNode::EncodingPolicy)policy);
}

void QDomNodeProto:: setNodeValue(const QString&value)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->setNodeValue(value);
}

void QDomNodeProto:: setPrefix(const QString& pre)
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    item->setPrefix(pre);
}

QDomAttr QDomNodeProto:: toAttr() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toAttr();
  return QDomAttr();
}

QDomCDATASection QDomNodeProto:: toCDATASection() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toCDATASection();
  return QDomCDATASection();
}

QDomCharacterData QDomNodeProto:: toCharacterData() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toCharacterData();
  return QDomCharacterData();
}

QDomComment QDomNodeProto:: toComment() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toComment();
  return QDomComment();
}

QDomDocument QDomNodeProto:: toDocument() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toDocument();
  return QDomDocument();
}

QDomDocumentFragment QDomNodeProto:: toDocumentFragment() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toDocumentFragment();
  return QDomDocumentFragment();
}

QDomDocumentType QDomNodeProto:: toDocumentType() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toDocumentType();
  return QDomDocumentType();
}

QDomElement QDomNodeProto:: toElement() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toElement();
  return QDomElement();
}

QDomEntity QDomNodeProto:: toEntity() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toEntity();
  return QDomEntity();
}

QDomEntityReference QDomNodeProto:: toEntityReference() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toEntityReference();
  return QDomEntityReference();
}

QDomNotation QDomNodeProto:: toNotation() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toNotation();
  return QDomNotation();
}

QDomProcessingInstruction QDomNodeProto:: toProcessingInstruction() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toProcessingInstruction();
  return QDomProcessingInstruction();
}

QDomText QDomNodeProto:: toText() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return item->toText();
  return QDomText();
}

QString QDomNodeProto::toString() const
{
  QDomNode *item = qscriptvalue_cast<QDomNode*>(thisObject());
  if (item)
    return QString("QDomNode(name %1, type %2, %3)")
                    .arg(item->nodeName())
                    .arg(item->nodeType())
                    .arg(item->hasChildNodes() ? "has children" : "leaf node");
  return QString("QDomNode(unknown)");
}
