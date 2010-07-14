/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "display.h"
#include "ui_display.h"

#include <QSqlError>

#include <metasql.h>
#include <mqlutil.h>
#include <openreports.h>
#include <parameter.h>


class displayPrivate : public Ui::display
{
public:
  displayPrivate(::display * parent) : _parent(parent)
  {
    setupUi(_parent);
    _print->hide(); // hide the print button until a reportName is set
    _useAltId = false;
  }

  QString reportName;
  QString metasqlName;
  QString metasqlGroup;

  bool _useAltId;

private:
  ::display * _parent;
};

display::display(QWidget* parent, const char* name, Qt::WindowFlags flags)
    : XWidget(parent, name, flags)
{
  _data = new displayPrivate(this);

  connect(_data->_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_data->_query, SIGNAL(clicked()), this, SLOT(sFillList()));
  connect(_data->_list, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
}

display::~display()
{
  delete _data;
  _data = 0;
}

void display::languageChange()
{
  _data->retranslateUi(this);
}

QWidget * display::optionsWidget()
{
  return _data->_optionsFrame;
}

XTreeWidget * display::list()
{
  return _data->_list;
}

void display::setReportName(const QString & reportName)
{
  _data->reportName = reportName;
  _data->_print->setVisible(!reportName.isEmpty());
}

void display::setMetaSQLOptions(const QString & group, const QString & name)
{
  _data->metasqlName = name;
  _data->metasqlGroup = group;
}

void display::setListLabel(const QString & pText)
{
  _data->_listLabel->setText(pText);
}

void display::setUseAltId(bool on)
{
  _data->_useAltId = on;
}

bool display::getUseAltId() const
{
  return _data->_useAltId;
}

void display::sPrint()
{
  ParameterList params;
  if (!setParams(params))
    return;
  orReport report(_data->reportName, params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void display::sFillList()
{
  ParameterList params;
  if (!setParams(params))
    return;
  int itemid = _data->_list->id();
  bool ok = true;
  QString errorString;
  MetaSQLQuery mql = MQLUtil::mqlLoad(_data->metasqlGroup, _data->metasqlName, errorString, &ok);
  if(!ok)
  {
    systemError(this, errorString, __FILE__, __LINE__);
    return;
  }
  XSqlQuery xq = mql.toQuery(params);
  _data->_list->populate(xq, itemid, _data->_useAltId);
  if (xq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, xq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void display::sPopulateMenu(QMenu *, QTreeWidgetItem *)
{
}
