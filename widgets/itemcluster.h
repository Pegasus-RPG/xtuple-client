/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef itemCluster_h
#define itemCluster_h

#include "xlineedit.h"

#include "qstringlist.h"

#include <parameter.h>

class QPushButton;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

class XTUPLEWIDGETS_EXPORT ItemLineEdit : public XLineEdit
{
  Q_OBJECT
  Q_PROPERTY(QString     number          READ text          WRITE setItemNumber)
  Q_PROPERTY(unsigned int type           READ type          WRITE setType       DESIGNABLE false)

friend class ItemCluster;

  public:
    ItemLineEdit(QWidget *, const char * = 0);

    enum Type {
      cUndefined           = 0x00,

      // Specific Item Types
      cPurchased           = 0x00000001,
      cManufactured        = 0x00000002,
      cPhantom             = 0x00000004,
      cBreeder             = 0x00000008,
      cCoProduct           = 0x00000010,
      cByProduct           = 0x00000020,
      cReference           = 0x00000040,
      cCosting             = 0x00000080,
      cTooling             = 0x00000100,
      cOutsideProcess      = 0x00000200,
      cPlanning            = 0x00000400,
      cJob                 = 0x00000800,
      cKit                 = 0x00001000,

      // The first 16 bits are reserved for individual item types and we
      // have this mask defined here for convenience.
      cAllItemTypes_Mask   = 0x0000FFFF,

      // Planning Systems
      // TODO
      // Planning Type moved to Itemsite, don't think this is used
       cPlanningMRP         = 0x00100000,
       cPlanningMPS         = 0x00200000,
       cPlanningNone        = 0x00400000,

       cPlanningAny         = cPlanningMRP | cPlanningMPS | cPlanningNone,

      // Misc. Options
      cItemActive          = 0x04000000,
      cSold                = 0x08000000,

      // Lot/Serial and Location Controlled
      cLocationControlled  = 0x10000000,
      cLotSerialControlled = 0x20000000,
      cDefaultLocation     = 0x40000000,

      // Active ItemSite
      cActive         = 0x80000000,
      
      // Groups of Item Types
      cGeneralManufactured = cManufactured | cPhantom | cBreeder | cKit,
      cGeneralPurchased    = cPurchased | cOutsideProcess,
      cGeneralComponents   = cManufactured | cPhantom | cCoProduct | cPurchased | cOutsideProcess,
      cGeneralInventory    = cAllItemTypes_Mask ^ cReference ^ cJob,
      cKitComponents       = cSold | (cAllItemTypes_Mask ^ cKit)
    };

    inline unsigned int type() const                   { return _type;                        }
    inline void setType(unsigned int pType)            { _type = pType; _defaultType = pType; } 
    inline void setDefaultType(unsigned int pType)     { _defaultType = pType; } 
    inline void setQuery(const QString &pSql) { _sql = pSql; _useQuery = TRUE; }
    inline void setValidationQuery(const QString &pSql) { _validationSql = pSql; _useValidationQuery = TRUE; }
    inline int queryUsed() const              { return _useQuery;              }
    inline int validationQueryUsed() const    { return _useValidationQuery;    }

    void addExtraClause(const QString &);
    inline QStringList getExtraClauseList() const { return _extraClauses; }
    inline void clearExtraClauseList() { _extraClauses.clear(); }

    QString itemNumber();
    QString uom();
    QString upc();
    QString itemType();
    bool    isConfigured();

  public slots:
    void sEllipses();
    void sList();
    void sSearch();
    void sSearch(ParameterList params);
    void sAlias();

    void silentSetId(int);
    void setId(int);
    void setItemNumber(QString);
    void setItemsiteid(int);
    void sParse();

  signals:
    void privateIdChanged(int);
    void newId(int);
    void aliasChanged(const QString &);
    void uomChanged(const QString &);
    void descrip1Changed(const QString &);
    void descrip2Changed(const QString &);
    void typeChanged(const QString &);
    void upcChanged(const QString &);
    void configured(bool);
    void warehouseIdChanged(int);
    void valid(bool);

  protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

    QPoint dragStartPosition;
  private:
    void constructor();

    QString _sql;
    QString _validationSql;
    QString _itemNumber;
    QString _uom;
    QString _upc;
    QString _itemType;
    QStringList _extraClauses;
    unsigned int _type;
    unsigned int _defaultType;
    bool    _configured;
    bool    _useQuery;
    bool    _useValidationQuery;
};

class XTUPLEWIDGETS_EXPORT ItemCluster : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString defaultNumber  READ defaultNumber  WRITE setDefaultNumber   DESIGNABLE false)
  Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName)
  Q_PROPERTY(QString number         READ itemNumber     WRITE setItemNumber      DESIGNABLE false)
  Q_PROPERTY(unsigned int type      READ type           WRITE setType            DESIGNABLE false)
  
  public:
    ItemCluster(QWidget *, const char * = 0);

    Q_INVOKABLE void setReadOnly(bool);
    void setEnabled(bool);
    void setDisabled(bool);

    Q_INVOKABLE inline void    setType(unsigned int pType)    { _itemNumber->setType(pType); _itemNumber->setDefaultType(pType); } 
    Q_INVOKABLE inline unsigned int type() const              { return _itemNumber->type();                 }
    Q_INVOKABLE inline void setDefaultType(unsigned int pType){ _itemNumber->setDefaultType(pType); } 
    Q_INVOKABLE inline void    setQuery(const QString &pSql)  { _itemNumber->setQuery(pSql); }
    Q_INVOKABLE inline void    setValidationQuery(const QString &pSql) { _itemNumber->setValidationQuery(pSql);      }
    Q_INVOKABLE QString itemNumber() const                 { return _itemNumber->itemNumber();           }
    Q_INVOKABLE QString itemType() const                   { return _itemNumber->itemType();             }
    Q_INVOKABLE bool isConfigured() const                  { return _itemNumber->isConfigured();         }
    Q_INVOKABLE int id() const                             { return _itemNumber->id();                   }
    Q_INVOKABLE int isValid() const                        { return _itemNumber->isValid();              }
    Q_INVOKABLE QString  uom() const                       { return _itemNumber->uom();                  }
    Q_INVOKABLE QString  upc() const                       { return _itemNumber->upc();                  }
    inline QString  defaultNumber()  const                 { return _default; };
    inline QString  fieldName()      const                 { return _fieldName;                          }

    Q_INVOKABLE inline void addExtraClause(const QString & pClause) { _itemNumber->addExtraClause(pClause);     }
    Q_INVOKABLE inline QStringList getExtraClauseList() const       { return _itemNumber->getExtraClauseList(); }
    Q_INVOKABLE inline void clearExtraClauseList()                  { _itemNumber->clearExtraClauseList();      }

  public slots:
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setDefaultNumber(QString p)                       { _default = p;                               };
    void setFieldName(QString p)                           { _fieldName = p;                             };
    void setId(int);
    void setItemNumber(QString);
    void setItemsiteid(int);
    void silentSetId(int);
    void updateMapperData();

  signals:
    void privateIdChanged(int);
    void newId(int);
    void aliasChanged(const QString &);
    void uomChanged(const QString &);
    void descrip1Changed(const QString &);
    void descrip2Changed(const QString &);
    void warehouseIdChanged(int);
    void typeChanged(const QString &);
    void upcChanged(const QString &);
    void configured(bool);
    void valid(bool);

  private:
    ItemLineEdit *_itemNumber;
    QPushButton  *_itemList;
    QLabel       *_uom;
    QLabel       *_descrip1;
    QLabel       *_descrip2;
    QString       _default;
    QString       _fieldName;
    XDataWidgetMapper *_mapper;
};

#endif
