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

#ifndef __QDOMELEMENTPROTO_H__
#define __QDOMELEMENTPROTO_H__

#include <QDomAttr>
#include <QDomElement>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QDomNodeList>
#include <QObject>
#include <QtScript>

class QDomCDATASection;
class QDomCharacterData;
class QDomComment;
class QDomDocument;
class QDomDocumentFragment;
class QDomDocumentType;
class QDomEntity;
class QDomEntityReference;
class QDomImplementation;
class QDomNotation;
class QDomProcessingInstruction;
class QDomText;

Q_DECLARE_METATYPE(QDomElement*)
Q_DECLARE_METATYPE(QDomElement)

void setupQDomElementProto(QScriptEngine *engine);
QScriptValue constructQDomElement(QScriptContext *context, QScriptEngine *engine);

class QDomElementProto : public QObject, public QScriptable
{
  Q_OBJECT

  public:
    QDomElementProto(QObject *parent);

    Q_INVOKABLE QString  attribute(const QString& name, const QString& defValue = QString() ) const;
    Q_INVOKABLE QString  attributeNS(const QString nsURI, const QString& localName, const QString& defValue = QString()) const;
    Q_INVOKABLE QDomAttr attributeNode(const QString& name);
    Q_INVOKABLE QDomAttr attributeNodeNS(const QString& nsURI, const QString& localName);
    Q_INVOKABLE QDomNamedNodeMap attributes() const;
    Q_INVOKABLE QDomNodeList     elementsByTagName(const QString& tagname) const;
    Q_INVOKABLE QDomNodeList     elementsByTagNameNS(const QString& nsURI, const QString& localName) const;
    Q_INVOKABLE bool     hasAttribute(const QString& name) const;
    Q_INVOKABLE bool     hasAttributeNS(const QString& nsURI, const QString& localName) const;
    Q_INVOKABLE int      nodeType() const;
    Q_INVOKABLE void     removeAttribute(const QString& name);
    Q_INVOKABLE void     removeAttributeNS(const QString& nsURI, const QString& localName);
    Q_INVOKABLE QDomAttr removeAttributeNode(const QDomAttr& oldAttr);
    Q_INVOKABLE void     setAttribute(const QString& name, const QString& value);
    Q_INVOKABLE void     setAttribute(const QString& name, double value);
    Q_INVOKABLE void     setAttribute(const QString& name, float value);
    Q_INVOKABLE void     setAttribute(const QString& name, int value);
    Q_INVOKABLE void     setAttribute(const QString& name, qlonglong value);
    Q_INVOKABLE void     setAttribute(const QString& name, qulonglong value);
    Q_INVOKABLE void     setAttribute(const QString& name, uint value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, const QString& value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, double value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, int value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, qlonglong value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, qulonglong value);
    Q_INVOKABLE void     setAttributeNS(const QString nsURI, const QString& qName, uint value);
    Q_INVOKABLE QDomAttr setAttributeNode(const QDomAttr& newAttr);
    Q_INVOKABLE QDomAttr setAttributeNodeNS(const QDomAttr& newAttr);
    Q_INVOKABLE void     setTagName(const QString& name);
    Q_INVOKABLE QString  tagName()  const;
    Q_INVOKABLE QString  text()     const;
    Q_INVOKABLE QString  toString() const;

    // now for the inherited stuff from QDomNode
    Q_INVOKABLE QDomNode         appendChild(const QDomNode& newChild);
    Q_INVOKABLE QDomNodeList childNodes() const;
    Q_INVOKABLE void         clear();
    Q_INVOKABLE QDomNode     cloneNode(bool deep = true) const;
    Q_INVOKABLE int          columnNumber() const;
    Q_INVOKABLE QDomNode     firstChild() const;
    Q_INVOKABLE QDomElement  firstChildElement(const QString &tagName = QString()) const;
    Q_INVOKABLE bool         hasAttributes() const;
    Q_INVOKABLE bool         hasChildNodes() const;
    Q_INVOKABLE QDomNode     insertAfter(const QDomNode& newChild, const QDomNode& refChild);
    Q_INVOKABLE QDomNode     insertBefore(const QDomNode& newChild, const QDomNode& refChild);
    Q_INVOKABLE bool         isAttr() const;
    Q_INVOKABLE bool         isCDATASection() const;
    Q_INVOKABLE bool         isCharacterData() const;
    Q_INVOKABLE bool         isComment() const;
    Q_INVOKABLE bool         isDocument() const;
    Q_INVOKABLE bool         isDocumentFragment() const;
    Q_INVOKABLE bool         isDocumentType() const;
    Q_INVOKABLE bool         isElement() const;
    Q_INVOKABLE bool         isEntity() const;
    Q_INVOKABLE bool         isEntityReference() const;
    Q_INVOKABLE bool         isNotation() const;
    Q_INVOKABLE bool         isNull() const;
    Q_INVOKABLE bool         isProcessingInstruction() const;
    Q_INVOKABLE bool         isSupported(const QString& feature, const QString& version) const;
    Q_INVOKABLE bool         isText() const;
    Q_INVOKABLE QDomNode     lastChild() const;
    Q_INVOKABLE QDomElement  lastChildElement(const QString &tagName = QString()) const;
    Q_INVOKABLE int          lineNumber() const;
    Q_INVOKABLE QString      localName() const;
    Q_INVOKABLE QDomNode     namedItem(const QString& name) const;
    Q_INVOKABLE QString      namespaceURI() const;
    Q_INVOKABLE QDomNode     nextSibling() const;
    Q_INVOKABLE QDomElement  nextSiblingElement(const QString &taName = QString()) const;
    Q_INVOKABLE QString      nodeName()        const;
    Q_INVOKABLE QString      nodeValue()       const;
    Q_INVOKABLE void         normalize();
    Q_INVOKABLE QDomDocument ownerDocument()   const;
    Q_INVOKABLE QDomNode     parentNode()      const;
    Q_INVOKABLE QString      prefix()          const;
    Q_INVOKABLE QDomNode     previousSibling() const;
    Q_INVOKABLE QDomElement  previousSiblingElement(const QString &tagName = QString()) const;
    Q_INVOKABLE QDomNode     removeChild(const QDomNode& oldChild);
    Q_INVOKABLE QDomNode     replaceChild(const QDomNode& newChild, const QDomNode& oldChild);
    Q_INVOKABLE void         save(QTextStream&, int)      const;
    Q_INVOKABLE void         save(QTextStream&, int, int) const;
    Q_INVOKABLE void         setNodeValue(const QString&);
    Q_INVOKABLE void         setPrefix(const QString& pre);
    Q_INVOKABLE QDomAttr     toAttr()                               const;
    Q_INVOKABLE QDomCDATASection          toCDATASection()          const;
    Q_INVOKABLE QDomCharacterData         toCharacterData()         const;
    Q_INVOKABLE QDomComment               toComment()               const;
    Q_INVOKABLE QDomDocument              toDocument()              const;
    Q_INVOKABLE QDomDocumentFragment      toDocumentFragment()      const;
    Q_INVOKABLE QDomDocumentType          toDocumentType()          const;
    Q_INVOKABLE QDomElement               toElement()               const;
    Q_INVOKABLE QDomEntity                toEntity()                const;
    Q_INVOKABLE QDomEntityReference       toEntityReference()       const;
    Q_INVOKABLE QDomNotation              toNotation()              const;
    Q_INVOKABLE QDomProcessingInstruction toProcessingInstruction() const;
    Q_INVOKABLE QDomText                  toText()                  const;

};

#endif
