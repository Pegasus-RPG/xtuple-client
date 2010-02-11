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

#include <QGroupBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QLineEdit>
#include "widgets.h"
#include <parameter.h>
#include <QSizePolicy>
#include <QApplication>
#include <QtDebug>
#include <QSignalMapper>
#include <QLayoutItem>
#include <QMap>
#include <QPair>
#include <QString>
#include <QDate>
#include <QSpacerItem>
#include <QMessageBox>

class ParameterList;
class XComboBox;

class XTUPLEWIDGETS_EXPORT ParameterWidget : public QWidget
{
  Q_OBJECT

  Q_ENUMS(ParameterWidgetTypes)
   
  public:
	enum ParameterWidgetTypes
    {
      Crmacct, User, Text, Date
    };

    ParameterWidget(QWidget *pParent, const char * = 0);
	void addFilters();
	void appendValue(ParameterList &);
	void applyDefaultFilterSet();
    
 public slots:
	void addParam();
	void applySaved(int);
	void applySaved(int, int filter_id);
	void changeFilterButton();
	void changeFilterObject(int);
	void clearFilters();
	void removeParam(int);
	void save();
	void setSavedFilters();
	void setSavedFilters(int defaultId);
	void setSavedFiltersIndex(QString);
	void setType(QString, QString, enum ParameterWidgetTypes = Text);
	void sManageFilters();
	void storeFilterValue();
	void storeFilterValue(QDate);
	void storeFilterValue(int);

  signals:
	void cleared();
	void filterSetSaved();
    void updated();

  private:
	QPushButton *_filterButton;
	QPushButton *_saveButton;
	QPushButton *_manageButton;
	XComboBox *_filterList;
	QGridLayout * _window;
	QHBoxLayout *_hboxLayout;
    enum ParameterWidgetTypes _type;
	QGroupBox *_groupBox;
	QVBoxLayout *vbox;
	QPushButton *addFilterRow;
	QLineEdit *_filterSetName;
	QSignalMapper *_filterSignalMapper;
	QMap<QString, QPair<QString, ParameterWidgetTypes>> _types;
	QMap<int, QPair<QString, QVariant>> _filterValues;
	
	void addGroupBox();
	QString getParameterTypeKey(QString);
	void setSelectedFilter(int filter_id);
};

#endif
