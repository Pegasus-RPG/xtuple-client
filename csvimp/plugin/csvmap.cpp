/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "csvmap.h"

#include <QDomDocument>
#include <QDomElement>
#include <QList>

QString CSVMap::DefaultDelimiter = QString(",");

CSVMap::CSVMap(const QString & name)
{
  _name = name;
  _description = QString::null;
  _action = Insert;
  _sqlPre = QString::null;
  _sqlPreContinueOnError = false;
  _sqlPost = QString::null;
}

CSVMap::CSVMap(const QDomElement & elem)
{
  _name = QString::null;
  _description = QString::null;
  _delimiter   = QString::null;
  _action = Insert;
  _sqlPre = QString::null;
  _sqlPreContinueOnError = false;
  _sqlPost = QString::null;

  QDomNodeList nList = elem.childNodes();
  for(int n = 0; n < nList.count(); ++n)
  {
    QDomElement elemThis = nList.item(n).toElement();
    if(elemThis.tagName() == "Name")
      setName(elemThis.text());
    else if(elemThis.tagName() == "Table")
      setTable(elemThis.text());
    else if(elemThis.tagName() == "Action")
      setAction(nameToAction(elemThis.text()));
    else if(elemThis.tagName() == "Description")
      setDescription(elemThis.text());
    else if (elemThis.tagName() == "Delimiter")
      setDelimiter(elemThis.text());
    else if(elemThis.tagName() == "PreSQL")
    {
      setSqlPre(elemThis.text());
      QDomAttr attr = elemThis.attributeNode("continueOnError");
      if(!attr.isNull() && attr.value() == "true")
        _sqlPreContinueOnError = true;
    }
    else if(elemThis.tagName() == "PostSQL")
      setSqlPost(elemThis.text());
    else if(elemThis.tagName() == "CSVMapField")
    {
      CSVMapField field(elemThis);
      _fields.append(field);
    }
    else
    {
      // ERROR
    }
  }
}

CSVMap::~CSVMap()
{
}

QDomElement CSVMap::createElement(QDomDocument & doc)
{
  QDomElement elem = doc.createElement("CSVMap");
  QDomElement elemThis;

  elemThis = doc.createElement("Name");
  elemThis.appendChild(doc.createTextNode(_name));
  elem.appendChild(elemThis);

  elemThis = doc.createElement("Table");
  elemThis.appendChild(doc.createTextNode(_table));
  elem.appendChild(elemThis);

  elemThis = doc.createElement("Action");
  elemThis.appendChild(doc.createTextNode(actionToName(_action)));
  elem.appendChild(elemThis);

  if(!_description.isEmpty())
  {
    elemThis = doc.createElement("Description");
    elemThis.appendChild(doc.createTextNode(_description));
    elem.appendChild(elemThis);
  }

  if (!_delimiter.isEmpty() && _delimiter != DefaultDelimiter)
  {
    elemThis = doc.createElement("Delimiter");
    elemThis.appendChild(doc.createTextNode(_delimiter));
    elem.appendChild(elemThis);
  }

  if(!_sqlPre.isEmpty())
  {
    elemThis = doc.createElement("PreSQL");
    if(_sqlPreContinueOnError)
      elemThis.setAttribute("continueOnError","true");
    elemThis.appendChild(doc.createTextNode(_sqlPre));
    elem.appendChild(elemThis);
  }

  if(!_sqlPost.isEmpty())
  {
    elemThis = doc.createElement("PostSQL");
    elemThis.appendChild(doc.createTextNode(_sqlPost));
    elem.appendChild(elemThis);
  }

  QList<CSVMapField>::iterator it;
  for(it = _fields.begin(); it != _fields.end(); ++it)
  {
    if(!(*it).isDefault())
      elem.appendChild((*it).createElement(doc));
  }

  return elem;
}

void CSVMap::setName(const QString & name)
{
  _name = name;
}

void CSVMap::setTable(const QString & table)
{
  _table = table;
}

void CSVMap::setDelimiter(const QString & delim)
{
  _delimiter = delim;
}

void CSVMap::setDescription(const QString & desc)
{
  _description = desc;
}

void CSVMap::setAction(Action act)
{
  _action = act;
}

void CSVMap::setField(const CSVMapField & f)
{
  for(int i = 0; i < _fields.count(); ++i)
  {
    if(_fields[i].name() == f.name())
    {
      _fields[i] = f;
      return;
    }
  }
  _fields.append(f);
}

