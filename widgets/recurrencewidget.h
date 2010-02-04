/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef RECURRENCEWIDGET_H
#define RECURRENCEWIDGET_H

#include "widgets.h"
#include "ui_recurrencewidget.h"

class RecurrenceWidget : public QWidget, public Ui::RecurrenceWidget
{
  Q_OBJECT
  Q_PROPERTY(bool maxVisible       READ maxVisible       WRITE setMaxVisible)
  Q_PROPERTY(bool startDateVisible READ startDateVisible WRITE setStartDateVisible)

  public:
    // Never must = XComboBox::id() when ! XComboBox::isValid()
    enum RecurrencePeriod
    { Never = -1, Minutely, Hourly, Daily, Weekly, Monthly, Yearly, Custom };

    enum RecurrenceChangePolicy
    { NoPolicy = -1, IgnoreFuture, ChangeFuture };
                          
    RecurrenceWidget(QWidget* parent = 0, const char* name = 0);
    ~RecurrenceWidget();

    Q_INVOKABLE virtual QDate            endDate()          const;
    Q_INVOKABLE virtual int              frequency()        const;
    Q_INVOKABLE virtual RecurrenceChangePolicy getChangePolicy();
    Q_INVOKABLE virtual bool             isRecurring()      const;
    Q_INVOKABLE virtual int              max()              const;
    /*useprop*/ virtual bool             maxVisible()       const;
    Q_INVOKABLE virtual bool             modified()         const;
    Q_INVOKABLE virtual int              parentId()         const;
    Q_INVOKABLE virtual QString          parentType()       const;
    Q_INVOKABLE virtual RecurrencePeriod period()           const;
    Q_INVOKABLE virtual QString          periodCode()       const;
    Q_INVOKABLE virtual QDate            startDate()        const;
    /*useprop*/ virtual bool             startDateVisible() const;
    Q_INVOKABLE virtual RecurrencePeriod stringToPeriod(QString p) const;

  public slots:
    virtual void clear();
    virtual bool save(bool intxn, RecurrenceChangePolicy cp, QString &msg);
    virtual void set(bool recurring = false, int frequency = 1, QString period = QString("W"), QDate startDate = QDate::currentDate(), QDate endDate = QDate(), int max = 10);
    virtual void setEndDate(QDate p);
    virtual void setFrequency(int p);
    virtual void setMax(int p);
    virtual void setMaxVisible(bool p);
    virtual bool setParent(int pid, QString ptype);
    virtual void setPeriod(RecurrencePeriod p);
    virtual void setPeriod(QString p);
    virtual void setRecurring(bool p);
    virtual void setStartDate(QDate p);
    virtual void setStartDateVisible(bool p);

  protected slots:
    virtual void languageChange();

  protected:
   QDate            _eot;
   int              _id;
   int              _parentId;
   QString          _parentType;
   QDate            _prevEndDate;
   int              _prevFrequency;
   int              _prevMax;
   int              _prevParentId;
   QString          _prevParentType;
   RecurrencePeriod _prevPeriod;
   bool             _prevRecurring;
   QDate            _prevStartDate;

  private:

};

#endif // RECURRENCEWIDGET_H
