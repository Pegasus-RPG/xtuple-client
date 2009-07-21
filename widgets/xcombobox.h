/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef xcombobox_h
#define xcombobox_h

#include <QComboBox>
#include <QLabel>
#include <QMouseEvent>
#include <QList>
#include <QSqlTableModel>

#include "widgets.h"
#include "xdatawidgetmapper.h"

class XSqlQuery;

class XTUPLEWIDGETS_EXPORT XComboBox : public QComboBox
{
  Q_OBJECT

  Q_ENUMS(Defaults)
  Q_ENUMS(XComboBoxTypes)

  Q_PROPERTY(bool           allowNull             READ allowNull            WRITE setAllowNull                            )
  Q_PROPERTY(QString        nullStr               READ nullStr              WRITE setNullStr                              )
  Q_PROPERTY(XComboBoxTypes type                  READ type                 WRITE setType                                 )
  Q_PROPERTY(QString        code                  READ code                 WRITE setCode                 DESIGNABLE false)
  Q_PROPERTY(Defaults       defaultCode           READ defaultCode          WRITE setDefaultCode                          )
  Q_PROPERTY(QString        fieldName             READ fieldName            WRITE setFieldName                            )
  Q_PROPERTY(QString        listSchemaName        READ listSchemaName       WRITE setListSchemaName                       )
  Q_PROPERTY(QString        listTableName         READ listTableName        WRITE setListTableName                        )
  Q_PROPERTY(QString        listIdFieldName       READ listIdFieldName      WRITE setListIdFieldName                      )
  Q_PROPERTY(QString        listDisplayFieldName  READ listDisplayFieldName WRITE setListDisplayFieldName                 )
  Q_PROPERTY(QString        currentDefault        READ currentDefault                                     DESIGNABLE false)
  Q_PROPERTY(QString        text                  READ currentText          WRITE setText                 DESIGNABLE false)

  public:
    XComboBox(QWidget * = 0, const char * = 0);
    XComboBox(bool, QWidget * = 0, const char * = 0);

    enum Defaults { First, None };
    enum XComboBoxTypes
      {
      AddressCommentTypes, Adhoc,
      APBankAccounts,	APTerms, ARBankAccounts,
      ARCMReasonCodes, ARDMReasonCodes,
      ARTerms, AccountingPeriods, Agent,
      AllCommentTypes, AllProjects,
      BBOMHeadCommentTypes, BBOMItemCommentTypes,
      BOMHeadCommentTypes, BOMItemCommentTypes,
      BOOHeadCommentTypes, BOOItemCommentTypes,
      CRMAccounts, CRMAccountCommentTypes,
      ClassCodes,	Companies, ContactCommentTypes,
      CostCategories, Countries,
      Currencies,	CurrenciesNotBase, CustomerCommentTypes,
      CustomerGroups,	CustomerTypes,
      EmployeeCommentTypes, ExpenseCategories,
      FinancialLayouts,	FiscalYears, FreightClasses, Honorifics,
      IncidentCategory, IncidentCommentTypes,
      IncidentPriority,	IncidentResolution,	IncidentSeverity,
      ItemCommentTypes,	ItemGroups, ItemSiteCommentTypes,
      ItemSourceCommentTypes, Locales,
      LocaleCountries,  LocaleLanguages,
      LocationCommentTypes, LotSerialCommentTypes, OpportunityCommentTypes,
      OpportunityStages, OpportunitySources, OpportunityTypes,
      PlannerCodes,	PoProjects, ProductCategories,
      ProfitCenters,	ProjectCommentTypes,
      PurchaseOrderCommentTypes, PurchaseOrderItemCommentTypes,
      ReasonCodes, RegistrationTypes, Reports, ReturnReasonCodes,
      ReturnAuthCommentTypes, ReturnAuthItemCommentTypes,
      QuoteCommentTypes, QuoteItemCommentTypes,
      SalesOrderCommentTypes, SalesOrderItemCommentTypes,               
      SalesCategories, SalesReps, SalesRepsActive,
      ShipVias,		ShippingCharges,	ShippingForms,
      SiteTypes, SoProjects,	Subaccounts,
      TaxAuths, TaxClasses, TaxCodes, TaxZones,
      TaxTypes, Terms, TaskCommentTypes, TodoItemCommentTypes,
      TransferOrderCommentTypes, TransferOrderItemCommentTypes,
      UOMs, Users, ActiveUsers, VendorCommentTypes, VendorGroups,
      VendorTypes, WarehouseCommentTypes, WoProjects,
      WorkCenters, WorkOrderCommentTypes
      };

    XComboBoxTypes type();
    void setType(XComboBoxTypes);

    void setCode(QString);


    virtual bool      allowNull()            const  { return _allowNull; };
    virtual Defaults  defaultCode()                 { return _default;};
    virtual void      setAllowNull(bool);
    virtual void      setNull();

    QString           nullStr()              const  { return _nullStr; };
    void              setNullStr(const QString &);

    Q_INVOKABLE bool  editable()             const;
    Q_INVOKABLE void  setEditable(bool);

    Q_INVOKABLE inline QLabel* label()        const  { return _label; };
    Q_INVOKABLE void   setLabel(QLabel* pLab);

    Q_INVOKABLE bool isValid()              const;
    int               id(int)                const;
    Q_INVOKABLE int   id()                   const;
    QString           code()                 const;
    QString           fieldName()            const  { return _fieldName;            };
    QString           listDisplayFieldName() const  { return _listDisplayFieldName; };
    QString           listIdFieldName()      const  { return _listIdFieldName;      };
    QString           listSchemaName()       const  { return _listSchemaName;       };
    QString           listTableName()        const  { return _listTableName;        };

    QSize             sizeHint()             const;

  public slots:
    void clear();
    void append(int, const QString &);
    void append(int, const QString &, const QString &);
    void populate(XSqlQuery &, int = -1);
    void populate(const QString &, int = -1);
    void populate();
    void setDataWidgetMap(XDataWidgetMapper* m);
    void setDefaultCode(Defaults p)                 { _default = p;               };
    void setFieldName(QString p)                    { _fieldName = p;             };
    void setListDisplayFieldName(QString p)         { _listDisplayFieldName = p;  };
    void setListIdFieldName(QString p)              { _listIdFieldName = p;       };
    void setListSchemaName(QString p);
    void setListTableName(QString p);
    void setId(int);
    void setText(QVariant &);
    void setText(const QString &);
    void setText(const QVariant &);
    void updateMapperData();

  private slots:
    void sHandleNewIndex(int);

  signals:
    void clicked();
    void newID(int);
    void notNull(bool);
    void valid(bool);

  protected:
    QString   currentDefault();
    void      mousePressEvent(QMouseEvent *);

    bool                _allowNull;
    int                 _lastId;
    enum XComboBoxTypes _type;
    QLabel*             _label;
    QList<int>          _ids;
    QList<QString>      _codes;
    QString             _nullStr;

  private:
    enum Defaults       _default;
    QString             _fieldName;
    QString             _listDisplayFieldName;
    QString             _listIdFieldName;
    QString             _listSchemaName;
    QString             _listTableName;
    XDataWidgetMapper   *_mapper;
};

#endif