bool CSVMap::removeField(const QString & name)
{
  for (int i = 0; i < _fields.size(); i++)
  {
    if(_fields.at(i).name() == name)
    {
      _fields.removeAt(i);
      return true;
    }
  }
  return false;
}

CSVMapField CSVMap::field(const QString & name) const
{
  QList<CSVMapField>::const_iterator it;
  for(it = _fields.begin(); it != _fields.end(); ++it)
  {
    if((*it).name() == name)
      return *it;
  }
  return CSVMapField();
}

QStringList CSVMap::fieldList() const
{
  QStringList list;
  QList<CSVMapField>::const_iterator it;
  for(it = _fields.begin(); it != _fields.end(); ++it)
  {
    list.append((*it).name());
  }
  return list;
}

void CSVMap::setSqlPre(const QString & sql)
{
  _sqlPre = sql;
}

void CSVMap::setSqlPreContinueOnError(bool b)
{
  _sqlPreContinueOnError = b;
}

void CSVMap::setSqlPost(const QString & sql)
{
  _sqlPost = sql;
}

void CSVMap::simplify()
{
  QList<CSVMapField>::iterator it = _fields.begin();
  for (int i = 0; i < _fields.size(); i++)
  {
    if(_fields.at(i).isDefault())
    {
      _fields.removeAt(i);
      i--;
    }
  }
}

QString CSVMap::actionToName(Action act)
{
  QString str = "Unknown";
  if(act == Insert)
    str = "Insert";
  else if(act == Update)
    str = "Update";
  else if(act == Append)
    str = "Append";
  return str;
}

CSVMap::Action CSVMap::nameToAction(const QString & name)
{
  if("Insert" == name)
    return Insert;
  else if("Update" == name)
    return Update;
  else if("Append" == name)
    return Append;
  return Insert;
}

//
// CSVMapField
//
CSVMapField::CSVMapField(const QString & name)
{
  _name = name;
  _isKey = false;
  _type  = QVariant::Invalid;
  _action = Action_Default;
  _column = 1;
  _ifNullAction = Nothing;
  _columnAlt = 1;
  _ifNullActionAlt = Nothing;
  _valueAlt = QString::null;
}

CSVMapField::CSVMapField(const QDomElement & elem)
{
  _name = QString::null;
  _isKey = false;
  _type  = QVariant::Invalid;
  _action = Action_Default;
  _column = 1;
  _ifNullAction = Nothing;
  _columnAlt = 1;
  _ifNullActionAlt = Nothing;
  _valueAlt = QString::null;

  Action action = Action_Default;

  QDomNodeList nList = elem.childNodes();
  for(int n = 0; n < nList.count(); ++n)
  {
    QDomElement elemThis = nList.item(n).toElement();
    if(elemThis.tagName() == "Name")
      setName(elemThis.text());
    else if(elemThis.tagName() == "isKey")
      setIsKey(true);
    else if(elemThis.tagName() == "Type")
      setType(QVariant::nameToType(elemThis.text().toLatin1().data()));
    else if(elemThis.tagName() == "Action")
      action = nameToAction(elemThis.text());
    else if(elemThis.tagName() == "Column")
    {
      if(action == Action_Default) action = Action_UseColumn;
      setColumn(elemThis.text().toInt());
    }
    else if(elemThis.tagName() == "AltColumn")
      setColumnAlt(elemThis.text().toInt());
    else if(elemThis.tagName() == "IfNull")
      setIfNullAction(nameToIfNull(elemThis.text()));
    else if(elemThis.tagName() == "AltIfNull")
      setIfNullActionAlt(nameToIfNull(elemThis.text()));
    else if(elemThis.tagName() == "AltValue")
      setValueAlt(elemThis.text());
    else
    {
      // ERROR
    }
  }
  if(action != Action_Default)
    setAction(action);
}

CSVMapField::~CSVMapField()
{
}

