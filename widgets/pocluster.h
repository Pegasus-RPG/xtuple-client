/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __POCLUSTER_H__
#define __POCLUSTER_H__

#include "xlineedit.h"

#include "widgets.h"

class QLabel;
class QPushButton;

class PoCluster;

//  Possible P/O Status
#define cPOUnposted 0x01
#define cPOOpen     0x02
#define cPOClosed   0x04

#define cPOItem     0x01
#define cPOItemsrc  0x02

class XTUPLEWIDGETS_EXPORT PoLineEdit : public XLineEdit
{
  Q_OBJECT

  Q_PROPERTY(int type READ type WRITE setType DESIGNABLE false)

friend class PoCluster;

  public:
    PoLineEdit(QWidget *, const char * = 0);

    Q_INVOKABLE inline int vendId()     { return _vendid; }
    inline int type()       { return _type;   }

    inline void setType(int pType) { _type = pType;  }

  public slots:
    void setId(int);
    void setNumber(int);
    void clear();
    void sParse();

  signals:
    void valid(bool);
    void newId(int);
    void vendidChanged(int);
    void vendNameChanged(const QString &);
    void vendAddress1Changed(const QString &);
    void vendAddress2Changed(const QString &);
    void vendAddress3Changed(const QString &);
    void vendCityStateZipChanged(const QString &);
    void numberChanged(int);
    void numberChanged(const QString &);

  private:
    int  _vendid;
    int  _type;
    int  _number;

  private slots:
    void sMarkDirty();
    
  protected:
    XDataWidgetMapper *_mapper;
    
};

class XTUPLEWIDGETS_EXPORT PoCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName)
  Q_PROPERTY(QString number         READ number         WRITE setNumber       DESIGNABLE false)
  Q_PROPERTY(QString defaultNumber  READ defaultNumber                        DESIGNABLE false)
  Q_PROPERTY(int     type           READ type           WRITE setType         DESIGNABLE false)

  public:
    PoCluster(QWidget *, const char * = 0);

    Q_INVOKABLE inline void     setId(int pId)     { _poNumber->setId(pId);     }
    Q_INVOKABLE inline int      id()               { return _poNumber->_id;     }
    Q_INVOKABLE inline int  vendid()   { return _poNumber->_vendid; }
    Q_INVOKABLE inline bool isValid()  { return _poNumber->_valid;  }
    inline QString  defaultNumber()    { return QString();          }
    inline QString  fieldName()        { return _fieldName;         }
    inline QString  number()           { return _poNumber->text();  }
    inline void setType(int pType)     { _poNumber->setType(pType); }
    inline int  type()                 { return _poNumber->type();  }

    inline QString poNumber()          { return QString("%1").arg(_poNumber->_number); }

    void setReadOnly(bool);

  signals:
    void newId(int);
    void newVendid(int);
    void valid(bool);

  public slots:
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setFieldName(const QString& name)        { _fieldName = name; }
    void setNumber(const QString& number);
    
  private slots:
    void sList();

  private:
    PoLineEdit  *_poNumber;
    QPushButton *_poList;
    QLabel      *_vendName;
    QString      _fieldName;
};

#endif

