/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef parameterwidget_h
#define parameterwidget_h

#include <QDate>
#include <QSignalMapper>
#include <QMap>
#include <QPair>
#include <QToolButton>
#include <QRegExp>
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
      Crmacct, User, Text, Date, XComBox, Contact
    };

    ParameterWidget(QWidget *pParent, const char * = 0);
                void appendValue(ParameterList &);
    Q_INVOKABLE void applyDefaultFilterSet();
    Q_INVOKABLE ParameterList parameters();

    
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
    void setXComboBoxType(QString, QString, enum XComboBox::XComboBoxTypes);
    void setXComboBoxType(QString, QString, QString);
    void sManageFilters();
    void storeFilterValue(QDate);
    void storeFilterValue(int pId = -1, QObject* filter = 0);
    void setFiltersVisabiltyPreference();
    void toggleSave();


  signals:
    void cleared();
    void filterSetSaved();
    void updated();
  protected:
    virtual void showEvent(QShowEvent *);

  private:
    enum ParameterWidgetTypes _type;
    QSignalMapper *_filterSignalMapper;
    QMap<QString, QPair<QString, ParameterWidgetTypes> > _types;
    QMap<int, QString > _usedTypes;
    QString _settingsName;
    QMap<int, QPair<QString, QVariant> > _filterValues;
    QMap<QString, XComboBox::XComboBoxTypes > _comboTypes;
    QMap<QString, QString > _comboQuery;
    bool _initialized;
    bool _shared;

    QString getParameterTypeKey(QString);
    void setSelectedFilter(int filter_id);
    void repopulateComboboxes();
    bool containsUsedType(QString);

};

Q_DECLARE_METATYPE(ParameterWidget*);

void setupParameterWidget(QScriptEngine *engine);

#endif
