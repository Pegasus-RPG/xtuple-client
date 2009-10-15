/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __USERNAMECLUSTER_H__
#define __USERNAMECLUSTER_H__

#include "xlineedit.h"

#include "widgets.h"

class QLabel;
class QPushButton;
class QScriptEngine;

class UsernameCluster;

void setupUsernameLineEdit(QScriptEngine *engine);
void setupUsernameCluster(QScriptEngine *engine);

class XTUPLEWIDGETS_EXPORT UsernameLineEdit : public XLineEdit
{
  Q_OBJECT

  friend class UsernameCluster;

  public:
    UsernameLineEdit(QWidget*, const char* = 0);

    enum Type {
      UsersAll = 0,
      UsersActive,
      UsersInactive
    };

    Q_INVOKABLE inline enum Type type() const { return _type; }
    Q_INVOKABLE inline void setType(enum Type pType) { _type = pType; }

    Q_INVOKABLE int id();
    Q_INVOKABLE const QString & username();

  public slots:
    void setId(int);
    void setUsername(const QString &);
    void clear();
    void sParse();

  signals:
    void valid(bool);
    void newId(int);

  private:
    enum Type _type;
    int _id;
    QString _username;
};

class XTUPLEWIDGETS_EXPORT UsernameCluster : public QWidget
{
  Q_OBJECT

  Q_PROPERTY( QString label READ label WRITE setLabel )

  public:
    UsernameCluster(QWidget*, const char* = 0);

    Q_INVOKABLE inline int  id()      const { return _username->id();  }

    Q_INVOKABLE inline const QString username() const { return _username->username(); }

    Q_INVOKABLE inline bool isValid() const { return _username->_valid; }

    Q_INVOKABLE inline UsernameLineEdit::Type type() const { return _username->type(); }
    Q_INVOKABLE inline void setType(UsernameLineEdit::Type pType) { _username->setType(pType); }

    QString label() const;

  public slots:
    void sList();
    void setLabel(QString);
    void setUsername(const QString & pUsername);
    void setReadOnly(bool);
    inline void setId(int pId)  { _username->setId(pId);   }

  signals:
    void newId(int);
    void valid(bool);

  private:
    UsernameLineEdit * _username;
    QPushButton      * _list;
    QLabel           * _label;
};

Q_DECLARE_METATYPE(UsernameLineEdit*)
Q_DECLARE_METATYPE(UsernameCluster*)

#endif
