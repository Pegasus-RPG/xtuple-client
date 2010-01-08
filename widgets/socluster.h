/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef soCluster_h
#define soCluster_h

#include "xlineedit.h"
#include "xdatawidgetmapper.h"
#include "widgets.h"
#include <QLabel>

class QLabel;
class QPushButton;

class SoCluster;

//  Possible Customer Order Status
#define cSoOpen               0x01
#define cSoClosed             0x02
#define cSoAtShipping         0x04
#define cSoReleased           0x08
#define cSoCustomer           0x16

class XTUPLEWIDGETS_EXPORT SoLineEdit : public XLineEdit
{
  Q_OBJECT

friend class SoCluster;

  public:
    SoLineEdit(QWidget *, const char * = 0);
       
    QString defaultNumber()         const { return _default;            };

  public slots:
    bool sitePrivsEnforced() const { return _sitePrivs;};
    void setId(int);
    void setCustId(int);
    void setDefaultNumber(QString p)            { _default = p;             };
    void setNumber(QString);
    void setSitePrivsEnforced(const bool p)     { _sitePrivs = p;};
    void clear();
    void sParse();

  signals:
    void valid(bool);
    void newId(int);
    void custidChanged(int);
    void custNameChanged(const QString &);
    void numberChanged(int);
    void numberChanged(const QString &);
    
  protected:
    bool _sitePrivs;
    XDataWidgetMapper *_mapper;

  private:
    int  _custid;
    int  _type;
    QString  _default;
    QString  _number;

};


class XTUPLEWIDGETS_EXPORT SoCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString label          READ label          WRITE setLabel                          )
  Q_PROPERTY(bool    readOnly       READ isReadOnly     WRITE setReadOnly                       )
  Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName                      )
  Q_PROPERTY(QString number         READ number()       WRITE setNumber)
  Q_PROPERTY(QString defaultNumber  READ defaultNumber  WRITE setDefaultNumber DESIGNABLE false )

  public:
    SoCluster(QWidget *, const char * = 0);
    SoCluster(int, QWidget *);
      
    Q_INVOKABLE bool sitePrivsEnforced() const { return _soNumber->_sitePrivs;};
    Q_INVOKABLE int  id()                      { return _soNumber->_id;      };
    Q_INVOKABLE int  custid()                  { return _soNumber->_custid;  };
    Q_INVOKABLE bool isValid()                 { return _soNumber->_valid;   };
    inline QString  defaultNumber()       { return _soNumber->defaultNumber();};
    inline QString  number()              { return _soNumber->text();   };
    inline virtual QString label()  const { return _label;              };
    inline bool isReadOnly()              { return _readOnly;           }; 
    QString fieldName()             const { return _fieldName;          };

  public slots:
    void setReadOnly(bool);
    void setId(int);
    void setCustId(int);
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setDefaultNumber(QString number)       { _soNumber->setDefaultNumber(number);}
    void setFieldName(QString p)                { _fieldName = p;           };
    void setSitePrivsEnforced(const bool p)     { _soNumber->_sitePrivs = p;};
    void setNumber(QString);
    void setLabel(const QString p);
    void setType(int pType)                     { _soNumber->_type = pType; };

  signals:
    void newId(int);
    void newCustid(int);
    void valid(bool);

  private slots:
    void sList();

  protected:
    QLabel* _soNumberLit;

  private:
    void constructor();

    SoLineEdit   *_soNumber;
    QPushButton  *_list;
    QLabel       *_custName;
    bool         _readOnly;
    QString      _fieldName;
    QString      _label;
    
};

#endif
