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

  public:
      // Never must = XComboBox::id() when ! XComboBox::isValid()
    enum RecurrencePeriod
    { Never = -1, Minutely, Hourly, Daily, Weekly, Monthly, Yearly, Other };
                          
    RecurrenceWidget(QWidget* parent = 0, const char* name = 0);
    ~RecurrenceWidget();

  public slots:
    virtual QDate            endDate()     const;
    virtual int              frequency()   const;
    virtual bool             isRecurring() const;
    virtual RecurrencePeriod period()      const;
    virtual QString          periodCode()  const;
    virtual void             set(bool recurring = false, int frequency = 0, QString period = QString("W"), QDate startDate = QDate(), QDate endDate = QDate());
    virtual void             setEndDate(QDate p);
    virtual void             setFrequency(int p);
    virtual void             setPeriod(RecurrencePeriod p);
    virtual void             setPeriod(QString p);
    virtual void             setRecurring(bool p);
    virtual void             setStartDate(QDate p);
    virtual QDate            startDate()   const;


  protected slots:
    virtual void languageChange();

  private:

};

#endif // RECURRENCEWIDGET_H
