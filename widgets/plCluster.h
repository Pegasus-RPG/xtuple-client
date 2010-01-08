/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef plCluster_h
#define plCluster_h

#include "xlineedit.h"

#include "widgets.h"

class QLabel;
class QPushButton;

class PlanOrdCluster;

class XTUPLEWIDGETS_EXPORT PlanOrdLineEdit : public XLineEdit
{
  Q_OBJECT

friend class PlanOrdCluster;

  public:
    PlanOrdLineEdit(QWidget *, const char * = 0);

  public slots:
    void setId(int);
    void sParse();

  private:
    QChar  _status;
    double _qtyOrdered;
    XDataWidgetMapper *_mapper;

  signals:
    void newId(int);
    void warehouseChanged(const QString &);
    void itemNumberChanged(const QString &);
    void uomChanged(const QString &);
    void itemDescrip1Changed(const QString &);
    void itemDescrip2Changed(const QString &);
    void dueDateChanged(const QString &);
    void qtyChanged(const QString &);
    void statusChanged(const QString &);
    void valid(bool);
};

class XTUPLEWIDGETS_EXPORT PlanOrdCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString  fieldName      READ fieldName      WRITE setFieldName)
  Q_PROPERTY(QString  number         READ number         WRITE setNumber         DESIGNABLE false)
  Q_PROPERTY(QString  defaultNumber  READ defaultNumber                          DESIGNABLE false)

  public:
    PlanOrdCluster(QWidget *, const char * = 0);
    PlanOrdCluster(int, QWidget *, const char * = 0);

    QString woNumber() const;

    inline int id()                 { return _number->_id;          }
    inline bool isValid()           { return _number->_valid;       }
    inline char status()            { return _number->_status.toAscii(); }
    inline double qty()             { return _number->_qtyOrdered;  }
    inline QString number()  const  { return _number->text();       }
    QString defaultNumber()  const  { return _default; }
    QString fieldName()      const  { return _fieldName; };

  public slots:
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setFieldName(QString p)    { _fieldName = p; };
    void setId(int);
    void setNumber(const QString& number);
    void setReadOnly(bool);

  private slots:
    void sList();

  private:
    void constructor();

    PlanOrdLineEdit  *_number;
    QPushButton      *_list;
    QLabel           *_warehouse;
    QLabel           *_itemNumber;
    QLabel           *_uom;
    QLabel           *_descrip1;
    QLabel           *_descrip2;
    QLabel           *_status;
    QString          _default;
    QString          _fieldName;

  signals:
    void newId(int);
    void dueDateChanged(const QString &);
    void qtyChanged(const QString &);

    void valid(bool);
};

#endif