QDomElement CSVMapField::createElement(QDomDocument & doc)
{
  QDomElement elem = doc.createElement("CSVMapField");
  QDomElement elemThis;

  elemThis = doc.createElement("Name");
  elemThis.appendChild(doc.createTextNode(_name));
  elem.appendChild(elemThis);

  if(_isKey)
    elem.appendChild(doc.createElement("isKey"));

  if(_type != QVariant::Invalid)
  {
    elemThis = doc.createElement("Type");
    elemThis.appendChild(doc.createTextNode(QVariant::typeToName(_type)));
    elem.appendChild(elemThis);
  }

  if(_action == Action_UseColumn)
  {
    elemThis = doc.createElement("Column");
    elemThis.appendChild(doc.createTextNode(QString("%1").arg(_column)));
    elem.appendChild(elemThis);

    if(_ifNullAction != Nothing)
    {
      elemThis = doc.createElement("IfNull");
      elemThis.appendChild(doc.createTextNode(ifNullToName(_ifNullAction)));
      elem.appendChild(elemThis);
    }

    if(_ifNullAction == UseAlternateColumn)
    {
      elemThis = doc.createElement("AltColumn");
      elemThis.appendChild(doc.createTextNode(QString("%1").arg(_columnAlt)));
      elem.appendChild(elemThis);

      if(_ifNullActionAlt != Nothing && _ifNullActionAlt != UseAlternateColumn)
      {
        elemThis = doc.createElement("AltIfNull");
        elemThis.appendChild(doc.createTextNode(ifNullToName(_ifNullActionAlt)));
        elem.appendChild(elemThis);
      }
    }
  }
  else if(_action != Action_Default)
  {
    elemThis = doc.createElement("Action");
    elemThis.appendChild(doc.createTextNode(actionToName(_action)));
    elem.appendChild(elemThis);
  }

  if(_action == Action_UseAlternateValue
    || (_action == Action_UseColumn
      && (_ifNullAction == UseAlternateValue
       || _ifNullActionAlt == UseAlternateValue) ) )
  {
    elemThis = doc.createElement("AltValue");
    elemThis.appendChild(doc.createTextNode(_valueAlt));
    elem.appendChild(elemThis);
  }

  return elem;
}

void CSVMapField::setName(const QString & name)
{
  _name = name;
}

void CSVMapField::setIsKey(bool y)
{
  _isKey = y;
}

void CSVMapField::setType(QVariant::Type t)
{
  _type = t;
}

void CSVMapField::setIfNullAction(IfNull in)
{
  _ifNullAction = in;
}

void CSVMapField::setIfNullActionAlt(IfNull in)
{
  _ifNullActionAlt = in;
}

void CSVMapField::setColumn(unsigned int col)
{
  if(col == 0)
    col = 1;
  _column = col;
}

void CSVMapField::setColumnAlt(unsigned int col)
{
  if(col == 0)
    col = 1;
  _columnAlt = col;
}

void CSVMapField::setValueAlt(const QString & str)
{
  _valueAlt = str;
}

void CSVMapField::setAction(Action a)
{
  _action = a;
}

bool CSVMapField::isDefault() const
{
  if(!_isKey && _action == Action_Default)
    return true;
  return false;
}

QString CSVMapField::ifNullToName(IfNull in)
{
  QString str = "Nothing";
  if(UseEmptyString == in)
    str = "UseEmptyString";
  else if(UseDefault == in)
    str = "UseDefault";
  else if(UseAlternateColumn == in)
    str = "UseAlternateColumn";
  else if(UseAlternateValue == in)
    str = "UseAlternateValue";
  return str;
}

CSVMapField::IfNull CSVMapField::nameToIfNull(const QString & name)
{
  if("UseEmptyString" == name)
    return UseEmptyString;
  else if("UseDefault" == name)
    return UseDefault;
  else if("UseAlternateColumn" == name)
    return UseAlternateColumn;
  else if("UseAlternateValue" == name)
    return UseAlternateValue;
  return Nothing;
}

QStringList CSVMapField::ifNullList(bool altList)
{
  QStringList list;
  list << "Nothing";
  list << "UseDefault";
  list << "UseEmptyString";
  list << "UseAlternateValue";
  if(!altList)
    list << "UseAlternateColumn";
  return list;
}

QString CSVMapField::actionToName(Action a)
{
  QString str = "Default";
  if(Action_UseColumn == a)
    str = "UseColumn";
  else if(Action_UseEmptyString == a)
    str = "UseEmptyString";
  else if(Action_UseAlternateValue == a)
    str = "UseAlternateValue";
  else if(Action_UseNull == a)
    str = "UseNull";
  return str;
}

CSVMapField::Action CSVMapField::nameToAction(const QString & name)
{
  if("UseColumn" == name)
    return Action_UseColumn;
  else if("UseEmptyString" == name)
    return Action_UseEmptyString;
  else if("UseAlternateValue" == name)
    return Action_UseAlternateValue;
  else if("UseNull" == name)
    return Action_UseNull;
  return Action_Default;
}

QStringList CSVMapField::actionList()
{
  QStringList list;
  list << "Default";
  list << "UseColumn";
  list << "UseEmptyString";
  list << "UseAlternateValue";
  list << "UseNull";
  return list;
}
