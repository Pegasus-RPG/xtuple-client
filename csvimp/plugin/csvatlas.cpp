/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "csvatlas.h"

#include <QDomElement>
#include <QDomNodeList>
#include <QDomDocument>
#include <QList>

CSVAtlas::CSVAtlas()
{
}

CSVAtlas::CSVAtlas(const QDomElement & elem)
{
  QDomNodeList nList = elem.childNodes();
  for(int n = 0; n < nList.count(); ++n)
  {
    QDomElement elemThis = nList.item(n).toElement();
    if(elemThis.tagName() == "Description")
      setDescription(elemThis.text());
    else if(elemThis.tagName() == "CSVMap")
    {
      CSVMap map(elemThis);
      _maps.append(map);
    }
    else
    {
      // ERROR
    }
  }
}

CSVAtlas::~CSVAtlas()
{
}

QDomElement CSVAtlas::createElement(QDomDocument & doc)
{
  QDomElement elem = doc.createElement("CSVAtlas");

  if(!_description.isEmpty())
  {
    QDomElement desc = doc.createElement("Description");
    desc.appendChild(doc.createTextNode(_description));
    elem.appendChild(desc);
  }

  for(int i = 0; i < _maps.size(); i++)
  {
    // tmp required because appendChild expects a const CSVMap
    CSVMap tmp = _maps.at(i);
    elem.appendChild(tmp.createElement(doc));
  }

  return elem;
}

void CSVAtlas::setDescription(const QString & desc)
{
  _description = desc;
}

void CSVAtlas::setMap(const CSVMap & m)
{
  for(int i = 0; i < _maps.count(); ++i)
  {
    if(_maps[i].name() == m.name())
    {
      _maps[i] = m;
      return;
    }
  }
  _maps.append(m);
}

bool CSVAtlas::removeMap(const QString & name)
{
  for (int i = 0; i < _maps.size(); i++)
  {
    if(_maps.at(i).name() == name)
    {
      _maps.removeAt(i);
      return true;
    }
  }
  return true;
}

CSVMap CSVAtlas::map(const QString & name) const
{
  for (int i = 0; i < _maps.size(); i++)
  {
    if(_maps.at(i).name() == name)
      return _maps.at(i);
  }
  return CSVMap();
}

QStringList CSVAtlas::mapList() const
{
  QStringList list;
  for (int i = 0; i < _maps.size(); i++)
    list.append(_maps.at(i).name());

  return list;
}
