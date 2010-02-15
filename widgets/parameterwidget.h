/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef parameterwidget_h
#define parameterwidget_h

#include <QSignalMapper>
#include <QMap>
#include <QPair>

#include <parameter.h>

#include "widgets.h"
#include "ui_parameterwidget.h"

class ParameterList;

class XTUPLEWIDGETS_EXPORT ParameterWidget : public QWidget, public Ui::ParameterWidget
{
  Q_OBJECT

  Q_ENUMS(ParameterWidgetTypes)
   
  public:
    enum ParameterWidgetTypes
    {
      Crmacct, User, Text, Date
    };

    ParameterWidget(QWidget *pParent, const char * = 0);
    void appendValue(ParameterList &);
    void applyDefaultFilterSet();
    
 public slots:
    void addParam();
    void applySaved(int pId = 0, int filter_id = 0);
    void changeFilterObject(int index);
    void clearFilters();
    void removeParam(int);
    void save();
    void setSavedFilters(int defaultId = -1);
    void setSavedFiltersIndex(QString);
    void setType(QString, QString, enum ParameterWidgetTypes = Text);
    void sManageFilters();
    void storeFilterValue(QDate);
    void storeFilterValue(int pId = -1);

  signals:
    void cleared();
    void filterSetSaved();
    void updated();

  private:
    enum ParameterWidgetTypes _type;
    QSignalMapper *_filterSignalMapper;
    QMap<QString, QPair<QString, ParameterWidgetTypes> > _types;
    QMap<int, QPair<QString, QVariant> > _filterValues;

    QString getParameterTypeKey(QString);
    void setSelectedFilter(int filter_id);
};

#endif
