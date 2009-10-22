/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef vendorcluster_h
#define vendorcluster_h

#include "widgets.h"
#include "xlineedit.h"

#define __allVendors    0x01
#define __activeVendors 0x02

class QPushButton;
class QLabel;

class VendorCluster;

class XTUPLEWIDGETS_EXPORT VendorLineEdit : public XLineEdit
{
  Q_OBJECT

friend class VendorInfo;
friend class VendorCluster;

  public:
    VendorLineEdit(QWidget *, const char * = 0);

  public slots:
    void setId(int);
    void setNumber(const QString &);
    void setType(int);
    void sParse();
    void sEllipses();
    void sSearch();
    void sList();

  signals:
    void valid(bool);
    void newId(int);
    void nameChanged(const QString &);
    void address1Changed(const QString &);
    void address2Changed(const QString &);
    void address3Changed(const QString &);
    void cityChanged(const QString &);
    void stateChanged(const QString &);
    void zipCodeChanged(const QString &);
    void countryChanged(const QString &);

  protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    XDataWidgetMapper *_mapper;

  private:
    bool _dragging;
    int  _type;
};

class XTUPLEWIDGETS_EXPORT VendorInfo : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(bool    readOnly       READ isReadOnly     WRITE setReadOnly                       )
  Q_PROPERTY(QString defaultNumber  READ defaultNumber  WRITE setDefaultNumber DESIGNABLE false )
  Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName                      )
  Q_PROPERTY(QString number         READ number         WRITE setNumber        DESIGNABLE false )

  public:
    VendorInfo(QWidget *parent, const char *name = 0);

    Q_INVOKABLE inline int id()          const { return _vendorNumber->_id;         };
    Q_INVOKABLE inline bool isValid()    const { return _vendorNumber->_valid;      };
    Q_INVOKABLE inline bool isReadOnly() const { return _vendorNumber->isEnabled(); };

    QString defaultNumber()  const { return _default;                   };
    QString fieldName()      const { return _fieldName;                 };
    QString number()         const { return _vendorNumber->text();      };

  public slots:
    void setId(int);
    void setType(int);
    void setReadOnly(bool);
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setDefaultNumber(QString p)            { _default = p;   };
    void setFieldName(QString p)                { _fieldName = p; };
    void setNumber(const QString& number);

  signals:
    void newId(int);
    void nameChanged(const QString &);
    void address1Changed(const QString &);
    void address2Changed(const QString &);
    void address3Changed(const QString &);
    void cityChanged(const QString &);
    void stateChanged(const QString &);
    void zipCodeChanged(const QString &);
    void countryChanged(const QString &);
    void valid(bool);

  private:
    VendorLineEdit *_vendorNumber;
    QPushButton    *_list;
    QPushButton    *_info;
    QString        _default;
    QString        _fieldName;
};


class XTUPLEWIDGETS_EXPORT VendorCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString defaultNumber  READ defaultNumber  WRITE setDefaultNumber DESIGNABLE false)
  Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName)
  Q_PROPERTY(QString number         READ number         WRITE setNumber        DESIGNABLE false)

  public:
    VendorCluster(QWidget *, const char * = 0);

    Q_INVOKABLE inline int      id()                  { return _vendorNumber->_id;         };
    Q_INVOKABLE inline bool     isValid()             { return _vendorNumber->_valid;      };
    inline QString  defaultNumber() const { return _default;                   };
    inline QString  fieldName()     const { return _fieldName;                 };
    inline QString  number()        const { return _vendorNumber->text();      };

    Q_INVOKABLE void setReadOnly(bool);
    Q_INVOKABLE void setType(int);
    
  public slots:
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setDefaultNumber(QString p)            { _default = p;               };
    void setFieldName(QString p)                { _fieldName = p;             };
    void setId(int pId)                         { _vendorNumber->setId(pId);  };
    void setNumber(const QString& number);

  signals:
    void newId(int);
    void valid(bool);

  private:
    VendorLineEdit *_vendorNumber;
    QPushButton    *_list;
    QLabel         *_vendorName;
    QString        _default;
    QString        _fieldName;
};

#endif

