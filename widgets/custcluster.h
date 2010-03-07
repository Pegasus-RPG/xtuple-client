/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef custCluster_h
#define custCluster_h

#include "widgets.h"
#include "xlineedit.h"

class QLabel;
class QPushButton;
class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;

class CustInfo;

#define __allCustomers    0x01
#define __activeCustomers 0x02

class XTUPLEWIDGETS_EXPORT CLineEdit : public XLineEdit
{
  Q_OBJECT

  Q_ENUMS(CLineEditTypes)

  Q_PROPERTY(bool	   autoFocus READ autoFocus WRITE setAutoFocus	)
  Q_PROPERTY(CLineEditTypes	type READ type	    WRITE setType	)
  Q_PROPERTY(QString     number          READ text          WRITE setNumber)

  friend class CustInfo;

  public:
    CLineEdit(QWidget * = 0, const char * = 0);

    enum CLineEditTypes
    {
      AllCustomers, 		ActiveCustomers,
      AllProspects,		ActiveProspects,
      AllCustomersAndProspects,	ActiveCustomersAndProspects
    };

    inline bool			autoFocus() const { return _autoFocus;  }
    inline CLineEditTypes	type()	    const { return _type;       }

    Q_INVOKABLE void setExtraClause(CLineEditTypes type, const QString &clause);

  public slots:
    void sEllipses();
    void sSearch();
    void sList();
    void setSilentId(int);
    void setId(int);
    void setNumber(const QString& pNumber);
    void setType(CLineEditTypes);
    void sParse();
    void setAutoFocus(bool);

  signals:
    void newId(int);
    void newCrmacctId(int);
    void custNumberChanged(const QString &);
    void custNameChanged(const QString &);
    void custAddr1Changed(const QString &);
    void custAddr2Changed(const QString &);
    void custAddr3Changed(const QString &);
    void custCityChanged(const QString &);
    void custStateChanged(const QString &);
    void custZipCodeChanged(const QString &);
    void custCountryChanged(const QString &);
    void creditStatusChanged(const QString &);
    void custAddressChanged(const int);
    void custContactChanged(const int);
    void valid(bool);

  protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);
    void keyPressEvent(QKeyEvent *);

  private:
    bool		_autoFocus;
    bool		_dragging;
    CLineEditTypes	_type;
    int                 _crmacctId;

    QString _all_extraclause;
    QString _customer_extraclause;
    QString _prospect_extraclause;
};

class XTUPLEWIDGETS_EXPORT CustInfoAction
{
  public:
    virtual ~CustInfoAction() {};
    virtual void customerInformation(QWidget* parent, int pCustid) = 0;
};

class XTUPLEWIDGETS_EXPORT CustInfo : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(bool                       autoFocus      READ autoFocus     WRITE setAutoFocus                     )
  Q_PROPERTY(CLineEdit::CLineEditTypes	type           READ type          WRITE setType                          )
  Q_PROPERTY(QString                    defaultNumber  READ defaultNumber WRITE setDefaultNumber DESIGNABLE false)
  Q_PROPERTY(QString                    fieldName      READ fieldName     WRITE setFieldName                     )
  Q_PROPERTY(QString                    number         READ number        WRITE setNumber        DESIGNABLE false)
  Q_PROPERTY(bool                       labelVisible   READ labelVisible  WRITE setLabelVisible                  )
  Q_PROPERTY(bool                       infoVisible    READ infoVisible   WRITE setInfoVisible                   )
  Q_PROPERTY(bool                       canEdit        READ canEdit       WRITE setCanEdit                       ) 

  public:
    CustInfo(QWidget *parent, const char *name = 0);

    Q_INVOKABLE void setReadOnly(bool);

    Q_INVOKABLE inline bool          autoFocus()     const  { return _customerNumber->autoFocus(); }
    Q_INVOKABLE inline bool          isValid()              { return _customerNumber->_valid;      }
    inline QString                   defaultNumber() const  { return _default;                     }
    inline QString                   fieldName()     const  { return _fieldName;                   }
    QString                   number();
    inline CLineEdit::CLineEditTypes type()          const  { return _customerNumber->type();      }

    inline bool canEdit()       { return _canEdit;                  }
    inline bool editMode()      { return _editMode;                 }
    inline bool labelVisible()  { return _labelVisible;             }
    inline bool infoVisible()   { return _infoVisible;             }
    
    void setLabelVisible(bool);
    void setInfoVisible(bool);
    void setCanEdit(bool);

    Q_INVOKABLE void setExtraClause(CLineEdit::CLineEditTypes type, const QString &);

    static CustInfoAction * _custInfoAction;

  public slots:
    int  id()    { return _customerNumber->_id;         }
    void setEditMode(bool);
    void setSilentId(int);
    void setId(int);
    void setType(CLineEdit::CLineEditTypes);
    void setAutoFocus(bool);
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setNumber(QString number);
    void setDefaultNumber(const QString& p)     { _default=p; };
    void setFieldName(const QString& p)         { _fieldName=p; };
    void clear();

  private slots:
    void setMode();
    void sInfo();
    void sHandleCreditStatus(const QString &);
    void updateMapperData();
    void sNewClicked();

  signals:
    void newId(int);
    void newCrmacctId(int);
    void numberChanged(const QString &);
    void nameChanged(const QString &);
    void address1Changed(const QString &);
    void address2Changed(const QString &);
    void address3Changed(const QString &);
    void cityChanged(const QString &);
    void stateChanged(const QString &);
    void zipCodeChanged(const QString &);
    void countryChanged(const QString &);
    void addressChanged(const int);
    void contactChanged(const int);
    void valid(bool);
    void editable(bool);
    void editingFinished();
    void deleteClicked();

  private:
    bool _labelVisible;
    bool _infoVisible;
    bool _canEdit;
    bool _editMode;
    QLabel *_customerNumberLit;
    QPushButton *_list;
    QPushButton *_info;
    QPushButton *_new;
    QPushButton *_edit;
    QPushButton *_delete;
    QString     _default;
    QString     _fieldName;
    XDataWidgetMapper *_mapper;
    
  protected:
    CLineEdit   *_customerNumber;
    XLineEdit   *_customerNumberEdit;
};


class XTUPLEWIDGETS_EXPORT CustCluster : public QWidget
{
  Q_OBJECT

  Q_PROPERTY(bool                       autoFocus      READ autoFocus     WRITE setAutoFocus                     )
  Q_PROPERTY(CLineEdit::CLineEditTypes	type           READ type	  WRITE setType                          )

  public:
    CustCluster(QWidget *parent, const char *name = 0);

    void setReadOnly(bool);

    Q_INVOKABLE inline bool          autoFocus()     const { return _custInfo->autoFocus();     };
    Q_INVOKABLE inline int           id()                  { return _custInfo->id();            };
    Q_INVOKABLE inline bool          isValid()             { return _custInfo->isValid();       };
    inline CLineEdit::CLineEditTypes type()          const { return _custInfo->type();          };
    Q_INVOKABLE inline QString       number()        const { return _custInfo->number();        };

    Q_INVOKABLE void   setType(CLineEdit::CLineEditTypes);
    Q_INVOKABLE void   setExtraClause(CLineEdit::CLineEditTypes type, const QString &);
    
  signals:
    void newId(int);
    void newCrmacctId(int);
    void valid(bool);

  public slots:
    void setId(int);
    void setSilentId(int);
    void setAutoFocus(bool);
    void setNumber(const QString& number)       { _custInfo->setNumber(number);};

  private:
    CustInfo    *_custInfo;
    QLabel      *_name;
    QLabel      *_address1;
};

#endif

