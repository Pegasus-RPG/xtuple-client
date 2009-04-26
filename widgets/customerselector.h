/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#ifndef CUSTOMERSELECTOR_H
#define CUSTOMERSELECTOR_H

#include "widgets.h"
#include "ui_customerselector.h"

class ParameterList;

class XTUPLEWIDGETS_EXPORT CustomerSelector : public QWidget, public Ui::CustomerSelector
{
  Q_OBJECT

  Q_ENUMS(CustomerSelectorState)

  Q_PROPERTY(enum CustomerSelectorState state READ state WRITE setState)

  public:
    CustomerSelector(QWidget * = 0, const char * = 0);
    virtual ~CustomerSelector();
    virtual void synchronize(CustomerSelector*);

    enum CustomerSelectorState
    {
      All, Selected, SelectedType, TypePattern
    };

    virtual void           appendValue(ParameterList &);
    virtual void           bindValue(XSqlQuery &);
    virtual QString        typePattern() { return _customerType->text(); }
    enum CustomerSelectorState  state()      { return (CustomerSelectorState)_select->currentIndex(); }
    virtual int            custId()     { return _cust->id(); }
    virtual int            custTypeId() { return _customerTypes->id(); }

    inline bool isAll()          { return _select->currentIndex() == All; }
    inline bool isSelectedCust() { return _select->currentIndex() == Selected; }
    inline bool isSelectedType() { return _select->currentIndex() == SelectedType; }
    inline bool isTypePattern() { return _select->currentIndex() == TypePattern; }
    virtual bool isValid();

  public slots:
    virtual void setCustId(int p);
    virtual void setCustTypeId(int p);
    virtual void setTypePattern(const QString &p);
    virtual void setState(int p) { setState((CustomerSelectorState)p); }
    virtual void setState(enum CustomerSelectorState p);

  signals:
    void newTypePattern(QString);
    void newState(int);
    void newCustId(int);
    void newCustTypeId(int);
    void updated();

  protected slots:
    virtual void languageChange();
    virtual void sTypePatternFinished();
};

#endif
