/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef shiptoCluster_h
#define shiptoCluster_h

#include "xlineedit.h"

#include "widgets.h"

class QLabel;
class QPushButton;

class ShiptoCluster;

class XTUPLEWIDGETS_EXPORT ShiptoEdit : public XLineEdit
{
  Q_OBJECT

  friend class ShiptoCluster;

  public:
    ShiptoEdit(QWidget *, const char * = 0);

    inline int custid()  { return _custid; };

  signals:
    void nameChanged(const QString &);
    void address1Changed(const QString &);
    void requestList();
    void newId(int);
    void newCustid(int);
    void valid(bool);
    void disableList(bool);

  public slots:
    void setId(int);
    void setCustid(int);
    void sParse();

  private:
    int  _custid;

    void clear();
};


class XTUPLEWIDGETS_EXPORT ShiptoCluster : public QWidget
{
  Q_OBJECT

  public:
    ShiptoCluster(QWidget *, const char * = 0);

    void setReadOnly(bool);

    Q_INVOKABLE inline int id()       { return _shiptoNumber->_id;     }
    inline int custid()   { return _shiptoNumber->_custid; }
    inline bool isValid() { return _shiptoNumber->_valid;  }

  signals:
    void newId(int);
    void valid(bool);
    void newCustid(int);

  public slots:
    void setId(int);
    void setCustid(int);

  private slots:
    void sList();

  private:
    ShiptoEdit  *_shiptoNumber;
    QPushButton *_list;
    QLabel      *_shiptoName;
    QLabel      *_shiptoAddress1;
};

#endif

