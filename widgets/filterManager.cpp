/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "filterManager.h"
#include "parameterwidget.h"
#include <xtreewidget.h>
#include <xsqlquery.h>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>
#include <QDialog>
#include <QtDebug>

filterManager::filterManager(QWidget* parent, const char* name)
    : QDialog(parent)
{
  if(name)
    setObjectName(name);

  setupUi(this);
  
  _filterId=-1;

  _filterSet->addColumn(tr("Filter Set Name"), -1, Qt::AlignLeft, true, "filter_name" );

  connect(_filterSet, SIGNAL(itemClicked(XTreeWidgetItem*, int)), this, SLOT( getXTreeWidgetItem(XTreeWidgetItem*, int) ) );
  connect(_edit, SIGNAL(clicked()), this, SLOT( applySaved() ));
  connect(_delete, SIGNAL(clicked()), this, SLOT( deleteFilter() ) );
  connect(this, SIGNAL(filterSelected(QString)), parent, SLOT( setSavedFiltersIndex(QString) ));
  connect(this, SIGNAL(filterDeleted()), parent, SLOT(setSavedFilters()) );
}

void filterManager::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("screen", &valid);
  if (valid)
  {
    _screen = param.toString();
    populate();
  }
}

void filterManager::populate()
{
  if (parent())
  {
    if (parent()->parent())
    {
      XSqlQuery qry;
      qry.prepare("select filter_id, filter_name "
                  "from filter "
                  "where filter_username=current_user "
                  " and filter_screen=:screen;");
      qry.bindValue(":screen", parent()->parent()->objectName());
      qry.exec();

      _filterSet->populate(qry, false);
    }
  }
}

void filterManager::getXTreeWidgetItem(XTreeWidgetItem* item, int column)
{
  _filterId = item->id();
  _filterText = item->text(0);
}

void filterManager::applySaved()
{
  ParameterWidget* pWidget = (ParameterWidget*)this->parent();
  pWidget->applySaved(0, _filterId);
  emit filterSelected(_filterText);
  this->close();
}

void filterManager::deleteFilter()
{
  QString query = "delete from filter where filter_id=:filter_id";
  XSqlQuery qry;

  qry.prepare(query);
  qry.bindValue(":filter_id", _filterId);
  qry.exec();

  this->populate();

  emit filterDeleted();
}
