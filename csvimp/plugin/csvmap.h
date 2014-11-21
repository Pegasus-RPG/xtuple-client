/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __CSVMAP_H__
#define __CSVMAP_H__

#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QSqlField>
#include <QString>
#include <QStringList>
#include <QVariant>

class CSVMapField
{
  public:
    CSVMapField(const QString & name = QString::null);
    CSVMapField(const QDomElement &);
    virtual ~CSVMapField();

    void setName(const QString &);
    QString name() const { return _name; }

    void setIsKey(bool);
    bool isKey() const { return _isKey; }

    void setType(QVariant::Type);
    QVariant::Type type() const { return _type; }

    enum Action {
      Action_Default,
      Action_UseColumn,
      Action_UseEmptyString,
      Action_UseAlternateValue,
      Action_UseNull
    };
    void setAction(Action);
    Action action() const { return _action; }

    enum IfNull {
      Nothing,
      UseDefault,
      UseEmptyString,
      UseAlternateValue,
      UseAlternateColumn
    };
    void setIfNullAction(IfNull);
    IfNull ifNullAction() const { return _ifNullAction; }
    void setIfNullActionAlt(IfNull);
    IfNull ifNullActionAlt() const { return _ifNullActionAlt; }

    void setColumn(unsigned int);
    unsigned int column() const { return _column; }
    void setColumnAlt(unsigned int);
    unsigned int columnAlt() const { return _columnAlt; }

    void setValueAlt(const QString &);
    QString valueAlt() const { return _valueAlt; }

    bool isEmpty() const { return _name.isEmpty(); }
    bool isDefault() const;

    QDomElement createElement(QDomDocument &);

    static QString ifNullToName(IfNull);
    static IfNull nameToIfNull(const QString &);
    static QStringList ifNullList(bool altList = false);

    static QString actionToName(Action);
    static Action nameToAction(const QString &);
    static QStringList actionList();

  private:
    QString _name;
    bool    _isKey;
    QVariant::Type _type;
    Action _action;
    unsigned int _column;
    IfNull  _ifNullAction;
    unsigned int _columnAlt;
    IfNull  _ifNullActionAlt;
    QString _valueAlt;
};

class CSVMap
{
  public:
    static QString DefaultDelimiter;

    CSVMap(const QString & name = QString::null);
    CSVMap(const QDomElement &);
    virtual ~CSVMap();

    void setName(const QString &);
    QString name() const { return _name; }
    void setTable(const QString &);
    QString table() const { return _table; }
    void setDescription(const QString &);
    QString description() const { return _description; }
    void setDelimiter(const QString &delim);
    QString delimiter()   const { return _delimiter; }
    enum Action { Insert, Update, Append };
    void setAction(Action);
    Action action() const { return _action; }

    void setField(const CSVMapField &);
    bool removeField(const QString &);
    CSVMapField field(const QString &) const;
    QStringList fieldList() const;
    QList<CSVMapField> fields() const { return _fields; }

    void setSqlPre(const QString &);
    QString sqlPre() const { return _sqlPre; }
    void setSqlPreContinueOnError(bool);
    bool sqlPreContinueOnError() { return _sqlPreContinueOnError; }
    void setSqlPost(const QString &);
    QString sqlPost() const { return _sqlPost; }

    bool isEmpty() const { return _name.isEmpty(); }

    void simplify();

    QDomElement createElement(QDomDocument &);

    static QString actionToName(Action);
    static Action nameToAction(const QString &);

  protected:
    QList<CSVMapField> _fields;

  private:
    QString _sqlPre;
    bool    _sqlPreContinueOnError;
    QString _sqlPost;
    QString _name;
    QString _table;
    Action _action;
    QString _description;
    QString _delimiter;
};

#endif

